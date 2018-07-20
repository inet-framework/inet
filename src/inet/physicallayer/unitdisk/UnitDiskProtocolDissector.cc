//
// Copyright (C) 2018 OpenSim Ltd.
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
// @author: Zoltan Bojthe
//

#include "inet//physicallayer/unitdisk/UnitDiskProtocolDissector.h"

#include "inet/common/ProtocolGroup.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/physicallayer/unitdisk/UnitDiskPhyHeader_m.h"


namespace inet {

Register_Protocol_Dissector(&Protocol::unitDisk, UnitDiskProtocolDissector);

void UnitDiskProtocolDissector::dissect(Packet *packet, ICallback& callback) const
{
    auto header = packet->popAtFront<UnitDiskPhyHeader>();
    callback.startProtocolDataUnit(&Protocol::unitDisk);
    callback.visitChunk(header, &Protocol::unitDisk);
    auto payloadProtocol = header->getPayloadProtocol();
    callback.dissectPacket(packet, payloadProtocol);
    ASSERT(packet->getDataLength() == B(0));
    callback.endProtocolDataUnit(&Protocol::unitDisk);
}

} // namespace inet

