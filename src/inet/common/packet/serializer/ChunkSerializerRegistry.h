//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CHUNKSERIALIZERREGISTRY_H
#define __INET_CHUNKSERIALIZERREGISTRY_H

#include <set>
#include <string>
#include <typeindex>
#include <unordered_map>

#include "inet/common/packet/serializer/ChunkSerializer.h"

namespace inet {

#define Register_Serializer(TYPE, CLASSNAME)    EXECUTE_PRE_NETWORK_SETUP(::inet::ChunkSerializerRegistry::getInstance().registerSerializer(typeid(TYPE), new CLASSNAME()));

class INET_API ChunkSerializerRegistry
{
  protected:
    std::unordered_map<std::type_index, const ChunkSerializer *> serializers;
    std::set<std::string> registeredTypeNames;
    mutable std::set<std::string> *usedRecorder = nullptr;

  public:
    ~ChunkSerializerRegistry();

    void registerSerializer(const std::type_info& typeInfo, const ChunkSerializer *serializer);

    const ChunkSerializer *getSerializer(const std::type_info& typeInfo) const;

    /**
     * Test/coverage support: the (demangled) names of all registered chunk types.
     */
    const std::set<std::string>& getRegisteredTypeNames() const { return registeredTypeNames; }

    /**
     * Test/coverage support: while a non-null recorder is installed, getSerializer()
     * inserts the demangled name of every dispatched chunk type into it (covering both
     * serialize and deserialize, top-level and nested). Pass nullptr to stop recording.
     * No effect in normal runs (recorder stays null).
     */
    void setUsedSerializerRecorder(std::set<std::string> *recorder) const { usedRecorder = recorder; }

    static ChunkSerializerRegistry& getInstance();
};

} // namespace

#endif

