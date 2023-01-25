//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211LayeredOfdmTransmitter.h"

#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/physicallayer/wireless/common/analogmodel/bitlevel/DimensionalSignalAnalogModel.h"
#include "inet/physicallayer/wireless/common/analogmodel/bitlevel/LayeredTransmission.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalAnalogModel.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalPacketModel.h"
#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211ConvolutionalCode.h"
#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211LayeredTransmission.h"
#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmDefs.h"
#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmEncoder.h"
#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmEncoderModule.h"
#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmModulator.h"
#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmModulatorModule.h"
#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmSymbolModel.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211OfdmCode.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211OfdmMode.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211OfdmModulation.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211ControlInfo_m.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211PhyHeader_m.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Tag_m.h"

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
                            || pulseShaper || digitalAnalogConverter || !std::isnan(channelSpacing.get()))) // TODO check modulations
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

std::ostream& Ieee80211LayeredOfdmTransmitter::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "Ieee80211LayeredOfdmTransmitter";
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(levelOfDetail)
               << EV_FIELD(mode, printFieldToString(mode, level + 1, evFlags))
               << EV_FIELD(signalEncoder, printFieldToString(signalEncoder, level + 1, evFlags))
               << EV_FIELD(dataEncoder, printFieldToString(dataEncoder, level + 1, evFlags))
               << EV_FIELD(signalModulator, printFieldToString(signalModulator, level + 1, evFlags))
               << EV_FIELD(dataModulator, printFieldToString(dataModulator, level + 1, evFlags))
               << EV_FIELD(pulseShaper, printFieldToString(pulseShaper, level + 1, evFlags))
               << EV_FIELD(digitalAnalogConverter, printFieldToString(digitalAnalogConverter, level + 1, evFlags))
               << EV_FIELD(isCompliant)
               << EV_FIELD(bandwidth)
               << EV_FIELD(channelSpacing)
               << EV_FIELD(centerFrequency)
               << EV_FIELD(power);
    return stream;
}

const ITransmissionPacketModel *Ieee80211LayeredOfdmTransmitter::createPacketModel(const Packet *packet) const
{
    return new TransmissionPacketModel(packet, mode->getHeaderMode()->getNetBitrate(), mode->getDataMode()->getNetBitrate());
}

const ITransmissionAnalogModel *Ieee80211LayeredOfdmTransmitter::createDimensionalAnalogModel(simtime_t startTime, const ITransmissionPacketModel *packetModel, const ITransmissionBitModel *bitModel) const
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
    auto endTime = startTime + preambleDuration + headerDuration + dataDuration;
    // TODO: centerFrequency doesn't take the channel into account
    auto powerFunction = makeShared<math::Boxcar2DFunction<WpHz, simsec, Hz>>(simsec(startTime), simsec(endTime), centerFrequency - bandwidth / 2, centerFrequency + bandwidth / 2, power / bandwidth);
    return new DimensionalTransmissionSignalAnalogModel(preambleDuration, headerDuration, dataDuration, centerFrequency, mode->getDataMode()->getBandwidth(), powerFunction);
}

const ITransmissionPacketModel *Ieee80211LayeredOfdmTransmitter::createSignalFieldPacketModel(const ITransmissionPacketModel *completePacketModel) const
{
    // The SIGNAL field is composed of RATE (4), Reserved (1), LENGTH (12), Parity (1), Tail (6),
    // fields, so the SIGNAL field is 24 bits (OFDM_SYMBOL_SIZE / 2) long.
    auto packet = completePacketModel->getPacket();
    const auto& signalChunk = packet->peekAt(b(0), b(NUMBER_OF_OFDM_DATA_SUBCARRIERS / 2));
    return new TransmissionPacketModel(new Packet(nullptr, signalChunk), bps(NaN), bps(NaN));
}

const ITransmissionPacketModel *Ieee80211LayeredOfdmTransmitter::createDataFieldPacketModel(const ITransmissionPacketModel *completePacketModel) const
{
    auto packet = completePacketModel->getPacket();
    const auto& dataChunk = packet->peekAt(b(NUMBER_OF_OFDM_DATA_SUBCARRIERS / 2), packet->getTotalLength() - b(NUMBER_OF_OFDM_DATA_SUBCARRIERS / 2));
    return new TransmissionPacketModel(new Packet(nullptr, dataChunk), bps(NaN), bps(NaN));
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
        const std::vector<const ISymbol *> *signalSymbols = signalFieldSymbolModel->getAllSymbols();
        std::vector<const ISymbol *> *mergedSymbols = new std::vector<const ISymbol *>();
        const Ieee80211OfdmSymbol *ofdmSymbol = nullptr;
        for (auto& signalSymbol : *signalSymbols) {
            ofdmSymbol = check_and_cast<const Ieee80211OfdmSymbol *>(signalSymbol);
            mergedSymbols->push_back(new Ieee80211OfdmSymbol(*ofdmSymbol));
        }
        const std::vector<const ISymbol *> *dataSymbols = dataFieldSymbolModel->getAllSymbols();
        for (auto& dataSymbol : *dataSymbols) {
            ofdmSymbol = dynamic_cast<const Ieee80211OfdmSymbol *>(dataSymbol);
            mergedSymbols->push_back(new Ieee80211OfdmSymbol(*ofdmSymbol));
        }
        const Ieee80211OfdmTransmissionSymbolModel *transmissionSymbolModel = new Ieee80211OfdmTransmissionSymbolModel(1, 1.0 / mode->getSignalMode()->getDuration(), mergedSymbols->size() - 1, 1.0 / mode->getSymbolInterval(), mergedSymbols, signalFieldSymbolModel->getHeaderModulation(), dataFieldSymbolModel->getDataModulation());
        delete signalFieldSymbolModel;
        delete dataFieldSymbolModel;
        return transmissionSymbolModel;
    }
    return new Ieee80211OfdmTransmissionSymbolModel(-1, NaN, -1, NaN, nullptr, mode->getSignalMode()->getModulation(), mode->getDataMode()->getModulation());
}

const ITransmissionBitModel *Ieee80211LayeredOfdmTransmitter::createBitModel(const ITransmissionBitModel *signalFieldBitModel, const ITransmissionBitModel *dataFieldBitModel, const ITransmissionPacketModel *packetModel) const
{
    if (levelOfDetail >= BIT_DOMAIN) {
        BitVector *encodedBits = new BitVector(*signalFieldBitModel->getAllBits());
        unsigned int signalBitLength = signalFieldBitModel->getAllBits()->getSize();
        const BitVector *dataFieldBits = dataFieldBitModel->getAllBits();
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

const ITransmissionAnalogModel *Ieee80211LayeredOfdmTransmitter::createAnalogModel(simtime_t startTime, const ITransmissionPacketModel *packetModel, const ITransmissionBitModel *bitModel, const ITransmissionSymbolModel *symbolModel, const ITransmissionSampleModel *sampleModel) const
{
    const ITransmissionAnalogModel *analogModel = nullptr;
    if (digitalAnalogConverter) {
        if (!sampleModel)
            analogModel = digitalAnalogConverter->convertDigitalToAnalog(sampleModel);
        else
            throw cRuntimeError("Digital/analog converter needs sample representation");
    }
    else // TODO: Analog model is obligatory, currently we use dimensional analog model as default analog model
        analogModel = createDimensionalAnalogModel(startTime, packetModel, bitModel);
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

const Ieee80211OfdmMode *Ieee80211LayeredOfdmTransmitter::getMode(const Packet *packet) const
{
    const auto& modeReq = const_cast<Packet *>(packet)->findTag<Ieee80211ModeReq>();
    if (isCompliant)
        return modeReq != nullptr ? check_and_cast<const Ieee80211OfdmMode *>(modeReq->getMode()) : &Ieee80211OfdmCompliantModes::getCompliantMode(11, MHz(20));
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
    analogModel = createAnalogModel(startTime, packetModel, bitModel, symbolModel, sampleModel);
    IMobility *mobility = transmitter->getAntenna()->getMobility();
    // assuming movement and rotation during transmission is negligible
    const simtime_t endTime = startTime + analogModel->getDuration();
    const Coord& startPosition = mobility->getCurrentPosition();
    const Coord& endPosition = mobility->getCurrentPosition();
    const Quaternion& startOrientation = mobility->getCurrentAngularPosition();
    const Quaternion& endOrientation = mobility->getCurrentAngularPosition();
    // TODO: compute channel
    const simtime_t preambleDuration = mode->getPreambleLength();
    const simtime_t headerDuration = mode->getHeaderMode()->getDuration();
    const simtime_t dataDuration = mode->getDataMode()->getDuration(packet->getDataLength());
    return new Ieee80211LayeredTransmission(packetModel, bitModel, symbolModel, sampleModel, analogModel, transmitter, packet, startTime, endTime, preambleDuration, headerDuration, dataDuration, startPosition, endPosition, startOrientation, endOrientation, mode, nullptr);
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

