//
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

#ifndef __INET_BYTEARRAYLIST_H
#define __INET_BYTEARRAYLIST_H

#include "ByteArray.h"

/**
 * Queue that carries raw bytes.
 */
class ByteArrayList
{
  protected:
    typedef std::list<ByteArray> DataList;
    uint64 dataLengthM;
    DataList dataListM;

  public:
    ByteArrayList();
    ByteArrayList(const ByteArrayList& other);
    ByteArrayList& operator=(const ByteArrayList& other);
    virtual ByteArrayList *dup() const {return new ByteArrayList(*this);}

    void clear();
    virtual void push(const ByteArray& byteArrayP);
    virtual void push(const void* bufferP, unsigned int bufferLengthP);
    virtual uint64 getLength() const { return dataLengthM; }
    virtual unsigned int getBytesToBuffer(void* bufferP, unsigned int bufferLengthP) const;
    virtual unsigned int popBytesToBuffer(void* bufferP, unsigned int bufferLengthP);
    virtual unsigned int drop(unsigned int lengthP);
};

#endif //  __INET_BYTEARRAYLIST_H
