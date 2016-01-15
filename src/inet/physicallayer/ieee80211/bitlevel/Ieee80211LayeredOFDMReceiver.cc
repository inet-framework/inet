//
// Copyright (C) 2014 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211LayeredOFDMReceiver.h"
#include "inet/physicallayer/contract/bitlevel/ISymbol.h"
#include "inet/physicallayer/common/bitlevel/LayeredReceptionResult.h"
#include "inet/physicallayer/common/bitlevel/LayeredReception.h"
#include "inet/physicallayer/common/bitlevel/SignalSymbolModel.h"
#include "inet/physicallayer/common/bitlevel/SignalSampleModel.h"
#include "inet/physicallayer/common/bitlevel/SignalBitModel.h"
#include "inet/physicallayer/analogmodel/bitlevel/ScalarSignalAnalogModel.h"
#include "inet/physicallayer/common/packetlevel/BandListening.h"
#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211OFDMDecoderModule.h"
#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211OFDMDemodulatorModule.h"
#include "inet/physicallayer/modulation/BPSKModulation.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211OFDMModulation.h"
#include "inet/physicallayer/base/packetlevel/NarrowbandNoiseBase.h"
#include "inet/physicallayer/common/packetlevel/ListeningDecision.h"
#include "inet/physicallayer/analogmodel/packetlevel/ScalarAnalogModel.h"
#include "inet/common/serializer/headerserializers/ieee80211/Ieee80211PhySerializer.h"
#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211OFDMDefs.h"
#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211OFDMSymbolModel.h"
#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211OFDMSymbol.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211OFDMMode.h"
#include "inet/physicallayer/modulation/BPSKModulation.h"

namespace inet {

namespace physicallayer {

using namespace serializer;

Define_Module(Ieee80211LayeredOFDMReceiver);

void Ieee80211LayeredOFDMReceiver::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        errorModel = dynamic_cast<ILayeredErrorModel *>(getSubmodule("errorModel"));
        dataDecoder = dynamic_cast<IDecoder *>(getSubmodule("dataDecoder"));
        signalDecoder = dynamic_cast<IDecoder *>(getSubmodule("signalDecoder"));
        dataDemodulator = dynamic_cast<IDemodulator *>(getSubmodule("dataDemodulator"));
        signalDemodulator = dynamic_cast<IDemodulator *>(getSubmodule("signalDemodulator"));
        pulseFilter = dynamic_cast<IPulseFilter *>(getSubmodule("pulseFilter"));
        analogDigitalConverter = dynamic_cast<IAnalogDigitalConverter *>(getSubmodule("analogDigitalConverter"));

        energyDetection = mW(math::dBm2mW(par("energyDetection")));
        sensitivity = mW(math::dBm2mW(par("sensitivity")));
        carrierFrequency = Hz(par("carrierFrequency"));
        bandwidth = Hz(par("bandwidth"));
        snirThreshold = math::dB2fraction(par("snirThreshold"));
        channelSpacing = Hz(par("channelSpacing"));
        isCompliant = par("isCompliant").boolValue();
        if (isCompliant && (dataDecoder || signalDecoder || dataDemodulator || signalDemodulator || pulseFilter || analogDigitalConverter))
        {
            throw cRuntimeError("In compliant mode it is forbidden to the following parameters: dataDecoder, signalDecoder, dataDemodulator, signalDemodulator, pulseFilter, analogDigitalConverter.");
        }
        const char *levelOfDetailStr = par("levelOfDetail").stringValue();
        if (strcmp("bit", levelOfDetailStr) == 0)
            levelOfDetail = BIT_DOMAIN;
        else if (strcmp("symbol", levelOfDetailStr) == 0)
            levelOfDetail = SYMBOL_DOMAIN;
        else if (strcmp("sample", levelOfDetailStr) == 0)
            levelOfDetail = SAMPLE_DOMAIN;
        else if (strcmp("packet", levelOfDetailStr) == 0)
            levelOfDetail = PACKET_DOMAIN;
        else
            throw cRuntimeError("Unknown level of detail='%s'", levelOfDetailStr);
    }
}

const IReceptionAnalogModel *Ieee80211LayeredOFDMReceiver::createAnalogModel(const LayeredTransmission *transmission, const ISNIR *snir) const
{
    return nullptr;
}

std::ostream& Ieee80211LayeredOFDMReceiver::printToStream(std::ostream& stream, int level) const
{
    stream << "Ieee80211LayeredOFDMReceiver";
    if (level >= PRINT_LEVEL_TRACE)
        stream << ", levelOfDetail = " << levelOfDetail
               << ", mode = " << printObjectToString(mode, level - 1)
               << ", errorModel = " << printObjectToString(errorModel, level - 1)
               << ", dataDecoder = " << printObjectToString(dataDecoder, level - 1)
               << ", signalDecoder = " << printObjectToString(signalDecoder, level - 1)
               << ", dataDemodulator = " << printObjectToString(dataDemodulator, level - 1)
               << ", signalDemodulator = " << printObjectToString(signalDemodulator, level - 1)
               << ", pulseFilter = " << printObjectToString(pulseFilter, level - 1)
               << ", analogDigitalConverter = " << printObjectToString(analogDigitalConverter, level - 1)
               << ", energyDetection = " << energyDetection
               << ", sensitivity = " << energyDetection
               << ", carrierFrequency = " << carrierFrequency
               << ", bandwidth = " << bandwidth
               << ", channelSpacing = " << channelSpacing
               << ", snirThreshold = " << snirThreshold
               << ", isCompliant = " << isCompliant;
    return stream;
}

const IReceptionSampleModel *Ieee80211LayeredOFDMReceiver::createSampleModel(const LayeredTransmission *transmission, const ISNIR *snir) const
{
    if (levelOfDetail == SAMPLE_DOMAIN)
        return errorModel->computeSampleModel(transmission, snir);
    return nullptr;
}

const IReceptionBitModel *Ieee80211LayeredOFDMReceiver::createBitModel(const LayeredTransmission *transmission, const ISNIR *snir) const
{
    if (levelOfDetail == BIT_DOMAIN)
        return errorModel->computeBitModel(transmission, snir);
    return nullptr;
}

const IReceptionPacketModel *Ieee80211LayeredOFDMReceiver::createPacketModel(const LayeredTransmission *transmission, const ISNIR *snir) const
{
    if (levelOfDetail == PACKET_DOMAIN)
        return errorModel->computePacketModel(transmission, snir);
    return nullptr;
}

const IReceptionSymbolModel *Ieee80211LayeredOFDMReceiver::createSymbolModel(const LayeredTransmission *transmission, const ISNIR *snir) const
{
    if (levelOfDetail == SYMBOL_DOMAIN)
        return errorModel->computeSymbolModel(transmission, snir);
    return nullptr;
}

double Ieee80211LayeredOFDMReceiver::getCodeRateFromDecoderModule(const IDecoder *decoder) const
{
    const Ieee80211OFDMDecoderModule *decoderModule = check_and_cast<const Ieee80211OFDMDecoderModule *>(decoder);
    const Ieee80211OFDMCode *code = decoderModule->getCode();
    const ConvolutionalCode *convolutionalCode = code->getConvolutionalCode();
    return convolutionalCode ? 1.0 * convolutionalCode->getCodeRatePuncturingN() / convolutionalCode->getCodeRatePuncturingK() : 1;
}

const IReceptionBitModel *Ieee80211LayeredOFDMReceiver::createCompleteBitModel(const IReceptionBitModel *signalFieldBitModel, const IReceptionBitModel *dataFieldBitModel) const
{
    if (levelOfDetail >= BIT_DOMAIN) {
        BitVector *bits = new BitVector(*signalFieldBitModel->getBits());
        const BitVector *dataBits = dataFieldBitModel->getBits();
        for (unsigned int i = 0; i < dataBits->getSize(); i++)
            bits->appendBit(dataBits->getBit(i));
        return new ReceptionBitModel(signalFieldBitModel->getHeaderBitLength(), signalFieldBitModel->getHeaderBitRate(), dataFieldBitModel->getPayloadBitLength(), dataFieldBitModel->getPayloadBitRate(), bits);
    }
    return nullptr;
}

const IReceptionPacketModel *Ieee80211LayeredOFDMReceiver::createDataFieldPacketModel(const IReceptionBitModel *signalFieldBitModel, const IReceptionBitModel *dataFieldBitModel, const IReceptionPacketModel *signalFieldPacketModel) const
{
    const IReceptionPacketModel *dataFieldPacketModel = nullptr;
    if (levelOfDetail > PACKET_DOMAIN) { // Create from the bit model
        if (dataDecoder)
            dataFieldPacketModel = dataDecoder->decode(dataFieldBitModel);
        else {
            const Ieee80211OFDMCode *code = mode->getDataMode()->getCode();
            const Ieee80211OFDMDecoder decoder(code);
            dataFieldPacketModel = decoder.decode(dataFieldBitModel);
        }
    }
    return dataFieldPacketModel;
}

const IReceptionPacketModel *Ieee80211LayeredOFDMReceiver::createSignalFieldPacketModel(const IReceptionBitModel *signalFieldBitModel) const
{
    const IReceptionPacketModel *signalFieldPacketModel = nullptr;
    if (levelOfDetail > PACKET_DOMAIN) { // Create from the bit model
        if (signalDecoder) // non-complaint
            signalFieldPacketModel = signalDecoder->decode(signalFieldBitModel);
        else { // complaint
               // In compliant mode the code for the signal field is always the following:
            const Ieee80211OFDMDecoder decoder(&Ieee80211OFDMCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling);
            signalFieldPacketModel = decoder.decode(signalFieldBitModel);
        }
    }
    return signalFieldPacketModel;
}

unsigned int Ieee80211LayeredOFDMReceiver::calculatePadding(unsigned int dataFieldLengthInBits, const APSKModulationBase *modulation, double codeRate) const
{
    ASSERT(modulation != nullptr);
    unsigned int codedBitsPerOFDMSymbol = modulation->getCodeWordSize() * NUMBER_OF_OFDM_DATA_SUBCARRIERS;
    unsigned int dataBitsPerOFDMSymbol = codedBitsPerOFDMSymbol * codeRate;
    return dataBitsPerOFDMSymbol - dataFieldLengthInBits % dataBitsPerOFDMSymbol;
}

unsigned int Ieee80211LayeredOFDMReceiver::getSignalFieldLength(const BitVector *signalField) const
{
    ShortBitVector length;
    for (int i = SIGNAL_LENGTH_FIELD_START; i <= SIGNAL_LENGTH_FIELD_END; i++)
        length.appendBit(signalField->getBit(i));
    return length.toDecimal();
}

uint8_t Ieee80211LayeredOFDMReceiver::getRate(const BitVector *serializedPacket) const
{
    ShortBitVector rate;
    for (unsigned int i = 0; i < 4; i++)
        rate.appendBit(serializedPacket->getBit(i));
    return rate.toDecimal();
}

const IReceptionSymbolModel *Ieee80211LayeredOFDMReceiver::createSignalFieldSymbolModel(const IReceptionSymbolModel *receptionSymbolModel) const
{
    const Ieee80211OFDMReceptionSymbolModel *signalFieldSymbolModel = nullptr;
    if (levelOfDetail > SYMBOL_DOMAIN)
        throw cRuntimeError("This level of detail is unimplemented!");
    else if (levelOfDetail == SYMBOL_DOMAIN) { // Create from symbol model (made by the error model)
        const std::vector<const ISymbol *> *symbols = receptionSymbolModel->getSymbols();
        std::vector<const ISymbol *> *signalSymbols = new std::vector<const ISymbol *>();
        const Ieee80211OFDMSymbol *signalSymbol = check_and_cast<const Ieee80211OFDMSymbol *>(symbols->at(0));
        signalSymbols->push_back(new Ieee80211OFDMSymbol(*signalSymbol)); // The first symbol is the signal field symbol
        signalFieldSymbolModel = new Ieee80211OFDMReceptionSymbolModel(1, receptionSymbolModel->getHeaderSymbolRate(), -1, NaN, signalSymbols);
    }
    return signalFieldSymbolModel;
}

const IReceptionSymbolModel *Ieee80211LayeredOFDMReceiver::createDataFieldSymbolModel(const IReceptionSymbolModel *receptionSymbolModel) const
{
    const Ieee80211OFDMReceptionSymbolModel *dataFieldSymbolModel = nullptr;
    if (levelOfDetail > SYMBOL_DOMAIN)
        throw cRuntimeError("This level of detail is unimplemented!");
    else if (levelOfDetail == SYMBOL_DOMAIN) { // Create from symbol model (made by the error model)
        const std::vector<const ISymbol *> *symbols = receptionSymbolModel->getSymbols();
        std::vector<const ISymbol *> *dataSymbols = new std::vector<const ISymbol *>();
        const Ieee80211OFDMSymbol *ofdmSymbol = nullptr;
        for (unsigned int i = 1; i < symbols->size(); i++) {
            ofdmSymbol = check_and_cast<const Ieee80211OFDMSymbol *>(symbols->at(i));
            dataSymbols->push_back(new Ieee80211OFDMSymbol(*ofdmSymbol));
        }
        dataFieldSymbolModel = new Ieee80211OFDMReceptionSymbolModel(-1, NaN, symbols->size() - 1, receptionSymbolModel->getPayloadSymbolRate(), dataSymbols);
    }
    return dataFieldSymbolModel;
}

const IReceptionBitModel *Ieee80211LayeredOFDMReceiver::createSignalFieldBitModel(const IReceptionBitModel *bitModel, const IReceptionSymbolModel *signalFieldSymbolModel) const
{
    const IReceptionBitModel *signalFieldBitModel = nullptr;
    if (levelOfDetail > BIT_DOMAIN) { // Create from symbol model
        if (signalDemodulator) // non-compliant
            signalFieldBitModel = signalDemodulator->demodulate(signalFieldSymbolModel);
        else { // compliant
               // In compliant mode, the signal field modulation is always BPSK
            const Ieee80211OFDMModulation *signalModulation = new Ieee80211OFDMModulation(&BPSKModulation::singleton);
            const Ieee80211OFDMDemodulator demodulator(signalModulation);
            signalFieldBitModel = demodulator.demodulate(signalFieldSymbolModel);
            delete signalModulation;
        }
    }
    else if (levelOfDetail == BIT_DOMAIN) { // Create from bit model (made by the error model)
        unsigned int signalFieldLength = 0;
        if (isCompliant)
            signalFieldLength = ENCODED_SIGNAL_FIELD_LENGTH;
        else {
            double codeRate = getCodeRateFromDecoderModule(signalDecoder);
            signalFieldLength = DECODED_SIGNAL_FIELD_LENGTH * codeRate;
        }
        BitVector *signalFieldBits = new BitVector();
        const BitVector *bits = bitModel->getBits();
        for (unsigned int i = 0; i < signalFieldLength; i++)
            signalFieldBits->appendBit(bits->getBit(i));
        signalFieldBitModel = new ReceptionBitModel(signalFieldLength, bitModel->getHeaderBitRate(), -1, bps(NaN), signalFieldBits);
    }
    return signalFieldBitModel;
}

const IReceptionBitModel *Ieee80211LayeredOFDMReceiver::createDataFieldBitModel(const IReceptionBitModel *bitModel, const IReceptionSymbolModel *dataFieldSymbolModel, const IReceptionPacketModel *signalFieldPacketModel, const IReceptionBitModel *signalFieldBitModel) const
{
    const IReceptionBitModel *dataFieldBitModel = nullptr;
    if (levelOfDetail > BIT_DOMAIN) { // Create from symbol model
        if (dataDemodulator) // non-compliant
            dataFieldBitModel = dataDemodulator->demodulate(dataFieldSymbolModel);
        else { // compliant
            const Ieee80211OFDMDataMode *dataMode = mode->getDataMode();
            const Ieee80211OFDMDemodulator ofdmDemodulator(dataMode->getModulation());
            dataFieldBitModel = ofdmDemodulator.demodulate(dataFieldSymbolModel);
        }
    }
    else if (levelOfDetail == BIT_DOMAIN) { // Create from bit model (made by the error model)
        const ConvolutionalCode *convolutionalCode = nullptr;
        const APSKModulationBase *modulation = nullptr;
        double codeRate = NaN;
        unsigned int psduLengthInBits = getSignalFieldLength(signalFieldPacketModel->getSerializedPacket()) * 8;
        unsigned int dataFieldLengthInBits = psduLengthInBits + PPDU_SERVICE_FIELD_BITS_LENGTH + PPDU_TAIL_BITS_LENGTH;
        if (isCompliant) {
            const Ieee80211OFDMDataMode *dataMode = mode->getDataMode();
            modulation = dataMode->getModulation()->getSubcarrierModulation();
            const Ieee80211OFDMCode *code = dataMode->getCode();
            convolutionalCode = code->getConvolutionalCode();
            codeRate = convolutionalCode->getCodeRatePuncturingN() * 1.0 / convolutionalCode->getCodeRatePuncturingK();
        }
        else {
            const Ieee80211OFDMDecoderModule *decoderModule = check_and_cast<const Ieee80211OFDMDecoderModule *>(dataDecoder);
            const Ieee80211OFDMCode *code = decoderModule->getCode();
            convolutionalCode = code->getConvolutionalCode();
            modulation = mode->getDataMode()->getModulation()->getSubcarrierModulation();
            codeRate = getCodeRateFromDecoderModule(dataDecoder);
        }
        dataFieldLengthInBits += calculatePadding(dataFieldLengthInBits, modulation, 1.0 / codeRate);
//        ASSERT(dataFieldLengthInBits % convolutionalCode->getCodeRatePuncturingK() == 0);
        unsigned int encodedDataFieldLengthInBits = dataFieldLengthInBits * codeRate;
        const BitVector *bits = bitModel->getBits();
        unsigned int encodedSignalFieldLength = signalFieldBitModel->getHeaderBitLength();
        if (dataFieldLengthInBits + encodedSignalFieldLength > bits->getSize())
            throw cRuntimeError("The calculated data field length = %d is greater then the actual bitvector length = %d", dataFieldLengthInBits, bits->getSize());
        BitVector *dataBits = new BitVector();
        for (unsigned int i = 0; i < encodedDataFieldLengthInBits; i++)
            dataBits->appendBit(bits->getBit(encodedSignalFieldLength + i));
        dataFieldBitModel = new ReceptionBitModel(-1, bps(NaN), encodedDataFieldLengthInBits, bitModel->getPayloadBitRate(), dataBits);
    }
    return dataFieldBitModel;
}

const IReceptionSymbolModel *Ieee80211LayeredOFDMReceiver::createCompleteSymbolModel(const IReceptionSymbolModel *signalFieldSymbolModel, const IReceptionSymbolModel *dataFieldSymbolModel) const
{
    if (levelOfDetail >= SYMBOL_DOMAIN) {
        const std::vector<const ISymbol *> *symbols = signalFieldSymbolModel->getSymbols();
        std::vector<const ISymbol *> *completeSymbols = new std::vector<const ISymbol *>(*symbols);
        symbols = dataFieldSymbolModel->getSymbols();
        for (unsigned int i = 0; i < symbols->size(); i++)
            completeSymbols->push_back(new Ieee80211OFDMSymbol(*static_cast<const Ieee80211OFDMSymbol *>(symbols->at(i))));
        return new Ieee80211OFDMReceptionSymbolModel(signalFieldSymbolModel->getHeaderSymbolLength(), signalFieldSymbolModel->getHeaderSymbolRate(), dataFieldSymbolModel->getPayloadSymbolLength(), dataFieldSymbolModel->getPayloadSymbolRate(), completeSymbols);
    }
    return nullptr;
}

const IReceptionPacketModel *Ieee80211LayeredOFDMReceiver::createCompletePacketModel(const IReceptionPacketModel *signalFieldPacketModel, const IReceptionPacketModel *dataFieldPacketModel) const
{
    const BitVector *headerBits = signalFieldPacketModel->getSerializedPacket();
    BitVector *mergedBits = new BitVector(*headerBits);
    const BitVector *dataBits = dataFieldPacketModel->getSerializedPacket();
    for (unsigned int i = 0; i < dataBits->getSize(); i++)
        mergedBits->appendBit(dataBits->getBit(i));
    Ieee80211PhySerializer deserializer;
    cPacket *phyFrame = deserializer.deserialize(mergedBits);
    bool isReceptionSuccessful = true;
    cPacket *packet = phyFrame;
    while (packet != nullptr) {
        isReceptionSuccessful &= !packet->hasBitError();
        packet = packet->getEncapsulatedPacket();
    }
    return new ReceptionPacketModel(phyFrame, mergedBits, bps(NaN), 0, isReceptionSuccessful);
}

const Ieee80211OFDMMode *Ieee80211LayeredOFDMReceiver::computeMode(Hz bandwidth) const
{
    const Ieee80211OFDMDecoderModule *ofdmSignalDecoderModule = check_and_cast<const Ieee80211OFDMDecoderModule *>(signalDecoder);
    const Ieee80211OFDMDecoderModule *ofdmDataDecoderModule = check_and_cast<const Ieee80211OFDMDecoderModule *>(dataDecoder);
    const Ieee80211OFDMDemodulatorModule *ofdmSignalDemodulatorModule = check_and_cast<const Ieee80211OFDMDemodulatorModule *>(signalDemodulator);
    const Ieee80211OFDMDemodulatorModule *ofdmDataDemodulatorModule = check_and_cast<const Ieee80211OFDMDemodulatorModule *>(dataDemodulator);
    const Ieee80211OFDMSignalMode *signalMode = new Ieee80211OFDMSignalMode(ofdmSignalDecoderModule->getCode(), ofdmSignalDemodulatorModule->getModulation(), channelSpacing, bandwidth, 0);
    const Ieee80211OFDMDataMode *dataMode = new Ieee80211OFDMDataMode(ofdmDataDecoderModule->getCode(), ofdmDataDemodulatorModule->getModulation(), channelSpacing, bandwidth);
    return new Ieee80211OFDMMode("", new Ieee80211OFDMPreambleMode(channelSpacing), signalMode, dataMode, channelSpacing, bandwidth);
}

const IReceptionResult *Ieee80211LayeredOFDMReceiver::computeReceptionResult(const IListening *listening, const IReception *reception, const IInterference *interference, const ISNIR *snir) const
{
    const LayeredTransmission *transmission = dynamic_cast<const LayeredTransmission *>(reception->getTransmission());
    const IReceptionAnalogModel *analogModel = createAnalogModel(transmission, snir);
    const IReceptionSampleModel *sampleModel = createSampleModel(transmission, snir);
    const IReceptionSymbolModel *symbolModel = createSymbolModel(transmission, snir);
    const IReceptionBitModel *bitModel = createBitModel(transmission, snir);
    const IReceptionPacketModel *packetModel = createPacketModel(transmission, snir);

    const IReceptionSymbolModel *signalFieldSymbolModel = createSignalFieldSymbolModel(symbolModel);
    const IReceptionSymbolModel *dataFieldSymbolModel = createDataFieldSymbolModel(symbolModel);
    const IReceptionBitModel *signalFieldBitModel = createSignalFieldBitModel(bitModel, signalFieldSymbolModel);
    const IReceptionPacketModel *signalFieldPacketModel = createSignalFieldPacketModel(signalFieldBitModel);
    if (isCompliant) {
        uint8_t rate = getRate(signalFieldPacketModel->getSerializedPacket());
        // TODO: handle erroneous rate field
        mode = &Ieee80211OFDMCompliantModes::getCompliantMode(rate, channelSpacing);
    }
    else if (!mode)
        mode = computeMode(bandwidth);
    const IReceptionBitModel *dataFieldBitModel = createDataFieldBitModel(bitModel, dataFieldSymbolModel, signalFieldPacketModel, signalFieldBitModel);
    const IReceptionPacketModel *dataFieldPacketModel = createDataFieldPacketModel(signalFieldBitModel, dataFieldBitModel, signalFieldPacketModel);

//    if (!sampleModel)
//        sampleModel = createCompleteSampleModel(signalFieldSampleModel, dataFieldSampleModel);
    if (!symbolModel)
        symbolModel = createCompleteSymbolModel(signalFieldSymbolModel, dataFieldSymbolModel);
    if (!bitModel)
        bitModel = createCompleteBitModel(signalFieldBitModel, dataFieldBitModel);
    if (!packetModel)
        packetModel = createCompletePacketModel(signalFieldPacketModel, dataFieldPacketModel);

    delete signalFieldSymbolModel;
    delete dataFieldSymbolModel;
    delete signalFieldBitModel;
    delete dataFieldBitModel;
    delete signalFieldPacketModel;
    delete dataFieldPacketModel;

    ReceptionIndication *receptionIndication = new ReceptionIndication();
    receptionIndication->setMinSNIR(snir->getMin());
    receptionIndication->setPacketErrorRate(packetModel->getPER());
// TODO: true, true, packetModel->isPacketErrorless()
    return new LayeredReceptionResult(reception, new std::vector<const IReceptionDecision *>(), receptionIndication, packetModel, bitModel, symbolModel, sampleModel, analogModel);
}

const IListening *Ieee80211LayeredOFDMReceiver::createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition) const
{
    // We assume that in compliant mode the bandwidth is always 20MHz.
    return new BandListening(radio, startTime, endTime, startPosition, endPosition, carrierFrequency, isCompliant ? Hz(20000000) : bandwidth);
}

// TODO: copy
const IListeningDecision *Ieee80211LayeredOFDMReceiver::computeListeningDecision(const IListening *listening, const IInterference *interference) const
{
    const IRadio *receiver = listening->getReceiver();
    const IRadioMedium *radioMedium = receiver->getMedium();
    const IAnalogModel *analogModel = radioMedium->getAnalogModel();
    const INoise *noise = analogModel->computeNoise(listening, interference);
    const NarrowbandNoiseBase *flatNoise = check_and_cast<const NarrowbandNoiseBase *>(noise);
    W maxPower = flatNoise->computeMaxPower(listening->getStartTime(), listening->getEndTime());
    bool isListeningPossible = maxPower >= energyDetection;
    delete noise;
    EV_DEBUG << "Computing listening possible: maximum power = " << maxPower << ", energy detection = " << energyDetection << " -> listening is " << (isListeningPossible ? "possible" : "impossible") << endl;
    return new ListeningDecision(listening, isListeningPossible);
}

// TODO: this is not purely functional, see interface comment
// TODO: copy
bool Ieee80211LayeredOFDMReceiver::computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const
{
    const BandListening *bandListening = check_and_cast<const BandListening *>(listening);
    const LayeredReception *scalarReception = check_and_cast<const LayeredReception *>(reception);
    // TODO: scalar
    const ScalarReceptionSignalAnalogModel *analogModel = check_and_cast<const ScalarReceptionSignalAnalogModel *>(scalarReception->getAnalogModel());
    if (bandListening->getCarrierFrequency() != analogModel->getCarrierFrequency() || bandListening->getBandwidth() != analogModel->getBandwidth()) {
        EV_DEBUG << "Computing reception possible: listening and reception bands are different -> reception is impossible" << endl;
        return false;
    }
    else {
        const INarrowbandSignal *narrowbandSignalAnalogModel = check_and_cast<const INarrowbandSignal *>(reception->getAnalogModel());
        W minReceptionPower = narrowbandSignalAnalogModel->computeMinPower(reception->getStartTime(), reception->getEndTime());
        bool isReceptionPossible = minReceptionPower >= sensitivity;
        EV_DEBUG << "Computing reception possible: minimum reception power = " << minReceptionPower << ", sensitivity = " << sensitivity << " -> reception is " << (isReceptionPossible ? "possible" : "impossible") << endl;
        return isReceptionPossible;
    }
}

Ieee80211LayeredOFDMReceiver::~Ieee80211LayeredOFDMReceiver()
{
    if (!isCompliant) {
        delete mode->getPreambleMode();
        delete mode->getSignalMode();
        delete mode->getDataMode();
        delete mode;
    }
}
} /* namespace physicallayer */
} /* namespace inet */

