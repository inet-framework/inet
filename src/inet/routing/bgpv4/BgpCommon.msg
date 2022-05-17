//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;

namespace inet::bgp;

enum BgpSessionType
{
    IGP = 0;
    EGP = 1;
    INCOMPLETE = 2;
}

enum BgpPathSegmentType
{
    AS_SET = 1;
    AS_SEQUENCE = 2;
}

enum BgpSelfMsgKind
{
    START_EVENT_KIND = 81;
    CONNECT_RETRY_KIND = 82;
    HOLD_TIME_KIND = 83;
    KEEP_ALIVE_KIND = 89;
}

enum BgpProcessResult
{
    RESULT0 = 0;
    ROUTE_DESTINATION_CHANGED = 90;
    NEW_ROUTE_ADDED = 91;
    NEW_SESSION_ESTABLISHED = 92;
    ASLOOP_NO_DETECTED = 93;
    ASLOOP_DETECTED = 94;
}
