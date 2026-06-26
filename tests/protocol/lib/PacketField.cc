//
// Protocol Test Framework for INET -- evaluate a "protocol.field" path to a value.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "PacketField.h"

#include <cctype>
#include <cstring>
#include <map>

#include "inet/common/Protocol.h"
#include "inet/common/packet/dissector/PacketDissector.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"

namespace inet {
namespace protocoltest {

// Dissects a packet and records the chunk for each protocol / chunk class name.
class FieldChunkCollector : public PacketDissector::ICallback
{
  public:
    std::map<const Protocol *, Ptr<const Chunk>> byProtocol;
    std::map<std::string, Ptr<const Chunk>> byClass;

    virtual bool shouldDissectProtocolDataUnit(const Protocol *protocol) override { return true; }
    virtual void startProtocolDataUnit(const Protocol *protocol) override {}
    virtual void endProtocolDataUnit(const Protocol *protocol) override {}
    virtual void markIncorrect() override {}
    virtual void visitChunk(const Ptr<const Chunk>& chunk, const Protocol *protocol) override
    {
        if (protocol != nullptr)
            byProtocol[protocol] = chunk;
        const char *className = chunk->getClassName();
        if (auto colon = strrchr(className, ':'))
            className = colon + 1;
        byClass[className] = chunk;
    }
};

cValue evalPacketField(const Packet *packet, const std::string& path)
{
    auto dot = path.find('.');
    if (dot == std::string::npos)
        throw cRuntimeError("ProtocolTest: field path '%s' must be 'protocol.field'", path.c_str());
    std::string protoName = path.substr(0, dot);
    std::string fieldName = path.substr(dot + 1);

    FieldChunkCollector collector;
    PacketDissector dissector(ProtocolDissectorRegistry::getInstance(), collector);
    dissector.dissectPacket(const_cast<Packet *>(packet));

    Ptr<const Chunk> chunk = nullptr;
    if (!protoName.empty() && std::isupper((unsigned char)protoName[0])) {
        auto it = collector.byClass.find(protoName);
        if (it != collector.byClass.end())
            chunk = it->second;
    }
    else if (auto protocol = Protocol::findProtocol(protoName.c_str())) {
        auto it = collector.byProtocol.find(protocol);
        if (it != collector.byProtocol.end())
            chunk = it->second;
    }
    if (chunk == nullptr)
        throw cRuntimeError("ProtocolTest: protocol '%s' not present in packet (field path '%s')",
                protoName.c_str(), path.c_str());

    auto descriptor = chunk->getDescriptor();
    int field = descriptor->findField(fieldName.c_str());
    if (field == -1)
        throw cRuntimeError("ProtocolTest: field '%s' not found on '%s'", fieldName.c_str(), chunk->getClassName());
    return descriptor->getFieldValue(toAnyPtr(const_cast<Chunk *>(chunk.get())), field, 0);
}

} // namespace protocoltest
} // namespace inet
