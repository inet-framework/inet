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

#include "inet/common/serializer/SerializerBase.h"

namespace inet {

namespace serializer {

GlobalSerializerRegistrationList serializers; ///< List of packet serializers (SerializerBase)

Buffer::Buffer(const Buffer& base, unsigned int trailerLength)
{
    buf = base.buf + base.pos;
    bufsize = base.bufsize - base.pos;
    if (bufsize >= trailerLength) {
        bufsize -= trailerLength;
    }
    else {
        bufsize = 0;
        errorFound = true;
    }
}

unsigned char Buffer::readByte()
{
    if (pos >= bufsize) {
        errorFound = true;
        return 0;
    }
    return buf[pos++];
}

void Buffer::readNBytes(unsigned int length, void *_dest)
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

uint16_t Buffer::readUint16()
{
    if (pos + 2 > bufsize) {
        errorFound = true;
        return 0;
    }
    uint16_t ret = ((uint16_t)(buf[pos]) << 8) + buf[pos+1];
    pos += 2;
    return ret;
}

uint32_t Buffer::readUint32()
{
    if (pos + 4 > bufsize) {
        errorFound = true;
        return 0;
    }
    uint32_t ret = ((uint32_t)(buf[pos]) << 24) + ((uint32_t)(buf[pos+1]) << 16) + ((uint32_t)(buf[pos+2]) << 8) + buf[pos+3];
    pos += 4;
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

MACAddress Buffer::readMACAddress()
{
    MACAddress addr;
    void *addrBytes = accessNBytes(MAC_ADDRESS_SIZE);
    if (addrBytes)
        addr.setAddressBytes((unsigned char *)addrBytes);
    return addr;
}

void Buffer::writeMACAddress(const MACAddress& addr)
{
    void *addrBytes = accessNBytes(MAC_ADDRESS_SIZE);
    if (addrBytes)
        addr.getAddressBytes((unsigned char *)addrBytes);
}

IPv6Address Buffer::readIPv6Address()
{
    uint32_t d[4];
    void *addrBytes = accessNBytes(MAC_ADDRESS_SIZE);
    for (int i = 0; i < 4; i++)
        d[i] = readUint32();
    return IPv6Address(d[0], d[1], d[2], d[3]);
}

void SerializerBase::serializeByteArrayPacket(const ByteArrayMessage *pkt, Buffer &b)
{
    unsigned int length = pkt->getByteLength();
    unsigned int wl = std::min(length, b.getRemainder());
    length = pkt->copyDataToBuffer(b.accessNBytes(0), wl);
    b.accessNBytes(length);
    if (pkt->getEncapsulatedPacket())
        throw cRuntimeError("Serializer: encapsulated packet in ByteArrayPacket is not allowed");
    return;
}

void SerializerBase::serialize(const cPacket *pkt, Buffer &b, Context& context, ProtocolGroup group, int id, unsigned int trailerLength)
{
    Buffer subBuffer(b, trailerLength);
    const ByteArrayMessage *bam = dynamic_cast<const ByteArrayMessage *>(pkt);
    if (bam) {
        serializeByteArrayPacket(bam, subBuffer);
        b.accessNBytes(subBuffer.getPos());
        return;
    }
    SerializerBase *serializer = serializers.getInstance()->lookup(group, id);
    if (!serializer) {
        serializer = serializers.getInstance()->lookup(pkt->getClassName());
    }
    if (serializer) {
        serializer->xSerialize(pkt, subBuffer, context);
        b.accessNBytes(subBuffer.getPos());
        return;
    }
    if (context.throwOnSerializerNotFound)
        throw cRuntimeError("Serializer not found for '%s' (%i, %i)", pkt->getClassName(), group, id);
    context.errorOccured = true;
    b.fillNBytes(pkt->getByteLength(), '?');
}

void SerializerBase::xSerialize(const cPacket *pkt, Buffer &b, Context& context)
{
    unsigned int startPos = b.getPos();
    const ByteArrayMessage *bam = dynamic_cast<const ByteArrayMessage *>(pkt);
    if (bam) {
        serializeByteArrayPacket(bam, b);
    }
    else {
        serialize(pkt, b, context);
    }
    if (!b.hasError() && (b.getPos() - startPos != pkt->getByteLength()))
        throw cRuntimeError("serializer error: packet %s (%s) length is %d but serialized length is %d", pkt->getName(), pkt->getClassName(), pkt->getByteLength(), b.getPos() - startPos);
}

cPacket *SerializerBase::xParse(Buffer &b, Context& context)
{
    unsigned int startPos = b.getPos();
    cPacket *pkt = deserialize(b, context);

    if (pkt) {
        pkt->setByteLength(b.getPos() - startPos);
        if (b.hasError())
            pkt->setBitError(true);
    }
    else {
        b.seek(startPos);
        pkt = parseByteArrayPacket(b);
    }

    return pkt;
}

ByteArrayMessage *SerializerBase::parseByteArrayPacket(Buffer &b)
{
    ByteArrayMessage *bam = nullptr;
    unsigned int bytes = b.getRemainder();
    if (bytes) {
        bam = new ByteArrayMessage("parsed-bytes");
        bam->setDataFromBuffer(b.accessNBytes(bytes), bytes);
        bam->setByteLength(bytes);
    }
    return bam;
}

cPacket *SerializerBase::parse(Buffer &b, Context& context, ProtocolGroup group, int id, unsigned int trailerLength)
{
    cPacket *encapPacket = nullptr;
    SerializerBase *serializer = serializers.getInstance()->lookup(group, id);
    Buffer subBuffer(b, trailerLength);
    if (serializer) {
        encapPacket = serializer->xParse(subBuffer, context);
    }
    else {
        encapPacket = parseByteArrayPacket(subBuffer);
    }
    b.accessNBytes(subBuffer.getPos());
    ASSERT(subBuffer.hasError() || subBuffer.getPos() == encapPacket->getByteLength());
    return encapPacket;
}

//

SerializerRegistrationList::~SerializerRegistrationList()
{
    for (auto elem : stringToSerializerMap) {
        dropAndDelete(elem.second);
    }
}

void SerializerRegistrationList::add(const char *name, int protocolGroup, int protocolId, SerializerBase *obj)
{
    Key key(protocolGroup, protocolId);

    if (protocolGroup && protocolId)
        keyToSerializerMap.insert(std::pair<Key,SerializerBase*>(key, obj));
    if (name)
        stringToSerializerMap.insert(std::pair<std::string,SerializerBase*>(name, obj));
}

SerializerBase *SerializerRegistrationList::lookup(int protocolGroup, int protocolId) const
{
    auto it = keyToSerializerMap.find(Key(protocolGroup, protocolId));
    return it==keyToSerializerMap.end() ? NULL : it->second;
    return nullptr;
}

SerializerBase *SerializerRegistrationList::lookup(const char *name) const
{
    auto it = stringToSerializerMap.find(name);
    return it==stringToSerializerMap.end() ? NULL : it->second;
    return nullptr;
}

//

GlobalSerializerRegistrationList::GlobalSerializerRegistrationList()
{
}

GlobalSerializerRegistrationList::GlobalSerializerRegistrationList(const char *name)
{
    tmpname = name;
}

GlobalSerializerRegistrationList::~GlobalSerializerRegistrationList()
{
    // delete inst; -- this is usually not a good idea, as things may be
    // in an inconsistent state by now; especially if the program is
    // exiting via exit() or abort(), ie. not by returning from main().
}

SerializerRegistrationList *GlobalSerializerRegistrationList::getInstance()
{
    if (!inst)
        inst = new SerializerRegistrationList(tmpname);
    return inst;
}

void GlobalSerializerRegistrationList::clear()
{
    delete inst;
    inst = NULL;
}

//

} // namespace serializer

} // namespace inet

