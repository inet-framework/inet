//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


#include "TCPSegment.h"

Register_Class(TCPSegment);


TCPSegment& TCPSegment::operator=(const TCPSegment& other)
{
    TCPSegment_Base::operator=(other);

    for (std::list<TCPPayloadMessage>::const_iterator i=other.payloadList.begin(); i!=other.payloadList.end(); ++i)
        addPayloadMessage((cMessage *)i->msg->dup(), i->endSequenceNo);

    return *this;
}

void TCPSegment::setPayloadArraySize(unsigned int size)
{
    throw new cException(this, "setPayloadArraySize() not supported, use addPayloadMessage()");
}

unsigned int TCPSegment::payloadArraySize() const
{
    return payloadList.size();
}

TCPPayloadMessage& TCPSegment::payload(unsigned int k)
{
    std::list<TCPPayloadMessage>::iterator i = payloadList.begin();
    while (k>0 && i!=payloadList.end())
        (++i, --k);
    return *i;
}

void TCPSegment::setPayload(unsigned int k, const TCPPayloadMessage& payload_var)
{
    throw new cException(this, "setPayload() not supported, use addPayloadMessage()");
}

void TCPSegment::addPayloadMessage(cMessage *msg, uint32 endSequenceNo)
{
    take(msg);

    TCPPayloadMessage payload;
    payload.endSequenceNo = endSequenceNo;
    payload.msg = msg;
    payloadList.push_back(payload);
}

cMessage *TCPSegment::removeFirstPayloadMessage(uint32& endSequenceNo)
{
    if (payloadList.empty())
        return NULL;

    cMessage *msg = payloadList.front().msg;
    endSequenceNo = payloadList.front().endSequenceNo;
    payloadList.pop_front();
    drop(msg);
    return msg;
}

