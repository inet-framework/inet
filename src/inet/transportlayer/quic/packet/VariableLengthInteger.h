//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_TRANSPORTLAYER_QUIC_PACKET_VARIABLELENGTHINTEGER_H_
#define INET_TRANSPORTLAYER_QUIC_PACKET_VARIABLELENGTHINTEGER_H_

#include "VariableLengthInteger_m.h"
#include <stddef.h>

namespace inet {
namespace quic {

size_t getVariableLengthIntegerSize(VariableLengthInteger i);

}  // namespace quic
}  // namespace inet

#endif /* INET_TRANSPORTLAYER_QUIC_PACKET_VARIABLELENGTHINTEGER_H_ */
