//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.TagBase;
import inet.networklayer.contract.netfilter.NetfilterHookDefs;

namespace inet::NetfilterHook;

class NetfilterQueuedDatagramTag extends TagBase
{
    int hookId;
    int priority;
    NetfilterHook::NetfilterHandler *handler;
};
