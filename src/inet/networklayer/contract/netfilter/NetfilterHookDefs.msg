//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.Packet;

cplusplus {{
#include <functional>
}}

namespace inet::NetfilterHook;

enum NetfilterType {
    PREROUTING = 0;
    LOCALIN = 1;
    FORWARD = 2;
    POSTROUTING = 3;
    LOCALOUT = 4;
    __FIRST_HOOK = 0;
    __NUM_HOOK_TYPES = 5;
};

enum NetfilterResult {
    ACCEPT = 0; ///< allows the datagram to pass to the next hook
    DROP = 1;   ///< doesn't allow the datagram to pass to the next hook, will be deleted
    QUEUE = 2;  ///< queues the datagram for later re-injection (e.g. when route discovery completes)
    STOLEN = 3; ///< doesn't allow datagram to pass to next hook, but won't be deleted
    REPEAT = 4; ///< packet accepted, repeat current hook
    STOP = 5;   ///< packet accepted, stop hook iteration
};

cplusplus {{
    typedef std::function<NetfilterHook::NetfilterResult(Packet*)> NetfilterHandler;
}}

class NetfilterHandler {
    @existingClass;
    @toString(""/* $ */);
}
