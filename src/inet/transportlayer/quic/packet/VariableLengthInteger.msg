//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

namespace inet::quic;

cplusplus{{
    typedef uint64_t VariableLengthInteger;
}}

class VariableLengthInteger {
    @existingClass;
    @primitive;
    @fromString(string2uint64($));
    @toString(uint642string($));
    @defaultValue(0);
    @toValue((omnetpp::intval_t)($));
    @fromValue(omnetpp::checked_int_cast<uint64_t>($.intValue()));
}
