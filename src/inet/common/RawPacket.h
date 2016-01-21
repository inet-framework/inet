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

#ifndef __INET_RAWPACKET_H
#define __INET_RAWPACKET_H

#include "inet/common/RawPacket_m.h"

namespace inet {

/**
 * Message that carries raw bytes. Used with emulation-related features.
 */
class INET_API RawPacket : public RawPacket_Base
{
  public:
    /**
     * Constructor
     */
    RawPacket(const char *name = nullptr, int kind = 0) : RawPacket_Base(name, kind) {}

    /**
     * Copy constructor
     */
    RawPacket(const RawPacket& other) : RawPacket_Base(other) {}

    /**
     * operator =
     */
    RawPacket& operator=(const RawPacket& other) { RawPacket_Base::operator=(other); return *this; }

    /**
     * Creates and returns an exact copy of this object.
     */
    virtual RawPacket *dup() const override { return new RawPacket(*this); }

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

#endif // ifndef __INET_RAWPACKET_H

