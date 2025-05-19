//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "VariableLengthInteger.h"

namespace inet {
namespace quic {

size_t getVariableLengthIntegerSize(VariableLengthInteger i) {
    if (i < 64ull) {
        return 1;
    }
    if (i < (1ull << 14)) {
        return 2;
    }
    if (i < (1ull << 30)) {
        return 4;
    }
    return 8;
}

}  // namespace quic
}  // namespace inet

