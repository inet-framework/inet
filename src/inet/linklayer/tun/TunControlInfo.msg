//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004,2011 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;

namespace inet;

enum TunCommandCode
{
    TUN_C_OPEN  = 1;
    TUN_C_CLOSE = 2;
    TUN_C_DESTROY = 3;
    TUN_C_DATA = 4;
}

enum TunSocketIndication
{
    TUN_I_CLOSED = 2;
    TUN_I_DATA = 4;
}

class TunControlInfo extends cObject
{
}

class TunOpenCommand extends TunControlInfo
{
}

class TunCloseCommand extends TunControlInfo
{
}

class TunDestroyCommand extends TunControlInfo
{
}

class TunSocketClosedIndication extends TunControlInfo
{
}
