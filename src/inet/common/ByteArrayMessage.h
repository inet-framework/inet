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

#include "inet/common/ByteArrayMessage_m.h"

namespace inet {

/**
 * Message that carries raw bytes. Used with emulation-related features.
 */
class ByteArrayMessage : public ByteArrayMessage_Base
{
  public:
    /**
     * Constructor
     */
    ByteArrayMessage(const char *name = NULL, int kind = 0) : ByteArrayMessage_Base(name, kind) {}

    /**
     * Copy constructor
     */
    ByteArrayMessage(const ByteArrayMessage& other) : ByteArrayMessage_Base(other) {}

    /**
     * operator =
     */
    ByteArrayMessage& operator=(const ByteArrayMessage& other) { ByteArrayMessage_Base::operator=(other); return *this; }

    /**
     * Creates and returns an exact copy of this object.
     */
    virtual ByteArrayMessage *dup() const { return new ByteArrayMessage(*this); }

    /**
     * Set data from buffer
     * @param ptr: pointer to buffer
     * @param length: length of data
     */
    virtual void setDataFromBuffer(const void *ptr, unsigned int length);

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
     * @return: length of copied data
     */
    virtual unsigned int copyDataToBuffer(void *ptr, unsigned int length) const;

    /**
     * Truncate data content
     * @param length: The number of bytes from the beginning of the content be removed
     * Generate assert when not have enough bytes for truncation
     */
    virtual void removePrefix(unsigned int length);
};

} // namespace inet

#endif // ifndef __INET_BYTEARRAYMESSAGE_H

