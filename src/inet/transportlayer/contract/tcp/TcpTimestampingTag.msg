//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.TagBase;

namespace inet;

//
// Indication tag TCP attaches to a TCP_I_DATA packet (Workstream H3,
// SO_TIMESTAMPING/SCM_TIMESTAMPING) when the connection has timestamping
// enabled (see ~TcpSocket::setTimestamping), recording when TCP delivered
// this data to the application (simTime() at the point of delivery in
// sendAvailableDataToApp()/process_READ_REQUEST()).
//
// This is an INET-native simplification of Linux's SCM_TIMESTAMPING: a
// single delivery-time stamp, not Linux's separate software/hardware/ACK
// timestamp triad, and not a reconstruction of each byte's original
// segment-arrival time across out-of-order reassembly -- for in-order,
// autoRead delivery (the common case) delivery time is effectively
// simultaneous with segment processing, making it a faithful proxy for
// "when received".
//
class TcpRxTimestampInd extends TagBase
{
    simtime_t timestamp = simTime(); // stamped at construction time, at the delivery call site
}
