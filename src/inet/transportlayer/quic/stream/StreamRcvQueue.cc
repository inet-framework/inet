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

#include "StreamRcvQueue.h"

namespace inet {
namespace quic {

StreamRcvQueue::StreamRcvQueue(Stream *stream, Statistics *stats) {
    this->stream = stream;
    reorderBuffer.clear();
    reorderBuffer.setExpectedOffset(B(startSeq));

    this->stats = stats;
    this->streamRcvFrameStartOffsetStat = stats->createStatisticEntry("streamRcvFrameStartOffset");
    this->streamRcvFrameEndOffsetStat = stats->createStatisticEntry("streamRcvFrameEndOffset");
}

StreamRcvQueue::~StreamRcvQueue() {
}

void StreamRcvQueue::push(Ptr<const Chunk> data, uint64_t offset){

    numStreamFramesReceived++;

    uint64_t dataStartOffset = offset;
    uint64_t dataEndOffset = offset + offsetToSeq(data->getChunkLength());

    stats->getMod()->emit(streamRcvFrameStartOffsetStat, dataStartOffset);
    stats->getMod()->emit(streamRcvFrameEndOffsetStat, dataEndOffset);


    CHUNK_CHECK_USAGE(data != nullptr, "data is nullptr");
    CHUNK_CHECK_USAGE(data->getChunkLength() > b(0), "data is empty");

    bool isPushChunkBeforeRegion = false;
    bool isMergeRegionStartOffset = false;
    bool isMergeRegionEndOffset = false;

    // Insert data in the right order
    if(!reorderBuffer.isEmpty()) {

        uint64_t regionStartOffset = 0;
        uint64_t regionEndOffset = 0;

        uint64_t lastRegionEndOffset = offsetToSeq(reorderBuffer.getRegionEndOffset(reorderBuffer.getNumRegions()-1));

        if(dataStartOffset > lastRegionEndOffset) {
            EV_DEBUG << "reorderBuffer is not empty! dataStartOffset= " << dataStartOffset << endl;
            reorderBuffer.replace(B(offset), data);
        } else {

           for (int i = 0; i < reorderBuffer.getNumRegions(); i++){

               regionStartOffset = offsetToSeq(reorderBuffer.getRegionStartOffset(i));
               regionEndOffset = offsetToSeq(reorderBuffer.getRegionEndOffset(i));

               //count number of duplicated frames
               if(dataStartOffset >= regionStartOffset && dataEndOffset <= regionEndOffset){
                   numDuplFrames++;
                   break;
               }

               if (dataEndOffset < regionStartOffset) isPushChunkBeforeRegion = true;
               if (dataEndOffset == regionStartOffset) isMergeRegionStartOffset = true;
               if (dataStartOffset == regionEndOffset) isMergeRegionEndOffset = true;

               if(isPushChunkBeforeRegion || isMergeRegionStartOffset || isMergeRegionEndOffset) {
                   reorderBuffer.replace(B(offset), data);
                   break;
               }
           }
       }
   } else {
       if(dataEndOffset <= offsetToSeq(reorderBuffer.getExpectedOffset())) { //when queue is empty and first retransmitted stream frame received
           numDuplFrames++;
       } else {
           reorderBuffer.replace(B(offset), data);
       }
   }
}

bool StreamRcvQueue::hasDataForApp(){
    if (!reorderBuffer.isEmpty()){
        return (offsetToSeq(reorderBuffer.getAvailableDataLength()) > 0);
    }
    return false;
}

const Ptr<const Chunk> StreamRcvQueue::pop(B dataSize) {

    if (reorderBuffer.isEmpty()) return nullptr;

    B length;
    if(dataSize == B(0)){ //0 = infinity
        length = reorderBuffer.getAvailableDataLength();
    }else{
        length = std::min(B(reorderBuffer.getAvailableDataLength()), dataSize);
    }

    Ptr<const Chunk> chunk = reorderBuffer.popAvailableData(length);
    return chunk;
}


} /* namespace quic */
} /* namespace inet */
