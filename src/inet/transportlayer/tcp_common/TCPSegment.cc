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

Register_Class(TcpHeader);

uint32_t TcpHeader::getSegLen()
{
    return payloadLength + (finBit ? 1 : 0) + (synBit ? 1 : 0);
}

void TcpHeader::truncateSegment(uint32 firstSeqNo, uint32 endSeqNo)
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

unsigned short TcpHeader::getHeaderOptionArrayLength()
{
    unsigned short usedLength = 0;

    for (uint i = 0; i < getHeaderOptionArraySize(); i++)
        usedLength += getHeaderOption(i)->getLength();

    return usedLength;
}

TcpHeader& TcpHeader::operator=(const TcpHeader& other)
{
    if (this == &other)
        return *this;
    clean();
    TcpHeader_Base::operator=(other);
    copy(other);
    return *this;
}

void TcpHeader::copy(const TcpHeader& other)
{
    for (const auto & elem : other.payloadList)
        addPayloadMessage(elem.msg->dup(), elem.endSequenceNo);
    for (const auto opt: other.headerOptionList)
        headerOptionList.push_back(opt->dup());
}

TcpHeader::~TcpHeader()
{
    clean();
}

void TcpHeader::clean()
{
    dropHeaderOptions();
    setHeaderLength(TCP_HEADER_OCTETS);

    while (!payloadList.empty()) {
        cPacket *msg = payloadList.front().msg;
        payloadList.pop_front();
        dropAndDelete(msg);
    }
    payloadLength = 0;

    setChunkByteLength(TCP_HEADER_OCTETS);
}

void TcpHeader::truncateData(unsigned int truncleft, unsigned int truncright)
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
    addChunkByteLength(-(truncleft + truncright));

    // truncate payload data correctly
    while (!payloadList.empty() && (payloadList.back().endSequenceNo - sequenceNo) > payloadLength) {
        cPacket *msg = payloadList.back().msg;
        payloadList.pop_back();
        dropAndDelete(msg);
    }
}

void TcpHeader::parsimPack(cCommBuffer *b) const
{
    TcpHeader_Base::parsimPack(b);
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

void TcpHeader::parsimUnpack(cCommBuffer *b)
{
    TcpHeader_Base::parsimUnpack(b);
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

void TcpHeader::setPayloadArraySize(unsigned int size)
{
    throw cRuntimeError(this, "setPayloadArraySize() not supported, use addPayloadMessage()");
}

unsigned int TcpHeader::getPayloadArraySize() const
{
    return payloadList.size();
}

TCPPayloadMessage& TcpHeader::getPayload(unsigned int k)
{
    auto i = payloadList.begin();
    while (k > 0 && i != payloadList.end())
        (++i, --k);
    if (i == payloadList.end())
        throw cRuntimeError("Model error at getPayload(): index out of range");
    return *i;
}

void TcpHeader::setPayload(unsigned int k, const TCPPayloadMessage& payload_var)
{
    throw cRuntimeError(this, "setPayload() not supported, use addPayloadMessage()");
}

void TcpHeader::addPayloadMessage(cPacket *msg, uint32 endSequenceNo)
{
    take(msg);

    TCPPayloadMessage payload;
    payload.endSequenceNo = endSequenceNo;
    payload.msg = msg;
    payloadList.push_back(payload);
}

cPacket *TcpHeader::removeFirstPayloadMessage(uint32& endSequenceNo)
{
    if (payloadList.empty())
        return nullptr;

    cPacket *msg = payloadList.front().msg;
    endSequenceNo = payloadList.front().endSequenceNo;
    payloadList.pop_front();
    drop(msg);
    return msg;
}

void TcpHeader::addHeaderOption(TCPOption *option)
{
    headerOptionList.push_back(option);
    headerLength += option->getLength();
    addChunkByteLength(option->getLength());
}

void TcpHeader::setHeaderOptionArraySize(unsigned int size)
{
    throw cRuntimeError(this, "setHeaderOptionArraySize() not supported, use addHeaderOption()");
}

unsigned int TcpHeader::getHeaderOptionArraySize() const
{
    return headerOptionList.size();
}

TCPOptionPtr& TcpHeader::getHeaderOption(unsigned int k)
{
    return headerOptionList.at(k);
}

void TcpHeader::setHeaderOption(unsigned int k, const TCPOptionPtr& headerOption)
{
    throw cRuntimeError(this, "setHeaderOption() not supported, use addHeaderOption()");
}

void TcpHeader::dropHeaderOptions()
{
    for (auto opt : headerOptionList)
        delete opt;
    headerOptionList.clear();
    setHeaderLength(TCP_HEADER_OCTETS);
    setChunkByteLength(TCP_HEADER_OCTETS + payloadLength);
}


} // namespace tcp

} // namespace inet

