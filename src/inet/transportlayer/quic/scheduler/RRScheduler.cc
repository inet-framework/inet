//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "RRScheduler.h"

namespace inet {
namespace quic {

Stream *RRScheduler::selectStream(uint64_t maxFrameSize)
{
    Stream *selectedStream = nullptr;
    uint32_t streamCounter = 0;

    if (streamMap->empty()) {
        // cannot select a stream if there is none
        return nullptr;
    }

    // check if in the packet is enough space to put the streamHeaderSize
    // TODO should it be + 1 byte? in order to have space for streamData?
    //if (maxFrameSize < StreamFrameHeader::MAX_HEADER_SIZE) return size;

    if(lastScheduledStream == nullptr){
        selectedStream = streamMap->begin()->second;
    }else{
        selectedStream = getNextStream(lastScheduledStream);
    }

    //check if stream sendQueue is not empty & if StreamFlowControl is allowed to send
    while(selectedStream->getNextStreamFrameSize(maxFrameSize) == 0){
        streamCounter++;

        //Solution in the case, if all streams have size = 0, is needed
        if (streamCounter == streamMap->size()){
            EV_TRACE << "There is no stream to send " << endl;
            selectedStream =  nullptr;
            break;
        }

        selectedStream = getNextStream(selectedStream);
    }

    if (selectedStream != nullptr) {
        EV_TRACE << "Selected Stream " << selectedStream->id << " for sending" << endl;
        lastScheduledStream = selectedStream;
    }

    return selectedStream;
}


Stream *RRScheduler::getNextStream(Stream *stream)
{
    Stream *nextStream = nullptr;

    auto streamIter = streamMap->find(stream->id);
    streamIter = std::next(streamIter);

    if (streamIter == streamMap->end())
        nextStream = streamMap->begin()->second;
    else
        nextStream = streamIter->second;

    return nextStream;
}

} /* namespace quic */
} /* namespace inet */
