//
// Copyright (C) 2004 Andras Varga
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


#include "TCPSegment.h"

Register_Class(TCPSegment);


TCPSegment& TCPSegment::operator=(const TCPSegment& other)
{
    TCPSegment_Base::operator=(other);

    for (PayloadList::const_iterator i=other.payloadList.begin(); i!=other.payloadList.end(); ++i)
        addPayloadMessage(i->msg->dup(), i->streamOffs, i->segmentOffs);

    return *this;
}

TCPSegment::~TCPSegment()
{
    while (!payloadList.empty())
    {
        cPacket *msg = payloadList.front().msg;
        payloadList.pop_front();
        dropAndDelete(msg);
    }
}

void TCPSegment::truncateSegment(uint32 firstSeqNo, uint32 endSeqNo)
{
    if(seqLess(sequenceNo_var, firstSeqNo))
    {
        unsigned int truncleft = firstSeqNo - sequenceNo_var;
        payloadLength_var -= truncleft;
        sequenceNo_var = firstSeqNo;

        // truncate payload data correctly
        while (!payloadList.empty() && payloadList.front().segmentOffs < truncleft)
        {
            cPacket *msg = payloadList.front().msg;
            payloadList.pop_front();
            dropAndDelete(msg);
        }
        for (PayloadList::iterator i = payloadList.begin(); i != payloadList.end(); ++i)
            i->segmentOffs -= truncleft;
    }

    if(seqGreater(sequenceNo_var+payloadLength_var, endSeqNo))
    {
        unsigned int truncright = sequenceNo_var + payloadLength_var - endSeqNo;
        payloadLength_var -= truncright;

        // truncate payload data correctly
        while (!payloadList.empty() && payloadList.back().segmentOffs >= payloadLength_var)
        {
            cPacket *msg = payloadList.back().msg;
            payloadList.pop_back();
            dropAndDelete(msg);
        }
    }
}

void TCPSegment::parsimPack(cCommBuffer *b)
{
    TCPSegment_Base::parsimPack(b);
    doPacking(b, payloadList);
}

void TCPSegment::parsimUnpack(cCommBuffer *b)
{
    TCPSegment_Base::parsimUnpack(b);
    doUnpacking(b, payloadList);
}

void TCPSegment::setPayloadArraySize(unsigned int size)
{
    throw cRuntimeError(this, "setPayloadArraySize() not supported, use addPayloadMessage()");
}

unsigned int TCPSegment::getPayloadArraySize() const
{
    return payloadList.size();
}

TCPPayloadMessage& TCPSegment::getPayload(unsigned int k)
{
    PayloadList::iterator i = payloadList.begin();
    while (k>0 && i!=payloadList.end())
        (++i, --k);
    return *i;
}

void TCPSegment::setPayload(unsigned int k, const TCPPayloadMessage& payload_var)
{
    throw cRuntimeError(this, "setPayload() not supported, use addPayloadMessage()");
}

void TCPSegment::addPayloadMessage(cPacket *msg, uint64 streamOffs, uint64 segmentOffs)
{
    take(msg);

    TCPPayloadMessage payload;
    payload.streamOffs = streamOffs;
    payload.segmentOffs = segmentOffs;
    payload.msg = msg;
    payloadList.push_back(payload);
}

cPacket *TCPSegment::removeFirstPayloadMessage(uint64& streamOffs, uint64& segmentOffs)
{
    if (payloadList.empty())
        return NULL;

    cPacket *msg = payloadList.front().msg;
    streamOffs = payloadList.front().streamOffs;
    segmentOffs = payloadList.front().segmentOffs;
    payloadList.pop_front();
    drop(msg);
    return msg;
}
