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

import inet.common.INETDefs;

namespace inet;

//
// Class that carries raw bytes.
// For example, used by ~RawPacket and some TCP queues.
//
class ByteArray extends cObject
{
    char data[];
}

cplusplus(ByteArray) {{
  public:
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

    virtual char *getDataPtr()  { return data; }
}}

