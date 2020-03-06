//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalBitModel.h"

namespace inet {
namespace physicallayer {

SignalBitModel::SignalBitModel(b headerLength, bps headerBitrate, b dataLength, bps dataBitrate, const BitVector *bits) :
    bits(bits),
    headerLength(headerLength),
    headerBitrate(headerBitrate),
    dataLength(dataLength),
    dataBitrate(dataBitrate)
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
               << EV_FIELD(headerBitrate)
               << EV_FIELD(dataLength)
               << EV_FIELD(dataBitrate);
    return stream;
}

TransmissionBitModel::TransmissionBitModel(const BitVector *bits, const IForwardErrorCorrection *forwardErrorCorrection, const IScrambling *scrambling, const IInterleaving *interleaving) :
    SignalBitModel(b(-1), bps(NaN), b(-1), bps(NaN), bits),
    forwardErrorCorrection(forwardErrorCorrection),
    scrambling(scrambling),
    interleaving(interleaving)
{
}

TransmissionBitModel::TransmissionBitModel(b headerLength, bps headerBitrate, b dataLength, bps dataBitrate, const BitVector *bits, const IForwardErrorCorrection *forwardErrorCorrection, const IScrambling *scrambling, const IInterleaving *interleaving) :
    SignalBitModel(headerLength, headerBitrate, dataLength, dataBitrate, bits),
    forwardErrorCorrection(forwardErrorCorrection),
    scrambling(scrambling),
    interleaving(interleaving)
{
}

ReceptionBitModel::ReceptionBitModel(b headerLength, bps headerBitrate, b dataLength, bps dataBitrate, const BitVector *bits, double bitErrorrate) :
    SignalBitModel(headerLength, headerBitrate, dataLength, dataBitrate, bits),
    bitErrorRate(bitErrorrate)
{
}

} // namespace physicallayer
} // namespace inet

