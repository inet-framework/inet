//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//


//TODO split it to more files

#include "inet/common/serializer/Buffer.h"

namespace inet {

namespace serializer {

Buffer::Buffer(const Buffer& base, unsigned int maxLength)
{
    buf = base.buf + base.pos;
    bufsize = std::min(base.bufsize - base.pos, maxLength);
}

unsigned char Buffer::readByte() const
{
    if (pos >= bufsize) {
        errorFound = true;
        return 0;
    }
    return buf[pos++];
}

void Buffer::readNBytes(unsigned int length, void *_dest) const
{
    unsigned char *dest = static_cast<unsigned char *>(_dest);
    while (length--) {
        if (pos >= bufsize) {
            errorFound = true;
            *dest++ = 0;
        } else
            *dest++ = buf[pos++];
    }
}

uint16_t Buffer::readUint16() const
{
    uint16_t ret = 0;
    if (pos < bufsize) ret += ((uint16_t)(buf[pos++]) << 8);
    if (pos < bufsize) ret += ((uint16_t)(buf[pos++]));
    else errorFound = true;
    return ret;
}

uint32_t Buffer::readUint32() const
{
    uint32_t ret = 0;
    if (pos < bufsize) ret += ((uint32_t)(buf[pos++]) << 24);
    if (pos < bufsize) ret += ((uint32_t)(buf[pos++]) << 16);
    if (pos < bufsize) ret += ((uint32_t)(buf[pos++]) <<  8);
    if (pos < bufsize) ret += ((uint32_t)(buf[pos++]));
    else errorFound = true;
    return ret;
}

uint64_t Buffer::readUint64() const
{
    uint64_t ret = 0;
    if (pos < bufsize) ret += ((uint64_t)(buf[pos++]) << 56);
    if (pos < bufsize) ret += ((uint64_t)(buf[pos++]) << 48);
    if (pos < bufsize) ret += ((uint64_t)(buf[pos++]) << 40);
    if (pos < bufsize) ret += ((uint64_t)(buf[pos++]) << 32);
    if (pos < bufsize) ret += ((uint64_t)(buf[pos++]) << 24);
    if (pos < bufsize) ret += ((uint64_t)(buf[pos++]) << 16);
    if (pos < bufsize) ret += ((uint64_t)(buf[pos++]) <<  8);
    if (pos < bufsize) ret += ((uint64_t)(buf[pos++]));
    else errorFound = true;
    return ret;
}

void Buffer::writeByte(unsigned char data)
{
    if (pos >= bufsize) {
        errorFound = true;
        return;
    }
    buf[pos++] = data;
}

void Buffer::writeByteTo(unsigned int position, unsigned char data)
{
    if (position >= bufsize) {
        errorFound = true;
        return;
    }
    buf[position] = data;
}

void Buffer::writeNBytes(unsigned int length, const void *_src)
{
    const unsigned char *src = static_cast<const unsigned char *>(_src);
    while (pos < bufsize && length > 0) {
        buf[pos++] = *src++;
        length--;
    }
    if (length)
        errorFound = true;
}

void Buffer::writeNBytes(Buffer& inputBuffer, unsigned int length)
{
    while (pos < bufsize && length > 0) {
        buf[pos++] = inputBuffer.readByte();
        length--;
    }
    if (length)
        errorFound = true;
}

void Buffer::fillNBytes(unsigned int length, unsigned char data)
{
    while (pos < bufsize && length > 0) {
        buf[pos++] = data;
        length--;
    }
    if (length)
        errorFound = true;
}

void Buffer::writeUint16(uint16_t data)    // hton
{
    if (pos < bufsize)
        buf[pos++] = (uint8_t)(data >> 8);
    if (pos < bufsize)
        buf[pos++] = (uint8_t)data;
    else
        errorFound = true;
}

void Buffer::writeUint16To(unsigned int position, uint16_t data)    // hton
{
    if (position < bufsize)
        buf[position++] = (uint8_t)(data >> 8);
    if (position < bufsize)
        buf[position++] = (uint8_t)data;
    else
        errorFound = true;
}

void Buffer::writeUint32(uint32_t data)    // hton
{
    if (pos < bufsize)
        buf[pos++] = (uint8_t)(data >> 24);
    if (pos < bufsize)
        buf[pos++] = (uint8_t)(data >> 16);
    if (pos < bufsize)
        buf[pos++] = (uint8_t)(data >> 8);
    if (pos < bufsize)
        buf[pos++] = (uint8_t)data;
    else
        errorFound = true;
}

void Buffer::writeUint64(uint64_t data)    // hton
{
    if (pos < bufsize)
        buf[pos++] = (uint8_t)(data >> 56);
    if (pos < bufsize)
        buf[pos++] = (uint8_t)(data >> 48);
    if (pos < bufsize)
        buf[pos++] = (uint8_t)(data >> 40);
    if (pos < bufsize)
        buf[pos++] = (uint8_t)(data >> 32);
    if (pos < bufsize)
        buf[pos++] = (uint8_t)(data >> 24);
    if (pos < bufsize)
        buf[pos++] = (uint8_t)(data >> 16);
    if (pos < bufsize)
        buf[pos++] = (uint8_t)(data >> 8);
    if (pos < bufsize)
        buf[pos++] = (uint8_t)data;
    else
        errorFound = true;
}

void *Buffer::accessNBytes(unsigned int length)
{
    if (pos + length <= bufsize) {
        void *ret = buf + pos;
        pos += length;
        return ret;
    }
    pos = bufsize;
    errorFound = true;
    return nullptr;
}

MACAddress Buffer::readMACAddress() const
{
    MACAddress addr;
    for (int i = 0; i < MAC_ADDRESS_SIZE; i++)
        addr.setAddressByte(i, readByte());
    return addr;
}

void Buffer::writeMACAddress(const MACAddress& addr)
{
    unsigned char buff[MAC_ADDRESS_SIZE];
    addr.getAddressBytes(buff);
    writeNBytes(MAC_ADDRESS_SIZE, buff);
}

IPv6Address Buffer::readIPv6Address() const
{
    uint32_t d[4];
    for (int i = 0; i < 4; i++)
        d[i] = readUint32();
    return IPv6Address(d[0], d[1], d[2], d[3]);
}

} // namespace serializer

} // namespace inet

