//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/rateselection/RateSelection.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"
#include "inet/linklayer/ieee80211/mac/contract/IRateControl.h"
#include "inet/linklayer/ieee80211/mac/rateselection/Ieee80211PeerModeSelection.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/physicallayer/wireless/ieee80211/mode/IIeee80211Mode.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211ModeSet.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Tag_m.h"

namespace inet {
namespace ieee80211 {

using namespace inet::physicallayer;

Define_Module(RateSelection);

void RateSelection::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        mib.reference(this, "mibModule", true);
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
        double responseAckFrameBitrate = par("responseAckFrameBitrate");
        responseAckFrameMode = (responseAckFrameBitrate == -1) ? nullptr : modeSet->getMode(bps(responseAckFrameBitrate));
        double responseCtsFrameBitrate = par("responseCtsFrameBitrate");
        responseCtsFrameMode = (responseCtsFrameBitrate == -1) ? nullptr : modeSet->getMode(bps(responseCtsFrameBitrate));
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

void RateSelection::ensurePerReceiverModesResolved()
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
    if (responseAckFrameMode)
        return getPeerCompatibleMode(dataOrMgmtHeader->getTransmitterAddress(), responseAckFrameMode);
    else {
        auto mode = getMode(packet, dataOrMgmtHeader);
        ASSERT(modeSet->containsMode(mode));
        auto responseMode = modeSet->getIsMandatory(mode) ? mode : modeSet->getSlowerMandatoryMode(mode); // TODO BSSBasicRateSet
        return getPeerCompatibleMode(dataOrMgmtHeader->getTransmitterAddress(), responseMode);
    }
}

const IIeee80211Mode *RateSelection::computeResponseCtsFrameMode(Packet *packet, const Ptr<const Ieee80211RtsFrame>& rtsFrame)
{
    if (responseCtsFrameMode)
        return getPeerCompatibleMode(rtsFrame->getTransmitterAddress(), responseCtsFrameMode);
    else {
        auto mode = getMode(packet, rtsFrame);
        ASSERT(modeSet->containsMode(mode));
        auto responseMode = modeSet->getIsMandatory(mode) ? mode : modeSet->getSlowerMandatoryMode(mode); // TODO BSSBasicRateSet
        return getPeerCompatibleMode(rtsFrame->getTransmitterAddress(), responseMode);
    }
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
    // Per-receiver override for originated unicast data frames (see dataFrameBitratePerReceiver).
    // Wins over the interface-wide dataFrameMode / rate control; group-addressed and management
    // frames are left to the existing rules below.
    if (dynamicPtrCast<const Ieee80211DataHeader>(dataOrMgmtHeader) && !dataOrMgmtHeader->getReceiverAddress().isMulticast()) {
        ensurePerReceiverModesResolved();
        auto it = perReceiverDataFrameMode.find(dataOrMgmtHeader->getReceiverAddress());
        if (it != perReceiverDataFrameMode.end())
            return getPeerCompatibleMode(dataOrMgmtHeader->getReceiverAddress(), it->second);
    }
    if (dataOrMgmtHeader->getReceiverAddress().isMulticast() && multicastFrameMode)
        return getPeerCompatibleMode(dataOrMgmtHeader->getReceiverAddress(), multicastFrameMode);
    if (dynamicPtrCast<const Ieee80211DataHeader>(dataOrMgmtHeader) && dataFrameMode)
        return getPeerCompatibleMode(dataOrMgmtHeader->getReceiverAddress(), dataFrameMode);
    if (dynamicPtrCast<const Ieee80211MgmtHeader>(dataOrMgmtHeader) && mgmtFrameMode)
        return getPeerCompatibleMode(dataOrMgmtHeader->getReceiverAddress(), mgmtFrameMode);
    // Rate control adapts to the feedback of one peer, and a group-addressed frame has no peer:
    // it is never acknowledged, so nothing would ever correct a rate chosen for it. Group-addressed
    // frames therefore take a mandatory rate, as the clause above requires.
    if (dataOrMgmtRateControl && !dataOrMgmtHeader->getReceiverAddress().isMulticast())
        return getPeerCompatibleMode(dataOrMgmtHeader->getReceiverAddress(), dataOrMgmtRateControl->getRate(dataOrMgmtHeader->getReceiverAddress()));
    else
        return getPeerCompatibleMode(dataOrMgmtHeader->getReceiverAddress(), fastestMandatoryMode);
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
    return getPeerCompatibleMode(header->getReceiverAddress(), fastestMandatoryMode);
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

void RateSelection::emitDatarateSelected(cComponent *emitter, const Ptr<const Ieee80211MacHeader>& header, const IIeee80211Mode *mode)
{
    double rate = mode->getDataMode()->getNetBitrate().get<bps>();
    auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(header);
    // naming the station sweeps the network, so skip it if nothing listens anyway
    if (dataHeader != nullptr && !dataHeader->getReceiverAddress().isMulticast() && emitter->mayHaveListeners(IRateSelection::datarateSelectedSignal)) {
        cNamedObject details(L3AddressResolver().getHostNameWithMacAddress(dataHeader->getReceiverAddress()).c_str());
        emitter->emit(IRateSelection::datarateSelectedSignal, rate, &details);
    }
    else
        emitter->emit(IRateSelection::datarateSelectedSignal, rate);
}

const IIeee80211Mode *RateSelection::getPeerCompatibleMode(const MacAddress& peerAddress, const IIeee80211Mode *mode) const
{
    if (mode == nullptr || peerAddress.isMulticast() || !mib || mode->getHtMcsIndex() < 0)
        return mode;
    return selectPeerCompatibleMode(modeSet, mib->findPeerHtState(peerAddress), mode, peerAddress);
}

} // namespace ieee80211
} // namespace inet

