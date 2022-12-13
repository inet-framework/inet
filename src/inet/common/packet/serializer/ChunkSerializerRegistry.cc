//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"

namespace inet {

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

ChunkSerializerRegistry& ChunkSerializerRegistry::getInstance()
{
    static int handle = cSimulationOrSharedDataManager::registerSharedVariableName("inet::ChunkSerializerRegistry::instance");
    return getSimulationOrSharedDataManager()->getSharedVariable<ChunkSerializerRegistry>(handle);
}

} // namespace

