//
// Protocol Test Framework for INET -- read and write a "protocol.field" path, and evaluate
// an expression over a packet or an object.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "PacketField.h"

#include <cctype>
#include <cstring>
#include <map>
#include <memory>
#include <sstream>
#include <vector>

#include "inet/common/Protocol.h"
#include "inet/common/packet/dissector/PacketDissector.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#ifdef INET_WITH_IPv6
#include "inet/networklayer/ipv6/Ipv6Header.h"
#endif
#include "inet/transportlayer/tcp_common/TcpChecksumInsertionHook.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"

namespace inet {
namespace protocoltest {

// Dissects a packet and records the chunk for each protocol / chunk class name.
class FieldChunkCollector final : public PacketDissector::ICallback
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

    void dissect(const Packet *packet)
    {
        PacketDissector dissector(ProtocolDissectorRegistry::getInstance(), *this);
        dissector.dissectPacket(const_cast<Packet *>(packet));
    }

    // The chunk of a protocol name ("tcp") or of a chunk class name ("TcpHeader"); null when
    // the packet has none.
    Ptr<const Chunk> find(const std::string& name) const
    {
        if (!name.empty() && std::isupper((unsigned char)name[0])) {
            auto it = byClass.find(name);
            return it != byClass.end() ? it->second : nullptr;
        }
        else if (auto protocol = Protocol::findProtocol(name.c_str())) {
            auto it = byProtocol.find(protocol);
            return it != byProtocol.end() ? it->second : nullptr;
        }
        return nullptr;
    }
};

// Splits "protocol.field" and returns the chunk and the index of the field in its descriptor.
static Ptr<const Chunk> findFieldChunk(const Packet *packet, const std::string& path, int& field)
{
    auto dot = path.find('.');
    if (dot == std::string::npos)
        throw cRuntimeError("ProtocolTest: field path '%s' must be 'protocol.field'", path.c_str());
    std::string protoName = path.substr(0, dot);
    std::string fieldName = path.substr(dot + 1);

    FieldChunkCollector collector;
    collector.dissect(packet);
    Ptr<const Chunk> chunk = collector.find(protoName);
    if (chunk == nullptr)
        throw cRuntimeError("ProtocolTest: protocol '%s' not present in packet (field path '%s')",
                protoName.c_str(), path.c_str());

    field = chunk->getDescriptor()->findField(fieldName.c_str());
    if (field == -1)
        throw cRuntimeError("ProtocolTest: field '%s' not found on '%s'", fieldName.c_str(), chunk->getClassName());
    return chunk;
}

cValue evalPacketField(const Packet *packet, const std::string& path)
{
    int field;
    auto chunk = findFieldChunk(packet, path, field);
    return chunk->getDescriptor()->getFieldValue(toAnyPtr(const_cast<Chunk *>(chunk.get())), field, 0);
}

static std::string formatCaptureValue(const cValue& value)
{
    // A value that carries a unit keeps it. intValue() and doubleValue() throw on such a
    // value -- "Attempt to use the value '24B' as a dimensionless number" -- and the
    // expression engine reads the quantity form, so 24B substitutes and compares correctly.
    const char *unit = value.getUnit();
    bool hasUnit = unit != nullptr && *unit != '\0';
    switch (value.getType()) {
        case cValue::INT: return hasUnit ? value.str() : std::to_string(value.intValue());
        case cValue::BOOL: return value.boolValue() ? "true" : "false";
        case cValue::DOUBLE: {
            if (hasUnit)
                return value.str();
            std::ostringstream os; os << value.doubleValue(); return os.str();
        }
        default: return value.str();
    }
}

bool substituteCaptures(std::string& text, const CaptureStore& captures)
{
    for (auto& capture : captures) {
        std::string placeholder = "{" + capture.first + "}";
        // Only convert a capture this expression names. Converting every stored capture
        // made one step fail on a capture that belonged to another step.
        if (text.find(placeholder) == std::string::npos)
            continue;
        std::string value = formatCaptureValue(capture.second);
        size_t pos;
        while ((pos = text.find(placeholder)) != std::string::npos)
            text.replace(pos, placeholder.size(), value);
    }
    return text.find('{') == std::string::npos;
}

// The value of a field of the object that a value points to: a cObject, or a struct that a
// field of a cObject holds.
static cValue readObjectField(const cValue& object, const char *name, int index)
{
    if (object.getType() != cValue::POINTER || object.isNullptr())
        throw cRuntimeError("ProtocolTest: '%s' is not a field of %s", name, object.str().c_str());
    any_ptr ptr = object.pointerValue();
    cClassDescriptor *descriptor = nullptr;
    if (ptr.contains<cObject>())
        descriptor = fromAnyPtr<cObject>(ptr)->getDescriptor();
    else {
        std::string typeName = ptr.pointerTypeName();
        while (!typeName.empty() && (typeName.back() == '*' || typeName.back() == ' '))
            typeName.pop_back();
        descriptor = cClassDescriptor::getDescriptorFor(typeName.c_str());
    }
    if (descriptor == nullptr)
        throw cRuntimeError("ProtocolTest: '%s' has no class descriptor, so its field '%s' cannot be read",
                ptr.pointerTypeName(), name);
    int field = descriptor->findField(name);
    if (field == -1)
        throw cRuntimeError("ProtocolTest: field '%s' not found on '%s'", name, descriptor->getName());
    return descriptor->getFieldValue(ptr, field, index);
}

// Resolves the names of an expression: the chunks of a packet, `length`, `value`, or the
// fields of a root object.
class ExpressionResolver : public cDynamicExpression::IResolver
{
  protected:
    const Packet *packet = nullptr;
    std::shared_ptr<FieldChunkCollector> chunks;
    cValue eventValue;
    cObject *root = nullptr;

  public:
    ExpressionResolver(const Packet *packet, const cValue& eventValue, cObject *root) :
        packet(packet), eventValue(eventValue), root(root)
    {
        if (packet != nullptr) {
            chunks = std::make_shared<FieldChunkCollector>();
            chunks->dissect(packet);
        }
    }

    virtual IResolver *dup() const override { return new ExpressionResolver(*this); }

    virtual cValue readVariable(cExpression::Context *context, const char *name) override
    {
        return readVariable(context, name, 0);
    }

    virtual cValue readVariable(cExpression::Context *context, const char *name, intval_t index) override
    {
        if (root != nullptr)
            return readObjectField(cValue(root), name, index);
        if (!strcmp(name, "value") && eventValue.isSet())
            return eventValue;
        if (packet != nullptr) {
            if (!strcmp(name, "length"))
                return cValue((intval_t)packet->getByteLength(), "B");
            if (!strcmp(name, "pk"))
                return cValue(const_cast<Packet *>(packet));
            if (auto chunk = chunks->find(name))
                return cValue(const_cast<Chunk *>(chunk.get()));
        }
        throw cRuntimeError("ProtocolTest: '%s' has no value here", name);
    }

    virtual cValue readMember(cExpression::Context *context, const cValue& object, const char *name) override
    {
        return readObjectField(object, name, 0);
    }

    virtual cValue readMember(cExpression::Context *context, const cValue& object, const char *name, intval_t index) override
    {
        return readObjectField(object, name, index);
    }
};

static cValue evaluate(const std::string& expression, const CaptureStore& captures, ExpressionResolver *resolver)
{
    std::unique_ptr<ExpressionResolver> owner(resolver);
    std::string text = expression;
    if (!substituteCaptures(text, captures))
        throw cRuntimeError("ProtocolTest: the expression '%s' names a capture that is not set yet", expression.c_str());
    cDynamicExpression dynamicExpression;
    dynamicExpression.parse(text.c_str(), owner.release());
    return dynamicExpression.evaluate();
}

cValue evalPacketExpression(const std::string& expression, const CaptureStore& captures, const Packet *packet, const cValue& value)
{
    return evaluate(expression, captures, new ExpressionResolver(packet, value, nullptr));
}

cValue evalObjectExpression(const std::string& expression, const CaptureStore& captures, cObject *object)
{
    return evaluate(expression, captures, new ExpressionResolver(nullptr, cValue(), object));
}

// Repairs the checksums that the change of a field makes wrong. Walks the chunks of the
// packet from the front up to the first transport header: an IPv4 header computes its
// checksum again, and a TCP header with a computed checksum too, over the pseudo header of
// the network header before it and the data of the datagram behind it. A declared checksum
// stays declared. The walk stops at the transport header, so a header that an ICMP error
// quotes keeps the checksum that its sender gave it.
static void repairChecksums(Packet *packet)
{
    std::vector<Ptr<Chunk>> front;
    const Protocol *networkProtocol = nullptr;
    L3Address srcAddress, destAddress;
    b datagramRest = b(0);   // the length of the datagram behind its network header
    while (packet->getDataLength() > b(0)) {
        const auto& chunk = packet->peekAtFront<Chunk>();
        if (dynamicPtrCast<const Ipv4Header>(chunk) != nullptr && networkProtocol == nullptr) {
            auto header = packet->removeAtFront<Ipv4Header>();
            header->updateChecksum();
            networkProtocol = &Protocol::ipv4;
            srcAddress = header->getSrcAddress();
            destAddress = header->getDestAddress();
            datagramRest = header->getTotalLengthField() - header->getChunkLength();
            front.push_back(header);
        }
#ifdef INET_WITH_IPv6
        else if (dynamicPtrCast<const Ipv6Header>(chunk) != nullptr && networkProtocol == nullptr) {
            auto header = packet->removeAtFront<Ipv6Header>();
            networkProtocol = &Protocol::ipv6;
            srcAddress = header->getSrcAddress();
            destAddress = header->getDestAddress();
            // the payload length counts the extension headers, which this chunk holds
            datagramRest = header->getPayloadLength() - (header->getChunkLength() - B(40));
            front.push_back(header);
        }
#endif
        else if (dynamicPtrCast<const tcp::TcpHeader>(chunk) != nullptr && networkProtocol != nullptr) {
            auto header = packet->removeAtFront<tcp::TcpHeader>();
            if (header->getChecksumMode() == CHECKSUM_COMPUTED) {
                // The frame can carry a link-layer trailer behind the datagram, which the
                // checksum does not cover.
                header->setChecksum(0);
                auto data = packet->peekAt(b(0), datagramRest - header->getChunkLength(), Chunk::PF_ALLOW_EMPTY);
                header->setChecksum(tcp::TcpChecksumInsertionHook::computeChecksum(networkProtocol, srcAddress, destAddress, header, data));
            }
            front.push_back(header);
            break;
        }
        else if (networkProtocol != nullptr)
            break;
        else
            front.push_back(packet->removeAtFront<Chunk>(chunk->getChunkLength()));
    }
    for (auto it = front.rbegin(); it != front.rend(); ++it)
        packet->insertAtFront(*it);
}

void setPacketField(Packet *packet, const std::string& path, const cValue& value)
{
    int field;
    auto target = findFieldChunk(packet, path, field);
    // Take the chunks off the front up to the one that the dissector showed, change a copy
    // of it, and put everything back.
    std::vector<Ptr<Chunk>> front;
    while (packet->getDataLength() > b(0) && packet->peekAtFront<Chunk>().get() != target.get()) {
        const auto& chunk = packet->peekAtFront<Chunk>();
        front.push_back(packet->removeAtFront<Chunk>(chunk->getChunkLength()));
    }
    bool found = packet->getDataLength() > b(0);
    if (found) {
        auto chunk = packet->removeAtFront<Chunk>(target->getChunkLength());
        chunk->getDescriptor()->setFieldValue(toAnyPtr(chunk.get()), field, 0, value);
        packet->insertAtFront(chunk);
    }
    for (auto it = front.rbegin(); it != front.rend(); ++it)
        packet->insertAtFront(*it);
    if (!found)
        throw cRuntimeError("ProtocolTest: cannot write '%s': the packet holds '%s' in another form than the dissector shows",
                path.c_str(), target->getClassName());
    repairChecksums(packet);
}

} // namespace protocoltest
} // namespace inet
