//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/fragmentation/Fragmentation.h"

#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/fragmentation/Ieee80211FragmentedActionContextTag.h"

namespace inet {
namespace ieee80211 {

namespace {

const Ptr<Ieee80211MgmtHeader> copyManagementHeader(const Ptr<const Ieee80211MgmtHeader>& source)
{
    auto destination = makeShared<Ieee80211MgmtHeader>();
    destination->setType(source->getType());
    destination->setToDS(source->getToDS());
    destination->setFromDS(source->getFromDS());
    destination->setMoreFragments(source->getMoreFragments());
    destination->setRetry(source->getRetry());
    destination->setPowerMgmt(source->getPowerMgmt());
    destination->setMoreData(source->getMoreData());
    destination->setProtectedFrame(source->getProtectedFrame());
    destination->setOrder(source->getOrder());
    destination->setDurationField(source->getDurationField());
    destination->setAID(source->getAID());
    destination->setReceiverAddress(source->getReceiverAddress());
    destination->setMACArrive(source->getMACArrive());
    destination->setTransmitterAddress(source->getTransmitterAddress());
    destination->setAddress3(source->getAddress3());
    destination->setFragmentNumber(source->getFragmentNumber());
    destination->setSequenceNumber(source->getSequenceNumber());
    return destination;
}

} // namespace

Register_Class(Fragmentation);

std::vector<Packet *> *Fragmentation::fragmentFrame(Packet *frame, const std::vector<int>& fragmentSizes)
{
    EV_DEBUG << "Fragmenting " << *frame << " into " << fragmentSizes.size() << " fragments.\n";
    B offset = B(0);
    std::vector<Packet *> *fragments = new std::vector<Packet *>();
    const auto& frameHeader = frame->popAtFront<Ieee80211DataOrMgmtHeader>();
    frame->popAtBack<Ieee80211MacTrailer>(B(4));
    const auto& actionFrame = dynamicPtrCast<const Ieee80211ActionFrame>(frameHeader);
    if (actionFrame != nullptr) {
        // IEEE Std 802.11-2024, 10.4: a fragment frame body carries only a
        // portion of the MMPDU. Move the action body out of INET's combined
        // typed header before slicing it into fragment bodies.
        Packet serializedHeader("serializedActionHeader", frameHeader);
        const auto& headerBytes = serializedHeader.peekDataAsBytes()->getBytes();
        auto bodyOffset = makeShared<Ieee80211MgmtHeader>()->getChunkLength().get<B>();
        frame->insertAtFront(makeShared<BytesChunk>(std::vector<uint8_t>(headerBytes.begin() + bodyOffset, headerBytes.end())));
    }
    B totalFragmentBodyLength = B(0);
    for (auto fragmentSize : fragmentSizes)
        totalFragmentBodyLength += B(fragmentSize);
    if (totalFragmentBodyLength != frame->getDataLength())
        throw cRuntimeError("Fragment sizes total %s but frame body length is %s", totalFragmentBodyLength.str().c_str(), frame->getDataLength().str().c_str());
    for (size_t i = 0; i < fragmentSizes.size(); i++) {
        bool lastFragment = i == fragmentSizes.size() - 1;
        std::string name = std::string(frame->getName()) + "-frag" + std::to_string(i);
        auto fragment = new Packet(name.c_str());
        B length = B(fragmentSizes.at(i));
        fragment->insertAtBack(frame->peekDataAt(offset, length));
        fragment->copyTags(*frame);
        fragment->getRegionTags().copyTags(frame->getRegionTags(), frame->getFrontOffset() + offset, fragment->getFrontOffset(), length);
        offset += length;
        Ptr<Ieee80211DataOrMgmtHeader> fragmentHeader;
        if (actionFrame != nullptr) {
            fragmentHeader = copyManagementHeader(actionFrame);
            auto actionContext = staticPtrCast<Ieee80211ActionFrame>(actionFrame->dupShared());
            actionContext->setFragmentNumber(i);
            actionContext->setMoreFragments(!lastFragment);
            fragment->addTag<Ieee80211FragmentedActionContextTag>()->setActionFrame(actionContext);
        }
        else
            fragmentHeader = staticPtrCast<Ieee80211DataOrMgmtHeader>(frameHeader->dupShared());
        fragmentHeader->setSequenceNumber(frameHeader->getSequenceNumber());
        fragmentHeader->setFragmentNumber(i);
        fragmentHeader->setMoreFragments(!lastFragment);
        fragment->insertAtFront(fragmentHeader);
        fragment->insertAtBack(makeShared<Ieee80211MacTrailer>());
        EV_TRACE << "Created " << *fragment << " fragment.\n";
        fragments->push_back(fragment);
    }
    delete frame;
    EV_TRACE << "Created " << fragments->size() << " fragments.\n";
    return fragments;
}

} // namespace ieee80211
} // namespace inet
