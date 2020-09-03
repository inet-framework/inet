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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
//

#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/linklayer/ieee8021d/common/Ieee8021dBpdu_m.h"
#include "inet/linklayer/ieee8021d/stp/StpProtocolDissector.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::stp, StpProtocolDissector);

void StpProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    auto stpPacket = packet->popAtFront<BpduBase>();
    callback.startProtocolDataUnit(&Protocol::stp);
    callback.visitChunk(stpPacket, &Protocol::stp);
    callback.endProtocolDataUnit(&Protocol::stp);
}

} // namespace inet

