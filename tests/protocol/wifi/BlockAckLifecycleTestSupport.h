// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __INET_PROTOCOLTEST_BLOCKACKLIFECYCLETESTSUPPORT_H
#define __INET_PROTOCOLTEST_BLOCKACKLIFECYCLETESTSUPPORT_H

#include "WifiTestSupport.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Mac.h"
#include "inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckAgreement.h"

namespace inet::protocoltest {
using namespace ieee80211;

// These queries use contracts that exist on both sides of PR #1343.
class BlockAckLifecycleHcf : public Hcf
{
  public:
    bool hasOriginator(const MacAddress& peer, Tid tid) {
        auto agreement = originatorBlockAckAgreementHandler->getAgreement(peer, tid);
        return agreement && agreement->getIsAddbaResponseReceived();
    }
    bool hasRecipient(const MacAddress& peer, Tid tid) {
        return recipientBlockAckAgreementHandler->getAgreement(tid, peer) != nullptr;
    }
    void sendTestBar(const MacAddress& peer, Tid tid, ITx::ICallback *callback) {
        Enter_Method("sendTestBar");
        ASSERT(!tx->hasTransmission());
        auto header = makeShared<Ieee80211BasicBlockAckReq>();
        header->setReceiverAddress(peer);
        header->setTransmitterAddress(mac->getAddress());
        header->setTidInfo(tid);
        header->setStartingSequenceNumber(SequenceNumberCyclic(0));
        header->setFragmentNumber(0);
        header->setDurationField(SIMTIME_ZERO);
        auto frame = new Packet("test-bar", header);
        frame->insertAtBack(makeShared<Ieee80211MacTrailer>());
        setFrameMode(frame, header, modeSet->getMode(bps(6000000)));
        // Tx retains a copy. The fixture retains and deletes the original.
        tx->transmitFrame(frame, header, false, callback);
        delete frame;
    }
};
Define_Module(BlockAckLifecycleHcf);

inline BlockAckLifecycleHcf *lifecycleHcf(cModule *network, const char *node) {
    return check_and_cast<BlockAckLifecycleHcf *>(network->getSubmodule(node)->getSubmodule("wlan", 0)
            ->getSubmodule("mac")->getSubmodule("hcf"));
}
inline Ieee80211Mib *lifecycleMib(cModule *network, const char *node) {
    return check_and_cast<Ieee80211Mib *>(network->getSubmodule(node)->getSubmodule("wlan", 0)->getSubmodule("mib"));
}
}
#endif
