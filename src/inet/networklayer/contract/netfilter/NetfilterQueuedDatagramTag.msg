//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.TagBase;
import inet.networklayer.ipv4modular.IIpv4HookManager;

namespace inet;

class Ipv4QueuedDatagramTag extends TagBase
{
    int hookId;
    int priority;
    Ipv4Hook::NetfilterHandler *handler;
};
