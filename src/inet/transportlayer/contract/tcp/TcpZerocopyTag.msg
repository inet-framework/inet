//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.TagBase;

namespace inet;

//
// Request tag an application attaches to a TCP_C_SEND data packet (Workstream
// H2, MSG_ZEROCOPY) to ask for a completion notification once this SEND's
// data has actually been transmitted. TCP assigns IDs itself, sequentially
// per connection starting at 0 for the first zerocopy-marked SEND -- mirroring
// Linux's own SO_ZEROCOPY id assignment (see setsockopt(2)/SO_EE_ORIGIN_
// ZEROCOPY), so the app can predict the id of its own Nth zerocopy SEND
// without TCP needing to echo it back synchronously.
//
// INET has no real zero-copy data path (packet payloads are always plain C++
// objects, not user-space memory pages TCP could avoid copying) -- this
// models only the completion-notification CONTRACT Linux's MSG_ZEROCOPY
// exposes to the app (see ~TcpZerocopyCompletionInfo, TCP_I_ZEROCOPY_
// COMPLETION), not a literal zero-copy send path. "Transmitted" here means
// TCP has put the corresponding bytes on the wire -- not that the peer has
// acknowledged them, matching Linux's own timing (the kernel signals
// completion once it is done reading the user buffer, not once the data is
// acked).
//
// @see ~TcpSocket::sendZerocopy
//
class TcpSendZerocopyReq extends TagBase
{
}
