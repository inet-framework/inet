//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.TagBase;

namespace inet;

//
// Announces which (re)transmission attempt a packet represents, counting from 0 for the first
// transmission. Set by a retransmission element (e.g. Resending) and read by elements whose
// behaviour depends on the attempt number, such as ExponentialBackoff.
//
class TransmissionAttemptReq extends TagBase
{
    int attempt = 0;
}
