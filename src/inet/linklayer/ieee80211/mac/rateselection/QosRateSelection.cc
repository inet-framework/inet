//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/rateselection/QosRateSelection.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"
#include "inet/linklayer/ieee80211/mac/rateselection/Ieee80211PeerModeSelection.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Tag_m.h"

namespace inet {
namespace ieee80211 {

using namespace inet::physicallayer;

Define_Module(QosRateSelection);

void QosRateSelection::initialize(int stage)
{
    RateSelectionBase::initialize(stage);
    if (stage == INITSTAGE_LINK_LAYER) {
        dataOrMgmtRateControl = dynamic_cast<IRateControl *>(findModuleByPath(par("rateControlModule")));
        double multicastFrameBitrate = par("multicastFrameBitrate");
        multicastFrameMode = (multicastFrameBitrate == -1) ? nullptr : modeSet->getMode(bps(multicastFrameBitrate));
        double dataFrameBitrate = par("dataFrameBitrate");
        dataFrameMode = (dataFrameBitrate == -1) ? nullptr : modeSet->getMode(bps(dataFrameBitrate), Hz(par("dataFrameBandwidth")), par("dataFrameNumSpatialStreams"));
        double mgmtFrameBitrate = par("mgmtFrameBitrate");
        mgmtFrameMode = (mgmtFrameBitrate == -1) ? nullptr : modeSet->getMode(bps(mgmtFrameBitrate));
        double controlFrameBitrate = par("controlFrameBitrate");
        controlFrameMode = (controlFrameBitrate == -1) ? nullptr : modeSet->getMode(bps(controlFrameBitrate));
        double responseAckFrameBitrate = par("responseAckFrameBitrate");
        responseAckFrameMode = (responseAckFrameBitrate == -1) ? nullptr : modeSet->getMode(bps(responseAckFrameBitrate));
        double responseBlockAckFrameBitrate = par("responseBlockAckFrameBitrate");
        responseBlockAckFrameMode = (responseBlockAckFrameBitrate == -1) ? nullptr : modeSet->getMode(bps(responseBlockAckFrameBitrate));
        double responseCtsFrameBitrate = par("responseCtsFrameBitrate");
        responseCtsFrameMode = (responseCtsFrameBitrate == -1) ? nullptr : modeSet->getMode(bps(responseCtsFrameBitrate));
    }
}

void QosRateSelection::ensurePerReceiverModesResolved()
{
    if (perReceiverResolved)
        return;
    perReceiverResolved = true;
    auto perReceiverBitrate = check_and_cast<cValueMap *>(par("dataFrameBitratePerReceiver").objectValue());
    for (auto& [path, value] : perReceiverBitrate->getFields()) {
        auto module = findModuleByPath(path.c_str());
        if (module == nullptr)
            throw cRuntimeError("dataFrameBitratePerReceiver: cannot resolve receiver interface module path '%s'", path.c_str());
        auto networkInterface = check_and_cast<NetworkInterface *>(module);
        try {
            auto mode = modeSet->getMode(bps(value.doubleValueInUnit("bps")), Hz(par("dataFrameBandwidth")), par("dataFrameNumSpatialStreams"));
            perReceiverDataFrameMode[networkInterface->getMacAddress()] = mode;
        }
        catch (const cRuntimeError& e) {
            throw cRuntimeError("dataFrameBitratePerReceiver: cannot use rate '%s' for receiver '%s': %s", value.str().c_str(), path.c_str(), e.what());
        }
    }
}

const IIeee80211Mode *QosRateSelection::getMode(Packet *packet, const Ptr<const Ieee80211MacHeader>& header)
{
    const auto& modeReqTag = packet->findTag<Ieee80211ModeReq>();
    if (modeReqTag)
        return modeReqTag->getMode();
    const auto& modeIndTag = packet->findTag<Ieee80211ModeInd>();
    if (modeIndTag)
        return modeIndTag->getMode();
    throw cRuntimeError("Missing mode");
}

// IEEE Std 802.11-2024, 10.6.6.5.2: use the primary response mode.
const IIeee80211Mode *QosRateSelection::computeResponseAckFrameMode(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader)
{
    auto mode = getMode(packet, dataOrMgmtHeader);
    const auto& peer = getResponsePeer(dataOrMgmtHeader);
    auto computed = computeResponseMode(mode, Ieee80211ResponseFrameKind::ACK, peer);
    return validateResponseOverride(computed, responseAckFrameMode, Ieee80211ResponseFrameKind::ACK, peer);
}

const IIeee80211Mode *QosRateSelection::computeResponseCtsFrameMode(Packet *packet, const Ptr<const Ieee80211RtsFrame>& rtsFrame)
{
    auto mode = getMode(packet, rtsFrame);
    const auto& peer = getResponsePeer(rtsFrame);
    auto computed = computeResponseMode(mode, Ieee80211ResponseFrameKind::CTS, peer);
    return validateResponseOverride(computed, responseCtsFrameMode, Ieee80211ResponseFrameKind::CTS, peer);
}

const IIeee80211Mode *QosRateSelection::computeResponseMode(const IIeee80211Mode *elicitingMode,
        Ieee80211ResponseFrameKind responseKind, const MacAddress& receiver)
{
    return computePrimaryResponseMode(elicitingMode, responseKind, receiver);
}

//
// If a Basic BlockAck frame is sent as an immediate response to a BlockAckReq frame that was
// carried in a non-HT PPDU and the Basic BlockAck frame is carried in a non-HT PPDU, the primary
// rate is defined to be the same rate and modulation class as the BlockAckReq frame, and the STA
// shall transmit the Basic BlockAck frame at the primary rate.
//
const IIeee80211Mode *QosRateSelection::computeResponseBlockAckFrameMode(Packet *packet, const Ptr<const Ieee80211BlockAckReq>& blockAckReq)
{
    if (dynamicPtrCast<const Ieee80211BasicBlockAckReq>(blockAckReq)) {
        auto mode = getMode(packet, blockAckReq);
        const auto& peer = getResponsePeer(blockAckReq);
        auto computed = computeResponseMode(mode, Ieee80211ResponseFrameKind::BASIC_BLOCK_ACK, peer);
        return validateResponseOverride(computed, responseBlockAckFrameMode, Ieee80211ResponseFrameKind::BASIC_BLOCK_ACK, peer);
    }
    else
        throw cRuntimeError("Unknown BlockAckReq frame type");
}

const IIeee80211Mode *QosRateSelection::computeDataOrMgmtFrameMode(const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader, bool useFastestMode)
{
    if (dataOrMgmtHeader->getReceiverAddress().isMulticast()) {
        auto preferredMode = dynamicPtrCast<const Ieee80211DataHeader>(dataOrMgmtHeader) ? dataFrameMode : mgmtFrameMode;
        return selectGroupMode(multicastFrameMode, preferredMode);
    }
    // Per-receiver override for originated unicast data frames (see dataFrameBitratePerReceiver).
    // Wins over the interface-wide dataFrameMode / rate control; group-addressed and management
    // frames are left to the existing rules below.
    if (dynamicPtrCast<const Ieee80211DataHeader>(dataOrMgmtHeader) && !dataOrMgmtHeader->getReceiverAddress().isMulticast()) {
        ensurePerReceiverModesResolved();
        auto it = perReceiverDataFrameMode.find(dataOrMgmtHeader->getReceiverAddress());
        if (it != perReceiverDataFrameMode.end())
            return validateConfiguredMode(it->second, dataOrMgmtHeader->getReceiverAddress(), "unicast data");
    }
    if (dynamicPtrCast<const Ieee80211DataHeader>(dataOrMgmtHeader) && dataFrameMode)
        return validateConfiguredMode(dataFrameMode, dataOrMgmtHeader->getReceiverAddress(), "unicast data");
    if (dynamicPtrCast<const Ieee80211MgmtHeader>(dataOrMgmtHeader) && mgmtFrameMode)
        return validateConfiguredMode(mgmtFrameMode, dataOrMgmtHeader->getReceiverAddress(), "management");
    if (useFastestMode) {
        const IIeee80211Mode *best = nullptr;
        for (int i = 0; i < modeSet->getNumModes(); ++i) {
            auto candidate = modeSet->getMode(i);
            if (isAllowedByRateState(candidate, dataOrMgmtHeader->getReceiverAddress(), false) &&
                    (best == nullptr || candidate->getDataMode()->getNetBitrate() > best->getDataMode()->getNetBitrate()))
                best = candidate;
        }
        if (best == nullptr)
            throw cRuntimeError("No eligible mode for a TXOP overrun");
        return best;
    }
    if (dataOrMgmtRateControl)
        return selectAllowedMode(dataOrMgmtHeader->getReceiverAddress(),
                getPeerCompatibleMode(dataOrMgmtHeader->getReceiverAddress(), dataOrMgmtRateControl->getRate(dataOrMgmtHeader->getReceiverAddress())));
    return selectAllowedMode(dataOrMgmtHeader->getReceiverAddress(), fastestMandatoryMode);
}

// IEEE Std 802.11-2024, 10.6.6.2 and 10.6.6.4.
const IIeee80211Mode *QosRateSelection::computeControlFrameMode(const Ptr<const Ieee80211MacHeader>& header,
        bool startsTxop, const IIeee80211Mode *previousModeForReceiver)
{
    auto receiver = header->getReceiverAddress();
    bool bar = dynamicPtrCast<const Ieee80211BasicBlockAckReq>(header) != nullptr;
    if (!bar && !dynamicPtrCast<const Ieee80211RtsFrame>(header))
        throw cRuntimeError("Unsupported originated control frame; responses require an eliciting mode");
    const IIeee80211Mode *selected = nullptr;
    const auto *peer = mib->findPeerRateSet(receiver);
    bool peerBar = !startsTxop && bar && peer != nullptr && peer->supported.known;
    if (peerBar) {
        for (auto candidate : modeSet->getLegacyOperationalModes())
            if (isAllowedByRateState(candidate, receiver, false) &&
                    (selected == nullptr || candidate->getDataMode()->getNetBitrate() > selected->getDataMode()->getNetBitrate()))
                selected = candidate;
        if (selected == nullptr)
            throw cRuntimeError("No supported legacy BAR mode for peer %s", receiver.str().c_str());
    }
    else {
        if (!startsTxop && !bar && previousModeForReceiver == nullptr)
            throw cRuntimeError("Later control frame has no prior mode for peer %s", receiver.str().c_str());
        selected = selectBasicMode(startsTxop || bar ? nullptr : previousModeForReceiver, receiver, false);
    }
    if (controlFrameMode != nullptr) {
        validateConfiguredMode(controlFrameMode, receiver, "control");
        auto rate = controlFrameMode->getDataMode()->getNetBitrate();
        const auto& basic = getBssRateSetForReceiver(receiver).basic;
        bool basicEligible = basic.known && !basic.legacyRates.empty() ? basic.legacyRates.count(rate) != 0 : modeSet->getIsMandatory(controlFrameMode);
        // 10.6.6.4 bounds a later RTS by the selected basic or mandatory rate; it does not require set membership.
        bool requiresBasicRate = startsTxop || bar;
        if (controlFrameMode->getHtMcsIndex() >= 0 ||
                (!peerBar && ((requiresBasicRate && !basicEligible) || rate > selected->getDataMode()->getNetBitrate())))
            throw cRuntimeError("Configured control mode '%s' violates the control frame rate rule", controlFrameMode->getName());
        selected = controlFrameMode;
    }
    return selected;
}

const IIeee80211Mode *QosRateSelection::computeMode(Packet *packet, const Ptr<const Ieee80211MacHeader>& header,
        bool startsTxop, const IIeee80211Mode *previousModeForReceiver, bool useFastestMode)
{
    if (auto dataOrMgmtHeader = dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(header))
        return computeDataOrMgmtFrameMode(dataOrMgmtHeader, useFastestMode);
    return computeControlFrameMode(header, startsTxop, previousModeForReceiver);
}

} /* namespace ieee80211 */
} /* namespace inet */
