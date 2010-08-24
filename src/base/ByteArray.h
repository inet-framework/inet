//
// (C) 2005 Vojtech Janota
// (C) 2010 Zoltan Bojthe
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

#ifndef __INET_BYTEARRAY_H
#define __INET_BYTEARRAY_H

#include "ByteArray_m.h"

/**
 * Message that carries raw bytes. Used with emulation-related features.
 */
class ByteArray : public ByteArray_Base
{
  public:
    ByteArray() : ByteArray_Base() {}
    ByteArray(const ByteArray& other) : ByteArray_Base() {operator=(other);}
    ByteArray& operator=(const ByteArray& other) {ByteArray_Base::operator=(other); return *this;}
    virtual ByteArray *dup() const {return new ByteArray(*this);}

    virtual void setDataFromBuffer(const void *ptr, unsigned int length);
    virtual void addDataFromBuffer(const void *ptr, unsigned int length);
    virtual void copyDataToBuffer(void *ptr, unsigned int length) const;
    virtual void truncateData(unsigned int truncleft, unsigned int truncright = 0);
};

#endif //  __INET_BYTEARRAY_H
