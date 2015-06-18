//
// Copyright (C) 2005 Christian Dankbar, Irene Ruengeler, Michael Tuexen
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_IPV4SERIALIZER_H
#define __INET_IPV4SERIALIZER_H

#include "inet/common/serializer/SerializerBase.h"
#include "inet/networklayer/ipv4/IPv4Datagram.h"

namespace inet {

namespace serializer {

class IPv4OptionSerializerBase : public cOwnedObject
{
  public:
    IPv4OptionSerializerBase(const char *name = nullptr) : cOwnedObject(name, false) {}
    virtual void serializeOption(const TLVOptionBase *option, Buffer &b, Context& c) = 0;
    virtual TLVOptionBase *deserializeOption(Buffer &b, Context& c) = 0;
};

class IPv4OptionDefaultSerializer : public IPv4OptionSerializerBase
{
  public:
    IPv4OptionDefaultSerializer(const char *name = nullptr) : IPv4OptionSerializerBase(name) {}
    void serializeOption(const TLVOptionBase *option, Buffer &b, Context& c) override;
    TLVOptionBase *deserializeOption(Buffer &b, Context& c) override;
};

class INET_API IPv4OptionSerializerRegistrationList : public cNamedObject, noncopyable
{
    protected:
        typedef int Key;
        typedef std::map<Key, IPv4OptionSerializerBase*> KeyToSerializerMap;
        KeyToSerializerMap keyToSerializerMap;
        static IPv4OptionDefaultSerializer defaultSerializer;

    public:
        IPv4OptionSerializerRegistrationList(const char *name) : cNamedObject(name, false) {}
        virtual ~IPv4OptionSerializerRegistrationList();

        virtual void clear();

        /**
         * Adds an object to the container.
         */
        virtual void add(int id, IPv4OptionSerializerBase *obj);

        /**
         * Returns the object with exactly the given ID.
         * Returns the defaultSerializer if not found.
         */
        virtual IPv4OptionSerializerBase *lookup(int id) const;
};

INET_API extern IPv4OptionSerializerRegistrationList ipv4OptionSerializers; ///< List of IPv4Option serializers (IPv4OptionSerializerBase)

#define Register_IPv4OptionSerializer(SERIALIZABLECLASSNAME, ID, SERIALIZERCLASSNAME)   \
        EXECUTE_ON_STARTUP(serializers.add(ID, new SERIALIZERCLASSNAME(#SERIALIZABLECLASSNAME)););

/**
 * Converts between IPv4Datagram and binary (network byte order) IPv4 header.
 */
class IPv4Serializer : public SerializerBase
{
  protected:
    virtual void serialize(const cPacket *pkt, Buffer &b, Context& context) override;
    virtual cPacket* deserialize(const Buffer &b, Context& context) override;

    void serializeOptions(const IPv4Datagram *dgram, Buffer& b, Context& c);
    void deserializeOptions(IPv4Datagram *dgram, Buffer &b, Context& c);

  public:
    IPv4Serializer(const char *name = nullptr) : SerializerBase(name) {}
};

} // namespace serializer

} // namespace inet

#endif // ifndef __INET_IPV4SERIALIZER_H

