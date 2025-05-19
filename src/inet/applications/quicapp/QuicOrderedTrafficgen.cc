//
// Copyright (C) 2019-2024 Timo Völker
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
