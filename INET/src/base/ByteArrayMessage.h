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

#ifndef __INET_BYTEARRAYMESSAGE_H
#define __INET_BYTEARRAYMESSAGE_H

#include "ByteArrayMessage_m.h"

/**
 * Message that carries raw bytes. Used with emulation-related features.
 */
class ByteArrayMessage : public ByteArrayMessage_Base
{
  public:
    ByteArrayMessage(const char *name=NULL, int kind=0) : ByteArrayMessage_Base(name,kind) {}
    ByteArrayMessage(const ByteArrayMessage& other) : ByteArrayMessage_Base(other.getName()) {operator=(other);}
    ByteArrayMessage& operator=(const ByteArrayMessage& other) {ByteArrayMessage_Base::operator=(other); return *this;}
    virtual ByteArrayMessage *dup() const {return new ByteArrayMessage(*this);}

    virtual void setDataFromBuffer(const void *ptr, int length);
    virtual void copyDataToBuffer(void *ptr, int length);
    virtual void removePrefix(int length);
};

#endif



