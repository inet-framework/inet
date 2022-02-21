//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;

cplusplus {{
#include "inet/common/packet/Message.h"
}}

namespace inet;

class Message extends cMessage
{
    @existingClass;
    @descriptor(readonly);
    cObject *tags[] @getter(_getTag) @sizeGetter(getNumTags);
}

class Request extends Message
{
    @existingClass;
    @descriptor(readonly);
}

class Indication extends Request
{
    @existingClass;
    @descriptor(readonly);
}
