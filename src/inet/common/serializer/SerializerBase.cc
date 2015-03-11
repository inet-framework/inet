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

SerializerRegistrationList serializers("default"); ///< List of packet serializers (SerializerBase)

EXECUTE_ON_SHUTDOWN(
        serializers.clear();
        );


DefaultSerializer SerializerRegistrationList::defaultSerializer;
ByteArraySerializer SerializerRegistrationList::byteArraySerializer;

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
    if (pos + 2 > bufsize) {
        errorFound = true;
        return 0;
    }
    uint16_t ret = ((uint16_t)(buf[pos]) << 8) + buf[pos+1];
    pos += 2;
    return ret;
}

uint32_t Buffer::readUint32() const
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

void SerializerBase::serializePacket(const cPacket *pkt, Buffer &b, Context& context)
{
    unsigned int startPos = b.getPos();
    serialize(pkt, b, context);
    if (!b.hasError() && (b.getPos() - startPos != pkt->getByteLength()))
        throw cRuntimeError("%s serializer error: packet %s (%s) length is %d but serialized length is %d", getClassName(), pkt->getName(), pkt->getClassName(), pkt->getByteLength(), b.getPos() - startPos);
}

cPacket *SerializerBase::deserializePacket(Buffer &b, Context& context)
{
    unsigned int startPos = b.getPos();
    cPacket *pkt = deserialize(b, context);
    if (pkt == nullptr) {
        b.seek(startPos);
        pkt = serializers.byteArraySerializer.deserialize(b, context);
    }
    if (!pkt->hasBitError() && !b.hasError() && (b.getPos() - startPos != pkt->getByteLength()))
        throw cRuntimeError("%s deserializer error: packet %s (%s) length is %d but deserialized length is %d", getClassName(), pkt->getName(), pkt->getClassName(), pkt->getByteLength(), b.getPos() - startPos);
    if (b.hasError())
        pkt->setBitError(true);
    return pkt;
}

SerializerBase & SerializerBase::lookupSerializer(const cPacket *pkt, Context& context, ProtocolGroup group, int id)
{
    const ByteArrayMessage *bam = dynamic_cast<const ByteArrayMessage *>(pkt);
    if (bam != nullptr)
        return serializers.byteArraySerializer;
    SerializerBase *serializer = serializers.lookup(group, id);
    if (serializer != nullptr)
        return *serializer;
    serializer = serializers.lookup(pkt->getClassName());
    if (serializer != nullptr)
        return *serializer;
    if (context.throwOnSerializerNotFound)
        throw cRuntimeError("Serializer not found for '%s' (%i, %i)", pkt->getClassName(), group, id);
    return serializers.defaultSerializer;
}

SerializerBase & SerializerBase::lookupDeserializer(Context& context, ProtocolGroup group, int id)
{
    SerializerBase *serializer = serializers.lookup(group, id);
    if (serializer != nullptr)
        return *serializer;
    else
        return serializers.byteArraySerializer;
}

void SerializerBase::lookupAndSerialize(const cPacket *pkt, Buffer &b, Context& context, ProtocolGroup group, int id, unsigned int trailerLength)
{
    ASSERT(pkt);
    Buffer subBuffer(b, trailerLength);
    SerializerBase & serializer = lookupSerializer(pkt, context, group, id);
    serializer.serializePacket(pkt, subBuffer, context);
    b.accessNBytes(subBuffer.getPos());
    if (subBuffer.hasError())
        b.setError();
}

cPacket *SerializerBase::lookupAndDeserialize(Buffer &b, Context& context, ProtocolGroup group, int id, unsigned int trailerLength)
{
    cPacket *encapPacket = nullptr;
    SerializerBase& serializer = lookupDeserializer(context, group, id);
    Buffer subBuffer(b, trailerLength);
    encapPacket = serializer.deserializePacket(subBuffer, context);
    b.accessNBytes(subBuffer.getPos());
    return encapPacket;
}

//

void DefaultSerializer::serialize(const cPacket *pkt, Buffer &b, Context& context)
{
    b.fillNBytes(pkt->getByteLength(), '?');
    context.errorOccured = true;
}

cPacket *DefaultSerializer::deserialize(Buffer &b, Context& context)
{
    unsigned int byteLength = b.getRemainder();
    if (byteLength) {
        cPacket *pkt = new cPacket();
        pkt->setByteLength(byteLength);
        b.accessNBytes(byteLength);
        context.errorOccured = true;
        return pkt;
    }
    else
        return nullptr;
}

//

void ByteArraySerializer::serialize(const cPacket *pkt, Buffer &b, Context& context)
{
    const ByteArrayMessage *bam = check_and_cast<const ByteArrayMessage *>(pkt);
    unsigned int length = bam->getByteLength();
    unsigned int wl = std::min(length, b.getRemainder());
    wl = bam->copyDataToBuffer(b.accessNBytes(0), wl);
    b.accessNBytes(wl);
    if (length > wl)
        b.fillNBytes(length - wl, '?');
    if (pkt->getEncapsulatedPacket())
        throw cRuntimeError("Serializer: encapsulated packet in ByteArrayPacket is not allowed");
}

cPacket *ByteArraySerializer::deserialize(Buffer &b, Context& context)
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

//

SerializerRegistrationList::~SerializerRegistrationList()
{
    if (!stringToSerializerMap.empty())
        throw cRuntimeError("SerializerRegistrationList not empty, should call the SerializerRegistrationList::clear() function");
}

void SerializerRegistrationList::clear()
{
    for (auto elem : stringToSerializerMap) {
        dropAndDelete(elem.second);
    }
    stringToSerializerMap.clear();
    keyToSerializerMap.clear();
}

void SerializerRegistrationList::add(const char *name, int protocolGroup, int protocolId, SerializerBase *obj)
{
    Key key(protocolGroup, protocolId);

    take(obj);
    if (protocolGroup != UNKNOWN)
        keyToSerializerMap.insert(std::pair<Key,SerializerBase*>(key, obj));
    if (!name)
        throw cRuntimeError("missing 'name' of registered serializer");
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

} // namespace serializer

} // namespace inet

