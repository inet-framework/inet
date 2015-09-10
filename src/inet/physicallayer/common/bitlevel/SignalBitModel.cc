//
// Copyright (C) 2013 OpenSim Ltd.
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

#include "inet/physicallayer/common/bitlevel/SignalBitModel.h"

namespace inet {

namespace physicallayer {

SignalBitModel::SignalBitModel(int headerBitLength, bps headerBitRate, int payloadBitLength, bps payloadBitRate, const BitVector *bits) :
    bits(bits),
    headerBitLength(headerBitLength),
    headerBitRate(headerBitRate),
    payloadBitLength(payloadBitLength),
    payloadBitRate(payloadBitRate)
{
}

SignalBitModel::~SignalBitModel()
{
    delete bits;
}

std::ostream& SignalBitModel::printToStream(std::ostream& stream, int level) const
{
    stream << "SignalBitModel";
    if (level >= PRINT_LEVEL_TRACE)
        stream << ", headerBitLength = " << headerBitLength
               << ", headerBitRate = " << headerBitRate
               << ", payloadBitLength = " << payloadBitLength
               << ", payloadBitRate = " << payloadBitRate;
    return stream;
}

TransmissionBitModel::TransmissionBitModel(const BitVector *bits, const IForwardErrorCorrection *forwardErrorCorrection, const IScrambling *scrambling, const IInterleaving *interleaving) :
    SignalBitModel(-1, bps(NaN), -1, bps(NaN), bits),
    forwardErrorCorrection(forwardErrorCorrection),
    scrambling(scrambling),
    interleaving(interleaving)
{
}

TransmissionBitModel::TransmissionBitModel(int headerBitLength, bps headerBitRate, int payloadBitLength, bps payloadBitRate, const BitVector *bits, const IForwardErrorCorrection *forwardErrorCorrection, const IScrambling *scrambling, const IInterleaving *interleaving) :
    SignalBitModel(headerBitLength, headerBitRate, payloadBitLength, payloadBitRate, bits),
    forwardErrorCorrection(forwardErrorCorrection),
    scrambling(scrambling),
    interleaving(interleaving)
{
}

ReceptionBitModel::ReceptionBitModel(int headerBitLength, bps headerBitRate, int payloadBitLength, bps payloadBitRate, const BitVector *bits) :
    SignalBitModel(headerBitLength, headerBitRate, payloadBitLength, payloadBitRate, bits)
{
}

} // namespace physicallayer

} // namespace inet

