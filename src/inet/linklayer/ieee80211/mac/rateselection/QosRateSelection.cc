//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/rateselection/QosRateSelection.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Tag_m.h"

namespace inet {
namespace ieee80211 {

using namespace inet::physicallayer;

Define_Module(QosRateSelection);

void QosRateSelection::initialize(int stage)
{
    ModeSetListener::initialize(stage);
    if (stage == INITSTAGE_LINK_LAYER) {
        dataOrMgmtRateControl = dynamic_cast<IRateControl *>(findModuleByPath(par("rateControlModule")));
        resolveConfiguredModes(modeSet);
    }
}

void QosRateSelection::resolveConfiguredModes(const Ieee80211ModeSet *newModeSet)
{
    if (newModeSet == nullptr)
        return;
    double multicastFrameBitrate = par("multicastFrameBitrate");
    auto newMulticastFrameMode = multicastFrameBitrate == -1 ? nullptr : newModeSet->getMode(bps(multicastFrameBitrate));
    double dataFrameBitrate = par("dataFrameBitrate");
    auto newDataFrameMode = dataFrameBitrate == -1 ? nullptr : newModeSet->getMode(bps(dataFrameBitrate), Hz(par("dataFrameBandwidth")), par("dataFrameNumSpatialStreams"));
    double mgmtFrameBitrate = par("mgmtFrameBitrate");
    auto newMgmtFrameMode = mgmtFrameBitrate == -1 ? nullptr : newModeSet->getMode(bps(mgmtFrameBitrate));
    double controlFrameBitrate = par("controlFrameBitrate");
    auto newControlFrameMode = controlFrameBitrate == -1 ? nullptr : newModeSet->getMode(bps(controlFrameBitrate));
    double responseAckFrameBitrate = par("responseAckFrameBitrate");
    auto newResponseAckFrameMode = responseAckFrameBitrate == -1 ? nullptr : newModeSet->getMode(bps(responseAckFrameBitrate));
    double responseBlockAckFrameBitrate = par("responseBlockAckFrameBitrate");
    auto newResponseBlockAckFrameMode = responseBlockAckFrameBitrate == -1 ? nullptr : newModeSet->getMode(bps(responseBlockAckFrameBitrate));
    double responseCtsFrameBitrate = par("responseCtsFrameBitrate");
    auto newResponseCtsFrameMode = responseCtsFrameBitrate == -1 ? nullptr : newModeSet->getMode(bps(responseCtsFrameBitrate));
    auto newFastestMandatoryMode = newModeSet->getFastestMandatoryMode();

    modeSet = newModeSet;
    multicastFrameMode = newMulticastFrameMode;
    dataFrameMode = newDataFrameMode;
    mgmtFrameMode = newMgmtFrameMode;
    controlFrameMode = newControlFrameMode;
    responseAckFrameMode = newResponseAckFrameMode;
    responseBlockAckFrameMode = newResponseBlockAckFrameMode;
    responseCtsFrameMode = newResponseCtsFrameMode;
    fastestMandatoryMode = newFastestMandatoryMode;
    lastTransmittedFrameMode.clear();
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
    if (responseAckFrameMode)
        return modeSet->getNonHtControlResponseMode(responseAckFrameMode, false);
    auto mode = getMode(packet, dataOrMgmtHeader);
    ASSERT(modeSet->supportsMode(mode));
    // IEEE 802.11-2024 10.6.6.1/10.6.6.5.2: this bounded model uses a
    // mandatory non-HT rate for ordinary ACK responses; BSSBasicRateSet is not modelled.
    return modeSet->getMandatoryControlResponseMode(mode);
}

const IIeee80211Mode *QosRateSelection::computeResponseCtsFrameMode(Packet *packet, const Ptr<const Ieee80211RtsFrame>& rtsFrame)
{
    auto mode = getMode(packet, rtsFrame);
    ASSERT(modeSet->supportsMode(mode));
    // The eliciting mode is required even when a CTS rate is configured because
    // the response format and CandidateMCSSet depend on the received PPDU.
    return modeSet->getControlResponseMode(mode, responseCtsFrameMode);
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
    if (responseBlockAckFrameMode)
        return modeSet->getNonHtControlResponseMode(responseBlockAckFrameMode, false);
    auto mode = getMode(packet, blockAckReq);
    ASSERT(modeSet->supportsMode(mode));
    // IEEE 802.11-2024 10.6.6.5.2 permits non-HT Basic BlockAck responses;
    // this model has no BSSBasicRateSet/HT Control context to select another PPDU.
    return modeSet->getNonHtControlResponseMode(mode, false);
}

const IIeee80211Mode *QosRateSelection::computeDataOrMgmtFrameMode(const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader)
{
    if (dynamicPtrCast<const Ieee80211DataHeader>(dataOrMgmtHeader) && dataFrameMode)
        return dataFrameMode;
    if (dynamicPtrCast<const Ieee80211MgmtHeader>(dataOrMgmtHeader) && mgmtFrameMode)
        return mgmtFrameMode;
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
        if (dataOrMgmtRateControl)
            return dataOrMgmtRateControl->getRate();
        else
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
            return dataOrMgmtRateControl->getRate();
        else
            return fastestMandatoryMode;
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
        return computeControlFrameMode(header, txopProcedure);
}

void QosRateSelection::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));

    if (signalID == modesetChangedSignal) {
        resolveConfiguredModes(check_and_cast<Ieee80211ModeSet *>(obj));
    }
}

void QosRateSelection::frameTransmitted(Packet *packet, const Ptr<const Ieee80211MacHeader>& header)
{
    auto receiverAddr = header->getReceiverAddress();
    lastTransmittedFrameMode[receiverAddr] = getMode(packet, header);
}

} /* namespace ieee80211 */
} /* namespace inet */
