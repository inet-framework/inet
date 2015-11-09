//
// Copyright (C) 2013 Irene Ruengeler
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

#ifndef __INET_IPV6SERIALIZER_H
#define __INET_IPV6SERIALIZER_H

#include "inet/common/serializer/SerializerBase.h"

namespace inet {

namespace serializer {

/**
 * Converts between IPv6Datagram and binary (network byte order) IPv6 header.
 */
class INET_API IPv6Serializer : public SerializerBase
{
  protected:
    virtual void serialize(const cPacket *pkt, Buffer &b, Context& context) override;
    virtual cPacket* deserialize(const Buffer &b, Context& context) override;

  public:
    IPv6Serializer(const char *name = nullptr) : SerializerBase(name) {}
};

} // namespace serializer

} // namespace inet

#endif // ifndef __INET_IPV6SERIALIZER_H

