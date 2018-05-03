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

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"

namespace inet {

ChunkSerializerRegistry ChunkSerializerRegistry::globalRegistry;

ChunkSerializerRegistry::~ChunkSerializerRegistry()
{
    for (auto it : serializers)
        delete it.second;
}

void ChunkSerializerRegistry::registerSerializer(const std::type_info& typeInfo, const ChunkSerializer *serializer)
{
    CHUNK_CHECK_USAGE(serializer != nullptr, "invalid serializer");
    serializers[&typeInfo] = serializer;
}

const ChunkSerializer *ChunkSerializerRegistry::getSerializer(const std::type_info& typeInfo) const
{
    auto it = serializers.find(&typeInfo);
    if (it != serializers.end())
        return it->second;
    else
        throw cRuntimeError("Cannot find serializer for %s", opp_typename(typeInfo));
}

} // namespace
