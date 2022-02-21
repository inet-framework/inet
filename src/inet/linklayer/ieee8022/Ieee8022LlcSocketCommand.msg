//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.socket.SocketCommand;

namespace inet;

class Ieee8022LlcSocketOpenCommand extends SocketCommandBase
{
    int localSap;
}
