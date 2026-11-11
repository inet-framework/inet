// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_MESHDATAATTEMPT_H
#define INET_MESHDATAATTEMPT_H

#include "inet/linklayer/common/MacAddress.h"
#include "inet/physicallayer/wireless/ieee80211/mode/IIeee80211Mode.h"

namespace inet::ieee80211 {

/** Synchronous fact about one actual unicast DATA MPDU attempt, emitted by HCF.
 * Failure means a missing immediate BA bit or an actual ACK/BA wait timeout;
 * rejected responses and interruptions do not supply an estimator sample.
 * terminalTimeout retains its ordinary ACK-timeout meaning.
 */
class INET_API MeshDataAttempt : public cObject
{
  public:
    const MacAddress peer;
    const physicallayer::IIeee80211Mode *const mode;
    const bool acknowledged;
    const bool terminalTimeout;
    MeshDataAttempt(MacAddress peer, const physicallayer::IIeee80211Mode *mode, bool acknowledged, bool terminalTimeout) :
        peer(peer), mode(mode), acknowledged(acknowledged), terminalTimeout(terminalTimeout) {}
};

/** Cleanup-complete failure of an aggregate DATA/recovery exchange. This is
 * separate from per-DATA error sampling and does not reinterpret a BA zero as
 * an ACK timeout. */
class INET_API MeshTerminalRecovery : public cObject
{
  public:
    enum Reason { DATA_RETRY_LIMIT = 0, BAR_WAIT_LIMIT = 1 };
    const MacAddress peer;
    const int tid;
    const int retiredMembers;
    const Reason reason;
    MeshTerminalRecovery(MacAddress peer, int tid, int retiredMembers, Reason reason) :
        peer(peer), tid(tid), retiredMembers(retiredMembers), reason(reason) {}
};

} // namespace inet::ieee80211
#endif
