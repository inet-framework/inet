//
// Copyright (C) 2010 Zoltan Bojthe
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


#include "TCPCommand.h"


Register_Class(TCPDataMsg);


TCPDataMsg::TCPDataMsg(const char *name) : ByteArrayMessage(name)
{
    this->payloadPacket_var = NULL;
    this->isPayloadStart_var = false;
}

TCPDataMsg::TCPDataMsg(const TCPDataMsg& other) : ByteArrayMessage(other.getName())
{
    operator=(other);
}

TCPDataMsg::~TCPDataMsg()
{
    if (payloadPacket_var)
    {
        drop(payloadPacket_var);
        delete payloadPacket_var;
    }
}

TCPDataMsg& TCPDataMsg::operator=(const TCPDataMsg& other)
{
    if (this==&other) return *this;
    ByteArrayMessage::operator=(other);
    removePayloadPacket();
    if (other.payloadPacket_var)
    {
        payloadPacket_var = other.payloadPacket_var->dup();
        take(payloadPacket_var);
    }

    this->isPayloadStart_var = other.isPayloadStart_var;
    return *this;
}

cPacket* TCPDataMsg::getPayloadPacket()
{
    return payloadPacket_var;
}

void TCPDataMsg::setPayloadPacket(cPacket* dataObject)
{
    if (payloadPacket_var)
        throw cRuntimeError(this,"setPayloadPacket: already contains a data object");
    payloadPacket_var = dataObject;
    if (payloadPacket_var)
        take(payloadPacket_var);
}

cPacket* TCPDataMsg::removePayloadPacket()
{
    if (! payloadPacket_var)
        return NULL;
    cPacket* ret = payloadPacket_var;
    payloadPacket_var = NULL;
    drop(ret);
    return ret;
}

void TCPDataMsg::parsimPack(cCommBuffer *b)
{
    ByteArrayMessage::parsimPack(b);
    if (b->packFlag(payloadPacket_var != NULL))
        b->packObject(payloadPacket_var);
    doPacking(b,this->isPayloadStart_var);
}

void TCPDataMsg::parsimUnpack(cCommBuffer *b)
{
    removePayloadPacket();
    ByteArrayMessage::parsimUnpack(b);
    if (b->checkFlag())
        take(payloadPacket_var = (cPacket *) b->unpackObject());
    doUnpacking(b,this->isPayloadStart_var);
}

bool TCPDataMsg::getIsPayloadStart() const
{
    return isPayloadStart_var;
}

void TCPDataMsg::setIsPayloadStart(bool isPayloadStart_var)
{
    this->isPayloadStart_var = isPayloadStart_var;
}
