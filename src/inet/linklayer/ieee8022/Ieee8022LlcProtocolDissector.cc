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

#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/linklayer/ieee8022/Ieee8022Llc.h"
#include "inet/linklayer/ieee8022/Ieee8022LlcHeader_m.h"
#include "inet/linklayer/ieee8022/Ieee8022LlcProtocolDissector.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::ieee8022, Ieee802LlcDissector);

void Ieee802LlcDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    const auto& header = packet->popAtFront<inet::Ieee8022LlcHeader>();
    callback.startProtocolDataUnit(&Protocol::ieee8022);
    callback.visitChunk(header, &Protocol::ieee8022);
    auto dataProtocol = Ieee8022Llc::getProtocol(header);
    callback.dissectPacket(packet, dataProtocol);
    callback.endProtocolDataUnit(&Protocol::ieee8022);
}

} // namespace inet

