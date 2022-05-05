//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/aggregation/MsduAggregation.h"

#include "inet/common/packet/chunk/ByteCountChunk.h"

namespace inet {
namespace ieee80211 {

Register_Class(MsduAggregation);

void MsduAggregation::setSubframeAddress(const Ptr<Ieee80211MsduSubframeHeader>& subframeHeader, const Ptr<const Ieee80211DataHeader>& header)
{
    // Note: Addr1 (RA), Addr2 (TA)
    // Table 8-19—Address field contents
    MacAddress da, sa;
    bool toDS = header->getToDS();
    bool fromDS = header->getFromDS();
    if (toDS == 0 && fromDS == 0) { // STA to STA
        da = header->getReceiverAddress();
        sa = header->getTransmitterAddress();
    }
    else if (toDS == 0 && fromDS == 1) { // AP to STA
        da = header->getReceiverAddress();
        sa = header->getAddress3();
    }
    else if (toDS == 1 && fromDS == 0) { // STA to AP
        da = header->getAddress3();
        sa = header->getTransmitterAddress();
    }
    else if (toDS == 1 && fromDS == 1) { // AP to AP
        da = header->getAddress3();
        sa = header->getAddress4();
    }
    ASSERT(!da.isUnspecified());
    ASSERT(!sa.isUnspecified());
    subframeHeader->setDa(da);
    subframeHeader->setSa(sa);
}

Packet *MsduAggregation::aggregateFrames(std::vector<Packet *> *frames)
{
    EV_DEBUG << "Aggregating " << frames->size() << " packets into A-MSDU.\n";
    auto firstFrame = frames->at(0);
    auto firstHeader = firstFrame->peekAtFront<Ieee80211DataHeader>();
    auto tid = firstHeader->getTid();
    auto toDS = firstHeader->getToDS();
    auto fromDS = firstHeader->getFromDS();
    auto ra = firstHeader->getReceiverAddress();
    auto aggregatedFrame = new Packet();
    std::string aggregatedName;
    for (int i = 0; i < (int)frames->size(); i++) {
        auto msduSubframeHeader = makeShared<Ieee80211MsduSubframeHeader>();
        auto frame = frames->at(i);
        const auto& header = frame->popAtFront<Ieee80211DataHeader>();
        frame->popAtBack<Ieee80211MacTrailer>(B(4));
        auto msdu = frame->peekData();
        msduSubframeHeader->setLength(B(msdu->getChunkLength()).get());
        setSubframeAddress(msduSubframeHeader, header);
        aggregatedFrame->insertAtBack(msduSubframeHeader);
        aggregatedFrame->insertAtBack(msdu);
        aggregatedFrame->getRegionTags().copyTags(frame->getRegionTags(), frame->getFrontOffset(), aggregatedFrame->getBackOffset() - frame->getDataLength(), frame->getDataLength());
        int paddingLength = 4 - B(msduSubframeHeader->getChunkLength() + msdu->getChunkLength()).get() % 4;
        if (i != (int)frames->size() - 1 && paddingLength != 4) {
            auto padding = makeShared<ByteCountChunk>(B(paddingLength));
            aggregatedFrame->insertAtBack(padding);
        }
        if (i != 0)
            aggregatedName.append("+");
        aggregatedName.append(frame->getName());
        delete frame;
    }
    // The MPDU containing the A-MSDU is carried in any of the following data frame subtypes: QoS Data,
    // QoS Data + CF-Ack, QoS Data + CF-Poll, QoS Data + CF-Ack + CF-Poll. The A-MSDU structure is
    // contained in the frame body of a single MPDU.
    auto amsduHeader = makeShared<Ieee80211DataHeader>();
    amsduHeader->setType(ST_DATA_WITH_QOS);
    amsduHeader->setToDS(toDS);
    amsduHeader->setFromDS(fromDS);
    amsduHeader->setAMsduPresent(true);
    amsduHeader->setReceiverAddress(ra);
    amsduHeader->setTid(tid);
    amsduHeader->addChunkLength(QOSCONTROL_PART_LENGTH);
    // TODO set addr3 and addr4 according to fromDS and toDS.
    aggregatedFrame->insertAtFront(amsduHeader);
    aggregatedFrame->insertAtBack(makeShared<Ieee80211MacTrailer>());
    aggregatedFrame->setName(aggregatedName.c_str());
    EV_TRACE << "Created A-MSDU " << *aggregatedFrame << ".\n";
    return aggregatedFrame;
}

} /* namespace ieee80211 */
} /* namespace inet */

