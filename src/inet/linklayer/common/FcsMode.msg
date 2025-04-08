//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;

cplusplus {{
#include "inet/common/checksum/ChecksumMode_m.h"
}}

namespace inet;

cplusplus {{
typedef ChecksumMode FcsMode;
inline FcsMode parseFcsMode(const char *modeString, bool allowDisabled=true) { return parseChecksumMode(modeString, allowDisabled); }
}}

enum FcsMode;