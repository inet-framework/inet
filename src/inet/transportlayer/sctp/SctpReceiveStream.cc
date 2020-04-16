//
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2010-2012 Thomas Dreibholz
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/transportlayer/contract/sctp/SctpCommand_m.h"
#include "inet/transportlayer/sctp/SctpAssociation.h"
#include "inet/transportlayer/sctp/SctpReceiveStream.h"

namespace inet {
namespace sctp {

SctpReceiveStream::SctpReceiveStream(SctpAssociation *assoc_)
{
    streamId = 0;
    expectedStreamSeqNum = 0;
    deliveryQ = new SctpQueue();
    orderedQ = new SctpQueue();
    unorderedQ = new SctpQueue();
    assoc = assoc_;
}

SctpReceiveStream::~SctpReceiveStream()
{
    delete deliveryQ;
    delete orderedQ;
    delete unorderedQ;
}

uint32 SctpReceiveStream::reassemble(SctpQueue *queue, uint32 tsn)
{
    uint32 begintsn = tsn, endtsn = 0;

    EV_INFO << "Trying to reassemble message..." << endl;

    /* test if we have all fragments down to the first */
    while (queue->getChunk(begintsn) && !(queue->getChunk(begintsn))->bbit)
        begintsn--;

    if (queue->getChunk(begintsn)) {
        endtsn = begintsn;

        /* test if we have all fragments up to the end */
        while (queue->getChunk(endtsn) && !(queue->getChunk(endtsn))->ebit)
            endtsn++;

        if (queue->getChunk(endtsn)) {
            EV_INFO << "All fragments found, now reassembling..." << endl;

            SctpDataVariables *firstVar = queue->getChunk(begintsn), *processVar;
            SctpSimpleMessage *firstSimple = check_and_cast<SctpSimpleMessage *>(firstVar->userData);

            EV_INFO << "First fragment has " << firstVar->len / 8 << " bytes." << endl;

            while (++begintsn <= endtsn) {
                processVar = queue->getAndExtractChunk(begintsn);
                SctpSimpleMessage *processSimple = check_and_cast<SctpSimpleMessage *>(processVar->userData);

                EV_INFO << "Adding fragment with " << processVar->len / 8 << " bytes." << endl;

                if ((firstSimple->getDataArraySize() > 0) && (processSimple->getDataArraySize() > 0)) {
                    firstSimple->setDataArraySize(firstSimple->getDataArraySize() + processSimple->getDataArraySize());
                    firstSimple->setDataLen(firstSimple->getDataLen() + processSimple->getDataLen());
                    firstSimple->setByteLength(firstSimple->getByteLength() + processSimple->getByteLength());
                    /* copy data */
                    for (uint32 i = 0; i < (processVar->len / 8); i++)
                        firstSimple->setData(i + (firstVar->len / 8), processSimple->getData(i));
                }

                firstVar->len += processVar->len;

                delete processVar->userData;
                delete processVar;
            }

            firstVar->ebit = 1;

            EV_INFO << "Reassembly done. Length=" << firstVar->len << "\n";
            return firstVar->tsn;
        }
    }
    return tsn;
}

uint32 SctpReceiveStream::enqueueNewDataChunk(SctpDataVariables *dchunk)
{
    uint32 delivery = 0;    //0:orderedQ=false && deliveryQ=false; 1:orderedQ=true && deliveryQ=false; 2:oderedQ=true && deliveryQ=true; 3:fragment

    SctpDataVariables *chunk;
    /* Enqueueing NEW data chunk. Append it to the respective queue */

    // ====== Unordered delivery =============================================
    if (!dchunk->ordered) {
        if (dchunk->bbit && dchunk->ebit) {
            /* put message into deliveryQ */
            if (deliveryQ->checkAndInsertChunk(dchunk->tsn, dchunk)) {
                delivery = 2;
            }
        } else {
            if (unorderedQ->checkAndInsertChunk(dchunk->tsn, dchunk)) {
                delivery = 3;
            }

            /* try to reassemble here */
            uint32 reassembled = reassemble(unorderedQ, dchunk->tsn);

            if ((unorderedQ->getChunk(reassembled))->bbit && (unorderedQ->getChunk(reassembled))->bbit) {  //FIXME There are identical sub-expressions '(unorderedQ->getChunk(reassembled))->bbit' to the left and to the right of the '&&' operator.
                /* put message into deliveryQ */
                if (deliveryQ->checkAndInsertChunk(reassembled, unorderedQ->getAndExtractChunk(reassembled))) {
                    delivery = 2;
                }
            }
        }
    }
    // ====== Ordered delivery ===============================================
    else if (dchunk->ordered) {
        /* put message into orderedQ */
        if (orderedQ->checkAndInsertChunk(dchunk->tsn, dchunk))
            delivery = 1;

        if (!dchunk->bbit || !dchunk->ebit) {
            delivery = 3;
            /* try to reassemble */
            reassemble(orderedQ, dchunk->tsn);
        }

        if (orderedQ->getQueueSize() > 0) {
            /* dequeue first from orderedQ */
            chunk = orderedQ->dequeueChunkBySSN(expectedStreamSeqNum);
            if (chunk) {
                if (deliveryQ->checkAndInsertChunk(chunk->tsn, chunk)) {
                    ++expectedStreamSeqNum;
                    if (expectedStreamSeqNum > 65535)
                        expectedStreamSeqNum = 0;
                    delivery = 2;
                }
            }
        }
    }

    return delivery;
}

int32 SctpReceiveStream::getExpectedStreamSeqNum() {
    return expectedStreamSeqNum;
}

void SctpReceiveStream::setExpectedStreamSeqNum(int32 seqNum) {
    expectedStreamSeqNum = seqNum;
}


} // namespace sctp

} // namespace inet

