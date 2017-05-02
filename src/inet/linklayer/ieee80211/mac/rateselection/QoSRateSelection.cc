//
// Copyright (C) 2016 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#include "inet/common/ModuleAccess.h"
#include "inet/common/NotifierConsts.h"
#include "QoSRateSelection.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211Tag_m.h"

namespace inet {
namespace ieee80211 {

Define_Module(QoSRateSelection);

void QoSRateSelection::initialize(int stage)
{
    ModeSetListener::initialize(stage);
    if (stage == INITSTAGE_LINK_LAYER_2) {
        dataOrMgmtRateControl = dynamic_cast<IRateControl*>(getModuleByPath(par("rateControlModule")));
        double multicastFrameBitrate = par("multicastFrameBitrate");
        multicastFrameMode = (multicastFrameBitrate == -1) ? nullptr : modeSet->getMode(bps(multicastFrameBitrate));
        double dataFrameBitrate = par("dataFrameBitrate");
        dataFrameMode = (dataFrameBitrate == -1) ? nullptr : modeSet->getMode(bps(dataFrameBitrate));
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

const IIeee80211Mode* QoSRateSelection::getMode(Packet *packet, const Ptr<Ieee80211Frame>& frame)
{
    auto modeReqTag = packet->getTag<Ieee80211ModeReq>();
    if (modeReqTag)
        return modeReqTag->getMode();
    auto modeIndTag = packet->getTag<Ieee80211ModeInd>();
    if (modeIndTag)
        return modeIndTag->getMode();
    throw cRuntimeError("Missing mode");
}

bool QoSRateSelection::isControlResponseFrame(const Ptr<Ieee80211Frame>& frame, TxopProcedure *txopProcedure)
{
    bool nonSelfCts = std::dynamic_pointer_cast<Ieee80211CTSFrame>(frame) && !txopProcedure->isTxopInitiator(frame);
    bool blockAck = std::dynamic_pointer_cast<Ieee80211BlockAck>(frame) != nullptr;
    bool ack = std::dynamic_pointer_cast<Ieee80211ACKFrame>(frame) != nullptr;
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
const IIeee80211Mode* QoSRateSelection::computeResponseAckFrameMode(Packet *packet, const Ptr<Ieee80211DataOrMgmtFrame>& dataOrMgmtFrame)
{
    // TODO: BSSBasicRateSet, alternate rate
    auto mode = getMode(packet, dataOrMgmtFrame);
    ASSERT(modeSet->containsMode(mode));
    if (!responseAckFrameMode) {
        if (modeSet->getIsMandatory(mode))
            return mode;
        else if (auto slowerMode = modeSet->getSlowerMandatoryMode(mode))
            return slowerMode;
        else
            throw cRuntimeError("Mandatory mode not found");
    }
    else
        return responseAckFrameMode;
}

const IIeee80211Mode* QoSRateSelection::computeResponseCtsFrameMode(Packet *packet, const Ptr<Ieee80211RTSFrame>& rtsFrame)
{
    // TODO: BSSBasicRateSet, alternate rate
    auto mode = getMode(packet, rtsFrame);
    ASSERT(modeSet->containsMode(mode));
    if (!responseCtsFrameMode) {
        if (modeSet->getIsMandatory(mode))
            return mode;
        else if (auto slowerMode = modeSet->getSlowerMandatoryMode(mode))
            return slowerMode;
        else
            throw cRuntimeError("Mandatory mode not found");
    }
    else
        return responseCtsFrameMode;
}

//
// If a Basic BlockAck frame is sent as an immediate response to a BlockAckReq frame that was
// carried in a non-HT PPDU and the Basic BlockAck frame is carried in a non-HT PPDU, the primary
// rate is defined to be the same rate and modulation class as the BlockAckReq frame, and the STA
// shall transmit the Basic BlockAck frame at the primary rate.
//
const IIeee80211Mode* QoSRateSelection::computeResponseBlockAckFrameMode(Packet *packet, const Ptr<Ieee80211BlockAckReq>& blockAckReq)
{
    if (std::dynamic_pointer_cast<Ieee80211BasicBlockAckReq>(blockAckReq))
        return responseBlockAckFrameMode ? responseBlockAckFrameMode : getMode(packet, blockAckReq);
    else
        throw cRuntimeError("Unknown BlockAckReq frame type");
}

const IIeee80211Mode* QoSRateSelection::computeDataOrMgmtFrameMode(const Ptr<Ieee80211DataOrMgmtFrame>& dataOrMgmtFrame)
{
    if (std::dynamic_pointer_cast<Ieee80211DataFrame>(dataOrMgmtFrame) && dataFrameMode)
        return dataFrameMode;
    if (std::dynamic_pointer_cast<Ieee80211ManagementHeader>(dataOrMgmtFrame) && mgmtFrameMode)
        return mgmtFrameMode;
    // This subclause describes the rate selection rules for group addressed data and management frames, excluding
    // the following:
    //   — Non-STBC Beacon and non-STBC PSMP frames
    //   — STBC group addressed data and management frames
    //   — Data frames located in an FMS stream (see 10.23.7)
    if (dataOrMgmtFrame->getReceiverAddress().isMulticast()) {
        // If the BSSBasicRateSet parameter is not empty, a data or management frame (excluding the frames listed
        // above) with a group address in the Address 1 field shall be transmitted in a non-HT PPDU using one of the
        // rates included in the BSSBasicRateSet parameter or the rate chosen by the AP, described in 10.23.7, if the data
        // frames are part of an FMS stream.
        // TODO: BSSBasicRateSet
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
        // TODO: Supported Rates element, Extended Supported Rates element
        // TODO: OperationalRateSet or the HTOperationalMCSset
        if (dataOrMgmtRateControl)
            return dataOrMgmtRateControl->getRate();
        else
            return fastestMandatoryMode;
    }
}

const IIeee80211Mode* QoSRateSelection::computeControlFrameMode(const Ptr<Ieee80211Frame>& frame, TxopProcedure *txopProcedure)
{
    ASSERT(!isControlResponseFrame(frame, txopProcedure));
    if (controlFrameMode)
        return controlFrameMode;
    // This subclause describes the rate selection rules for control frames that initiate a TXOP and that are not carried
    // in an A-MPDU.
    if (txopProcedure->isTxopInitiator(frame)) {
        // If a control frame other than a Basic BlockAckReq or Basic BlockAck is carried in a non-HT PPDU, the
        // transmitting STA shall transmit the frame using one of the rates in the BSSBasicRateSet parameter or a rate
        // from the mandatory rate set of the attached PHY if the BSSBasicRateSet is empty.
        if (!std::dynamic_pointer_cast<Ieee80211BasicBlockAck>(frame) && !std::dynamic_pointer_cast<Ieee80211BasicBlockAckReq>(frame)) {
            // TODO: BSSBasicRateSet
            return fastestMandatoryMode;
        }
        // If a Basic BlockAckReq or Basic BlockAck frame is carried in a non-HT PPDU, the transmitting STA shall
        // transmit the frame using a rate supported by the receiver STA, if known (as reported in the Supported Rates
        // element and/or Extended Supported Rates element in frames transmitted by that STA). If the supported rate set
        // of the receiving STA or STAs is not known, the transmitting STA shall transmit using a rate from the
        // BSSBasicRateSet parameter or using a rate from the mandatory rate set of the attached PHY if the
        // BSSBasicRateSet is empty.
        else {
            // TODO: supported rate set of the receiving STA
            return fastestMandatoryMode;
        }
    }
    // This subclause describes the rate selection rules for control frames that are not control response frames, are not
    // the frame that initiates a TXOP, are not the frame that terminates a TXOP, and are not carried in an A-MPDU.
    else if (!txopProcedure->isTxopTerminator(frame)) {
        // A frame other than a BlockAckReq or BlockAck that is carried in a non-HT PPDU shall be transmitted by the
        // STA using a rate no higher than the highest rate in the BSSBasicRateSet parameter that is less than or equal to
        // the rate or non-HT reference rate (see 9.7.9) of the previously transmitted frame that was directed to the same
        // receiving STA. If no rate in the BSSBasicRateSet parameter meets these conditions, the control frame shall be
        // transmitted at a rate no higher than the highest mandatory rate of the attached PHY that is less than or equal to
        // the rate or non-HT reference rate (see 9.7.9) of the previously transmitted frame that was directed to the same
        // receiving STA.
        // TODO: BSSBasicRateSet
        if (!std::dynamic_pointer_cast<Ieee80211BasicBlockAck>(frame) && !std::dynamic_pointer_cast<Ieee80211BasicBlockAckReq>(frame)) {
            // TODO: frame sequence context
            auto it = lastTransmittedFrameMode.find(frame->getReceiverAddress());
            if (it != lastTransmittedFrameMode.end()) {
                return it->second;
            }
            else
                return fastestMandatoryMode;
        }
        // A BlockAckReq or BlockAck that is carried in a non-HT PPDU shall be transmitted by the STA using a rate
        // supported by the receiver STA, as reported in the Supported Rates element and/or Extended Supported Rates
        // element in frames transmitted by that STA. When the supported rate set of the receiving STA or STAs is not
        // known, the transmitting STA shall transmit using a rate from the BSSBasicRateSet parameter or from the
        // mandatory rate set of the attached PHY if the BSSBasicRateSet is empty.
        else {
            // TODO: BSSBasicRateSet
            // TODO: Supported Rates element and/or Extended Supported Rates
            return fastestMandatoryMode;
        }
    }
    else
        throw cRuntimeError("Control frames cannot terminate TXOPs");
}

const IIeee80211Mode* QoSRateSelection::computeMode(Packet *packet, const Ptr<Ieee80211Frame>& frame, TxopProcedure *txopProcedure)
{
    if (auto dataOrMgmtFrame = std::dynamic_pointer_cast<Ieee80211DataOrMgmtFrame>(frame))
        return computeDataOrMgmtFrameMode(dataOrMgmtFrame);
    else
        return computeControlFrameMode(frame, txopProcedure);
}

void QoSRateSelection::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details)
{
    Enter_Method("receiveModeSetChangeNotification");
    if (signalID == NF_MODESET_CHANGED) {
        modeSet = check_and_cast<Ieee80211ModeSet*>(obj);
        fastestMandatoryMode = modeSet->getFastestMandatoryMode();
    }
}

void QoSRateSelection::frameTransmitted(Packet *packet, const Ptr<Ieee80211Frame>& frame)
{
    auto receiverAddr = frame->getReceiverAddress();
    lastTransmittedFrameMode[receiverAddr] = getMode(packet, frame);
}

} /* namespace ieee80211 */
} /* namespace inet */
