//
// Copyright (C) 2015 Irene Ruengeler
//
// SPDX-License-Identifier: GPL-3.0-or-later
//

import inet.common.INETDefs;

message PacketDrillInfo
{
    simtime_t scriptTime;
    simtime_t scriptTimeEnd;
    simtime_t offset;
    uint16_t timeType;
    simtime_t liveTime;
}
