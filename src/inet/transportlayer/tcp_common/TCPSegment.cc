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

#include "inet/transportlayer/tcp_common/TCPSegment.h"

namespace inet {

namespace tcp {

Register_Class(Sack);

bool Sack::empty() const
{
    return start == 0 && end == 0;
}

bool Sack::contains(const Sack& other) const
{
    return seqLE(start, other.start) && seqLE(other.end, end);
}

void Sack::clear()
{
    start = end = 0;
}

void Sack::setSegment(unsigned int start_par, unsigned int end_par)
{
    setStart(start_par);
    setEnd(end_par);
}

std::string Sack::str() const
{
    std::stringstream out;

    out << "[" << start << ".." << end << ")";
    return out.str();
}

Register_Class(TCPSegment);

uint32_t TCPSegment::getSegLen()
{
    return payloadLength + (finBit ? 1 : 0) + (synBit ? 1 : 0);
}

void TCPSegment::truncateSegment(uint32 firstSeqNo, uint32 endSeqNo)
{
    ASSERT(payloadLength > 0);

    // must have common part:
#ifndef NDEBUG
    if (!(seqLess(sequenceNo, endSeqNo) && seqLess(firstSeqNo, sequenceNo + payloadLength))) {
        throw cRuntimeError(this, "truncateSegment(%u,%u) called on [%u, %u) segment\n",
                firstSeqNo, endSeqNo, sequenceNo, sequenceNo + payloadLength);
    }
#endif // ifndef NDEBUG

    unsigned int truncleft = 0;
    unsigned int truncright = 0;

    if (seqLess(sequenceNo, firstSeqNo)) {
        truncleft = firstSeqNo - sequenceNo;
    }

    if (seqGreater(sequenceNo + payloadLength, endSeqNo)) {
        truncright = sequenceNo + payloadLength - endSeqNo;
    }

    truncateData(truncleft, truncright);
}

unsigned short TCPSegment::getHeaderOptionArrayLength()
{
    unsigned short usedLength = 0;

    for (uint i = 0; i < getHeaderOptionArraySize(); i++)
        usedLength += getHeaderOption(i)->getLength();

    return usedLength;
}

TCPSegment& TCPSegment::operator=(const TCPSegment& other)
{
    if (this == &other)
        return *this;
    clean();
    TCPSegment_Base::operator=(other);
    copy(other);
    return *this;
}

void TCPSegment::copy(const TCPSegment& other)
{
    for (const auto & elem : other.payloadList)
        addPayloadMessage(elem.msg->dup(), elem.endSequenceNo);
    for (const auto opt: other.headerOptionList)
        addHeaderOption(opt->dup());
}

TCPSegment::~TCPSegment()
{
    clean();
}

void TCPSegment::clean()
{
    dropHeaderOptions();

    while (!payloadList.empty()) {
        cPacket *msg = payloadList.front().msg;
        payloadList.pop_front();
        dropAndDelete(msg);
    }
}

void TCPSegment::truncateData(unsigned int truncleft, unsigned int truncright)
{
    ASSERT(payloadLength >= truncleft + truncright);

    if (0 != byteArray.getDataArraySize())
        byteArray.truncateData(truncleft, truncright);

    while (!payloadList.empty() && (payloadList.front().endSequenceNo - sequenceNo) <= truncleft) {
        cPacket *msg = payloadList.front().msg;
        payloadList.pop_front();
        dropAndDelete(msg);
    }

    sequenceNo += truncleft;
    payloadLength -= truncleft + truncright;

    // truncate payload data correctly
    while (!payloadList.empty() && (payloadList.back().endSequenceNo - sequenceNo) > payloadLength) {
        cPacket *msg = payloadList.back().msg;
        payloadList.pop_back();
        dropAndDelete(msg);
    }
}

void TCPSegment::parsimPack(cCommBuffer *b) const
{
    TCPSegment_Base::parsimPack(b);
    b->pack((int)headerOptionList.size());
    for (const auto opt: headerOptionList) {
        b->packObject(opt);
    }
    b->pack((int)payloadList.size());
    for (PayloadList::const_iterator it = payloadList.begin(); it != payloadList.end(); it++) {
        b->pack(it->endSequenceNo);
        b->packObject(it->msg);
    }
}

void TCPSegment::parsimUnpack(cCommBuffer *b)
{
    TCPSegment_Base::parsimUnpack(b);
    int i, n;
    b->unpack(n);
    for (i = 0; i < n; i++) {
        TCPOption *opt = check_and_cast<TCPOption*>(b->unpackObject());
        headerOptionList.push_back(opt);
    }
    b->unpack(n);
    for (i = 0; i < n; i++) {
        TCPPayloadMessage payload;
        b->unpack(payload.endSequenceNo);
        payload.msg = check_and_cast<cPacket*>(b->unpackObject());
        payloadList.push_back(payload);
    }
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
    auto i = payloadList.begin();
    while (k > 0 && i != payloadList.end())
        (++i, --k);
    if (i == payloadList.end())
        throw cRuntimeError("Model error at getPayload(): index out of range");
    return *i;
}

void TCPSegment::setPayload(unsigned int k, const TCPPayloadMessage& payload_var)
{
    throw cRuntimeError(this, "setPayload() not supported, use addPayloadMessage()");
}

void TCPSegment::addPayloadMessage(cPacket *msg, uint32 endSequenceNo)
{
    take(msg);

    TCPPayloadMessage payload;
    payload.endSequenceNo = endSequenceNo;
    payload.msg = msg;
    payloadList.push_back(payload);
}

cPacket *TCPSegment::removeFirstPayloadMessage(uint32& endSequenceNo)
{
    if (payloadList.empty())
        return nullptr;

    cPacket *msg = payloadList.front().msg;
    endSequenceNo = payloadList.front().endSequenceNo;
    payloadList.pop_front();
    drop(msg);
    return msg;
}

void TCPSegment::addHeaderOption(TCPOption *option)
{
    headerOptionList.push_back(option);
}

void TCPSegment::setHeaderOptionArraySize(unsigned int size)
{
    throw cRuntimeError(this, "setHeaderOptionArraySize() not supported, use addHeaderOption()");
}

unsigned int TCPSegment::getHeaderOptionArraySize() const
{
    return headerOptionList.size();
}

TCPOptionPtr& TCPSegment::getHeaderOption(unsigned int k)
{
    return headerOptionList.at(k);
}

void TCPSegment::setHeaderOption(unsigned int k, const TCPOptionPtr& headerOption)
{
    throw cRuntimeError(this, "setHeaderOption() not supported, use addHeaderOption()");
}

void TCPSegment::dropHeaderOptions()
{
    for (auto opt : headerOptionList)
        delete opt;
    headerOptionList.clear();
}


} // namespace tcp

} // namespace inet

