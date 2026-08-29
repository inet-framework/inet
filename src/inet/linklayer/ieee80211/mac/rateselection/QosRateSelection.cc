//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/rateselection/QosRateSelection.h"

#include <tuple>

#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"
#include "inet/linklayer/ieee80211/mac/rateselection/Ieee80211PeerModeSelection.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Tag_m.h"

namespace inet {
namespace ieee80211 {

using namespace inet::physicallayer;

static const IIeee80211Mode *resolveConfiguredResponseCtsFrameMode(const Ieee80211ModeSet *modeSet, double bitrate, const char *modulePath)
{
    if (bitrate == -1)
        return nullptr;
    try {
        auto result = modeSet->getMode(bps(bitrate));
        if (modeSet->isHtOperationSupported() && result->getHtMcsIndex() < 0)
            throw cRuntimeError("legacy mode '%s' is not selectable for HT CTS responses", result->getName());
        return result;
    }
    catch (const cRuntimeError& error) {
        throw cRuntimeError("%s has invalid responseCtsFrameBitrate=%g bps for operation mode '%s'; "
                "the configured CTS rate must resolve to a selectable mode (HT RTS responses require an HT mode): %s",
                modulePath, bitrate, modeSet->getName(), error.getFormattedMessage().c_str());
    }
}

Define_Module(QosRateSelection);

void QosRateSelection::initialize(int stage)
{
    ModeSetListener::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        mib.reference(this, "mibModule", true);
    if (stage == INITSTAGE_LINK_LAYER) {
        dataOrMgmtRateControl = dynamic_cast<IRateControl *>(findModuleByPath(par("rateControlModule")));
        if (modeSet == nullptr)
            throw cRuntimeError("QosRateSelection module %s has no mode set at link-layer initialization", getFullPath().c_str());
        updateModes();
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
            auto mode = modeSet->getMode(bps(value.doubleValueInUnit("bps")), Hz(par("dataFrameBandwidth")), par("dataFrameNumSpatialStreams"), par("dataFrameGuardInterval"));
            perReceiverDataFrameMode[networkInterface->getMacAddress()] = mode;
        }
        catch (const cRuntimeError& e) {
            throw cRuntimeError("dataFrameBitratePerReceiver: cannot use rate '%s' for receiver '%s': %s", value.str().c_str(), path.c_str(), e.what());
        }
    }
}

void QosRateSelection::updateModes()
{
    if (modeSet == nullptr)
        return;
    double multicastFrameBitrate = par("multicastFrameBitrate");
    auto newMulticastFrameMode = multicastFrameBitrate == -1 ? nullptr : modeSet->getMode(bps(multicastFrameBitrate));
    double dataFrameBitrate = par("dataFrameBitrate");
    auto newDataFrameMode = dataFrameBitrate == -1 ? nullptr : modeSet->getMode(bps(dataFrameBitrate), Hz(par("dataFrameBandwidth")), par("dataFrameNumSpatialStreams"), par("dataFrameGuardInterval"));
    double mgmtFrameBitrate = par("mgmtFrameBitrate");
    auto newMgmtFrameMode = mgmtFrameBitrate == -1 ? nullptr : modeSet->getMode(bps(mgmtFrameBitrate));
    double controlFrameBitrate = par("controlFrameBitrate");
    auto newControlFrameMode = controlFrameBitrate == -1 ? nullptr : modeSet->getMode(bps(controlFrameBitrate));
    double responseAckFrameBitrate = par("responseAckFrameBitrate");
    auto newResponseAckFrameMode = responseAckFrameBitrate == -1 ? nullptr : modeSet->getMode(bps(responseAckFrameBitrate));
    double responseBlockAckFrameBitrate = par("responseBlockAckFrameBitrate");
    auto newResponseBlockAckFrameMode = responseBlockAckFrameBitrate == -1 ? nullptr : modeSet->getMode(bps(responseBlockAckFrameBitrate));
    double responseCtsFrameBitrate = par("responseCtsFrameBitrate");
    auto newResponseCtsFrameMode = resolveConfiguredResponseCtsFrameMode(modeSet, responseCtsFrameBitrate, getFullPath().c_str());
    auto newFastestMandatoryMode = modeSet->getFastestMandatoryMode();

    multicastFrameMode = newMulticastFrameMode;
    dataFrameMode = newDataFrameMode;
    mgmtFrameMode = newMgmtFrameMode;
    controlFrameMode = newControlFrameMode;
    responseAckFrameMode = newResponseAckFrameMode;
    responseBlockAckFrameMode = newResponseBlockAckFrameMode;
    responseCtsFrameMode = newResponseCtsFrameMode;
    fastestMandatoryMode = newFastestMandatoryMode;
    lastTransmittedFrameMode.clear();
    perReceiverDataFrameMode.clear();
    perReceiverResolved = false;
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

bool QosRateSelection::isControlResponseFrame(const Ptr<const Ieee80211MacHeader>& header, TxopProcedure *txopProcedure)
{
    bool nonSelfCts = dynamicPtrCast<const Ieee80211CtsFrame>(header) && !txopProcedure->isTxopInitiator(header);
    bool blockAck = dynamicPtrCast<const Ieee80211BlockAck>(header) != nullptr;
    bool ack = dynamicPtrCast<const Ieee80211AckFrame>(header) != nullptr;
    return ack || blockAck || nonSelfCts;
}

//
// If a CTS or ACK control response frame is carried in a non-HT PPDU, the primary rate is defined to
// be the highest rate in the BSSBasicRateSet parameter that is less than or equal to the rate (or non-HT
// reference rate; see 9.7.9) of the previous frame. If no rate in the BSSBasicRateSet parameter meets
// these conditions, the primary rate is defined to be the highest mandatory rate of the attached PHY
// that is less than or equal to the rate (or non-HT reference rate; see 9.7.9) of the previous frame. The
// STA may select an alternate rate according to the rules in 9.7.6.5.4. The STA shall transmit the
// non-HT PPDU CTS or ACK control response frame at either the primary rate or the alternate rate, if
// one exists.
//
const IIeee80211Mode *QosRateSelection::computeResponseAckFrameMode(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader)
{
    const IIeee80211Mode *responseMode = nullptr;
    if (responseAckFrameMode)
        responseMode = modeSet->getNonHtControlResponseMode(responseAckFrameMode, false);
    else {
        auto mode = getMode(packet, dataOrMgmtHeader);
        ASSERT(modeSet->supportsMode(mode));
        // IEEE 802.11-2024 10.6.6.1/10.6.6.5.2: this bounded model uses a
        // mandatory non-HT rate for ordinary ACK responses; BSSBasicRateSet is not modelled.
        responseMode = modeSet->getMandatoryControlResponseMode(mode);
    }
    return dataOrMgmtHeader ? getPeerCompatibleMode(dataOrMgmtHeader->getTransmitterAddress(), responseMode) : responseMode;
}

const IIeee80211Mode *QosRateSelection::computeResponseCtsFrameMode(Packet *packet, const Ptr<const Ieee80211RtsFrame>& rtsFrame)
{
    auto mode = getMode(packet, rtsFrame);
    ASSERT(modeSet->supportsMode(mode));
    // The eliciting mode is required even when a CTS rate is configured because
    // the response format and CandidateMCSSet depend on the received PPDU.
    auto responseMode = modeSet->getControlResponseMode(mode, responseCtsFrameMode);
    return rtsFrame ? getPeerCompatibleMode(rtsFrame->getTransmitterAddress(), responseMode) : responseMode;
}

//
// If a Basic BlockAck frame is sent as an immediate response to a BlockAckReq frame that was
// carried in a non-HT PPDU and the Basic BlockAck frame is carried in a non-HT PPDU, the primary
// rate is defined to be the same rate and modulation class as the BlockAckReq frame, and the STA
// shall transmit the Basic BlockAck frame at the primary rate.
//
const IIeee80211Mode *QosRateSelection::computeResponseBlockAckFrameMode(Packet *packet, const Ptr<const Ieee80211BlockAckReq>& blockAckReq)
{
    if (!dynamicPtrCast<const Ieee80211BasicBlockAckReq>(blockAckReq))
        throw cRuntimeError("Unknown BlockAckReq frame type");
    const IIeee80211Mode *responseMode = nullptr;
    if (responseBlockAckFrameMode)
        responseMode = modeSet->getNonHtControlResponseMode(responseBlockAckFrameMode, false);
    else {
        auto mode = getMode(packet, blockAckReq);
        ASSERT(modeSet->supportsMode(mode));
        // IEEE 802.11-2024 10.6.6.5.2 permits non-HT Basic BlockAck responses;
        // this model has no BSSBasicRateSet/HT Control context to select another PPDU.
        responseMode = modeSet->getNonHtControlResponseMode(mode, false);
    }
    return blockAckReq ? getPeerCompatibleMode(blockAckReq->getTransmitterAddress(), responseMode) : responseMode;
}

const IIeee80211Mode *QosRateSelection::computeDataOrMgmtFrameMode(const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader)
{
    if (dataOrMgmtHeader->getReceiverAddress().isMulticast()) {
        const auto *requestedMode = multicastFrameMode;
        if (requestedMode == nullptr)
            requestedMode = dynamicPtrCast<const Ieee80211DataHeader>(dataOrMgmtHeader) ? dataFrameMode : mgmtFrameMode;
        return selectGroupAddressedMode(modeSet, requestedMode != nullptr ? requestedMode : fastestMandatoryMode);
    }
    // Per-receiver override for originated unicast data frames (see dataFrameBitratePerReceiver).
    // Wins over the interface-wide dataFrameMode / rate control; group-addressed and management
    // frames were handled above.
    if (dynamicPtrCast<const Ieee80211DataHeader>(dataOrMgmtHeader) && !dataOrMgmtHeader->getReceiverAddress().isMulticast()) {
        ensurePerReceiverModesResolved();
        auto it = perReceiverDataFrameMode.find(dataOrMgmtHeader->getReceiverAddress());
        if (it != perReceiverDataFrameMode.end())
            return getPeerCompatibleMode(dataOrMgmtHeader->getReceiverAddress(), it->second);
    }
    if (dynamicPtrCast<const Ieee80211DataHeader>(dataOrMgmtHeader) && dataFrameMode)
        return getPeerCompatibleMode(dataOrMgmtHeader->getReceiverAddress(), dataFrameMode);
    if (dynamicPtrCast<const Ieee80211MgmtHeader>(dataOrMgmtHeader) && mgmtFrameMode)
        return getPeerCompatibleMode(dataOrMgmtHeader->getReceiverAddress(), mgmtFrameMode);
    if (dataOrMgmtRateControl)
        return getPeerCompatibleMode(dataOrMgmtHeader->getReceiverAddress(), dataOrMgmtRateControl->getRate(dataOrMgmtHeader->getReceiverAddress()));
    return getPeerCompatibleMode(dataOrMgmtHeader->getReceiverAddress(), fastestMandatoryMode);
}

const IIeee80211Mode *QosRateSelection::computeControlFrameMode(const Ptr<const Ieee80211MacHeader>& header, TxopProcedure *txopProcedure)
{
    ASSERT(!isControlResponseFrame(header, txopProcedure));
    if (controlFrameMode)
        return controlFrameMode;
    // This subclause describes the rate selection rules for control frames that initiate a TXOP and that are not carried
    // in an A-MPDU.
    if (txopProcedure->isTxopInitiator(header)) {
        // If a control frame other than a Basic BlockAckReq or Basic BlockAck is carried in a non-HT PPDU, the
        // transmitting STA shall transmit the frame using one of the rates in the BSSBasicRateSet parameter or a rate
        // from the mandatory rate set of the attached PHY if the BSSBasicRateSet is empty.
        if (!dynamicPtrCast<const Ieee80211BasicBlockAck>(header) && !dynamicPtrCast<const Ieee80211BasicBlockAckReq>(header)) {
            // TODO BSSBasicRateSet
            return fastestMandatoryMode;
        }
        // If a Basic BlockAckReq or Basic BlockAck frame is carried in a non-HT PPDU, the transmitting STA shall
        // transmit the frame using a rate supported by the receiver STA, if known (as reported in the Supported Rates
        // element and/or Extended Supported Rates element in frames transmitted by that STA). If the supported rate set
        // of the receiving STA or STAs is not known, the transmitting STA shall transmit using a rate from the
        // BSSBasicRateSet parameter or using a rate from the mandatory rate set of the attached PHY if the
        // BSSBasicRateSet is empty.
        else {
            // TODO supported rate set of the receiving STA
            return fastestMandatoryMode;
        }
    }
    // This subclause describes the rate selection rules for control frames that are not control response frames, are not
    // the frame that initiates a TXOP, are not the frame that terminates a TXOP, and are not carried in an A-MPDU.
    else if (!txopProcedure->isTxopTerminator(header)) {
        // A frame other than a BlockAckReq or BlockAck that is carried in a non-HT PPDU shall be transmitted by the
        // STA using a rate no higher than the highest rate in the BSSBasicRateSet parameter that is less than or equal to
        // the rate or non-HT reference rate (see 9.7.9) of the previously transmitted frame that was directed to the same
        // receiving STA. If no rate in the BSSBasicRateSet parameter meets these conditions, the control frame shall be
        // transmitted at a rate no higher than the highest mandatory rate of the attached PHY that is less than or equal to
        // the rate or non-HT reference rate (see 9.7.9) of the previously transmitted frame that was directed to the same
        // receiving STA.
        // TODO BSSBasicRateSet
        if (!dynamicPtrCast<const Ieee80211BasicBlockAck>(header) && !dynamicPtrCast<const Ieee80211BasicBlockAckReq>(header)) {
            // TODO frame sequence context
            auto it = lastTransmittedFrameMode.find(header->getReceiverAddress());
            return (it != lastTransmittedFrameMode.end()) ? it->second : fastestMandatoryMode;
        }
        // A BlockAckReq or BlockAck that is carried in a non-HT PPDU shall be transmitted by the STA using a rate
        // supported by the receiver STA, as reported in the Supported Rates element and/or Extended Supported Rates
        // element in frames transmitted by that STA. When the supported rate set of the receiving STA or STAs is not
        // known, the transmitting STA shall transmit using a rate from the BSSBasicRateSet parameter or from the
        // mandatory rate set of the attached PHY if the BSSBasicRateSet is empty.
        else {
            // TODO BSSBasicRateSet
            // TODO Supported Rates element and/or Extended Supported Rates
            return fastestMandatoryMode;
        }
    }
    else
        throw cRuntimeError("Control frames cannot terminate TXOPs");
}

const IIeee80211Mode *QosRateSelection::computeMode(Packet *packet, const Ptr<const Ieee80211MacHeader>& header, TxopProcedure *txopProcedure)
{
    if (auto dataOrMgmtHeader = dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(header))
        return computeDataOrMgmtFrameMode(dataOrMgmtHeader);
    else
        return getPeerCompatibleMode(header->getReceiverAddress(), computeControlFrameMode(header, txopProcedure));
}

void QosRateSelection::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));
    if (signalID == modesetChangedSignal && obj != modeSet)
        applyModeSet(check_and_cast<physicallayer::Ieee80211ModeSet *>(obj));
}

void QosRateSelection::applyModeSet(const physicallayer::Ieee80211ModeSet *newModeSet)
{
    Enter_Method_Silent();
    modeSet = const_cast<physicallayer::Ieee80211ModeSet *>(newModeSet);
    updateModes();
    if (getSimulation()->getContextType() != CTX_INITIALIZE)
        ensurePerReceiverModesResolved();
}

std::function<void()> QosRateSelection::saveModeSetState()
{
    Enter_Method_Silent();
    return [this, state = std::make_tuple(modeSet, fastestMandatoryMode, multicastFrameMode, dataFrameMode, mgmtFrameMode,
            controlFrameMode, responseAckFrameMode, responseCtsFrameMode, responseBlockAckFrameMode,
            lastTransmittedFrameMode, perReceiverDataFrameMode, perReceiverResolved)]() mutable {
        std::tie(modeSet, fastestMandatoryMode, multicastFrameMode, dataFrameMode, mgmtFrameMode,
                controlFrameMode, responseAckFrameMode, responseCtsFrameMode, responseBlockAckFrameMode,
                lastTransmittedFrameMode, perReceiverDataFrameMode, perReceiverResolved) = std::move(state);
    };
}

void QosRateSelection::frameTransmitted(Packet *packet, const Ptr<const Ieee80211MacHeader>& header)
{
    auto receiverAddr = header->getReceiverAddress();
    lastTransmittedFrameMode[receiverAddr] = getMode(packet, header);
}

const IIeee80211Mode *QosRateSelection::getPeerCompatibleMode(const MacAddress& peerAddress, const IIeee80211Mode *mode) const
{
    // IEEE Std 802.11-2024, 10.6.5.8: Peer compatibility filtering is currently
    // supported for HT (802.11n) modes using negotiated PeerHtState. Non-HT
    // modes (legacy and VHT) return unchanged because VHT capability negotiation
    // (VHT Capabilities/Operation elements) is not yet modeled in MIB.
    if (mode == nullptr || peerAddress.isMulticast() || !mib || mib->mode == Ieee80211Mib::INDEPENDENT || mode->getHtMcsIndex() < 0)
        return mode;
    return selectPeerCompatibleMode(modeSet, mib->findPeerHtState(peerAddress), mode, peerAddress);
}

} /* namespace ieee80211 */
} /* namespace inet */
