//
// Copyright (C) 2019 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;

namespace inet;

enum RtpAppSelfMsgKinds {
    RTPAPP_ENTER_SESSION = 1;
    RTPAPP_START_TRANSMISSION = 2;
    RTPAPP_STOP_TRANSMISSION = 3;
    RTPAPP_LEAVE_SESSION = 4;
};
