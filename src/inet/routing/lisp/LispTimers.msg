//
// Copyright (C) 2013 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// LISP (Locator/ID Separation Protocol, RFC 6830) ported from the ANSAINET project.
// Original author: Vladimir Vesely (Brno University of Technology).
//

import inet.common.INETDefs;
import inet.networklayer.common.L3Address;

cplusplus {{
#include "inet/routing/lisp/LispEidPrefix.h"
}}

namespace inet::lisp;

class LispEidPrefix
{
    @existingClass;
    @opaque;
    @toString(.str());
}

//
// Self-messages driving the LISP control-plane timers.
//

message LispRegisterTimer
{
    L3Address serverAddress;
    uint16_t quickRegCycles;
}

message LispRequestTimer
{
    L3Address srcEid;
    L3Address dstEid;
    uint64_t previousNonce;
    uint8_t retryCount;
}

message LispRlocProbeTimer
{
    L3Address rlocAddr;
    LispEidPrefix eidPref;
    uint64_t previousNonce;
    uint8_t retryCount;
}

message LispSyncTimer
{
    L3Address setMember;
    LispEidPrefix eidPref;
    uint64_t previousNonce;
    uint8_t retryCount;
}

message LispMapEntryTimer
{
    LispEidPrefix eidPref;
}

message LispEtrTimer
{
    string siteName;
    L3Address etrAddr;
}
