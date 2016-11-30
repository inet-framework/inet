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

#include "inet/common/packet/SerializerRegistry.h"

namespace inet {

SerializerRegistry SerializerRegistry::globalRegistry;

void SerializerRegistry::registerSerializer(const std::type_info& typeInfo, const ChunkSerializer *serializer)
{
    assert(serializer != nullptr);
    serializers[&typeInfo] = serializer;
}

const ChunkSerializer *SerializerRegistry::getSerializer(const std::type_info& typeInfo) const
{
    auto it = serializers.find(&typeInfo);
    if (it != serializers.end())
        return it->second;
    else
        throw cRuntimeError("Cannot find serializer for %s", typeInfo.name());
}

} // namespace
