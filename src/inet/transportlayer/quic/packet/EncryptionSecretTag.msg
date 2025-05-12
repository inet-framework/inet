//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.TagBase;

namespace inet::quic;

class EncryptionSecretTag extends TagBase
{
    string secret; // the key, initialization vector, and nonce are derived from this and the packet number
}
