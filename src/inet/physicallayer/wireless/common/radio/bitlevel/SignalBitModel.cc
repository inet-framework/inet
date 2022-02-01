//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalBitModel.h"

namespace inet {
namespace physicallayer {

SignalBitModel::SignalBitModel(b headerLength, bps headerBitRate, b dataLength, bps dataBitRate, const BitVector *bits) :
    bits(bits),
    headerLength(headerLength),
    headerBitRate(headerBitRate),
    dataLength(dataLength),
    dataBitRate(dataBitRate)
{
}

SignalBitModel::~SignalBitModel()
{
    delete bits;
}

std::ostream& SignalBitModel::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "SignalBitModel";
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(headerLength)
               << EV_FIELD(headerBitRate)
               << EV_FIELD(dataLength)
               << EV_FIELD(dataBitRate);
    return stream;
}

TransmissionBitModel::TransmissionBitModel(const BitVector *bits, const IForwardErrorCorrection *forwardErrorCorrection, const IScrambling *scrambling, const IInterleaving *interleaving) :
    SignalBitModel(b(-1), bps(NaN), b(-1), bps(NaN), bits),
    forwardErrorCorrection(forwardErrorCorrection),
    scrambling(scrambling),
    interleaving(interleaving)
{
}

TransmissionBitModel::TransmissionBitModel(b headerLength, bps headerBitRate, b dataLength, bps dataBitRate, const BitVector *bits, const IForwardErrorCorrection *forwardErrorCorrection, const IScrambling *scrambling, const IInterleaving *interleaving) :
    SignalBitModel(headerLength, headerBitRate, dataLength, dataBitRate, bits),
    forwardErrorCorrection(forwardErrorCorrection),
    scrambling(scrambling),
    interleaving(interleaving)
{
}

ReceptionBitModel::ReceptionBitModel(b headerLength, bps headerBitRate, b dataLength, bps dataBitRate, const BitVector *bits, double bitErrorRate) :
    SignalBitModel(headerLength, headerBitRate, dataLength, dataBitRate, bits),
    bitErrorRate(bitErrorRate)
{
}

} // namespace physicallayer
} // namespace inet

