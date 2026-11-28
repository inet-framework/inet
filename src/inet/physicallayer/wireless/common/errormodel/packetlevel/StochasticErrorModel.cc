//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/errormodel/packetlevel/StochasticErrorModel.h"

#include "inet/physicallayer/wireless/common/contract/packetlevel/IFlatTransmission.h"

namespace inet {

namespace physicallayer {

Define_Module(StochasticErrorModel);

StochasticErrorModel::StochasticErrorModel() :
    packetErrorRate(NaN),
    bitErrorRate(NaN),
    symbolErrorRate(NaN)
{
}

static void checkProbability(const char *name, double value)
{
    if (!std::isnan(value) && !(0.0 <= value && value <= 1.0))
        throw cRuntimeError("Parameter '%s' must be a probability in the range [0, 1] or NaN, got %g", name, value);
}

void StochasticErrorModel::initialize(int stage)
{
    ErrorModelBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        packetErrorRate = par("packetErrorRate");
        bitErrorRate = par("bitErrorRate");
        symbolErrorRate = par("symbolErrorRate");
        checkProbability("packetErrorRate", packetErrorRate);
        checkProbability("bitErrorRate", bitErrorRate);
        checkProbability("symbolErrorRate", symbolErrorRate);
    }
}

std::ostream& StochasticErrorModel::printToStream(std::ostream& stream, int level, int evFlags) const
{
    if (level <= PRINT_LEVEL_TRACE)
        stream << "StochasticErrorModel"
               << ", packetErrorRate = " << packetErrorRate
               << ", bitErrorRate = " << bitErrorRate
               << ", symbolErrorRate = " << symbolErrorRate;
    return stream;
}

// Header and data length of a transmission in bits. A transmission that does
// not tell its lengths (a negative length means unknown) is described by its
// bit model, and a transmission without a header/data split is all data.
static void getPartLengths(const ITransmission *transmission, b& headerLength, b& dataLength)
{
    if (auto flatTransmission = dynamic_cast<const IFlatTransmission *>(transmission)) {
        headerLength = flatTransmission->getHeaderLength();
        dataLength = flatTransmission->getDataLength();
        if (headerLength >= b(0) && dataLength >= b(0))
            return;
    }
    if (auto bitModel = transmission->getBitModel()) {
        headerLength = bitModel->getHeaderLength();
        dataLength = bitModel->getDataLength();
    }
    else {
        headerLength = b(0);
        dataLength = transmission->getPacket()->getTotalLength();
    }
}

// The modulation of the header or the data part, or nullptr if unknown. The
// modulation of the whole transmission is that of its data part.
static const IModulation *getPartModulation(const ITransmission *transmission, IRadioSignal::SignalPart part)
{
    if (auto flatTransmission = dynamic_cast<const IFlatTransmission *>(transmission))
        return part == IRadioSignal::SIGNAL_PART_HEADER ? flatTransmission->getHeaderModulation() : flatTransmission->getDataModulation();
    else
        return nullptr;
}

static unsigned int getBitsPerSymbol(const IModulation *modulation)
{
    return modulation != nullptr ? modulation->getBitsPerSymbol() : 0;
}

// Under the assumption of independent errors, a packet of the given length
// has an error unless all of its bits are received correctly.
static double packetErrorRateFromBitErrorRate(double ber, b length)
{
    if (std::isnan(ber) || length < b(0))
        return NaN;
    else if (length == b(0))
        return 0.0;
    else
        return -std::expm1(length.get<b>() * std::log1p(-ber));
}

static double bitErrorRateFromPacketErrorRate(double per, b length)
{
    if (std::isnan(per) || length <= b(0))
        return NaN;
    else
        return -std::expm1(std::log1p(-per) / length.get<b>());
}

static double bitErrorRateFromSymbolErrorRate(double ser, unsigned int bitsPerSymbol)
{
    if (std::isnan(ser) || bitsPerSymbol == 0)
        return NaN;
    else
        return -std::expm1(std::log1p(-ser) / bitsPerSymbol);
}

static double symbolErrorRateFromBitErrorRate(double ber, unsigned int bitsPerSymbol)
{
    if (std::isnan(ber) || bitsPerSymbol == 0)
        return NaN;
    else
        return -std::expm1(bitsPerSymbol * std::log1p(-ber));
}

double StochasticErrorModel::computePacketErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const
{
    Enter_Method("computePacketErrorRate");
    auto transmission = snir->getReception()->getTransmission();
    b headerLength, dataLength;
    getPartLengths(transmission, headerLength, dataLength);
    if (part == IRadioSignal::SIGNAL_PART_PREAMBLE)
        return 0.0;
    if (part != IRadioSignal::SIGNAL_PART_WHOLE && part != IRadioSignal::SIGNAL_PART_HEADER && part != IRadioSignal::SIGNAL_PART_DATA)
        throw cRuntimeError("Unknown signal part: '%s'", IRadioSignal::getSignalPartName(part));
    if (!std::isnan(packetErrorRate)) {
        if (part == IRadioSignal::SIGNAL_PART_WHOLE)
            return packetErrorRate;
        // the share of the packet error rate that belongs to the bits of the part
        b partLength = part == IRadioSignal::SIGNAL_PART_HEADER ? headerLength : dataLength;
        if (partLength == b(0))
            return 0.0;
        else
            return -std::expm1(std::log1p(-packetErrorRate) * partLength.get<b>() / (headerLength + dataLength).get<b>());
    }
    // derived from the bit error rate of the header and of the data part
    auto partPacketErrorRate = [&] (IRadioSignal::SignalPart partOfPacket, b length) {
        if (length == b(0))
            return 0.0;
        double ber = computeBitErrorRate(snir, partOfPacket);
        if (std::isnan(ber) && !std::isnan(symbolErrorRate)) {
            auto modulation = getPartModulation(transmission, partOfPacket);
            throw cRuntimeError("The symbolErrorRate parameter is set, but the modulation of the %s part of the %s (%s) has no number of bits per symbol, set the bitErrorRate or the packetErrorRate parameter instead",
                    IRadioSignal::getSignalPartName(partOfPacket), check_and_cast<const cObject *>(transmission)->getClassName(), modulation != nullptr ? modulation->getClassName() : "modulation not specified");
        }
        return packetErrorRateFromBitErrorRate(ber, length);
    };
    if (part == IRadioSignal::SIGNAL_PART_HEADER)
        return partPacketErrorRate(part, headerLength);
    else if (part == IRadioSignal::SIGNAL_PART_DATA)
        return partPacketErrorRate(part, dataLength);
    else {
        // the whole packet is received correctly if both of its parts are
        double headerPacketErrorRate = partPacketErrorRate(IRadioSignal::SIGNAL_PART_HEADER, headerLength);
        double dataPacketErrorRate = partPacketErrorRate(IRadioSignal::SIGNAL_PART_DATA, dataLength);
        if (std::isnan(headerPacketErrorRate) || std::isnan(dataPacketErrorRate))
            return NaN;
        else
            return -std::expm1(std::log1p(-headerPacketErrorRate) + std::log1p(-dataPacketErrorRate));
    }
}

double StochasticErrorModel::computeBitErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const
{
    Enter_Method("computeBitErrorRate");
    if (!std::isnan(bitErrorRate))
        return bitErrorRate;
    auto transmission = snir->getReception()->getTransmission();
    if (!std::isnan(symbolErrorRate)) {
        double ber = bitErrorRateFromSymbolErrorRate(symbolErrorRate, getBitsPerSymbol(getPartModulation(transmission, part)));
        if (!std::isnan(ber))
            return ber;
    }
    // a constant bit error rate over the whole transmitted packet
    b headerLength, dataLength;
    getPartLengths(transmission, headerLength, dataLength);
    return bitErrorRateFromPacketErrorRate(packetErrorRate, headerLength + dataLength);
}

double StochasticErrorModel::computeSymbolErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const
{
    Enter_Method("computeSymbolErrorRate");
    if (!std::isnan(symbolErrorRate))
        return symbolErrorRate;
    auto transmission = snir->getReception()->getTransmission();
    return symbolErrorRateFromBitErrorRate(computeBitErrorRate(snir, part), getBitsPerSymbol(getPartModulation(transmission, part)));
}

} // namespace physicallayer

} // namespace inet
