// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __INET_PROTOCOLTEST_BLOCKACKREQUESTPROBE_H
#define __INET_PROTOCOLTEST_BLOCKACKREQUESTPROBE_H

#include "BlockAckLifecycleTestSupport.h"

namespace inet::protocoltest {

// The controlled peer uses production Tx and Rx without an originator agreement.
class BlockAckRequestProbeHcf : public BlockAckLifecycleHcf
{
  public:
    simtime_t recipientDeadline(const MacAddress& peer, Tid tid) {
        auto agreement = recipientBlockAckAgreementHandler->getAgreement(tid, peer);
        ASSERT(agreement != nullptr);
        return agreement->getExpirationTime();
    }
    void sendProbeRequest(simtime_t timeout, ITx::ICallback *callback) {
        Enter_Method("sendProbeRequest");
        ASSERT(!tx->hasTransmission());
        auto header = makeShared<Ieee80211AddbaRequest>();
        header->setReceiverAddress(MacAddress("02:00:00:00:00:01"));
        header->setTransmitterAddress(mac->getAddress());
        header->setTid(5);
        header->setDialogToken(37);
        header->setSequenceNumber(SequenceNumberCyclic(300));
        header->setStartingSequenceNumber(SequenceNumberCyclic(100));
        header->setBlockAckPolicy(true);
        header->setAMsduSupported(false);
        header->setBufferSize(64);
        header->setBlockAckTimeoutValue(timeout);
        header->setDurationField(SIMTIME_ZERO);
        auto frame = new Packet("probe-addba-request", header);
        frame->insertAtBack(makeShared<Ieee80211MacTrailer>());
        setFrameMode(frame, header, modeSet->getMode(bps(6000000)));
        tx->transmitFrame(frame, header, false, callback);
        delete frame;
    }
};
namespace request_probe_hcf_registration { Define_Module(::inet::protocoltest::BlockAckRequestProbeHcf); }

class BlockAckRequestProbeMac : public Ieee80211Mac, public ITx::ICallback
{
    bool responseAck = false;
  public:
    bool probeActive = false;
    bool responseAcknowledged = false;
  protected:
    void handleLowerPacket(Packet *frame) override {
        auto header = frame->peekAtFront<Ieee80211MacHeader>();
        bool addressed = header->getReceiverAddress() == mib->address;
        bool response = dynamicPtrCast<const Ieee80211AddbaResponse>(header) != nullptr;
        bool delba = dynamicPtrCast<const Ieee80211Delba>(header) != nullptr;
        if (probeActive && addressed && (header->getType() == ST_ACK || response || delba)) {
            ASSERT(rx->lowerFrameReceived(frame));
            take(frame);
            if (response || delba) {
                auto ack = makeShared<Ieee80211AckFrame>();
                ack->setReceiverAddress(check_and_cast<const Ieee80211TwoAddressHeader *>(header.get())->getTransmitterAddress());
                ack->setDurationField(SIMTIME_ZERO);
                auto reply = new Packet("probe-ack", ack);
                reply->insertAtBack(makeShared<Ieee80211MacTrailer>());
                reply->addTag<Ieee80211ModeReq>()->setMode(modeSet->getMode(bps(6000000)));
                responseAck = response;
                tx->transmitFrame(reply, ack, modeSet->getSifsTime(), false, this);
                delete reply;
            }
            delete frame;
        }
        else
            Ieee80211Mac::handleLowerPacket(frame);
    }
    void transmissionComplete(Packet *, const Ptr<const Ieee80211MacHeader>&) override {
        if (responseAck) {
            responseAcknowledged = true;
            emit(registerSignal("probeResponseAcknowledged"), 1L);
        }
        else
            emit(registerSignal("probeDelbaAcknowledged"), 1L);
    }
};
namespace request_probe_mac_registration { Define_Module(::inet::protocoltest::BlockAckRequestProbeMac); }

class BlockAckRequestProbeStimulus : public cSimpleModule, public ITx::ICallback
{
  protected:
    void initialize() override {
        if (par("active"))
            scheduleAt(SimTime(1), new cMessage("request"));
    }
    void handleMessage(cMessage *message) override {
        delete message;
        auto station = getParentModule()->getSubmodule("sta1")->getSubmodule("wlan", 0);
        check_and_cast<BlockAckRequestProbeMac *>(station->getSubmodule("mac"))->probeActive = true;
        check_and_cast<BlockAckRequestProbeHcf *>(station->getSubmodule("mac")->getSubmodule("hcf"))
            ->sendProbeRequest(SimTime(par("timeout").doubleValue()), this);
    }
    void transmissionComplete(Packet *, const Ptr<const Ieee80211MacHeader>&) override {}
};
namespace request_probe_stimulus_registration { Define_Module(::inet::protocoltest::BlockAckRequestProbeStimulus); }
}
#endif
