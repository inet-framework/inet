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


TCPDataMsg::TCPDataMsg(const char *name)
    : TCPDataMsg_Base(name)
{
    dataObject_var = NULL;
    isBegin_var = false;
}

TCPDataMsg::TCPDataMsg(const TCPDataMsg& other) : TCPDataMsg_Base(other.getName())
{
    operator=(other);
}

TCPDataMsg::~TCPDataMsg()
{
    if (dataObject_var)
    {
        drop(dataObject_var);
        delete dataObject_var;
    }
}

TCPDataMsg& TCPDataMsg::operator=(const TCPDataMsg& other)
{
    if (this==&other) return *this;
    TCPDataMsg_Base::operator=(other);
    this->dataObject_var = other.dataObject_var->dup();
    return *this;
}

void TCPDataMsg::setDataObject(const cPacketPtr& dataObject)
{
    if (dataObject_var)
        throw cRuntimeError(this,"setDataObject: already contains a data object");
    dataObject_var = dataObject;
    if (dataObject_var)
        take(dataObject_var);
}

cPacket* TCPDataMsg::removeDataObject()
{
    if (! dataObject_var)
        return NULL;
    cPacket* ret = dataObject_var;
    dataObject_var = NULL;
    drop(ret);
    return ret;
}
