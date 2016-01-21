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

#include "inet/common/ByteArray_m.h"

namespace inet {

/**
 * Class that carries raw bytes.
 */
class INET_API ByteArray : public ByteArray_Base
{
  public:
    /**
     * Constructor
     */
    ByteArray() : ByteArray_Base() {}

    /**
     * Copy constructor
     */
    ByteArray(const ByteArray& other) : ByteArray_Base(other) {}

    /**
     * operator =
     */
    ByteArray& operator=(const ByteArray& other) { ByteArray_Base::operator=(other); return *this; }

    /**
     * Creates and returns an exact copy of this object.
     */
    virtual ByteArray *dup() const override { return new ByteArray(*this); }

    /**
     * Copy data from buffer
     * @param ptr: pointer to buffer
     * @param length: length of data
     */
    virtual void setDataFromBuffer(const void *ptr, unsigned int length);

    /**
     * Copy data from buffer
     * @param destOffset: first byte from buffer will be written to here
     * @param ptr: pointer to buffer
     * @param length: length of data
     */
    virtual void copyDataFromBuffer(unsigned int destOffset, const void *ptr, unsigned int length);

    /**
     * Copy data from other ByteArray
     * @param other: reference to other ByteArray
     * @param srcOffset: skipped first bytes from other
     * @param length: length of data
     */
    virtual void setDataFromByteArray(const ByteArray& other, unsigned int srcOffset, unsigned int length);

    /**
     * Copy data from other ByteArray
     * @param destOffset: first byte from other will be written to here
     * @param other: reference to other ByteArray
     * @param srcOffset: skipped first bytes from other
     * @param length: length of data from other
     */
    virtual void copyDataFromByteArray(unsigned int destOffset, const ByteArray& other, unsigned int srcOffset, unsigned int length);

    /**
     * Add data from buffer to the end of existing content
     * @param ptr: pointer to input buffer
     * @param length: length of data
     */
    virtual void addDataFromBuffer(const void *ptr, unsigned int length);

    /**
     * Copy data content to buffer
     * @param ptr: pointer to output buffer
     * @param length: length of buffer, maximum of copied bytes
     * @param srcOffs: number of skipped bytes from source
     * @return: length of copied data
     */
    virtual unsigned int copyDataToBuffer(void *ptr, unsigned int length, unsigned int srcOffs = 0) const;

    /**
     * Set buffer pointer and buffer length
     * @param ptr: pointer to new buffer, must created by `buffer = new char[length1];` where length1>=length
     * @param length: length of buffer
     */
    virtual void assignBuffer(void *ptr, unsigned int length);

    /**
     * Truncate data content
     * @param truncleft: The number of bytes from the beginning of the content be remove
     * @param truncright: The number of bytes from the end of the content be remove
     * Generate assert when not have enough bytes for truncation
     */
    virtual void truncateData(unsigned int truncleft, unsigned int truncright);

    /**
     * Expand data content
     * @param addLeft: The number of bytes will be added at beginning of the content
     * @param addRight: The number of bytes will be added at end of the content
     * added bytes are unfilled
     */
    virtual void expandData(unsigned int addLeft, unsigned int addRight);

    virtual char *getDataPtr()  { return data_var; }
};

} // namespace inet

#endif // ifndef __INET_BYTEARRAY_H

