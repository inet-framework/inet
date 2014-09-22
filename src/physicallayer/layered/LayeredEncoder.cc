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

#include "LayeredEncoder.h"
#include "BitVector.h"
#include "ModuleAccess.h"

namespace inet {

namespace physicallayer {

Define_Module(LayeredEncoder);

void LayeredEncoder::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        serializer = check_and_cast<ISerializer *>(getSubmodule("serializer"));
        scrambler = dynamic_cast<IScrambler *>(getSubmodule("scrambler"));
        fecEncoder = dynamic_cast<IFECEncoder *>(getSubmodule("fecEncoder"));
        interleaver = dynamic_cast<IInterleaver *>(getSubmodule("interleaver"));
    }
}

const ITransmissionBitModel *LayeredEncoder::encode(const ITransmissionPacketModel *packetModel) const
{
    const cPacket *packet = packetModel->getPacket();
    BitVector serializedPacket = serializer->serialize(packet);
    BitVector scrambledBits = serializedPacket;
    if (scrambler)
        scrambledBits = scrambler->scramble(serializedPacket);
    BitVector fecEncodedBits = scrambledBits;
    if (fecEncoder)
        fecEncodedBits = fecEncoder->encode(scrambledBits);
    BitVector *interleavedBits = new BitVector(fecEncodedBits);
    if (interleaver)
        *interleavedBits = interleaver->interleaving(fecEncodedBits);
    return new TransmissionBitModel(interleavedBits->getSize(), bitRate, interleavedBits, fecEncoder->getInfo(), scrambler->getInfo(), interleaver->getInfo());
}

} // namespace physicallayer

} // namespace inet
