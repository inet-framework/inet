//
// (C) 2005 Vojtech Janota
//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//

#ifndef __SOCKETMSG_H__
#define __SOCKETMSG_H__

#include "SocketMsg_m.h"

class SocketMsg : public SocketMsg_Base
{
  public:
    SocketMsg(const char *name=NULL, int kind=0) : SocketMsg_Base(name,kind) {}
    SocketMsg(const SocketMsg& other) : SocketMsg_Base(other.name()) {operator=(other);}
    SocketMsg& operator=(const SocketMsg& other) {SocketMsg_Base::operator=(other); return *this;}
    virtual cPolymorphic *dup() const {return new SocketMsg(*this);}

    void setDataFromBuffer(const void *ptr, int length);
    void copyDataToBuffer(void *ptr, int length);
    void removePrefix(int length);
};

#endif



