//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_COAP_COAPHEADERSERIALIZER_H
#define __INET_COAP_COAPHEADERSERIALIZER_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "inet/applications/coap/CoapHeader_m.h"
#include "inet/common/packet/serializer/FieldsChunkSerializer.h"

namespace inet {
namespace coap {

class INET_API CoapHeaderSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    CoapHeaderSerializer() : FieldsChunkSerializer() {}
    friend INET_API bool parseCoapDatagramBytes(const std::vector<uint8_t>& bytes, Ptr<CoapHeader>& header, std::vector<uint8_t>& payload);
};

// Encodes and decodes CoAP code classes and details.
inline uint8_t encodeCoapCode(uint8_t codeClass, uint8_t codeDetail)
{
    return static_cast<uint8_t>(((codeClass & 0x07U) << 5U) | (codeDetail & 0x1FU));
}

inline uint8_t getCoapCodeClass(uint8_t code)
{
    return static_cast<uint8_t>((code >> 5U) & 0x07U);
}

inline uint8_t getCoapCodeDetail(uint8_t code)
{
    return static_cast<uint8_t>(code & 0x1FU);
}

inline bool isCoapCodeReservedClass(uint8_t code)
{
    uint8_t cls = getCoapCodeClass(code);
    return cls == 1 || cls == 6 || cls == 7;
}

inline bool isCoapOptionCritical(uint16_t number)
{
    return (number & 1U) != 0;
}

inline bool isCoapOptionUnsafe(uint16_t number)
{
    return (number & 2U) != 0;
}

inline bool isCoapOptionNoCacheKey(uint16_t number)
{
    return (number & 0x1EU) == 0x1CU;
}

INET_API std::vector<uint8_t> encodeCoapUintValue(uint32_t value);
INET_API bool decodeCoapUintValue(const CoapOption& option, uint32_t& value, size_t maxBytes = 4);

INET_API void addCoapOption(const Ptr<CoapHeader>& header, uint16_t number, const std::vector<uint8_t>& value);
INET_API void addCoapOptionString(const Ptr<CoapHeader>& header, uint16_t number, const std::string& value);
INET_API void addCoapOptionUint(const Ptr<CoapHeader>& header, uint16_t number, uint32_t value);

INET_API B computeCoapHeaderLength(const CoapHeader& header);
INET_API void finalizeCoapHeader(const Ptr<CoapHeader>& header, bool hasPayload);
INET_API bool parseCoapDatagramBytes(const std::vector<uint8_t>& bytes, Ptr<CoapHeader>& header, std::vector<uint8_t>& payload);
INET_API bool validateCoapHeaderStructure(const CoapHeader& header, size_t payloadSize);

} // namespace coap
} // namespace inet

#endif
