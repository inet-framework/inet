//
// Copyright (C) 2008-2009 Irene Ruengeler
// Copyright (C) 2009-2012 Thomas Dreibholz
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

#include "inet/transportlayer/sctp/SctpAssociation.h"
#include "inet/transportlayer/sctp/SctpHeader.h"

namespace inet {
namespace sctp {

Register_Class(SctpHeader);

SctpHeader& SctpHeader::operator=(const SctpHeader& other)
{
    if (this == &other)
        return *this;
    clean();
    SctpHeader_Base::operator=(other);
    copy(other);
    return *this;
}


void SctpHeader::copy(const SctpHeader& other)
{
   // handleChange();
    setVTag(other.getVTag());
    setSrcPort(other.getSrcPort());
    setDestPort(other.getDestPort());
    setChecksumOk(other.getChecksumOk());
    for (const auto & elem : other.sctpChunkList) {
        SctpChunk *chunk = (elem)->dup();
        take(chunk);
        sctpChunkList.push_back(chunk);
    }
    ASSERT(B(getChunkLength()).get() == B(other.getChunkLength()).get());
}

SctpHeader::~SctpHeader()
{
    clean();
}

void SctpHeader::clean()
{
  // handleChange();

    if (this->getSctpChunksArraySize() > 0) {
        auto iterator = sctpChunkList.begin();
        while (iterator != sctpChunkList.end()) {
           // SctpChunk *chunk = (*iterator);
            sctpChunkList.erase(iterator);
           // delete chunk;
        }
    }

   /* sctpChunkList.clear();
    setHeaderLength(SCTP_COMMON_HEADER);
    setChunkLength(B(SCTP_COMMON_HEADER));*/
}

void SctpHeader::setSctpChunksArraySize(size_t size)
{
    throw cException(this, "setSctpChunkArraySize() not supported, use insertSctpChunks()");
}

void SctpHeader::setSctpChunks(size_t k, SctpChunk *chunk)
{
    handleChange();
    SctpChunk *tmp = sctpChunkList.at(k);
    if (tmp == chunk)
        return;
    headerLength -= ADD_PADDING(tmp->getByteLength());
    dropAndDelete(tmp);
    sctpChunkList[k] = chunk;
    take(chunk);
    headerLength += ADD_PADDING(chunk->getByteLength());
    setChunkLength(B(headerLength));
}

size_t SctpHeader::getSctpChunksArraySize() const
{
    return sctpChunkList.size();
}


void SctpHeader::replaceSctpChunk(SctpChunk *chunk, uint32 k)
{
    setSctpChunks(k, chunk);
}

void SctpHeader::insertSctpChunks(SctpChunk * chunk)
{
    handleChange();
    sctpChunkList.push_back(chunk);
    take(chunk);
    headerLength += ADD_PADDING(chunk->getByteLength());
    setChunkLength(B(headerLength));
}

void SctpHeader::insertSctpChunks(size_t k, SctpChunk * chunk)
{
    handleChange();
    sctpChunkList.insert(sctpChunkList.begin()+k, chunk);
    take(chunk);
    headerLength += ADD_PADDING(chunk->getByteLength());
    setChunkLength(B(headerLength));
}

//void SctpHeader::eraseSctpChunks(size_t k)
SctpChunk *SctpHeader::removeFirstChunk()
{
    handleChange();
    if (sctpChunkList.empty())
        return nullptr;

    SctpChunk *msg = sctpChunkList.front();
    headerLength -= ADD_PADDING(msg->getByteLength());
    sctpChunkList.erase(sctpChunkList.begin());
    drop(msg);
    setChunkLength(B(headerLength));
    return msg;
}

SctpChunk *SctpHeader::removeLastChunk()
{
    handleChange();
    if (sctpChunkList.empty())
        return nullptr;

    SctpChunk *msg = sctpChunkList.back();
    sctpChunkList.pop_back();
    drop(msg);
    this->addChunkLength(B(ADD_PADDING(msg->getByteLength())));
    return msg;
}

SctpChunk *SctpHeader::peekFirstChunk() const
{
    if (sctpChunkList.empty())
        return nullptr;

    SctpChunk *msg = sctpChunkList.front();
    return msg;
}

SctpChunk *SctpHeader::peekLastChunk() const
{
    if (sctpChunkList.empty())
        return nullptr;

    SctpChunk *msg = sctpChunkList.back();
    return msg;
}

Register_Class(SctpErrorChunk);

SctpErrorChunk& SctpErrorChunk::operator=(const SctpErrorChunk& other)
{
    if (this == &other)
        return *this;
    clean();
    SctpErrorChunk_Base::operator=(other);
    copy(other);
    return *this;
}

void SctpErrorChunk::copy(const SctpErrorChunk& other)
{
    for (const auto & elem : other.parameterList) {
        SctpParameter *param = (elem)->dup();
        take(param);
        parameterList.push_back(param);
    }
}

void SctpErrorChunk::setParametersArraySize(size_t size)
{
    throw cException(this, "setParametersArraySize() not supported, use addParameter()");
}

size_t SctpErrorChunk::getParametersArraySize() const
{
    return parameterList.size();
}

SctpParameter * SctpErrorChunk::getParameters(size_t k) const
{
    return parameterList.at(k);
}

void SctpErrorChunk::setParameters(size_t k, SctpParameter * parameters)
{
    throw cException(this, "setParameter() not supported, use addParameter()");
}

void SctpErrorChunk::addParameters(SctpParameter *msg)
{
    take(msg);

    this->setByteLength(this->getByteLength() + ADD_PADDING(msg->getByteLength()));
    parameterList.push_back(msg);
}

SctpParameter *SctpErrorChunk::removeParameter()
{
    if (parameterList.empty())
        return nullptr;

    SctpParameter *msg = parameterList.front();
    parameterList.erase(parameterList.begin());
    drop(msg);
    this->setByteLength(this->getByteLength() + ADD_PADDING(msg->getByteLength()));
    return msg;
}

SctpErrorChunk::~SctpErrorChunk()
{
    clean();
}

void SctpErrorChunk::clean()
{
    while (!parameterList.empty()) {
        cPacket *msg = parameterList.front();
        parameterList.erase(parameterList.begin());
        dropAndDelete(msg);
    }
}

Register_Class(SctpStreamResetChunk);

SctpStreamResetChunk& SctpStreamResetChunk::operator=(const SctpStreamResetChunk& other)
{
    SctpStreamResetChunk_Base::operator=(other);

    this->setByteLength(SCTP_STREAM_RESET_CHUNK_LENGTH);
    for (const auto & elem : other.parameterList)
        addParameter((elem)->dup());

    return *this;
}

void SctpStreamResetChunk::copy(const SctpStreamResetChunk& other)
{
    for (const auto & elem : other.parameterList) {
        SctpParameter *param = (elem)->dup();
        take(param);
        parameterList.push_back(param);
    }
}

void SctpStreamResetChunk::setParametersArraySize(size_t size)
{
    throw cException(this, "setParametersArraySize() not supported, use addParameter()");
}

size_t SctpStreamResetChunk::getParametersArraySize() const
{
    return parameterList.size();
}

const SctpParameter * SctpStreamResetChunk::getParameters(size_t k) const
{
    return parameterList.at(k);
}

void SctpStreamResetChunk::setParameters(size_t k, SctpParameter * parameters)
{
    throw cException(this, "setParameters() not supported, use addParameter()");
}

void SctpStreamResetChunk::addParameter(SctpParameter *msg)
{
    take(msg);
    if (this->parameterList.size() < 2) {
        this->setByteLength(this->getByteLength() + ADD_PADDING(msg->getByteLength()));
        parameterList.push_back(msg);
    }
    else
        throw cRuntimeError("Not more than two parameters allowed!");
}

cPacket *SctpStreamResetChunk::removeParameter()
{
    if (parameterList.empty())
        return nullptr;

    cPacket *msg = parameterList.front();
    parameterList.erase(parameterList.begin());
    drop(msg);
    this->setByteLength(this->getByteLength() + ADD_PADDING(msg->getByteLength()));
    return msg;
}

SctpStreamResetChunk::~SctpStreamResetChunk()
{
    clean();
}

void SctpStreamResetChunk::clean()
{
    while (!parameterList.empty()) {
        cPacket *msg = parameterList.front();
        parameterList.erase(parameterList.begin());
        dropAndDelete(msg);
    }
}

Register_Class(SctpIncomingSsnResetRequestParameter);

void SctpIncomingSsnResetRequestParameter::copy(const SctpIncomingSsnResetRequestParameter& other)
{
    setSrReqSn(other.getSrReqSn());
    setStreamNumbersArraySize(other.getStreamNumbersArraySize());
    for (uint16 i = 0; i < other.getStreamNumbersArraySize(); i++) {
        setStreamNumbers(i, other.getStreamNumbers(i));
    }
}

Register_Class(SctpAsconfChunk);

SctpAsconfChunk& SctpAsconfChunk::operator=(const SctpAsconfChunk& other)
{
    SctpAsconfChunk_Base::operator=(other);

    this->setByteLength(SCTP_ADD_IP_CHUNK_LENGTH + 8);
    this->setAddressParam(other.getAddressParam());
    for (const auto & elem : other.parameterList)
        addAsconfParam((elem)->dup());

    return *this;
}

void SctpAsconfChunk::setAsconfParamsArraySize(size_t size)
{
    throw cException(this, "setAsconfParamsArraySize() not supported, use addAsconfParam()");
}

size_t SctpAsconfChunk::getAsconfParamsArraySize() const
{
    return parameterList.size();
}

const SctpParameter * SctpAsconfChunk::getAsconfParams(size_t k) const
{
    return parameterList.at(k);
}

void SctpAsconfChunk::setAsconfParams(size_t k, SctpParameter * asconfParams)
{
    throw cException(this, "setAsconfParams() not supported, use addAsconfParam()");
}

void SctpAsconfChunk::addAsconfParam(SctpParameter *msg)
{
    take(msg);
    this->setByteLength(this->getByteLength() + ADD_PADDING(msg->getByteLength()));
    parameterList.push_back(msg);
}

SctpParameter *SctpAsconfChunk::removeAsconfParam()
{
    if (parameterList.empty())
        return nullptr;

    SctpParameter *msg = parameterList.front();
    parameterList.erase(parameterList.begin());
    drop(msg);
    this->setByteLength(this->getByteLength() + ADD_PADDING(msg->getByteLength()));
    return msg;
}

Register_Class(SctpAsconfAckChunk);

SctpAsconfAckChunk& SctpAsconfAckChunk::operator=(const SctpAsconfAckChunk& other)
{
    SctpAsconfAckChunk_Base::operator=(other);

    this->setByteLength(SCTP_ADD_IP_CHUNK_LENGTH);
    for (const auto & elem : other.parameterList)
        addAsconfResponse((elem)->dup());

    return *this;
}

void SctpAsconfAckChunk::setAsconfResponseArraySize(size_t size)
{
    throw cException(this, "setAsconfResponseArraySize() not supported, use addAsconfResponse()");
}

size_t SctpAsconfAckChunk::getAsconfResponseArraySize() const
{
    return parameterList.size();
}

SctpParameter * SctpAsconfAckChunk::getAsconfResponse(size_t k) const
{
    return parameterList.at(k);
}

void SctpAsconfAckChunk::setAsconfResponse(size_t k, SctpParameter * asconfResponse)
{
    throw cException(this, "setAsconfresponse() not supported, use addAsconfResponse()");
}

void SctpAsconfAckChunk::addAsconfResponse(SctpParameter *msg)
{
    take(msg);
    this->setByteLength(this->getByteLength() + ADD_PADDING(msg->getByteLength()));
    parameterList.push_back(msg);
}

SctpParameter *SctpAsconfAckChunk::removeAsconfResponse()
{
    if (parameterList.empty())
        return nullptr;

    SctpParameter *msg = parameterList.front();
    parameterList.erase(parameterList.begin());
    drop(msg);
    this->setByteLength(this->getByteLength() + ADD_PADDING(msg->getByteLength()));
    return msg;
}

} // namespace sctp
} // namespace inet

