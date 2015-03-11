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

#ifndef __INET_SERIALIZERBASE_H_
#define __INET_SERIALIZERBASE_H_

#include "inet/common/INETDefs.h"

#include "inet/common/ByteArrayMessage.h"
#include "inet/linklayer/common/MACAddress.h"
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include "inet/networklayer/contract/ipv6/IPv6Address.h"


namespace inet {

namespace serializer {

enum ProtocolGroup {
    UNKNOWN = -1,       // lookup serializer by classname only
    GLOBAL = 1,
    IP_PROT,
    ETHERTYPE,
    LINKTYPE
};

enum {  // from libpcap
    LINKTYPE_ETHERNET = 1,
    LINKTYPE_IEEE802_11 = 105
};

/**
 * Buffer for serializer/deserializer
 */
class INET_API Buffer
{
  protected:
    unsigned char *buf = nullptr;
    unsigned int bufsize = 0;
    mutable unsigned int pos = 0;
    mutable bool errorFound = false;

  public:
    Buffer(const Buffer& base, unsigned int trailerLength);
    Buffer(void *buf, unsigned int bufLen) : buf(static_cast<unsigned char *>(buf)), bufsize(bufLen) {}

    // position
    void seek(unsigned int newpos) const { if (newpos <= bufsize) { pos = newpos; } else { pos = bufsize; errorFound = true; } }
    unsigned int getPos() const  { return pos; }
    unsigned int getRemainder() const  { return bufsize - pos; }

    bool hasError() const  { return errorFound; }
    void setError() const  { errorFound = true; }

    // read
    unsigned char readByte() const;  // returns 0 when not enough space
    void readNBytes(unsigned int length, void *dest) const;    // padding with 0 when not enough space
    uint16_t readUint16() const;    // ntoh, returns 0 when not enough space
    uint32_t readUint32() const;    // ntoh, returns 0 when not enough space
    MACAddress readMACAddress() const;
    IPv4Address readIPv4Address() const  { return IPv4Address(readUint32()); }
    IPv6Address readIPv6Address() const;

    // write
    void writeByte(unsigned char data);
    void writeByteTo(unsigned int position, unsigned char data);
    void writeNBytes(unsigned int length, const void *src);
    void writeNBytes(Buffer& inputBuffer, unsigned int length);

    void fillNBytes(unsigned int length, unsigned char data);
    void writeUint16(uint16_t data);    // hton
    void writeUint16To(unsigned int position, uint16_t data);    // hton
    void writeUint32(uint32_t data);    // hton
    void writeMACAddress(const MACAddress& addr);
    void writeIPv4Address(IPv4Address addr)  { writeUint32(addr.getInt()); }
    void writeIPv6Address(const IPv6Address &addr)  { for (int i = 0; i < 4; i++) { writeUint32(addr.words()[i]); } }

    // read/write
    void *accessNBytes(unsigned int length);    // returns nullptr when haven't got enough space

    //TODO bit manipulation???

    //DEPRECATED:
    unsigned char *_getBuf() const { return buf; }
    unsigned int _getBufSize() const { return bufsize; }
};

/**
 * class for data transfer from any serializers to subserializers
 * e.g. store IP addresses in IP serializers for TCP serializer
 */
class Context
{
  public:
    const void *l3AddressesPtr = nullptr;
    unsigned int l3AddressesLength = 0;
    bool throwOnSerializerNotFound = true;
    bool errorOccured = false;
};

/**
 * Converts between cPacket and binary (network byte order) packet.
 */
class INET_API SerializerBase : public cOwnedObject
{
  protected:
    /**
     * Serializes a cPacket for transmission on the wire.
     * Returns the length of data written into buffer.
     */
    virtual void serialize(const cPacket *pkt, Buffer &b, Context& context) = 0;

    /**
     * Puts a packet sniffed from the wire into an EtherFrame.
     */
    virtual cPacket *deserialize(Buffer &b, Context& context) = 0;

  public:
    SerializerBase(const char *name = nullptr) : cOwnedObject(name) {}

    static SerializerBase & lookupSerializer(const cPacket *pkt, Context& context, ProtocolGroup group, int id);
    static void lookupAndSerialize(const cPacket *pkt, Buffer &b, Context& context, ProtocolGroup group, int id, unsigned int trailerLength);
    void serializePacket(const cPacket *pkt, Buffer &b, Context& context);

    static SerializerBase & lookupDeserializer(Context& context, ProtocolGroup group, int id);
    static cPacket *lookupAndDeserialize(Buffer &b, Context& context, ProtocolGroup group, int id, unsigned int trailerLength);
    cPacket *deserializePacket(Buffer &b, Context& context);
};

class INET_API DefaultSerializer : public SerializerBase
{
  public:
    virtual void serialize(const cPacket *pkt, Buffer &b, Context& context);
    virtual cPacket *deserialize(Buffer &b, Context& context);
};

class INET_API ByteArraySerializer : public SerializerBase
{
  public:
    virtual void serialize(const cPacket *pkt, Buffer &b, Context& context);
    virtual cPacket *deserialize(Buffer &b, Context& context);
};

class INET_API SerializerRegistrationList : public cNamedObject, noncopyable
{
    public:
      static DefaultSerializer defaultSerializer;
      static ByteArraySerializer byteArraySerializer;

    protected:
        typedef std::pair<int, int> Key;
        typedef std::map<Key, SerializerBase*> KeyToSerializerMap;
        typedef std::map<std::string, SerializerBase*> StringToSerializerMap;
        KeyToSerializerMap keyToSerializerMap;
        StringToSerializerMap stringToSerializerMap;

    public:
        SerializerRegistrationList(const char *name) : cNamedObject(name, false) {}
        virtual ~SerializerRegistrationList();

        virtual void clear();

        /**
         * Adds an object to the container.
         */
        virtual void add(const char *name, int protocolGroup, int protocolId, SerializerBase *obj);

        /**
         * Returns the object with exactly the given group ID and protocol ID.
         * Returns NULL if not found.
         */
        virtual SerializerBase *lookup(int protocolGroup, int protocolId) const;

        /**
         * Returns the object with exactly the given name.
         * Returns NULL if not found.
         */
        virtual SerializerBase *lookup(const char *name) const;
};

INET_API extern SerializerRegistrationList serializers; ///< List of packet serializers (SerializerBase)

#define Register_Serializer(SERIALIZABLECLASSNAME, PROTOCOLGROUP, PROTOCOLID, SERIALIZERCLASSNAME)   \
        EXECUTE_ON_STARTUP(serializers.add(opp_typename(typeid(SERIALIZABLECLASSNAME)), \
                PROTOCOLGROUP, PROTOCOLID, new SERIALIZERCLASSNAME(#SERIALIZABLECLASSNAME)););

} // namespace serializer

} // namespace inet

#endif  // __INET_SERIALIZERBASE_H_

