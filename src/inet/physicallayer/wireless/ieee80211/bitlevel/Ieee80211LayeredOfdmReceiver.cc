//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211LayeredOfdmReceiver.h"

#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/physicallayer/wireless/common/analogmodel/bitlevel/LayeredReception.h"
#include "inet/physicallayer/wireless/common/analogmodel/bitlevel/DimensionalSignalAnalogModel.h"
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/DimensionalAnalogModel.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/NarrowbandNoiseBase.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISymbol.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/SignalTag_m.h"
#include "inet/physicallayer/wireless/common/modulation/BpskModulation.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/LayeredReceptionResult.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalBitModel.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalSampleModel.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalSymbolModel.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/BandListening.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/ListeningDecision.h"
#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211LayeredTransmission.h"
#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmDecoderModule.h"
#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmDefs.h"
#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmDemodulatorModule.h"
#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmSymbol.h"
#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmSymbolModel.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211OfdmMode.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211OfdmModulation.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Tag_m.h"

namespace inet {
namespace physicallayer {

Define_Module(Ieee80211LayeredOfdmReceiver);

void Ieee80211LayeredOfdmReceiver::initialize(int stage)
{
    SnirReceiverBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        errorModel = dynamic_cast<ILayeredErrorModel *>(getSubmodule("errorModel"));
        dataDecoder = dynamic_cast<IDecoder *>(getSubmodule("dataDecoder"));
        signalDecoder = dynamic_cast<IDecoder *>(getSubmodule("signalDecoder"));
        dataDemodulator = dynamic_cast<IDemodulator *>(getSubmodule("dataDemodulator"));
        signalDemodulator = dynamic_cast<IDemodulator *>(getSubmodule("signalDemodulator"));
        pulseFilter = dynamic_cast<IPulseFilter *>(getSubmodule("pulseFilter"));
        analogDigitalConverter = dynamic_cast<IAnalogDigitalConverter *>(getSubmodule("analogDigitalConverter"));

        energyDetection = mW(math::dBmW2mW(par("energyDetection")));
        sensitivity = mW(math::dBmW2mW(par("sensitivity")));
        centerFrequency = Hz(par("centerFrequency"));
        bandwidth = Hz(par("bandwidth"));
        channelSpacing = Hz(par("channelSpacing"));
        isCompliant = par("isCompliant");
        if (isCompliant && (dataDecoder || signalDecoder || dataDemodulator || signalDemodulator || pulseFilter || analogDigitalConverter)) {
            throw cRuntimeError("In compliant mode it is forbidden to the following parameters: dataDecoder, signalDecoder, dataDemodulator, signalDemodulator, pulseFilter, analogDigitalConverter.");
        }
        const char *levelOfDetailStr = par("levelOfDetail");
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

const Ieee80211OfdmMode *Ieee80211LayeredOfdmReceiver::getMode(const Packet *packet) const
{
    const auto& modeInd = const_cast<Packet *>(packet)->findTag<Ieee80211ModeInd>();
    if (isCompliant)
        return modeInd != nullptr ? check_and_cast<const Ieee80211OfdmMode *>(modeInd->getMode()) : &Ieee80211OfdmCompliantModes::getCompliantMode(11, MHz(20));
    else
        return mode;
}

const IReceptionAnalogModel *Ieee80211LayeredOfdmReceiver::createAnalogModel(const LayeredTransmission *transmission, const ISnir *snir) const
{
    return nullptr;
}

std::ostream& Ieee80211LayeredOfdmReceiver::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "Ieee80211LayeredOfdmReceiver";
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(levelOfDetail)
               << EV_FIELD(mode, printFieldToString(mode, level + 1, evFlags))
               << EV_FIELD(errorModel, printFieldToString(errorModel, level + 1, evFlags))
               << EV_FIELD(dataDecoder, printFieldToString(dataDecoder, level + 1, evFlags))
               << EV_FIELD(signalDecoder, printFieldToString(signalDecoder, level + 1, evFlags))
               << EV_FIELD(dataDemodulator, printFieldToString(dataDemodulator, level + 1, evFlags))
               << EV_FIELD(signalDemodulator, printFieldToString(signalDemodulator, level + 1, evFlags))
               << EV_FIELD(pulseFilter, printFieldToString(pulseFilter, level + 1, evFlags))
               << EV_FIELD(analogDigitalConverter, printFieldToString(analogDigitalConverter, level + 1, evFlags))
               << EV_FIELD(energyDetection)
               << EV_FIELD(sensitivity)
               << EV_FIELD(centerFrequency)
               << EV_FIELD(bandwidth)
               << EV_FIELD(channelSpacing)
               << EV_FIELD(snirThreshold)
               << EV_FIELD(isCompliant);
    return stream;
}

const IReceptionSampleModel *Ieee80211LayeredOfdmReceiver::createSampleModel(const LayeredTransmission *transmission, const ISnir *snir) const
{
    if (levelOfDetail == SAMPLE_DOMAIN)
        return errorModel->computeSampleModel(transmission, snir);
    return nullptr;
}

const IReceptionBitModel *Ieee80211LayeredOfdmReceiver::createBitModel(const LayeredTransmission *transmission, const ISnir *snir) const
{
    if (levelOfDetail == BIT_DOMAIN)
        return errorModel->computeBitModel(transmission, snir);
    return nullptr;
}

const IReceptionPacketModel *Ieee80211LayeredOfdmReceiver::createPacketModel(const LayeredTransmission *transmission, const ISnir *snir) const
{
    if (levelOfDetail == PACKET_DOMAIN)
        return errorModel->computePacketModel(transmission, snir);
    return nullptr;
}

const IReceptionSymbolModel *Ieee80211LayeredOfdmReceiver::createSymbolModel(const LayeredTransmission *transmission, const ISnir *snir) const
{
    if (levelOfDetail == SYMBOL_DOMAIN)
        return errorModel->computeSymbolModel(transmission, snir);
    return nullptr;
}

double Ieee80211LayeredOfdmReceiver::getCodeRateFromDecoderModule(const IDecoder *decoder) const
{
    const Ieee80211OfdmDecoderModule *decoderModule = check_and_cast<const Ieee80211OfdmDecoderModule *>(decoder);
    const Ieee80211OfdmCode *code = decoderModule->getCode();
    const ConvolutionalCode *convolutionalCode = code->getConvolutionalCode();
    return convolutionalCode ? 1.0 * convolutionalCode->getCodeRatePuncturingN() / convolutionalCode->getCodeRatePuncturingK() : 1;
}

const IReceptionBitModel *Ieee80211LayeredOfdmReceiver::createCompleteBitModel(const IReceptionBitModel *signalFieldBitModel, const IReceptionBitModel *dataFieldBitModel) const
{
    if (levelOfDetail >= BIT_DOMAIN) {
        BitVector *bits = new BitVector(*signalFieldBitModel->getAllBits());
        if (dataFieldBitModel != nullptr) {
            const BitVector *dataBits = dataFieldBitModel->getAllBits();
            for (unsigned int i = 0; i < dataBits->getSize(); i++)
                bits->appendBit(dataBits->getBit(i));
            return new ReceptionBitModel(signalFieldBitModel->getHeaderLength(), signalFieldBitModel->getHeaderGrossBitrate(), dataFieldBitModel->getDataLength(), dataFieldBitModel->getDataGrossBitrate(), bits, NaN);
        }
        else
            return new ReceptionBitModel(signalFieldBitModel->getHeaderLength(), signalFieldBitModel->getHeaderGrossBitrate(), b(0), bps(NaN), bits, NaN);
    }
    return nullptr;
}

const IReceptionPacketModel *Ieee80211LayeredOfdmReceiver::createDataFieldPacketModel(const IReceptionBitModel *signalFieldBitModel, const IReceptionBitModel *dataFieldBitModel, const IReceptionPacketModel *signalFieldPacketModel) const
{
    const IReceptionPacketModel *dataFieldPacketModel = nullptr;
    if (levelOfDetail > PACKET_DOMAIN) { // Create from the bit model
        if (dataDecoder)
            dataFieldPacketModel = dataDecoder->decode(dataFieldBitModel);
        else if (mode != nullptr) {
            const Ieee80211OfdmCode *code = mode->getDataMode()->getCode();
            const Ieee80211OfdmDecoder decoder(code);
            dataFieldPacketModel = decoder.decode(dataFieldBitModel);
        }
    }
    return dataFieldPacketModel;
}

const IReceptionPacketModel *Ieee80211LayeredOfdmReceiver::createSignalFieldPacketModel(const IReceptionBitModel *signalFieldBitModel) const
{
    const IReceptionPacketModel *signalFieldPacketModel = nullptr;
    if (levelOfDetail > PACKET_DOMAIN) { // Create from the bit model
        if (signalDecoder) // non-complaint
            signalFieldPacketModel = signalDecoder->decode(signalFieldBitModel);
        else { // complaint
               // In compliant mode the code for the signal field is always the following:
            const Ieee80211OfdmDecoder decoder(&Ieee80211OfdmCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling);
            signalFieldPacketModel = decoder.decode(signalFieldBitModel);
        }
    }
    return signalFieldPacketModel;
}

unsigned int Ieee80211LayeredOfdmReceiver::calculatePadding(unsigned int dataFieldLengthInBits, const ApskModulationBase *modulation, double codeRate) const
{
    ASSERT(modulation != nullptr);
    unsigned int codedBitsPerOFDMSymbol = modulation->getCodeWordSize() * NUMBER_OF_OFDM_DATA_SUBCARRIERS;
    unsigned int dataBitsPerOFDMSymbol = codedBitsPerOFDMSymbol * codeRate;
    return dataBitsPerOFDMSymbol - dataFieldLengthInBits % dataBitsPerOFDMSymbol;
}

unsigned int Ieee80211LayeredOfdmReceiver::getSignalFieldLength(const BitVector *signalField) const
{
    ShortBitVector length;
    for (int i = SIGNAL_LENGTH_FIELD_START; i <= SIGNAL_LENGTH_FIELD_END; i++)
        length.appendBit(signalField->getBit(i));
    return length.toDecimal();
}

const IReceptionSymbolModel *Ieee80211LayeredOfdmReceiver::createSignalFieldSymbolModel(const IReceptionSymbolModel *receptionSymbolModel) const
{
    const Ieee80211OfdmReceptionSymbolModel *signalFieldSymbolModel = nullptr;
    if (levelOfDetail > SYMBOL_DOMAIN)
        throw cRuntimeError("This level of detail is unimplemented!");
    else if (levelOfDetail == SYMBOL_DOMAIN) { // Create from symbol model (made by the error model)
        const std::vector<const ISymbol *> *symbols = receptionSymbolModel->getAllSymbols();
        std::vector<const ISymbol *> *signalSymbols = new std::vector<const ISymbol *>();
        const Ieee80211OfdmSymbol *signalSymbol = check_and_cast<const Ieee80211OfdmSymbol *>(symbols->at(0));
        signalSymbols->push_back(new Ieee80211OfdmSymbol(*signalSymbol)); // The first symbol is the signal field symbol
        signalFieldSymbolModel = new Ieee80211OfdmReceptionSymbolModel(1, receptionSymbolModel->getHeaderSymbolRate(), -1, NaN, signalSymbols);
    }
    return signalFieldSymbolModel;
}

const IReceptionSymbolModel *Ieee80211LayeredOfdmReceiver::createDataFieldSymbolModel(const IReceptionSymbolModel *receptionSymbolModel) const
{
    const Ieee80211OfdmReceptionSymbolModel *dataFieldSymbolModel = nullptr;
    if (levelOfDetail > SYMBOL_DOMAIN)
        throw cRuntimeError("This level of detail is unimplemented!");
    else if (levelOfDetail == SYMBOL_DOMAIN) { // Create from symbol model (made by the error model)
        const std::vector<const ISymbol *> *symbols = receptionSymbolModel->getAllSymbols();
        std::vector<const ISymbol *> *dataSymbols = new std::vector<const ISymbol *>();
        const Ieee80211OfdmSymbol *ofdmSymbol = nullptr;
        for (unsigned int i = 1; i < symbols->size(); i++) {
            ofdmSymbol = check_and_cast<const Ieee80211OfdmSymbol *>(symbols->at(i));
            dataSymbols->push_back(new Ieee80211OfdmSymbol(*ofdmSymbol));
        }
        dataFieldSymbolModel = new Ieee80211OfdmReceptionSymbolModel(-1, NaN, symbols->size() - 1, receptionSymbolModel->getDataSymbolRate(), dataSymbols);
    }
    return dataFieldSymbolModel;
}

const IReceptionBitModel *Ieee80211LayeredOfdmReceiver::createSignalFieldBitModel(const IReceptionBitModel *bitModel, const IReceptionSymbolModel *signalFieldSymbolModel) const
{
    const IReceptionBitModel *signalFieldBitModel = nullptr;
    if (levelOfDetail > BIT_DOMAIN) { // Create from symbol model
        if (signalDemodulator) // non-compliant
            signalFieldBitModel = signalDemodulator->demodulate(signalFieldSymbolModel);
        else { // compliant
               // In compliant mode, the signal field modulation is always BPSK
            Ieee80211OfdmModulation modulation(52, &BpskModulation::singleton);
            const Ieee80211OfdmDemodulator demodulator(&modulation);
            signalFieldBitModel = demodulator.demodulate(signalFieldSymbolModel);
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
        const BitVector *bits = bitModel->getAllBits();
        for (unsigned int i = 0; i < signalFieldLength; i++)
            signalFieldBits->appendBit(bits->getBit(i));
        signalFieldBitModel = new ReceptionBitModel(b(signalFieldLength), bitModel->getHeaderGrossBitrate(), b(-1), bps(NaN), signalFieldBits, NaN);
    }
    return signalFieldBitModel;
}

const IReceptionBitModel *Ieee80211LayeredOfdmReceiver::createDataFieldBitModel(const IReceptionBitModel *bitModel, const IReceptionSymbolModel *dataFieldSymbolModel, const IReceptionPacketModel *signalFieldPacketModel, const IReceptionBitModel *signalFieldBitModel) const
{
    const IReceptionBitModel *dataFieldBitModel = nullptr;
    if (levelOfDetail > BIT_DOMAIN) { // Create from symbol model
        if (dataDemodulator) // non-compliant
            dataFieldBitModel = dataDemodulator->demodulate(dataFieldSymbolModel);
        else if (mode != nullptr) { // compliant
            const Ieee80211OfdmDataMode *dataMode = mode->getDataMode();
            const Ieee80211OfdmDemodulator ofdmDemodulator(dataMode->getModulation());
            dataFieldBitModel = ofdmDemodulator.demodulate(dataFieldSymbolModel);
        }
    }
    else if (levelOfDetail == BIT_DOMAIN) { // Create from bit model (made by the error model)
        const ConvolutionalCode *convolutionalCode = nullptr;
        const ApskModulationBase *modulation = nullptr;
        double codeRate = NaN;
        const auto& bytesChunk = signalFieldPacketModel->getPacket()->peekAllAsBytes();
        unsigned int psduLengthInBits = getSignalFieldLength(new BitVector(bytesChunk->getBytes())) * 8;
        unsigned int dataFieldLengthInBits = psduLengthInBits + PPDU_SERVICE_FIELD_BITS_LENGTH + PPDU_TAIL_BITS_LENGTH;
        if (isCompliant) {
            const Ieee80211OfdmDataMode *dataMode = mode->getDataMode();
            modulation = dataMode->getModulation()->getSubcarrierModulation();
            const Ieee80211OfdmCode *code = dataMode->getCode();
            convolutionalCode = code->getConvolutionalCode();
            codeRate = convolutionalCode->getCodeRatePuncturingN() * 1.0 / convolutionalCode->getCodeRatePuncturingK();
        }
        else {
            const Ieee80211OfdmDecoderModule *decoderModule = check_and_cast<const Ieee80211OfdmDecoderModule *>(dataDecoder);
            const Ieee80211OfdmCode *code = decoderModule->getCode();
            convolutionalCode = code->getConvolutionalCode();
            modulation = mode->getDataMode()->getModulation()->getSubcarrierModulation();
            codeRate = getCodeRateFromDecoderModule(dataDecoder);
        }
        dataFieldLengthInBits += calculatePadding(dataFieldLengthInBits, modulation, 1.0 / codeRate);
//        ASSERT(dataFieldLengthInBits % convolutionalCode->getCodeRatePuncturingK() == 0);
        unsigned int encodedDataFieldLengthInBits = dataFieldLengthInBits * codeRate;
        const BitVector *bits = bitModel->getAllBits();
        unsigned int encodedSignalFieldLength = b(signalFieldBitModel->getHeaderLength()).get();
        if (dataFieldLengthInBits + encodedSignalFieldLength > bits->getSize())
            throw cRuntimeError("The calculated data field length = %d is greater then the actual bitvector length = %d", dataFieldLengthInBits, bits->getSize());
        BitVector *dataBits = new BitVector();
        for (unsigned int i = 0; i < encodedDataFieldLengthInBits; i++)
            dataBits->appendBit(bits->getBit(encodedSignalFieldLength + i));
        dataFieldBitModel = new ReceptionBitModel(b(-1), bps(NaN), b(encodedDataFieldLengthInBits), bitModel->getDataGrossBitrate(), dataBits, NaN);
    }
    return dataFieldBitModel;
}

const IReceptionSymbolModel *Ieee80211LayeredOfdmReceiver::createCompleteSymbolModel(const IReceptionSymbolModel *signalFieldSymbolModel, const IReceptionSymbolModel *dataFieldSymbolModel) const
{
    if (levelOfDetail >= SYMBOL_DOMAIN) {
        const std::vector<const ISymbol *> *symbols = signalFieldSymbolModel->getAllSymbols();
        std::vector<const ISymbol *> *completeSymbols = new std::vector<const ISymbol *>(*symbols);
        if (dataFieldSymbolModel != nullptr) {
            symbols = dataFieldSymbolModel->getAllSymbols();
            for (auto & symbol : *symbols)
                completeSymbols->push_back(new Ieee80211OfdmSymbol(*static_cast<const Ieee80211OfdmSymbol *>(symbol)));
            return new Ieee80211OfdmReceptionSymbolModel(signalFieldSymbolModel->getHeaderSymbolLength(), signalFieldSymbolModel->getHeaderSymbolRate(), dataFieldSymbolModel->getDataSymbolLength(), dataFieldSymbolModel->getDataSymbolRate(), completeSymbols);
        }
        else
            return new Ieee80211OfdmReceptionSymbolModel(signalFieldSymbolModel->getHeaderSymbolLength(), signalFieldSymbolModel->getHeaderSymbolRate(), 0, NaN, completeSymbols);
    }
    return nullptr;
}

const IReceptionPacketModel *Ieee80211LayeredOfdmReceiver::createCompletePacketModel(const char *name, const IReceptionPacketModel *signalFieldPacketModel, const IReceptionPacketModel *dataFieldPacketModel) const
{
    Packet *packet = new Packet(name);
    packet->insertAtBack(signalFieldPacketModel->getPacket()->peekAll());
    if (dataFieldPacketModel != nullptr) {
        packet->insertAtBack(dataFieldPacketModel->getPacket()->peekAll());
        packet->setBitError(signalFieldPacketModel->getPacket()->hasBitError() || dataFieldPacketModel->getPacket()->hasBitError());
        return new ReceptionPacketModel(packet, signalFieldPacketModel->getHeaderNetBitrate(), dataFieldPacketModel->getDataNetBitrate());
    }
    else
        return new ReceptionPacketModel(packet, signalFieldPacketModel->getHeaderNetBitrate(), bps(NaN));
}

const Ieee80211OfdmMode *Ieee80211LayeredOfdmReceiver::computeMode(Hz bandwidth) const
{
    const Ieee80211OfdmDecoderModule *ofdmSignalDecoderModule = check_and_cast<const Ieee80211OfdmDecoderModule *>(signalDecoder);
    const Ieee80211OfdmDecoderModule *ofdmDataDecoderModule = check_and_cast<const Ieee80211OfdmDecoderModule *>(dataDecoder);
    const Ieee80211OfdmDemodulatorModule *ofdmSignalDemodulatorModule = check_and_cast<const Ieee80211OfdmDemodulatorModule *>(signalDemodulator);
    const Ieee80211OfdmDemodulatorModule *ofdmDataDemodulatorModule = check_and_cast<const Ieee80211OfdmDemodulatorModule *>(dataDemodulator);
    const Ieee80211OfdmSignalMode *signalMode = new Ieee80211OfdmSignalMode(ofdmSignalDecoderModule->getCode(), ofdmSignalDemodulatorModule->getModulation(), channelSpacing, bandwidth, 0);
    const Ieee80211OfdmDataMode *dataMode = new Ieee80211OfdmDataMode(ofdmDataDecoderModule->getCode(), ofdmDataDemodulatorModule->getModulation(), channelSpacing, bandwidth);
    return new Ieee80211OfdmMode("", new Ieee80211OfdmPreambleMode(channelSpacing), signalMode, dataMode, channelSpacing, bandwidth);
}

const IReceptionResult *Ieee80211LayeredOfdmReceiver::computeReceptionResult(const IListening *listening, const IReception *reception, const IInterference *interference, const ISnir *snir, const std::vector<const IReceptionDecision *> *decisions) const
{
    const Ieee80211LayeredTransmission *transmission = check_and_cast<const Ieee80211LayeredTransmission *>(reception->getTransmission());
    // corruted model
    const IReceptionAnalogModel *analogModel = createAnalogModel(transmission, snir);
    const IReceptionSampleModel *sampleModel = createSampleModel(transmission, snir);
    const IReceptionSymbolModel *symbolModel = createSymbolModel(transmission, snir);
    const IReceptionBitModel *bitModel = createBitModel(transmission, snir);
    const IReceptionPacketModel *packetModel = createPacketModel(transmission, snir);
    // signal field model
    const IReceptionSymbolModel *signalFieldSymbolModel = createSignalFieldSymbolModel(symbolModel);
    const IReceptionSymbolModel *dataFieldSymbolModel = createDataFieldSymbolModel(symbolModel);
    const IReceptionBitModel *signalFieldBitModel = createSignalFieldBitModel(bitModel, signalFieldSymbolModel);
    const IReceptionPacketModel *signalFieldPacketModel = createSignalFieldPacketModel(signalFieldBitModel);
    // determine mode
    if (isCompliant) {
        auto packet = signalFieldPacketModel != nullptr ? signalFieldPacketModel->getPacket() : packetModel->getPacket();
        const auto& signalFieldBytesChunk = packet->peekAllAsBytes();
        uint8_t rate = signalFieldBytesChunk->getByte(0) >> 4;
        mode = Ieee80211OfdmCompliantModes::findCompliantMode(rate, channelSpacing);
        if (mode == nullptr)
            const_cast<Packet *>(packet)->setBitError(true);
    }
    else if (!mode)
        mode = computeMode(bandwidth);
    // data field model
    const IReceptionBitModel *dataFieldBitModel = mode != nullptr ? createDataFieldBitModel(bitModel, dataFieldSymbolModel, signalFieldPacketModel, signalFieldBitModel) : nullptr;
    const IReceptionPacketModel *dataFieldPacketModel = mode != nullptr ? createDataFieldPacketModel(signalFieldBitModel, dataFieldBitModel, signalFieldPacketModel) : nullptr;
    // complete model
//    if (!sampleModel)
//        sampleModel = createCompleteSampleModel(signalFieldSampleModel, dataFieldSampleModel);
    if (!symbolModel)
        symbolModel = createCompleteSymbolModel(signalFieldSymbolModel, dataFieldSymbolModel);
    if (!bitModel)
        bitModel = createCompleteBitModel(signalFieldBitModel, dataFieldBitModel);
    if (!packetModel)
        packetModel = createCompletePacketModel(transmission->getPacket()->getName(), signalFieldPacketModel, dataFieldPacketModel);
    // delete signal and data field models
    delete signalFieldSymbolModel;
    delete dataFieldSymbolModel;
    delete signalFieldBitModel;
    delete dataFieldBitModel;
    if (signalFieldPacketModel != nullptr) {
        delete signalFieldPacketModel->getPacket();
        delete signalFieldPacketModel;
    }
    if (dataFieldPacketModel != nullptr) {
        delete dataFieldPacketModel->getPacket();
        delete dataFieldPacketModel;
    }
    // add indications
    auto packet = const_cast<Packet *>(packetModel->getPacket());
    auto snirInd = packet->addTagIfAbsent<SnirInd>();
    snirInd->setMinimumSnir(snir->getMin());
    snirInd->setMaximumSnir(snir->getMax());
    snirInd->setAverageSnir(snir->getMean());
    auto errorRateInd = packet->addTagIfAbsent<ErrorRateInd>();
    if (symbolModel != nullptr)
        errorRateInd->setSymbolErrorRate(symbolModel->getSymbolErrorRate());
    if (bitModel != nullptr)
        errorRateInd->setBitErrorRate(bitModel->getBitErrorRate());
    if (packetModel != nullptr)
        errorRateInd->setPacketErrorRate(packetModel->getPacketErrorRate());
    auto modeInd = packet->addTagIfAbsent<Ieee80211ModeInd>();
    modeInd->setMode(transmission->getMode());
    auto channelInd = packet->addTagIfAbsent<Ieee80211ChannelInd>();
    channelInd->setChannel(transmission->getChannel());
    return new LayeredReceptionResult(reception, decisions, packetModel, bitModel, symbolModel, sampleModel, analogModel);
}

const IListening *Ieee80211LayeredOfdmReceiver::createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord& startPosition, const Coord& endPosition) const
{
    // We assume that in compliant mode the bandwidth is always 20MHz.
    return new BandListening(radio, startTime, endTime, startPosition, endPosition, centerFrequency, isCompliant ? Hz(20000000) : bandwidth);
}

// TODO copy
const IListeningDecision *Ieee80211LayeredOfdmReceiver::computeListeningDecision(const IListening *listening, const IInterference *interference) const
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

bool Ieee80211LayeredOfdmReceiver::computeIsReceptionPossible(const IListening *listening, const ITransmission *transmission) const
{
    auto ieee80211Transmission = dynamic_cast<const Ieee80211LayeredTransmission *>(transmission);
    return ieee80211Transmission && SnirReceiverBase::computeIsReceptionPossible(listening, transmission);
}

// TODO this is not purely functional, see interface comment
// TODO copy
bool Ieee80211LayeredOfdmReceiver::computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const
{
    auto ieee80211Transmission = dynamic_cast<const Ieee80211LayeredTransmission *>(reception->getTransmission());
    if (ieee80211Transmission == nullptr)
        return false;
    else {
        const BandListening *bandListening = check_and_cast<const BandListening *>(listening);
        const LayeredReception *dimensionalReception = check_and_cast<const LayeredReception *>(reception);
        const DimensionalReceptionSignalAnalogModel *analogModel = check_and_cast<const DimensionalReceptionSignalAnalogModel *>(dimensionalReception->getAnalogModel());
        if (bandListening->getCenterFrequency() != analogModel->getCenterFrequency() || bandListening->getBandwidth() != analogModel->getBandwidth()) {
            EV_DEBUG << "Computing reception possible: listening and reception bands are different -> reception is impossible" << endl;
            return false;
        }
        else {
            const INarrowbandSignal *narrowbandSignalAnalogModel = check_and_cast<const INarrowbandSignal *>(reception->getAnalogModel());
            W minReceptionPower = narrowbandSignalAnalogModel->computeMinPower(reception->getStartTime(), reception->getEndTime());
            bool isReceptionPossible = minReceptionPower >= sensitivity;
            EV_DEBUG << "Computing reception possible" << EV_FIELD(minReceptionPower) << EV_FIELD(sensitivity) << " -> reception is " << (isReceptionPossible ? "possible" : "impossible") << endl;
            return isReceptionPossible;
        }
    }
}

Ieee80211LayeredOfdmReceiver::~Ieee80211LayeredOfdmReceiver()
{
    if (!isCompliant) {
        delete mode->getPreambleMode();
        delete mode->getSignalMode();
        delete mode->getDataMode();
        delete mode;
    }
}

} // namespace physicallayer
} // namespace inet

