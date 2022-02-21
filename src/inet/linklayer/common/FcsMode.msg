//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;

namespace inet;

enum FcsMode
{
    FCS_MODE_UNDEFINED = -1; // mode is undefined, FCS is not yet set, not serializable
    FCS_DISABLED = 0; // FCS is disabled (not set), serializable
    FCS_DECLARED_CORRECT = 1; // FCS is correctly set without the actual value, not serializable
    FCS_DECLARED_INCORRECT = 2; // the FCS is incorrectly set without the actual value, not serializable
    FCS_COMPUTED = 3; // FCS is set to a value that is potentially incorrect, serializable
}

cplusplus {{
INET_API FcsMode parseFcsMode(const char *fcsModeString);
}}
