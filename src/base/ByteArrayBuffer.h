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

#ifndef __INET_BYTEARRAYBUFFER_H
#define __INET_BYTEARRAYBUFFER_H

#include "ByteArray.h"

/**
 * Buffer that carries BytesArrays.
 */
class ByteArrayBuffer : public cObject
{
  protected:
    typedef std::list<ByteArray> DataList;
    uint64 dataLengthM;
    DataList dataListM;

  private:
    void copy(const ByteArrayBuffer& other) { dataLengthM = other.dataLengthM; dataListM = other.dataListM; }

  public:
    /** Ctor. */
    ByteArrayBuffer();

    /** Copy ctor. */
    ByteArrayBuffer(const ByteArrayBuffer& other);
    ByteArrayBuffer& operator=(const ByteArrayBuffer& other);

    virtual ByteArrayBuffer *dup() const {return new ByteArrayBuffer(*this);}

    /** Clear buffer */
    virtual void clear();

    /** Push data to end of buffer */
    virtual void push(const ByteArray& byteArrayP);

    /** Push data to end of buffer */
    virtual void push(const void* bufferP, unsigned int bufferLengthP);

    /** Returns length of stored data */
    virtual uint64 getLength() const { return dataLengthM; }

    /**
     * Copy bytes to an external buffer
     * @param bufferP: pointer to output buffer
     * @param bufferLengthP: length of output buffer
     * @param srcOffsP: source offset
     * @return count of copied bytes
     */
    virtual unsigned int getBytesToBuffer(void* bufferP, unsigned int bufferLengthP, unsigned int srcOffsP = 0) const;

    /**
     * Move bytes to an external buffer
     * @param bufferP: pointer to output buffer
     * @param bufferLengthP: length of output buffer
     * @return count of moved bytes
     */
    virtual unsigned int popBytesToBuffer(void* bufferP, unsigned int bufferLengthP);

    /**
     * Drop bytes from buffer
     * @param lengthP: count of droppable bytes
     * @return count of dropped bytes
     */
    virtual unsigned int drop(unsigned int lengthP);
};

#endif //  __INET_BYTEARRAYBUFFER_H
