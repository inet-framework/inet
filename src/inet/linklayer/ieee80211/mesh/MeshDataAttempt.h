// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __INET_MESHDATAATTEMPT_H
#define __INET_MESHDATAATTEMPT_H

#include "inet/linklayer/common/MacAddress.h"
#include "inet/physicallayer/wireless/ieee80211/mode/IIeee80211Mode.h"

namespace inet {
namespace ieee80211 {

/** Synchronous fact about one actual unicast DATA attempt, emitted by HCF.
 * A failed attempt means ACK timeout, not a rejected response or interruption.
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

} // namespace ieee80211
} // namespace inet
#endif
