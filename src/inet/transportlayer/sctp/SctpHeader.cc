//
// Copyright (C) 2008-2009 Irene Ruengeler
// Copyright (C) 2009-2012 Thomas Dreibholz
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/transportlayer/sctp/SctpHeader.h"

#include "inet/transportlayer/sctp/SctpAssociation.h"

namespace inet {
namespace sctp {

void SctpHeader::clean()
{
    for (SctpChunk *chunk : sctpChunkList) {
        drop(chunk);
        delete chunk;
    }
    sctpChunkList.clear();
}

void SctpHeader::setSctpChunksArraySize(size_t size)
{
    throw cException(this, "setSctpChunkArraySize() not supported, use appendSctpChunks()");
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

void SctpHeader::replaceSctpChunk(SctpChunk *chunk, uint32_t k)
{
    setSctpChunks(k, chunk);
}

void SctpHeader::appendSctpChunks(SctpChunk *chunk)
{
    handleChange();
    sctpChunkList.push_back(chunk);
    take(chunk);
    headerLength += ADD_PADDING(chunk->getByteLength());
    setChunkLength(B(headerLength));
}

void SctpHeader::insertSctpChunks(size_t k, SctpChunk *chunk)
{
    handleChange();
    sctpChunkList.insert(sctpChunkList.begin() + k, chunk);
    take(chunk);
    headerLength += ADD_PADDING(chunk->getByteLength());
    setChunkLength(B(headerLength));
}

// void SctpHeader::eraseSctpChunks(size_t k)
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

void SctpErrorChunk::setParametersArraySize(size_t size)
{
    throw cException(this, "setParametersArraySize() not supported, use addParameter()");
}

size_t SctpErrorChunk::getParametersArraySize() const
{
    return parameterList.size();
}

SctpParameter *SctpErrorChunk::getParameters(size_t k) const
{
    return parameterList.at(k);
}

void SctpErrorChunk::setParameters(size_t k, SctpParameter *parameters)
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

void SctpErrorChunk::clean()
{
    while (!parameterList.empty()) {
        cPacket *msg = parameterList.front();
        parameterList.erase(parameterList.begin());
        dropAndDelete(msg);
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

const SctpParameter *SctpStreamResetChunk::getParameters(size_t k) const
{
    return parameterList.at(k);
}

void SctpStreamResetChunk::setParameters(size_t k, SctpParameter *parameters)
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
    for (uint16_t i = 0; i < other.getStreamNumbersArraySize(); i++) {
        setStreamNumbers(i, other.getStreamNumbers(i));
    }
}

void SctpAsconfChunk::clean()
{
    for (SctpParameter *param : parameterList) {
        dropAndDelete(param);
    }
    parameterList.clear();
}

void SctpAsconfChunk::setAsconfParamsArraySize(size_t size)
{
    throw cException(this, "setAsconfParamsArraySize() not supported, use addAsconfParam()");
}

size_t SctpAsconfChunk::getAsconfParamsArraySize() const
{
    return parameterList.size();
}

const SctpParameter *SctpAsconfChunk::getAsconfParams(size_t k) const
{
    return parameterList.at(k);
}

void SctpAsconfChunk::setAsconfParams(size_t k, SctpParameter *asconfParams)
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

void SctpAsconfAckChunk::clean()
{
    for (SctpParameter *param : parameterList) {
        dropAndDelete(param);
    }
    parameterList.clear();
}

void SctpAsconfAckChunk::setAsconfResponseArraySize(size_t size)
{
    throw cException(this, "setAsconfResponseArraySize() not supported, use addAsconfResponse()");
}

size_t SctpAsconfAckChunk::getAsconfResponseArraySize() const
{
    return parameterList.size();
}

SctpParameter *SctpAsconfAckChunk::getAsconfResponse(size_t k) const
{
    return parameterList.at(k);
}

void SctpAsconfAckChunk::setAsconfResponse(size_t k, SctpParameter *asconfResponse)
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

