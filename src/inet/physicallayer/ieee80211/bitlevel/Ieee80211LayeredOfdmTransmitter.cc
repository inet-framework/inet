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

#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/physicallayer/analogmodel/bitlevel/ScalarSignalAnalogModel.h"
#include "inet/physicallayer/common/bitlevel/LayeredTransmission.h"
#include "inet/physicallayer/common/bitlevel/SignalPacketModel.h"
#include "inet/physicallayer/contract/bitlevel/ISignalAnalogModel.h"
#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211ConvolutionalCode.h"
#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211LayeredOfdmTransmitter.h"
#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211LayeredTransmission.h"
#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211OfdmDefs.h"
#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211OfdmEncoder.h"
#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211OfdmEncoderModule.h"
#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211OfdmModulator.h"
#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211OfdmModulatorModule.h"
#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211OfdmSymbolModel.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211OfdmCode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211OfdmMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211OfdmModulation.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211ControlInfo_m.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211PhyHeader_m.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211Tag_m.h"

namespace inet {

namespace physicallayer {

Define_Module(Ieee80211LayeredOfdmTransmitter);

void Ieee80211LayeredOfdmTransmitter::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        isCompliant = par("isCompliant");
        dataEncoder = dynamic_cast<const IEncoder *>(getSubmodule("dataEncoder"));
        signalEncoder = dynamic_cast<const IEncoder *>(getSubmodule("signalEncoder"));
        dataModulator = dynamic_cast<const IModulator *>(getSubmodule("dataModulator"));
        signalModulator = dynamic_cast<const IModulator *>(getSubmodule("signalModulator"));
        pulseShaper = dynamic_cast<const IPulseShaper *>(getSubmodule("pulseShaper"));
        digitalAnalogConverter = dynamic_cast<const IDigitalAnalogConverter *>(getSubmodule("digitalAnalogConverter"));
        channelSpacing = Hz(par("channelSpacing"));
        power = W(par("power"));
        centerFrequency = Hz(par("centerFrequency"));
        bandwidth = Hz(par("bandwidth"));
        if (isCompliant && (dataEncoder || signalEncoder || dataModulator || signalModulator
                            || pulseShaper || digitalAnalogConverter || !std::isnan(channelSpacing.get()))) // TODO: check modulations
        {
            throw cRuntimeError("In compliant mode it is forbidden to set the following parameters: dataEncoder, signalEncoder, modulator, signalModulator, pulseShaper, digitalAnalogConverter, bandwidth, channelSpacing");
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
    else if (stage == INITSTAGE_LAST) {
        if (!isCompliant)
            mode = computeMode(bandwidth);
    }
}

std::ostream& Ieee80211LayeredOfdmTransmitter::printToStream(std::ostream& stream, int level) const
{
    stream << "Ieee80211LayeredOfdmTransmitter";
    if (level <= PRINT_LEVEL_TRACE)
        stream << ", levelOfDetail = " << levelOfDetail
               << ", mode = " << printObjectToString(mode, level + 1)
               << ", signalEncoder = " << printObjectToString(signalEncoder, level + 1)
               << ", dataEncoder = " << printObjectToString(dataEncoder, level + 1)
               << ", signalModulator = " << printObjectToString(signalModulator, level + 1)
               << ", dataModulator = " << printObjectToString(dataModulator, level + 1)
               << ", pulseShaper = " << printObjectToString(pulseShaper, level + 1)
               << ", digitalAnalogConverter = " << printObjectToString(digitalAnalogConverter, level + 1)
               << ", isCompliant = " << isCompliant
               << ", bandwidth = " << bandwidth
               << ", channelSpacing = " << channelSpacing
               << ", centerFrequency = " << centerFrequency
               << ", power = " << power;
    return stream;
}

const ITransmissionPacketModel *Ieee80211LayeredOfdmTransmitter::createPacketModel(const Packet *packet) const
{
    return new TransmissionPacketModel(packet, mode->getDataMode()->getNetBitrate());
}

const ITransmissionAnalogModel *Ieee80211LayeredOfdmTransmitter::createScalarAnalogModel(const ITransmissionPacketModel *packetModel, const ITransmissionBitModel *bitModel) const
{
    int headerBitLength = -1;
    int dataBitLength = -1;
    if (levelOfDetail > PACKET_DOMAIN) {
        headerBitLength = b(bitModel->getHeaderLength()).get();
        dataBitLength = b(bitModel->getDataLength()).get();
    }
    else {
        if (isCompliant) {
            const ConvolutionalCode *convolutionalCode = mode->getDataMode()->getCode()->getConvolutionalCode();
            headerBitLength = ENCODED_SIGNAL_FIELD_LENGTH;
            dataBitLength = convolutionalCode->getEncodedLength((packetModel->getPacket()->getByteLength() * 8 - DECODED_SIGNAL_FIELD_LENGTH));
        }
        else {
            throw cRuntimeError("Unimplemented");
        }
    }
    simtime_t preambleDuration = mode->getPreambleMode()->getDuration();
    simtime_t headerDuration = 0;
    if (!isCompliant) {
        unsigned int headerCodeWordSize = mode->getSignalMode()->getModulation()->getSubcarrierModulation()->getCodeWordSize();
        ASSERT(headerBitLength % headerCodeWordSize == 0);
        unsigned int numberOfSignalApskSymbols = headerBitLength / headerCodeWordSize;
        unsigned int numberOfSignalOFDMSymbols = numberOfSignalApskSymbols / NUMBER_OF_OFDM_DATA_SUBCARRIERS;
        headerDuration = numberOfSignalOFDMSymbols * mode->getSymbolInterval();
    }
    else
        headerDuration = mode->getSignalMode()->getDuration();
    unsigned int dataCodeWordSize = mode->getDataMode()->getModulation()->getSubcarrierModulation()->getCodeWordSize();
    ASSERT(dataBitLength % dataCodeWordSize == 0);
    unsigned int numberOfDataApskSymbols = dataBitLength / dataCodeWordSize;
    unsigned int numberOfDataOFDMSymbols = numberOfDataApskSymbols / NUMBER_OF_OFDM_DATA_SUBCARRIERS;
    simtime_t dataDuration = numberOfDataOFDMSymbols * mode->getSymbolInterval();
    simtime_t duration = preambleDuration + headerDuration + dataDuration;
    return new ScalarTransmissionSignalAnalogModel(duration, centerFrequency, mode->getDataMode()->getBandwidth(), power);
}

const ITransmissionPacketModel *Ieee80211LayeredOfdmTransmitter::createSignalFieldPacketModel(const ITransmissionPacketModel *completePacketModel) const
{
    // The SIGNAL field is composed of RATE (4), Reserved (1), LENGTH (12), Parity (1), Tail (6),
    // fields, so the SIGNAL field is 24 bits (OFDM_SYMBOL_SIZE / 2) long.
    auto packet = completePacketModel->getPacket();
    const auto& signalChunk = packet->peekAt(b(0), b(NUMBER_OF_OFDM_DATA_SUBCARRIERS / 2));
    return new TransmissionPacketModel(new Packet(nullptr, signalChunk), bps(NaN));
}

const ITransmissionPacketModel *Ieee80211LayeredOfdmTransmitter::createDataFieldPacketModel(const ITransmissionPacketModel *completePacketModel) const
{
    auto packet = completePacketModel->getPacket();
    const auto& dataChunk = packet->peekAt(b(NUMBER_OF_OFDM_DATA_SUBCARRIERS / 2), packet->getTotalLength() - b(NUMBER_OF_OFDM_DATA_SUBCARRIERS / 2));
    return new TransmissionPacketModel(new Packet(nullptr, dataChunk), bps(NaN));
}

void Ieee80211LayeredOfdmTransmitter::encodeAndModulate(const ITransmissionPacketModel *packetModel, const ITransmissionBitModel *& fieldBitModel, const ITransmissionSymbolModel *& fieldSymbolModel, const IEncoder *encoder, const IModulator *modulator, bool isSignalField) const
{
    const ITransmissionPacketModel *fieldPacketModel = nullptr;
    if (isSignalField)
        fieldPacketModel = createSignalFieldPacketModel(packetModel);
    else
        fieldPacketModel = createDataFieldPacketModel(packetModel);
    if (levelOfDetail >= BIT_DOMAIN) {
        if (fieldPacketModel) {
            if (encoder) // non-compliant mode
                fieldBitModel = encoder->encode(fieldPacketModel);
            else { // compliant mode
                const Ieee80211OfdmCode *code = isSignalField ? mode->getSignalMode()->getCode() : mode->getDataMode()->getCode();
                const Ieee80211OfdmEncoder encoder(code);
                fieldBitModel = encoder.encode(fieldPacketModel);
            }
        }
        else
            throw cRuntimeError("Encoder needs packet representation");
    }
    if (levelOfDetail >= SYMBOL_DOMAIN) {
        if (fieldBitModel) {
            if (modulator) // non-compliant mode
                fieldSymbolModel = modulator->modulate(fieldBitModel);
            else { // compliant mode
                const Ieee80211OfdmModulation *ofdmModulation = isSignalField ? mode->getSignalMode()->getModulation() : mode->getDataMode()->getModulation();
                Ieee80211OfdmModulator modulator(ofdmModulation, isSignalField ? 0 : 1);
                fieldSymbolModel = modulator.modulate(fieldBitModel);
            }
        }
        else
            throw cRuntimeError("Modulator needs bit representation");
    }
    delete fieldPacketModel->getPacket();
    delete fieldPacketModel;
}

const ITransmissionSymbolModel *Ieee80211LayeredOfdmTransmitter::createSymbolModel(const ITransmissionSymbolModel *signalFieldSymbolModel, const ITransmissionSymbolModel *dataFieldSymbolModel) const
{
    if (levelOfDetail >= SYMBOL_DOMAIN) {
        const std::vector<const ISymbol *> *signalSymbols = signalFieldSymbolModel->getSymbols();
        std::vector<const ISymbol *> *mergedSymbols = new std::vector<const ISymbol *>();
        const Ieee80211OfdmSymbol *ofdmSymbol = nullptr;
        for (auto & signalSymbol : *signalSymbols) {
            ofdmSymbol = check_and_cast<const Ieee80211OfdmSymbol *>(signalSymbol);
            mergedSymbols->push_back(new Ieee80211OfdmSymbol(*ofdmSymbol));
        }
        const std::vector<const ISymbol *> *dataSymbols = dataFieldSymbolModel->getSymbols();
        for (auto & dataSymbol : *dataSymbols) {
            ofdmSymbol = dynamic_cast<const Ieee80211OfdmSymbol *>(dataSymbol);
            mergedSymbols->push_back(new Ieee80211OfdmSymbol(*ofdmSymbol));
        }
        const Ieee80211OfdmTransmissionSymbolModel *transmissionSymbolModel = new Ieee80211OfdmTransmissionSymbolModel(1, 1.0 / mode->getSignalMode()->getDuration(), mergedSymbols->size() - 1, 1.0 / mode->getSymbolInterval(), mergedSymbols, signalFieldSymbolModel->getHeaderModulation(), dataFieldSymbolModel->getPayloadModulation());
        delete signalFieldSymbolModel;
        delete dataFieldSymbolModel;
        return transmissionSymbolModel;
    }
    return new Ieee80211OfdmTransmissionSymbolModel(-1, NaN, -1, NaN, nullptr, mode->getSignalMode()->getModulation()->getSubcarrierModulation(), mode->getDataMode()->getModulation()->getSubcarrierModulation());
}

const ITransmissionBitModel *Ieee80211LayeredOfdmTransmitter::createBitModel(const ITransmissionBitModel *signalFieldBitModel, const ITransmissionBitModel *dataFieldBitModel, const ITransmissionPacketModel *packetModel) const
{
    if (levelOfDetail >= BIT_DOMAIN) {
        BitVector *encodedBits = new BitVector(*signalFieldBitModel->getBits());
        unsigned int signalBitLength = signalFieldBitModel->getBits()->getSize();
        const BitVector *dataFieldBits = dataFieldBitModel->getBits();
        unsigned int dataBitLength = dataFieldBits->getSize();
        for (unsigned int i = 0; i < dataFieldBits->getSize(); i++)
            encodedBits->appendBit(dataFieldBits->getBit(i));
        const TransmissionBitModel *transmissionBitModel = new TransmissionBitModel(b(signalBitLength), mode->getSignalMode()->getGrossBitrate(), b(dataBitLength), mode->getDataMode()->getGrossBitrate(), encodedBits, dataFieldBitModel->getForwardErrorCorrection(), dataFieldBitModel->getScrambling(), dataFieldBitModel->getInterleaving());
        delete signalFieldBitModel;
        delete dataFieldBitModel;
        return transmissionBitModel;
    }
    // TODO
    return new TransmissionBitModel(b(-1), mode->getSignalMode()->getGrossBitrate(), b(-1), mode->getDataMode()->getGrossBitrate(), nullptr, nullptr, nullptr, nullptr);
}

b Ieee80211LayeredOfdmTransmitter::getPaddingLength(const Ieee80211OfdmMode *mode, b length) const
{
    // 18.3.5.4 Pad bits (PAD), 1597p.
    // TODO: in non-compliant mode: header padding.
    unsigned int codedBitsPerOFDMSymbol = mode->getDataMode()->getModulation()->getSubcarrierModulation()->getCodeWordSize() * NUMBER_OF_OFDM_DATA_SUBCARRIERS;
    const Ieee80211OfdmCode *code = mode->getDataMode()->getCode();
    unsigned int dataBitsPerOFDMSymbol = codedBitsPerOFDMSymbol; // N_DBPS
    if (code->getConvolutionalCode()) {
        const ConvolutionalCode *convolutionalCode = code->getConvolutionalCode();
        dataBitsPerOFDMSymbol = convolutionalCode->getDecodedLength(codedBitsPerOFDMSymbol);
    }
    unsigned int dataBitsLength = 6 + b(length).get() + 16;
    unsigned int numberOfOFDMSymbols = lrint(ceil(1.0*dataBitsLength / dataBitsPerOFDMSymbol));
    unsigned int numberOfBitsInTheDataField = dataBitsPerOFDMSymbol * numberOfOFDMSymbols; // N_DATA
    unsigned int numberOfPadBits = numberOfBitsInTheDataField - dataBitsLength; // N_PAD
    return b(numberOfPadBits);
}

const ITransmissionSampleModel *Ieee80211LayeredOfdmTransmitter::createSampleModel(const ITransmissionSymbolModel *symbolModel) const
{
    if (levelOfDetail >= SAMPLE_DOMAIN) {
        throw cRuntimeError("This level of detail is unimplemented.");
//        if (symbolModel)
//        {
//            if (pulseShaper) // non-compliant mode
//                sampleModel = pulseShaper->shape(symbolModel);
//            else // compliant mode
//            {
//            }
//        }
//        else
//            throw cRuntimeError("Pulse shaper needs symbol representation");
    }
    else
        return nullptr;
}

const ITransmissionAnalogModel *Ieee80211LayeredOfdmTransmitter::createAnalogModel(const ITransmissionPacketModel *packetModel, const ITransmissionBitModel *bitModel, const ITransmissionSymbolModel *symbolModel, const ITransmissionSampleModel *sampleModel) const
{
    const ITransmissionAnalogModel *analogModel = nullptr;
    if (digitalAnalogConverter) {
        if (!sampleModel)
            analogModel = digitalAnalogConverter->convertDigitalToAnalog(sampleModel);
        else
            throw cRuntimeError("Digital/analog converter needs sample representation");
    }
    else // TODO: Analog model is obligatory, currently we use scalar analog model as default analog model
        analogModel = createScalarAnalogModel(packetModel, bitModel);
    return analogModel;
}

const Ieee80211OfdmMode *Ieee80211LayeredOfdmTransmitter::computeMode(Hz bandwidth) const
{
    const Ieee80211OfdmEncoderModule *ofdmSignalEncoderModule = check_and_cast<const Ieee80211OfdmEncoderModule *>(signalEncoder);
    const Ieee80211OfdmEncoderModule *ofdmDataEncoderModule = check_and_cast<const Ieee80211OfdmEncoderModule *>(dataEncoder);
    const Ieee80211OfdmModulatorModule *ofdmSignalModulatorModule = check_and_cast<const Ieee80211OfdmModulatorModule *>(signalModulator);
    const Ieee80211OfdmModulatorModule *ofdmDataModulatorModule = check_and_cast<const Ieee80211OfdmModulatorModule *>(dataModulator);
    const Ieee80211OfdmSignalMode *signalMode = new Ieee80211OfdmSignalMode(ofdmSignalEncoderModule->getCode(), ofdmSignalModulatorModule->getModulation(), channelSpacing, bandwidth, 0);
    const Ieee80211OfdmDataMode *dataMode = new Ieee80211OfdmDataMode(ofdmDataEncoderModule->getCode(), ofdmDataModulatorModule->getModulation(), channelSpacing, bandwidth);
    return new Ieee80211OfdmMode("", new Ieee80211OfdmPreambleMode(channelSpacing), signalMode, dataMode, channelSpacing, bandwidth);
}

const Ieee80211OfdmMode *Ieee80211LayeredOfdmTransmitter::getMode(const Packet* packet) const
{
    auto modeReq = const_cast<Packet*>(packet)->findTag<Ieee80211ModeReq>();
    if (isCompliant)
        return modeReq != nullptr ? check_and_cast<const Ieee80211OfdmMode*>(modeReq->getMode()) : &Ieee80211OfdmCompliantModes::getCompliantMode(11, MHz(20));
    else
        return mode;
}

const ITransmission *Ieee80211LayeredOfdmTransmitter::createTransmission(const IRadio *transmitter, const Packet *packet, const simtime_t startTime) const
{
    mode = getMode(packet);
    const ITransmissionBitModel *bitModel = nullptr;
    const ITransmissionBitModel *signalFieldBitModel = nullptr;
    const ITransmissionBitModel *dataFieldBitModel = nullptr;
    const ITransmissionSymbolModel *symbolModel = nullptr;
    const ITransmissionSymbolModel *signalFieldSymbolModel = nullptr;
    const ITransmissionSymbolModel *dataFieldSymbolModel = nullptr;
    const ITransmissionSampleModel *sampleModel = nullptr;
    const ITransmissionAnalogModel *analogModel = nullptr;
    const ITransmissionPacketModel *packetModel = createPacketModel(packet);
    encodeAndModulate(packetModel, signalFieldBitModel, signalFieldSymbolModel, signalEncoder, signalModulator, true);
    encodeAndModulate(packetModel, dataFieldBitModel, dataFieldSymbolModel, dataEncoder, dataModulator, false);
    bitModel = createBitModel(signalFieldBitModel, dataFieldBitModel, packetModel);
    symbolModel = createSymbolModel(signalFieldSymbolModel, dataFieldSymbolModel);
    sampleModel = createSampleModel(symbolModel);
    analogModel = createAnalogModel(packetModel, bitModel, symbolModel, sampleModel);
    IMobility *mobility = transmitter->getAntenna()->getMobility();
    // assuming movement and rotation during transmission is negligible
    const simtime_t endTime = startTime + analogModel->getDuration();
    const Coord startPosition = mobility->getCurrentPosition();
    const Coord endPosition = mobility->getCurrentPosition();
    const Quaternion startOrientation = mobility->getCurrentAngularPosition();
    const Quaternion endOrientation = mobility->getCurrentAngularPosition();
    // TODO: compute channel
    return new Ieee80211LayeredTransmission(packetModel, bitModel, symbolModel, sampleModel, analogModel, transmitter, packet, startTime, endTime, -1, -1, -1, startPosition, endPosition, startOrientation, endOrientation, mode, nullptr);
}

Ieee80211LayeredOfdmTransmitter::~Ieee80211LayeredOfdmTransmitter()
{
    if (!isCompliant) {
        delete mode->getDataMode()->getModulation();
        delete mode->getSignalMode()->getModulation();
        delete mode->getPreambleMode();
        delete mode->getSignalMode();
        delete mode->getDataMode();
        delete mode;
    }
}

} // namespace physicallayer

} // namespace inet

