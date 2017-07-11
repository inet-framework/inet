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

#ifndef __INET_CHUNKSERIALIZERREGISTRY_H_
#define __INET_CHUNKSERIALIZERREGISTRY_H_

#include "inet/common/packet/serializer/ChunkSerializer.h"

namespace inet {

#define Register_Serializer(TYPE, CLASSNAME) EXECUTE_ON_STARTUP(ChunkSerializerRegistry::globalRegistry.registerSerializer(typeid(TYPE), new CLASSNAME()));

class INET_API ChunkSerializerRegistry
{
  public:
    static ChunkSerializerRegistry globalRegistry;

  protected:
    std::map<const std::type_info *, const ChunkSerializer *> serializers;

  public:
    ~ChunkSerializerRegistry();

    void registerSerializer(const std::type_info& typeInfo, const ChunkSerializer *serializer);

    const ChunkSerializer *getSerializer(const std::type_info& typeInfo) const;
};

} // namespace

#endif // #ifndef __INET_CHUNKSERIALIZERREGISTRY_H_

