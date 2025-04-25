//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "CloseConnectionState.h"
#include "DrainConnectionState.h"

namespace inet {
namespace quic {

ConnectionState *CloseConnectionState::processHandshakePacket(const Ptr<const HandshakePacketHeader>& packetHeader, Packet *pkt) {
    EV_DEBUG << "processHandshakePacket in " << name << endl;
    discardFrames(pkt);
    return this;
}

ConnectionState *CloseConnectionState::processOneRttPacket(const Ptr<const OneRttPacketHeader>& packetHeader, Packet *pkt)
{
    EV_DEBUG << "processOneRttPacket in " << name << endl;

    if (containsFrame(pkt, FRAME_HEADER_TYPE_CONNECTION_CLOSE_QUIC)) {
        discardFrames(pkt);
        return new DrainConnectionState(context);
    }
    context->sendConnectionClose(false, false, 0);
    if (containsFrame(pkt, FRAME_HEADER_TYPE_CONNECTION_CLOSE_APP)) {
        discardFrames(pkt);
        return new DrainConnectionState(context);
    }
    discardFrames(pkt);
    return this;
}

} /* namespace quic */
} /* namespace inet */
