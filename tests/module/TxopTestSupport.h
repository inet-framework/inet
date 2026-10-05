// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __TXOP_TEST_SUPPORT_H
#define __TXOP_TEST_SUPPORT_H
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Tag_m.h"

namespace inet {
namespace ieee80211 {

struct ExchangeEvent {
    Ieee80211FrameType type;
    simtime_t start;
    simtime_t airtime;
    simtime_t duration;
    b length;
    const physicallayer::IIeee80211Mode *mode;
    bool retry;
    int sequence;
    int fragment;
    bool amsdu;
};

class ExchangeHcf : public Hcf
{
  public:
    std::vector<ExchangeEvent> events;
    std::vector<int> burstSizes;
    std::vector<simtime_t> burstDurations;
    int currentBurst = 0;
    int grants = 0;
    int sequenceStarts = 0;
    int failures = 0;
    int acknowledgments = 0;
    int responseCount = 0;
    bool initialRefused = false;

    void inject(bool management = false, bool group = false)
    {
        Enter_Method("inject");
        for (int i = 0; i < 3; i++) {
            Ptr<Ieee80211DataOrMgmtHeader> header;
            if (management) {
                auto mgmt = makeShared<Ieee80211MgmtHeader>();
                mgmt->setType(ST_ACTION);
                header = mgmt;
            }
            else {
                auto data = makeShared<Ieee80211DataHeader>();
                data->setType(ST_DATA_WITH_QOS);
                data->addChunkLength(QOSCONTROL_PART_LENGTH);
                data->setTid(5);
                header = data;
            }
            header->setReceiverAddress(group ? MacAddress::BROADCAST_ADDRESS : MacAddress("02:00:00:00:00:02"));
            header->setTransmitterAddress(MacAddress("02:00:00:00:00:01"));
            header->setAddress3(MacAddress("02:00:00:00:00:01"));
            auto packet = new Packet("duration-data", header);
            packet->insertAtBack(makeShared<ByteCountChunk>(B(200)));
            packet->insertAtBack(makeShared<Ieee80211MacTrailer>());
            processUpperFrame(packet, header);
        }
    }

  protected:
    virtual void channelGranted(IChannelAccess *access) override
    {
        grants++;
        currentBurst = 0;
        Hcf::channelGranted(access);
    }

    Packet *observedFrame = nullptr;
    virtual void transmitFrame(Packet *packet, simtime_t ifs, const PreparedTransmit *prepared) override
    {
        observedFrame = packet;
        Hcf::transmitFrame(packet, ifs, prepared);
    }

    virtual void transmissionStarted(TxRequestId id) override
    {
        if (responseRequest) {
            Hcf::transmissionStarted(id);
            return;
        }
        ASSERT(sequenceStarts == grants);
        auto packet = observedFrame;
        auto header = packet->peekAtFront<Ieee80211MacHeader>();
        auto data = dynamicPtrCast<const Ieee80211DataHeader>(header);
        auto dataOrMgmt = dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(header);
        auto mode = packet->getTag<physicallayer::Ieee80211ModeReq>()->getMode();
        events.push_back({header->getType(), simTime(), mode->getDuration(packet->getDataLength()), header->getDurationField(), packet->getDataLength(), mode,
            header->getRetry(), dataOrMgmt ? dataOrMgmt->getSequenceNumber().get() : -1,
            dataOrMgmt ? dataOrMgmt->getFragmentNumber() : -1, data && data->getAMsduPresent()});
        if (dataOrMgmt)
            currentBurst++;
        Hcf::transmissionStarted(id);
    }

    virtual void frameSequenceStarted() override
    {
        ASSERT(frameSequenceHandler->getContext() != nullptr);
        sequenceStarts++;
        Hcf::frameSequenceStarted();
    }

    virtual void frameSequenceFinished() override
    {
        burstSizes.push_back(currentBurst);
        burstDurations.push_back(edca->getChannelOwner()->getTxopProcedure()->getDuration());
        Hcf::frameSequenceFinished();
    }

    virtual void originatorProcessFailedFrame(Packet *packet) override
    {
        failures++;
        Hcf::originatorProcessFailedFrame(packet);
    }

    virtual void originatorProcessReceivedFrame(Packet *packet, Packet *request) override
    {
        if (packet->peekAtFront<Ieee80211MacHeader>()->getType() == ST_ACK)
            acknowledgments++;
        Hcf::originatorProcessReceivedFrame(packet, request);
    }

    virtual void recipientProcessTransmittedControlResponseFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header) override
    {
        auto mode = packet->getTag<physicallayer::Ieee80211ModeReq>()->getMode();
        ASSERT(packet->getDataLength() == B(14));
        ASSERT(mode->getDataMode()->getNetBitrate() == bps(24000000));
        responseCount++;
        Hcf::recipientProcessTransmittedControlResponseFrame(packet, header);
    }

    // Delivery above MAC is outside this exchange test. Frames retain real recipient processing.
    virtual void sendUp(const std::vector<Packet *>& packets) override
    {
        for (auto packet : packets) {
            take(packet);
            delete packet;
        }
    }
};
Define_Module(ExchangeHcf);

} // namespace ieee80211
} // namespace inet

#endif
