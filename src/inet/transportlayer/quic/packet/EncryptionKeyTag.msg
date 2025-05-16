//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.TagBase;

namespace inet::quic;

class EncryptionKeyTag extends TagBase
{
    string key;
    string iv;
    string hpkey;
}
