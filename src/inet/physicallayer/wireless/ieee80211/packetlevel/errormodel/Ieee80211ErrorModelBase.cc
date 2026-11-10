//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/packetlevel/errormodel/Ieee80211ErrorModelBase.h"

#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211OfdmMode.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Radio.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Transmission.h"

namespace inet {

namespace physicallayer {

Ieee80211ErrorModelBase::Ieee80211ErrorModelBase()
{
}

double Ieee80211ErrorModelBase::computePacketErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const
{
    Enter_Method("computePacketErrorRate");
    auto transmission = check_and_cast<const Ieee80211Transmission *>(snir->getReception()->getTransmission());
    auto mode = transmission->getMode();
    auto phyHeader = Ieee80211Radio::peekIeee80211PhyHeaderAtFront(transmission->getPacket());
    auto headerLength = mode->getHeaderMode()->getLength();
    auto dataLength = computeDataLength(mode, phyHeader->getLengthField());
    // TODO check header length and data length for OFDM (signal) field
    double headerSuccessRate = getHeaderSuccessRate(mode, headerLength.get<b>(), getScalarSnir(snir));
    double dataSuccessRate = getDataSuccessRate(mode, dataLength.get<b>(), getScalarSnir(snir));
    switch (part) {
        case IRadioSignal::SIGNAL_PART_WHOLE:
            return 1.0 - headerSuccessRate * dataSuccessRate;
        case IRadioSignal::SIGNAL_PART_PREAMBLE:
            return 0;
        case IRadioSignal::SIGNAL_PART_HEADER:
            return 1.0 - headerSuccessRate;
        case IRadioSignal::SIGNAL_PART_DATA:
            return 1.0 - dataSuccessRate;
        default:
            throw cRuntimeError("Unknown signal part: '%s'", IRadioSignal::getSignalPartName(part));
    }
}

b Ieee80211ErrorModelBase::computeDataLength(const IIeee80211Mode *mode, B psduLength) const
{
    auto dataLength = b(mode->getDataMode()->getCompleteLength(psduLength));
    // the SERVICE field of an OFDM mode precedes the PSDU in the DATA part
    if (auto ofdmMode = dynamic_cast<const Ieee80211OfdmMode *>(mode))
        dataLength += ofdmMode->getHeaderMode()->getServiceFieldLength();
    return dataLength;
}

double Ieee80211ErrorModelBase::computeBitErrorRate(const ISnir *snir, IRadioSignal::SignalPart part, b length, simtime_t startTime, simtime_t endTime) const
{
    Enter_Method("computeBitErrorRate");
    checkRegionPart(part);
    auto transmission = check_and_cast<const Ieee80211Transmission *>(snir->getReception()->getTransmission());
    auto mode = transmission->getMode();
    auto phyHeader = Ieee80211Radio::peekIeee80211PhyHeaderAtFront(transmission->getPacket());
    double scalarSnir = getScalarSnir(snir, startTime, endTime);
    if (part == IRadioSignal::SIGNAL_PART_HEADER) {
        // each bit of the PHY header chunk has its share of the success rate of the header
        double headerSuccessRate = getHeaderSuccessRate(mode, mode->getHeaderMode()->getLength().get<b>(), scalarSnir);
        return -std::expm1(std::log(headerSuccessRate) / phyHeader->getChunkLength().get<b>());
    }
    else {
        // each bit of the PSDU has its share of the bits that the rate of the data part counts:
        // the PSDU, and the SERVICE, tail, and pad bits
        double bitSuccessRate = getDataSuccessRate(mode, 1, scalarSnir);
        double countedLength = computeDataLength(mode, phyHeader->getLengthField()).get<b>();
        double psduLength = b(phyHeader->getLengthField()).get<b>();
        return -std::expm1(std::log(bitSuccessRate) * countedLength / psduLength);
    }
}

double Ieee80211ErrorModelBase::computeBitErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const
{
    Enter_Method("computeBitErrorRate");
    return NaN;
}

double Ieee80211ErrorModelBase::computeSymbolErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const
{
    Enter_Method("computeSymbolErrorRate");
    return NaN;
}

Packet *Ieee80211ErrorModelBase::computeCorruptedPacket(const Packet *packet, double ber) const
{
    if (corruptionMode == CorruptionMode::CM_PACKET)
        return ErrorModelBase::computeCorruptedPacket(packet, ber);
    else
        throw cRuntimeError("Unimplemented corruption mode");
}

double Ieee80211ErrorModelBase::getDsssDbpskSuccessRate(uint32_t bitLength, double snir) const
{
    double EbN0 = snir * spectralEfficiency1bit; // 1 bit per symbol with 1 MSPS
    double bitErrorRate = 0.5 * exp(-EbN0);
    return pow((1.0 - bitErrorRate), (int)bitLength);
}

double Ieee80211ErrorModelBase::getDsssDqpskSuccessRate(uint32_t bitLength, double snir) const
{
    double EbN0 = snir * spectralEfficiency2bit; // 2 bits per symbol, 1 MSPS
    double bitErrorRate = ((sqrt(2.0) + 1.0) / sqrt(8.0 * 3.1415926 * sqrt(2.0))) * (1.0 / sqrt(EbN0)) * exp(-(2.0 - sqrt(2.0)) * EbN0);
    return pow((1.0 - bitErrorRate), (int)bitLength);
}

double Ieee80211ErrorModelBase::getDsssDqpskCck5_5SuccessRate(uint32_t bitLength, double snir) const
{
    double bitErrorRate;
    if (snir > sirPerfect)
        bitErrorRate = 0.0;
    else if (snir < sirImpossible)
        bitErrorRate = 0.5;
    else {
        double a1 = 5.3681634344056195e-001;
        double a2 = 3.3092430025608586e-003;
        double a3 = 4.1654372361004000e-001;
        double a4 = 1.0288981434358866e+000;
        bitErrorRate = a1 * exp(-(pow((snir - a2) / a3, a4)));
    }
    return pow((1.0 - bitErrorRate), (int)bitLength);
}

double Ieee80211ErrorModelBase::getDsssDqpskCck11SuccessRate(uint32_t bitLength, double snir) const
{
    double bitErrorRate;
    if (snir > sirPerfect)
        bitErrorRate = 0.0;
    else if (snir < sirImpossible)
        bitErrorRate = 0.5;
    else {
        double a1 = 7.9056742265333456e-003;
        double a2 = -1.8397449399176360e-001;
        double a3 = 1.0740689468707241e+000;
        double a4 = 1.0523316904502553e+000;
        double a5 = 3.0552298746496687e-001;
        double a6 = 2.2032715128698435e+000;
        bitErrorRate = (a1 * snir * snir + a2 * snir + a3) / (snir * snir * snir + a4 * snir * snir + a5 * snir + a6);
    }
    return pow((1.0 - bitErrorRate), (int)bitLength);
}

} // namespace physicallayer

} // namespace inet

