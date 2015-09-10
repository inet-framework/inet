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
    return start_var == 0 && end_var == 0;
}

bool Sack::contains(const Sack& other) const
{
    return seqLE(start_var, other.start_var) && seqLE(other.end_var, end_var);
}

void Sack::clear()
{
    start_var = end_var = 0;
}

void Sack::setSegment(unsigned int start_par, unsigned int end_par)
{
    setStart(start_par);
    setEnd(end_par);
}

std::string Sack::str() const
{
    std::stringstream out;

    out << "[" << start_var << ".." << end_var << ")";
    return out.str();
}

Register_Class(TCPSegment);

uint32_t TCPSegment::getSegLen()
{
    return payloadLength_var + (finBit_var ? 1 : 0) + (synBit_var ? 1 : 0);
}

void TCPSegment::truncateSegment(uint32 firstSeqNo, uint32 endSeqNo)
{
    ASSERT(payloadLength_var > 0);

    // must have common part:
#ifndef NDEBUG
    if (!(seqLess(sequenceNo_var, endSeqNo) && seqLess(firstSeqNo, sequenceNo_var + payloadLength_var))) {
        throw cRuntimeError(this, "truncateSegment(%u,%u) called on [%u, %u) segment\n",
                firstSeqNo, endSeqNo, sequenceNo_var, sequenceNo_var + payloadLength_var);
    }
#endif // ifndef NDEBUG

    unsigned int truncleft = 0;
    unsigned int truncright = 0;

    if (seqLess(sequenceNo_var, firstSeqNo)) {
        truncleft = firstSeqNo - sequenceNo_var;
    }

    if (seqGreater(sequenceNo_var + payloadLength_var, endSeqNo)) {
        truncright = sequenceNo_var + payloadLength_var - endSeqNo;
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
    for (PayloadList::const_iterator i = other.payloadList.begin(); i != other.payloadList.end(); ++i)
        addPayloadMessage(i->msg->dup(), i->endSequenceNo);
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
    ASSERT(payloadLength_var >= truncleft + truncright);

    if (0 != byteArray_var.getDataArraySize())
        byteArray_var.truncateData(truncleft, truncright);

    while (!payloadList.empty() && (payloadList.front().endSequenceNo - sequenceNo_var) <= truncleft) {
        cPacket *msg = payloadList.front().msg;
        payloadList.pop_front();
        dropAndDelete(msg);
    }

    sequenceNo_var += truncleft;
    payloadLength_var -= truncleft + truncright;

    // truncate payload data correctly
    while (!payloadList.empty() && (payloadList.back().endSequenceNo - sequenceNo_var) > payloadLength_var) {
        cPacket *msg = payloadList.back().msg;
        payloadList.pop_back();
        dropAndDelete(msg);
    }
}

void TCPSegment::parsimPack(cCommBuffer *b) PARSIMPACK_CONST
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

