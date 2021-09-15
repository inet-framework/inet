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

#include "inet/transportlayer/sctp/SCTPMessage.h"
#include "inet/transportlayer/sctp/SCTPAssociation.h"

namespace inet {

namespace sctp {

Register_Class(SCTPMessage);

SCTPMessage& SCTPMessage::operator=(const SCTPMessage& other)
{
    if (this == &other)
        return *this;
    SCTPMessage_Base::operator=(other);
    return *this;
}

void SCTPMessage::setChunksArraySize(size_t size)
{
    throw cRuntimeError("setChunkArraySize() not supported, use addChunk()");
}

void SCTPMessage::replaceChunk(SCTPChunk *msg, size_t k)
{
    auto oldMsg = dropChunks(k);
    this->setByteLength(this->getByteLength() - ADD_PADDING(oldMsg->getByteLength()));
    delete oldMsg;
    this->setByteLength(this->getByteLength() + ADD_PADDING(msg->getByteLength()));
    SCTPMessage_Base::setChunks(k, msg);
}

void SCTPMessage::setChunks(size_t k, SCTPChunk *chunks_var)
{
    throw cRuntimeError("setChunks() not supported, use addChunk()");
}

void SCTPMessage::addChunk(SCTPChunk *msg)
{
    ASSERT(msg != nullptr);

    char str[256];
    if (chunks_arraysize < 9) {
        snprintf(str, sizeof(str), "%s %s", this->getName(), msg->getName());
        this->setName(str);
    }
    this->setByteLength(this->getByteLength() + ADD_PADDING(msg->getByteLength()));
    SCTPMessage_Base::insertChunks(msg);
}

SCTPChunk *SCTPMessage::removeChunk()
{
    if (chunks_arraysize == 0)
        return nullptr;
    SCTPChunk *msg = dropChunks(0);
    eraseChunks(0);
    this->setByteLength(this->getByteLength() - ADD_PADDING(msg->getByteLength()));
    return msg;
}

SCTPChunk *SCTPMessage::removeLastChunk()
{
    if (chunks_arraysize == 0)
        return nullptr;
    SCTPChunk *msg = dropChunks(chunks_arraysize - 1);
    eraseChunks(chunks_arraysize - 1);
    this->setByteLength(this->getByteLength() - ADD_PADDING(msg->getByteLength()));
    return msg;
}

SCTPChunk *SCTPMessage::peekFirstChunk()
{
    if (chunks_arraysize == 0)
        return nullptr;
    return getChunksForUpdate(0);
}

SCTPChunk *SCTPMessage::peekLastChunk()
{
    if (chunks_arraysize == 0)
        return nullptr;
    return getChunksForUpdate(chunks_arraysize - 1);
}


Register_Class(SCTPErrorChunk);

SCTPErrorChunk& SCTPErrorChunk::operator=(const SCTPErrorChunk& other)
{
    if (this == &other)
        return *this;
    SCTPErrorChunk_Base::operator=(other);
    return *this;
}

void SCTPErrorChunk::setParametersArraySize(size_t size)
{
    throw new cException(this, "setParametersArraySize() not supported, use addParameter()");
}

void SCTPErrorChunk::setParameters(size_t k, SCTPParameter *chunks_var)
{
    throw new cException(this, "setParameter() not supported, use addParameter()");
}

void SCTPErrorChunk::addParameters(SCTPParameter *msg)
{
    insertParameters(msg);
    this->setByteLength(this->getByteLength() + ADD_PADDING(msg->getByteLength()));
}

SCTPParameter *SCTPErrorChunk::removeParameter()
{
    if (parameters_arraysize == 0)
        return nullptr;

    SCTPParameter *msg = dropParameters(0);
    eraseParameters(0);
    this->setByteLength(this->getByteLength() - ADD_PADDING(msg->getByteLength()));
    return msg;
}

SCTPErrorChunk::~SCTPErrorChunk()
{
}

Register_Class(SCTPStreamResetChunk);

SCTPStreamResetChunk& SCTPStreamResetChunk::operator=(const SCTPStreamResetChunk& other)
{
    SCTPStreamResetChunk_Base::operator=(other);
    return *this;
}

void SCTPStreamResetChunk::setParametersArraySize(size_t size)
{
    throw cRuntimeError("setParametersArraySize() not supported, use addParameter()");
}

void SCTPStreamResetChunk::setParameters(size_t k, SCTPParameter *chunks_var)
{
    throw cRuntimeError("setParameters() not supported, use addParameter()");
}

void SCTPStreamResetChunk::addParameter(SCTPParameter *msg)
{
    if (parameters_arraysize < 2) {
        this->setByteLength(this->getByteLength() + ADD_PADDING(msg->getByteLength()));
        insertParameters(msg);
    }
    else
        throw cRuntimeError("Not more than two parameters allowed!");
}

SCTPParameter *SCTPStreamResetChunk::removeParameter()
{
    if (parameters_arraysize == 0)
        return nullptr;

    SCTPParameter *msg = dropParameters(0);
    eraseParameters(0);
    this->setByteLength(this->getByteLength() - ADD_PADDING(msg->getByteLength()));
    return msg;
}

SCTPStreamResetChunk::~SCTPStreamResetChunk()
{
}

Register_Class(SCTPIncomingSSNResetRequestParameter);

void SCTPIncomingSSNResetRequestParameter::copy(const SCTPIncomingSSNResetRequestParameter& other)
{
    setSrReqSn(other.getSrReqSn());
    setStreamNumbersArraySize(other.getStreamNumbersArraySize());
    for (uint16 i = 0; i < other.getStreamNumbersArraySize(); i++) {
        setStreamNumbers(i, other.getStreamNumbers(i));
    }
}

Register_Class(SCTPAsconfChunk);

SCTPAsconfChunk& SCTPAsconfChunk::operator=(const SCTPAsconfChunk& other)
{
    SCTPAsconfChunk_Base::operator=(other);
    return *this;
}

void SCTPAsconfChunk::setAsconfParamsArraySize(const size_t size)
{
    throw cRuntimeError("setAsconfParamsArraySize() not supported, use addAsconfParam()");
}

void SCTPAsconfChunk::setAsconfParams(size_t k, SCTPParameter *chunks_var)
{
    throw cRuntimeError("setAsconfParams() not supported, use addAsconfParam()");
}

void SCTPAsconfChunk::addAsconfParam(SCTPParameter *msg)
{
    insertAsconfParams(msg);
    this->setByteLength(this->getByteLength() + ADD_PADDING(msg->getByteLength()));
}

SCTPParameter *SCTPAsconfChunk::removeAsconfParam()
{
    if (asconfParams_arraysize == 0)
        return nullptr;

    SCTPParameter *msg = dropAsconfParams(0);
    eraseAsconfParams(0);
    this->setByteLength(this->getByteLength() - ADD_PADDING(msg->getByteLength()));
    return msg;
}

Register_Class(SCTPAsconfAckChunk);

SCTPAsconfAckChunk& SCTPAsconfAckChunk::operator=(const SCTPAsconfAckChunk& other)
{
    SCTPAsconfAckChunk_Base::operator=(other);
    return *this;
}

void SCTPAsconfAckChunk::setAsconfResponseArraySize(const size_t size)
{
    throw cRuntimeError("setAsconfResponseArraySize() not supported, use addAsconfResponse()");
}

void SCTPAsconfAckChunk::setAsconfResponse(const size_t k, SCTPParameter *chunks_var)
{
    throw cRuntimeError("setAsconfresponse() not supported, use addAsconfResponse()");
}

void SCTPAsconfAckChunk::addAsconfResponse(SCTPParameter *msg)
{
    insertAsconfResponse(msg);
    this->setByteLength(this->getByteLength() + ADD_PADDING(msg->getByteLength()));
}

SCTPParameter *SCTPAsconfAckChunk::removeAsconfResponse()
{
    if (asconfResponse_arraysize == 0)
        return nullptr;

    SCTPParameter *msg = dropAsconfResponse(0);
    eraseAsconfResponse(0);
    this->setByteLength(this->getByteLength() - ADD_PADDING(msg->getByteLength()));
    return msg;
}

} // namespace sctp

} // namespace inet

