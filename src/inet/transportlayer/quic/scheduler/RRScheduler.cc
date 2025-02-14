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
