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

#include "QuicOrderedTrafficgen.h"
#include "inet/common/packet/chunk/BytesChunk.h"

namespace inet {

Define_Module(QuicOrderedTrafficgen);

void QuicOrderedTrafficgen::sendData(TrafficgenData* gmsg)
{
    EV_INFO << "sendData ordered - stream " << gmsg->getId() << " - size " << gmsg->getByteLength() << " byte" << endl;

    Packet *packet = new Packet("ApplicationData");

    std::vector<uint8_t> bytes;
    for (int64_t i=0; i<gmsg->getByteLength(); i++) {
        bytes.push_back(currentByte);
        currentByte++;
    }

    auto applicationData = makeShared<BytesChunk>(bytes);

    // set streamId for applicationData
    auto& tags = packet->getTags();
    tags.addTagIfAbsent<QuicStreamReq>()->setStreamID(getStreamId(gmsg->getId()));

    packet->insertAtBack(applicationData);
    //emit(packetSentSignal, packet);
    socket.send(packet);
}

} //namespace
