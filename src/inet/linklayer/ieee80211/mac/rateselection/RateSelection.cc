//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/rateselection/RateSelection.h"

#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211HtMode.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"
#include "inet/linklayer/ieee80211/mac/contract/IRateControl.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/physicallayer/wireless/ieee80211/mode/IIeee80211Mode.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211ModeSet.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Tag_m.h"

namespace inet {
namespace ieee80211 {

using namespace inet::physicallayer;

static const IIeee80211Mode *getConfiguredControlResponseMode(const Ieee80211ModeSet *modeSet, const IIeee80211Mode *mode)
{
    // Explicit legacy/VHT overrides retain their requested mode; HT rates need the
    // bounded non-HT control-response conversion for ACK/BlockAck.
    return dynamic_cast<const Ieee80211HtMode *>(mode) != nullptr ? modeSet->getNonHtControlResponseMode(mode) : mode;
}

static const IIeee80211Mode *getDynamicControlResponseMode(const Ieee80211ModeSet *modeSet, const IIeee80211Mode *mode)
{
    if (dynamic_cast<const Ieee80211HtMode *>(mode) != nullptr || !modeSet->containsMode(mode))
        return modeSet->getNonHtControlResponseMode(mode);
    if (modeSet->getIsMandatory(mode))
        return mode;
    if (auto slowerMode = modeSet->getSlowerMandatoryMode(mode))
        return slowerMode;
    return modeSet->getNonHtControlResponseMode(mode);
}

Define_Module(RateSelection);

void RateSelection::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        getContainingNicModule(this)->subscribe(modesetChangedSignal, this);
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        dataOrMgmtRateControl = dynamic_cast<IRateControl *>(findModuleByPath(par("rateControlModule")));
        double multicastFrameBitrate = par("multicastFrameBitrate");
        multicastFrameMode = (multicastFrameBitrate == -1) ? nullptr : modeSet->getMode(bps(multicastFrameBitrate));
        double dataFrameBitrate = par("dataFrameBitrate");
        dataFrameMode = (dataFrameBitrate == -1) ? nullptr : modeSet->getMode(bps(dataFrameBitrate), Hz(par("dataFrameBandwidth")), par("dataFrameNumSpatialStreams"));
        double mgmtFrameBitrate = par("mgmtFrameBitrate");
        mgmtFrameMode = (mgmtFrameBitrate == -1) ? nullptr : modeSet->getMode(bps(mgmtFrameBitrate));
        double controlFrameBitrate = par("controlFrameBitrate");
        controlFrameMode = (controlFrameBitrate == -1) ? nullptr : modeSet->getMode(bps(controlFrameBitrate));
        resolveConfiguredResponseModes();
        fastestMandatoryMode = modeSet->getFastestMandatoryMode();
//        WATCH(dataOrMgmtRateControl);

//        WATCH(*((cObject**)&fastestMandatoryMode));
//        WATCH(*((cObject**)&modeSet));
        WATCH(lastTransmittedFrameMode);
//        WATCH(*((cObject**)&multicastFrameMode));
//        WATCH(*((cObject**)&dataFrameMode));
//        WATCH(*((cObject**)&mgmtFrameMode));
//        WATCH(*((cObject**)&controlFrameMode));
//        WATCH(*((cObject**)&responseAckFrameMode));
//        WATCH(*((cObject**)&responseCtsFrameMode));
//        WATCH();
//        WATCH();
//        WATCH();
//        WATCH();
    }
}

void RateSelection::resolveConfiguredResponseModes()
{
    if (modeSet == nullptr)
        return;
    double responseAckFrameBitrate = par("responseAckFrameBitrate");
    responseAckFrameMode = responseAckFrameBitrate == -1 ? nullptr : modeSet->getMode(bps(responseAckFrameBitrate));
    double responseCtsFrameBitrate = par("responseCtsFrameBitrate");
    responseCtsFrameMode = responseCtsFrameBitrate == -1 ? nullptr : modeSet->getMode(bps(responseCtsFrameBitrate));
}

const IIeee80211Mode *RateSelection::getMode(Packet *packet, const Ptr<const Ieee80211MacHeader>& header)
{
    const auto& modeReqTag = packet->findTag<Ieee80211ModeReq>();
    if (modeReqTag)
        return modeReqTag->getMode();
    const auto& modeIndTag = packet->findTag<Ieee80211ModeInd>();
    if (modeIndTag)
        return modeIndTag->getMode();
    throw cRuntimeError("Missing mode");
}

//
// In order to allow the transmitting STA to calculate the contents of the Duration/ID field, the responding STA
// shall transmit its Control Response frame (either CTS or ACK) at the same rate as the immediately previous
// frame in the frame exchange sequence (as defined in 9.7), if this rate belongs to the PHY mandatory rates, or
// else at the highest possible rate belonging to the PHY rates in the BSSBasicRateSet.
//
const IIeee80211Mode *RateSelection::computeResponseAckFrameMode(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader)
{
    // Keep configured responses independent of the eliciting packet; callers may
    // resolve a configured response while constructing a test packet.
    if (responseAckFrameMode)
        return getConfiguredControlResponseMode(modeSet, responseAckFrameMode);
    auto mode = getMode(packet, dataOrMgmtHeader);
    ASSERT(modeSet->supportsMode(mode));
    // IEEE 802.11-2024 10.6.6.1/10.6.6.5.2: this bounded model uses a
    // mandatory non-HT rate for ordinary ACK responses; BSSBasicRateSet is not modelled.
    return getDynamicControlResponseMode(modeSet, mode);
}

const IIeee80211Mode *RateSelection::computeResponseCtsFrameMode(Packet *packet, const Ptr<const Ieee80211RtsFrame>& rtsFrame)
{
    auto mode = getMode(packet, rtsFrame);
    ASSERT(modeSet->supportsMode(mode));
    auto responseMode = responseCtsFrameMode ? responseCtsFrameMode : mode;
    if (dynamic_cast<const Ieee80211HtMode *>(mode) != nullptr) {
        // IEEE 802.11-2024 10.6.6.1 and 10.6.6.5.7 require an HT response to HT RTS; HT-GF is never a response.
        return responseCtsFrameMode && dynamic_cast<const Ieee80211HtMode *>(responseMode) == nullptr ? responseMode : modeSet->getControlResponseMode(responseMode);
    }
    return responseCtsFrameMode ? getConfiguredControlResponseMode(modeSet, responseMode) : getDynamicControlResponseMode(modeSet, responseMode);
}

// 802.11-1999 Std.
//
// All frames with multicast and broadcast RA shall be transmitted at one of the rates included in the
// BSSBasicRateSet, regardless of their type.
//
// TODO Data and/or management MPDUs with a unicast immediate address shall be sent on any supported data rate
// selected by the rate switching mechanism (whose output is an internal MAC variable called MACCurrentRate,
// defined in units of 500 kbit/s, which is used for calculating the Duration/ID field of each frame). A STA shall
// not transmit at a rate that is known not to be supported by the destination STA, as reported in the supported
// rates element in the management frames. For frames of type Data+CF-ACK, Data+CF-Poll+CF-ACK, and CF-
// Poll+CF-ACK, the rate chosen to transmit the frame must be supported by both the addressed recipient STA
// and the STA to which the ACK is intended.
//
const IIeee80211Mode *RateSelection::computeDataOrMgmtFrameMode(const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader)
{
    if (dataOrMgmtHeader->getReceiverAddress().isMulticast() && multicastFrameMode)
        return multicastFrameMode;
    if (dynamicPtrCast<const Ieee80211DataHeader>(dataOrMgmtHeader) && dataFrameMode)
        return dataFrameMode;
    if (dynamicPtrCast<const Ieee80211MgmtHeader>(dataOrMgmtHeader) && mgmtFrameMode)
        return mgmtFrameMode;
    if (dataOrMgmtRateControl)
        return dataOrMgmtRateControl->getRate();
    else
        return fastestMandatoryMode;
}

// 802.11-1999 Std.
//
// All Control frames shall be transmitted at one of the rates in the BSSBasicRateSet
// (see 10.3.10.1), or at one of the rates in the PHY mandatory rate set so they will
// be understood by all STAs.
//
const IIeee80211Mode *RateSelection::computeControlFrameMode(const Ptr<const Ieee80211MacHeader>& header)
{
    // TODO BSSBasicRateSet
    return fastestMandatoryMode;
}

const IIeee80211Mode *RateSelection::computeMode(Packet *packet, const Ptr<const Ieee80211MacHeader>& header)
{
    if (auto dataOrMgmtHeader = dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(header))
        return computeDataOrMgmtFrameMode(dataOrMgmtHeader);
    else
        return computeControlFrameMode(header);
}

void RateSelection::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));

    if (signalID == modesetChangedSignal) {
        modeSet = check_and_cast<Ieee80211ModeSet *>(obj);
        fastestMandatoryMode = modeSet->getFastestMandatoryMode();
        resolveConfiguredResponseModes();
    }
}

void RateSelection::frameTransmitted(Packet *packet, const Ptr<const Ieee80211MacHeader>& header)
{
    auto receiverAddr = header->getReceiverAddress();
    lastTransmittedFrameMode[receiverAddr] = getMode(packet, header);
}

void RateSelection::setFrameMode(Packet *packet, const Ptr<const Ieee80211MacHeader>& header, const IIeee80211Mode *mode)
{
    ASSERT(mode != nullptr);
    packet->addTagIfAbsent<Ieee80211ModeReq>()->setMode(mode);
}

} // namespace ieee80211
} // namespace inet
