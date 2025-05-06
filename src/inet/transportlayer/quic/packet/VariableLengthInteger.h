//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_TRANSPORTLAYER_QUIC_PACKET_VARIABLELENGTHINTEGER_H_
#define INET_TRANSPORTLAYER_QUIC_PACKET_VARIABLELENGTHINTEGER_H_

#include "VariableLengthInteger_m.h"
#include <stddef.h>
#include "inet/common/MemoryInputStream.h"
#include "inet/common/MemoryOutputStream.h"

namespace inet {
namespace quic {

size_t getVariableLengthIntegerSize(VariableLengthInteger i);

// Helper methods for serializing and deserializing variable length integers
void serializeVariableLengthInteger(MemoryOutputStream& stream, VariableLengthInteger value);

VariableLengthInteger deserializeVariableLengthInteger(MemoryInputStream& stream);

}  // namespace quic
}  // namespace inet

#endif /* INET_TRANSPORTLAYER_QUIC_PACKET_VARIABLELENGTHINTEGER_H_ */
