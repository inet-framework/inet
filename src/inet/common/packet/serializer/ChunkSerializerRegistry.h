//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CHUNKSERIALIZERREGISTRY_H
#define __INET_CHUNKSERIALIZERREGISTRY_H

#include "inet/common/packet/serializer/ChunkSerializer.h"

namespace inet {

#define Register_Serializer(TYPE, CLASSNAME)    EXECUTE_PRE_NETWORK_SETUP(ChunkSerializerRegistry::getInstance().registerSerializer(typeid(TYPE), new CLASSNAME()));

class INET_API ChunkSerializerRegistry
{
  protected:
    std::map<const std::type_info *, const ChunkSerializer *> serializers;

  public:
    ~ChunkSerializerRegistry();

    void registerSerializer(const std::type_info& typeInfo, const ChunkSerializer *serializer);

    const ChunkSerializer *getSerializer(const std::type_info& typeInfo) const;

    static ChunkSerializerRegistry& getInstance();
};

} // namespace

#endif

