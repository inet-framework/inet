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
    ModeSetListener::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        mib.reference(this, "mibModule", true);
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
const IIeee80211Mode *QosRateSelection::computeResponseAckFrameMode(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader, ResponseQueryRole role)
{
    // TODO BSSBasicRateSet, alternate rate
    auto mode = getMode(packet, dataOrMgmtHeader);
    ASSERT(modeSet->containsMode(mode));
    const IIeee80211Mode *responseMode;
    if (!responseAckFrameMode) {
        if (modeSet->getIsMandatory(mode))
            responseMode = mode;
        else if (auto slowerMode = modeSet->getSlowerMandatoryMode(mode))
            responseMode = slowerMode;
        else
            throw cRuntimeError("Mandatory mode not found");
    }
    else
        responseMode = responseAckFrameMode;
    return getPeerCompatibleResponseMode(dataOrMgmtHeader, responseMode, role);
}

const IIeee80211Mode *QosRateSelection::computeResponseCtsFrameMode(Packet *packet, const Ptr<const Ieee80211RtsFrame>& rtsFrame, ResponseQueryRole role)
{
    // TODO BSSBasicRateSet, alternate rate
    auto mode = getMode(packet, rtsFrame);
    ASSERT(modeSet->containsMode(mode));
    const IIeee80211Mode *responseMode;
    if (!responseCtsFrameMode) {
        if (modeSet->getIsMandatory(mode))
            responseMode = mode;
        else if (auto slowerMode = modeSet->getSlowerMandatoryMode(mode))
            responseMode = slowerMode;
        else
            throw cRuntimeError("Mandatory mode not found");
    }
    else
        responseMode = responseCtsFrameMode;
    return getPeerCompatibleResponseMode(rtsFrame, responseMode, role);
}

//
// If a Basic BlockAck frame is sent as an immediate response to a BlockAckReq frame that was
// carried in a non-HT PPDU and the Basic BlockAck frame is carried in a non-HT PPDU, the primary
// rate is defined to be the same rate and modulation class as the BlockAckReq frame, and the STA
// shall transmit the Basic BlockAck frame at the primary rate.
//
const IIeee80211Mode *QosRateSelection::computeResponseBlockAckFrameMode(Packet *packet, const Ptr<const Ieee80211BlockAckReq>& blockAckReq, ResponseQueryRole role)
{
    if (dynamicPtrCast<const Ieee80211BasicBlockAckReq>(blockAckReq)) {
        auto mode = responseBlockAckFrameMode ? responseBlockAckFrameMode : getMode(packet, blockAckReq);
        return getPeerCompatibleResponseMode(blockAckReq, mode, role);
    }
    else if (dynamicPtrCast<const Ieee80211CompressedBlockAckReq>(blockAckReq)) {
        return computeCompressedBlockAckResponseMode(getMode(packet, blockAckReq));
    }
    else
        throw cRuntimeError("Unknown BlockAckReq frame type");
}

const IIeee80211Mode *QosRateSelection::computeCompressedBlockAckResponseMode(const IIeee80211Mode *received)
{
    auto referenceRate = received->getNonHtReferenceRate();
    auto receivedClass = received->getModulationClass();
    bool highThroughput = receivedClass == IIeee80211Mode::ModulationClass::HT || receivedClass == IIeee80211Mode::ModulationClass::VHT;
    if (!std::isfinite(referenceRate.get()))
        throw cRuntimeError("Compressed Block Ack has no non-HT reference rate for the BAR mode");
    const Ieee80211RateSet *basic = mib ? &mib->getBssRateSet().basic : nullptr;
    auto matchesClass = [received, receivedClass, highThroughput](const IIeee80211Mode *candidate) {
        auto modulation = candidate->getModulationClass();
        return highThroughput ? modulation == IIeee80211Mode::ModulationClass::OFDM || modulation == IIeee80211Mode::ModulationClass::ERP_OFDM :
            modulation == receivedClass && candidate->getLegacyPreambleType() == received->getLegacyPreambleType();
    };
    const IIeee80211Mode *primary = nullptr;
    // IEEE Std 802.11-2024, 10.6.6.5.2: prefer a qualifying BSS basic rate, then a mandatory rate.
    for (bool useBasic : {true, false}) {
        if (useBasic && (!basic || !basic->known))
            continue; // INET fallback for unknown BSS state uses the mandatory rate set.
        for (int i = 0; i < modeSet->getNumModes(); i++) {
            auto candidate = modeSet->getMode(i);
            auto rate = candidate->getDataMode()->getNetBitrate();
            bool eligible = useBasic ? basic->legacyRates.count(rate) != 0 : modeSet->isMandatory(i);
            if (eligible && matchesClass(candidate) && rate <= referenceRate &&
                (!primary || rate > primary->getDataMode()->getNetBitrate()))
                primary = candidate;
        }
        if (primary)
            break;
    }
    if (!primary)
        throw cRuntimeError("Compressed Block Ack has no legal non-HT response mode");
    if (!responseBlockAckFrameMode || responseBlockAckFrameMode == primary)
        return primary;
    auto alternate = responseBlockAckFrameMode;
    auto rate = alternate->getDataMode()->getNetBitrate();
    bool basicOrMandatory = (basic && basic->known && basic->legacyRates.count(rate)) || modeSet->getIsMandatory(alternate);
    // IEEE Std 802.11-2024, 10.6.6.5.4: an alternate preserves modulation and actual response duration.
    if (!matchesClass(alternate) || alternate->getModulationClass() != primary->getModulationClass() || !basicOrMandatory ||
        alternate->getDuration(LENGTH_COMPRESSED_BLOCKACK) != primary->getDuration(LENGTH_COMPRESSED_BLOCKACK))
        throw cRuntimeError("responseBlockAckFrameBitrate is not a legal alternate Compressed Block Ack response rate");
    return alternate;
}

const IIeee80211Mode *QosRateSelection::computeCompressedBlockAckReqMode(Packet *packet,
        const Ptr<const Ieee80211CompressedBlockAckReq>& blockAckReq, bool startsTxop)
{
    const Ieee80211RateSet *local = mib ? &mib->getLocalRateSet().operational : nullptr;
    const Ieee80211RateSet *basic = mib ? &mib->getBssRateSet().basic : nullptr;
    auto peerState = mib ? mib->findPeerRateSet(blockAckReq->getReceiverAddress()) : nullptr;
    const Ieee80211RateSet *peer = peerState ? &peerState->operational : nullptr;
    auto eligible = [&](const IIeee80211Mode *candidate) {
        auto modulation = candidate->getModulationClass();
        bool legacy = modulation == IIeee80211Mode::ModulationClass::DSSS_HRDSSS ||
            modulation == IIeee80211Mode::ModulationClass::OFDM || modulation == IIeee80211Mode::ModulationClass::ERP_OFDM;
        auto rate = candidate->getDataMode()->getNetBitrate();
        if (!legacy || (local && local->known && !local->legacyRates.count(rate)))
            return false;
        // IEEE Std 802.11-2024, 10.6.6.2 and 10.6.6.4: initial BAR and later BAR use different sets.
        if (!startsTxop && peer && peer->known)
            return peer->legacyRates.count(rate) != 0;
        if (basic && basic->known && !basic->legacyRates.empty())
            return basic->legacyRates.count(rate) != 0;
        // INET uses mandatory rates when BSS rate state is unknown.
        return modeSet->getIsMandatory(candidate);
    };
    if (controlFrameMode) {
        if (!eligible(controlFrameMode))
            throw cRuntimeError("controlFrameBitrate is not a legal non-HT Compressed BAR rate for this TXOP position");
        return controlFrameMode;
    }
    const IIeee80211Mode *selected = nullptr;
    for (int i = 0; i < modeSet->getNumModes(); i++) {
        auto candidate = modeSet->getMode(i);
        if (eligible(candidate) && (!selected || candidate->getDataMode()->getNetBitrate() > selected->getDataMode()->getNetBitrate()))
            selected = candidate;
    }
    if (!selected)
        throw cRuntimeError("Compressed BAR has no legal local non-HT mode");
    return selected;
}

const IIeee80211Mode *QosRateSelection::computeDataOrMgmtFrameMode(const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader)
{
    // Per-receiver override for originated unicast data frames (see dataFrameBitratePerReceiver).
    // Wins over the interface-wide dataFrameMode / rate control; group-addressed and management
    // frames are left to the existing rules below.
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
    // This subclause describes the rate selection rules for group addressed data and management frames, excluding
    // the following:
    //   — Non-STBC Beacon and non-STBC PSMP frames
    //   — STBC group addressed data and management frames
    //   — Data frames located in an FMS stream (see 10.23.7)
    if (dataOrMgmtHeader->getReceiverAddress().isMulticast()) {
        // If the BSSBasicRateSet parameter is not empty, a data or management frame (excluding the frames listed
        // above) with a group address in the Address 1 field shall be transmitted in a non-HT PPDU using one of the
        // rates included in the BSSBasicRateSet parameter or the rate chosen by the AP, described in 10.23.7, if the data
        // frames are part of an FMS stream.
        // TODO BSSBasicRateSet
        // If the BSSBasicRateSet parameter is empty and the BSSBasicMCSSet parameter is not empty, the frame shall
        // be transmitted in an HT PPDU using one of the MCSs included in the BSSBasicMCSSet parameter.

        // If both the BSSBasicRateSet parameter and the BSSBasicMCSSet parameter are empty (e.g., a scanning STA
        // that is not yet associated with a BSS), the frame shall be transmitted in a non-HT PPDU using one of the
        // mandatory PHY rates.
        // The rate control is not consulted for these frames. It adapts to the feedback of one
        // peer, and a group-addressed frame has no peer: it is never acknowledged, so nothing
        // would ever correct a rate chosen for it.
        return fastestMandatoryMode;
    }
    // A data or management frame not identified in 9.7.5.1 through 9.7.5.5 shall be sent using any data rate or MCS
    // subject to the following constraints:
    //    — A STA shall not transmit a frame using a rate or MCS that is not supported by the receiver STA or
    //      STAs, as reported in any Supported Rates element, Extended Supported Rates element, or
    //      Supported MCS field in management frames transmitted by the receiver STA.
    //    — A STA shall not transmit a frame using a value for the CH_BANDWIDTH parameter of the
    //      TXVECTOR that is not supported by the receiver STA.
    //    — A STA shall not initiate transmission of a frame at a data rate higher than the greatest rate in the
    //      OperationalRateSet or the HTOperationalMCSset, which are parameters of the MLME-
    //      JOIN.request primitive.
    else {
        // TODO Supported Rates element, Extended Supported Rates element
        // TODO OperationalRateSet or the HTOperationalMCSset
        if (dataOrMgmtRateControl)
            return getPeerCompatibleMode(dataOrMgmtHeader->getReceiverAddress(), dataOrMgmtRateControl->getRate(dataOrMgmtHeader->getReceiverAddress()));
        else
            return getPeerCompatibleMode(dataOrMgmtHeader->getReceiverAddress(), fastestMandatoryMode);
    }
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

    if (signalID == modesetChangedSignal) {
        modeSet = check_and_cast<Ieee80211ModeSet *>(obj);
        fastestMandatoryMode = modeSet->getFastestMandatoryMode();
    }
}

void QosRateSelection::frameTransmitted(Packet *packet, const Ptr<const Ieee80211MacHeader>& header)
{
    auto receiverAddr = header->getReceiverAddress();
    lastTransmittedFrameMode[receiverAddr] = getMode(packet, header);
}

const IIeee80211Mode *QosRateSelection::getPeerCompatibleResponseMode(const Ptr<const Ieee80211TwoAddressHeader>& header,
        const IIeee80211Mode *mode, ResponseQueryRole role) const
{
    const auto& peerAddress = role == ResponseQueryRole::PREDICT ? header->getReceiverAddress() : header->getTransmitterAddress();
    if (mode == nullptr || peerAddress.isMulticast() || mib == nullptr || mode->getHtMcsIndex() < 0)
        return mode;
    return selectPeerCompatibleResponseMode(modeSet, mib->findPeerHtState(peerAddress), mode, peerAddress, role);
}

const IIeee80211Mode *QosRateSelection::getPeerCompatibleMode(const MacAddress& peerAddress, const IIeee80211Mode *mode) const
{
    if (mode == nullptr || peerAddress.isMulticast() || !mib || mode->getHtMcsIndex() < 0)
        return mode;
    return selectPeerCompatibleMode(modeSet, mib->findPeerHtState(peerAddress), mode, peerAddress);
}

} /* namespace ieee80211 */
} /* namespace inet */
