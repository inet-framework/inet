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

void SerializerBase::serializePacket(const cPacket *pkt, Buffer &b, Context& c)
{
    unsigned int startPos = b.getPos();
    serialize(pkt, b, c);
    if (!b.hasError() && (b.getPos() - startPos != pkt->getByteLength()))
        throw cRuntimeError("%s serializer error: packet %s (%s) length is %d but serialized length is %d", getClassName(), pkt->getName(), pkt->getClassName(), pkt->getByteLength(), b.getPos() - startPos);
}

cPacket *SerializerBase::deserializePacket(const Buffer &b, Context& context)
{
    if (b.getRemainingSize() == 0)
        return nullptr;

    unsigned int startPos = b.getPos();
    cPacket *pkt = deserialize(b, context);
    if (pkt == nullptr) {
        b.seek(startPos);
        pkt = serializers.byteArraySerializer.deserialize(b, context);
    }
    ASSERT(pkt);
    if (!pkt->hasBitError() && !b.hasError() && (b.getPos() - startPos != pkt->getByteLength())) {
        const char *encclass = pkt->getEncapsulatedPacket() ? pkt->getEncapsulatedPacket()->getClassName() : "<nullptr>";
        throw cRuntimeError("%s deserializer error: packet %s (%s) length is %d but deserialized length is %d (encapsulated packet is %s)", getClassName(), pkt->getName(), pkt->getClassName(), pkt->getByteLength(), b.getPos() - startPos, encclass);
    }
    if (b.hasError())
        pkt->setBitError(true);
    return pkt;
}

SerializerBase & SerializerBase::lookupSerializer(const cPacket *pkt, Context& context, ProtocolGroup group, int id)
{
    const RawPacket *bam = dynamic_cast<const RawPacket *>(pkt);
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

void SerializerBase::lookupAndSerialize(const cPacket *pkt, Buffer &b, Context& context, ProtocolGroup group, int id, unsigned int maxLength)
{
    ASSERT(pkt);
    Buffer subBuffer(b, maxLength);
    SerializerBase & serializer = lookupSerializer(pkt, context, group, id);
    serializer.serializePacket(pkt, subBuffer, context);
    b.accessNBytes(subBuffer.getPos());
    if (subBuffer.hasError())
        b.setError();
}

cPacket *SerializerBase::lookupAndDeserialize(const Buffer &b, Context& context, ProtocolGroup group, int id, unsigned int maxLength)
{
    cPacket *encapPacket = nullptr;
    SerializerBase& serializer = lookupDeserializer(context, group, id);
    Buffer subBuffer(b, maxLength);
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

cPacket *DefaultSerializer::deserialize(const Buffer &b, Context& context)
{
    unsigned int byteLength = b.getRemainingSize();
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
    const RawPacket *bam = check_and_cast<const RawPacket *>(pkt);
    unsigned int length = bam->getByteLength();
    unsigned int wl = std::min(length, b.getRemainingSize());
    wl = bam->copyDataToBuffer(b.accessNBytes(0), wl);
    b.accessNBytes(wl);
    if (length > wl)
        b.fillNBytes(length - wl, '?');
    if (pkt->getEncapsulatedPacket())
        throw cRuntimeError("Serializer: encapsulated packet in ByteArrayPacket is not allowed");
}

cPacket *ByteArraySerializer::deserialize(const Buffer &b, Context& context)
{
    RawPacket *bam = nullptr;
    unsigned int bytes = b.getRemainingSize();
    if (bytes) {
        bam = new RawPacket("parsed-bytes");
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
    return it==keyToSerializerMap.end() ? nullptr : it->second;
}

SerializerBase *SerializerRegistrationList::lookup(const char *name) const
{
    auto it = stringToSerializerMap.find(name);
    return it==stringToSerializerMap.end() ? nullptr : it->second;
}

//

} // namespace serializer

} // namespace inet

