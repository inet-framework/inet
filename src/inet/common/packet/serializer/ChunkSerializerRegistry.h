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
    mutable std::set<std::string> *usedSerializerClassRecorder = nullptr;

  public:
    ~ChunkSerializerRegistry();

    void registerSerializer(const std::type_info& typeInfo, const ChunkSerializer *serializer);

    const ChunkSerializer *getSerializer(const std::type_info& typeInfo) const;

    /**
     * Test/coverage support: the (demangled) names of all registered chunk types.
     */
    const std::set<std::string>& getRegisteredTypeNames() const { return registeredTypeNames; }

    /**
     * Test/coverage support: the (demangled) names of the distinct serializer *classes*
     * registered. Several chunk types typically share one serializer class, so this is a
     * coarser, code-level view than getRegisteredTypeNames(): a serializer class not among
     * the used ones (see setUsedSerializerClassRecorder) is not exercised by any type.
     */
    std::set<std::string> getRegisteredSerializerClassNames() const;

    /**
     * Test/coverage support: while a non-null recorder is installed, getSerializer()
     * inserts the demangled name of every dispatched chunk type into it (covering both
     * serialize and deserialize, top-level and nested). Pass nullptr to stop recording.
     * No effect in normal runs (recorder stays null).
     */
    void setUsedSerializerRecorder(std::set<std::string> *recorder) const { usedRecorder = recorder; }

    /**
     * Test/coverage support: like setUsedSerializerRecorder, but records the (demangled)
     * name of the dispatched serializer *class* (typeid of the serializer instance) rather
     * than the chunk type. This reveals serializer code that no type exercises. Pass
     * nullptr to stop recording.
     */
    void setUsedSerializerClassRecorder(std::set<std::string> *recorder) const { usedSerializerClassRecorder = recorder; }

    static ChunkSerializerRegistry& getInstance();
};

} // namespace

#endif

