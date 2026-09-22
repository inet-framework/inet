//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/Ieee80211Mac.h"

#include <algorithm>

#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/physicallayer/wireless/ieee80211/contract/packetlevel/IIeee80211ModeSetCoordinator.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/common/UserPriorityTag_m.h"
#include "inet/linklayer/ieee80211/llc/IIeee80211Llc.h"
#include "inet/linklayer/ieee80211/llc/LlcProtocolTag_m.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211SubtypeTag_m.h"
#include "inet/linklayer/ieee80211/mac/Rx.h"
#include "inet/linklayer/ieee80211/mac/contract/IContention.h"
#include "inet/linklayer/ieee80211/mac/contract/IRx.h"
#include "inet/linklayer/ieee80211/mac/contract/ITx.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211Channel.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211ControlInfo_m.h"
#include "inet/physicallayer/wireless/ieee80211/contract/IIeee80211ReceiverCapabilities.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Tag_m.h"
#include "inet/physicallayer/wireless/ieee80211/contract/IIeee80211TransmitterCapabilities.h"

namespace inet {
namespace ieee80211 {

using namespace inet::physicallayer;

Define_Module(Ieee80211Mac);

simsignal_t Ieee80211Mac::frameTransmissionOutcomeSignal = cComponent::registerSignal("frameTransmissionOutcome");

Ieee80211Mac::Ieee80211Mac()
{
}

Ieee80211Mac::~Ieee80211Mac()
{
    if (pendingRadioConfigMsg)
        delete pendingRadioConfigMsg;
}

void Ieee80211Mac::initialize(int stage)
{
    MacProtocolBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        modeSet = Ieee80211ModeSet::getModeSet(par("modeSet"));
        fcsMode = parseFcsMode(par("fcsMode"));
        mib.reference(this, "mibModule", true);
        mib->qos = par("qosStation");
        radio = check_and_cast<IRadio *>(gate("lowerLayerOut")->getNextGate()->getOwnerModule());
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        cModule *llcModule = gate("upperLayerOut")->getNextGate()->getOwnerModule();
        llc = check_and_cast<IIeee80211Llc *>(llcModule);
        cModule *radioModule = gate("lowerLayerOut")->getNextGate()->getOwnerModule();
        radioModule->subscribe(IRadio::radioModeChangedSignal, this);
        radioModule->subscribe(IRadio::receptionStateChangedSignal, this);
        radioModule->subscribe(IRadio::transmissionStateChangedSignal, this);
        radioModule->subscribe(IRadio::receivedSignalPartChangedSignal, this);
        radio = check_and_cast<IRadio *>(radioModule);
        ds = check_and_cast<IDs *>(getSubmodule("ds"));
        rx = check_and_cast<IRx *>(getSubmodule("rx"));
        tx = check_and_cast<ITx *>(getSubmodule("tx"));
        prepareLocalCapabilities();
        if (isUp())
            initializeRadioMode();
        rx = check_and_cast<IRx *>(getSubmodule("rx"));
        tx = check_and_cast<ITx *>(getSubmodule("tx"));
        dcf = check_and_cast<Dcf *>(getSubmodule("dcf"));
        hcf = check_and_cast_nullable<Hcf *>(getSubmodule("hcf"));
        if (mib->qos && !hcf)
            throw cRuntimeError("Missing hcf module, required for QoS");
    }
    else if (stage == INITSTAGE_NETWORK_CONFIGURATION)
        modeSetInitialized = true;
}

void Ieee80211Mac::prepareLocalCapabilities()
{
    Enter_Method("prepareLocalCapabilities");
    if (mib->hasPreparedLocalCapabilities())
        return;
    updateLocalHtCapabilities();
}

void Ieee80211Mac::updateLocalHtCapabilities(bool reconfiguration)
{
    updateLocalVhtCapabilities();
    if (!modeSet->isHtOperationSupported()) {
        if (reconfiguration)
            mib->reconfigureLocalHtCapabilities(Ieee80211HtCapabilities(), false);
        else
            mib->installLocalHtCapabilities(Ieee80211HtCapabilities(), false);
        return;
    }
    auto *configuredRadio = check_and_cast<IRadio *>(gate("lowerLayerOut")->getNextGate()->getOwnerModule());
    const auto *transmitter = dynamic_cast<const IIeee80211TransmitterCapabilities *>(configuredRadio->getTransmitter());
    const auto *receiver = dynamic_cast<const IIeee80211ReceiverCapabilities *>(configuredRadio->getReceiver());
    if (transmitter == nullptr || receiver == nullptr)
        throw cRuntimeError("HT operation requires transmitter and receiver capability providers");
    int operationalHtSpatialStreamLimit = std::min(configuredRadio->getAntenna()->getNumAntennas(),
            modeSet->getMaximumNumberOfSpatialStreams());
    std::set<Hz> operationalChannelWidths;
    for (auto width : modeSet->getHtSupportedChannelWidths())
        if (transmitter->isHtChannelWidthSupported(width) && receiver->isHtChannelWidthSupported(width))
            operationalChannelWidths.insert(width);
    Ieee80211HtCapabilities localHtCapabilities;
    if (operationalHtSpatialStreamLimit <= 0)
        throw cRuntimeError("HT operation requires a positive operational spatial-stream limit");

    // IEEE Std 802.11-2024, 9.4.2.54.4 and 9.4.2.55: advertise exactly the
    // HT modes from the authoritative mode set, while advertised channel
    // widths are restricted to those the configured transmitter and receiver
    // can actually operate. In particular, do not infer dense MCS blocks or HT
    // widths from legacy/VHT modes that happen to share the set.
    for (auto channelWidth : modeSet->getHtSupportedChannelWidths())
        if (operationalChannelWidths.count(channelWidth) != 0)
            localHtCapabilities.supportedChannelWidths.insert(channelWidth);
    localHtCapabilities.shortGi20 = localHtCapabilities.supportedChannelWidths.count(MHz(20)) != 0 &&
            modeSet->isHtShortGuardIntervalSupported(MHz(20)) && receiver->isHtShortGuardIntervalSupported(MHz(20));
    localHtCapabilities.shortGi40 = localHtCapabilities.supportedChannelWidths.count(MHz(40)) != 0 &&
            modeSet->isHtShortGuardIntervalSupported(MHz(40)) && receiver->isHtShortGuardIntervalSupported(MHz(40));
    for (int index = 0; index < modeSet->getNumModes(); index++) {
        const auto *mode = modeSet->getMode(index);
        int mcs = mode->getHtMcsIndex();
        if (mcs >= 0 && mcs < 77 && operationalChannelWidths.count(mode->getDataMode()->getBandwidth()) != 0 &&
                mode->getDataMode()->getNumberOfSpatialStreams() <= operationalHtSpatialStreamLimit)
            localHtCapabilities.rxMcsSupported[mcs] = true;
    }
    // The equal-case Tx MCS set is represented by the maximum MCS index per
    // spatial-stream group. Rebuild it from the filtered Rx bitmap; MCS 32 is
    // not part of this map's MCS 0..31 NSS encoding.
    localHtCapabilities.greenfield = modeSet->isHtGreenfieldSupported();
    localHtCapabilities.txMcsNss = Ieee80211HtMcsNssMap();
    for (int mcs = 0; mcs < 32; mcs++) {
        if (localHtCapabilities.rxMcsSupported[mcs]) {
            int nss = mcs / 8;
            localHtCapabilities.txMcsNss.maxMcsPerNss[nss] = std::max(localHtCapabilities.txMcsNss.maxMcsPerNss[nss], mcs % 8);
        }
    }
    if (localHtCapabilities.supportedChannelWidths.empty())
        throw cRuntimeError("HT operation mode set '%s' does not provide an HT channel width", modeSet->getName());
    localHtCapabilities.maxAmpduLengthExponent = mib->par("htMaxAmpduLengthExponent");
    if (localHtCapabilities.maxAmpduLengthExponent < 0 || localHtCapabilities.maxAmpduLengthExponent > 3)
        throw cRuntimeError("htMaxAmpduLengthExponent must be between 0 and 3");

    if (reconfiguration)
        mib->reconfigureLocalHtCapabilities(localHtCapabilities, true);
    else
        mib->installLocalHtCapabilities(localHtCapabilities, true);
}

void Ieee80211Mac::updateLocalVhtCapabilities()
{
    Ieee80211VhtCapabilities localVhtCapabilities;
    bool supported = modeSet != nullptr && modeSet->getPhyType() == Ieee80211ModeSet::PhyType::VHT && mib->par("vhtSupported");
    if (!supported) {
        mib->installLocalVhtCapabilities(localVhtCapabilities, false);
        return;
    }
    int spatialStreamLimit = std::min(radio->getAntenna()->getNumAntennas(), modeSet->getMaximumNumberOfSpatialStreams());
    // Advertised maps are bounded by the actual long-GI primary-20 catalog,
    // as well as the configured directional and antenna limits.
    std::array<std::array<bool, 10>, 8> catalogMcs = {};
    for (int i = 0; i < modeSet->getNumModes(); i++) {
        auto mode = modeSet->getMode(i);
        auto data = mode->getDataMode();
        int mcs = mode->getVhtMcsIndex();
        int nss = data->getNumberOfSpatialStreams();
        if (mcs >= 0 && mcs <= 9 && nss >= 1 && nss <= 8 && data->getBandwidth() == MHz(20) &&
                data->getGuardInterval() == SimTime(800, SIMTIME_NS))
            catalogMcs[nss - 1][mcs] = true;
    }
    auto readMap = [&](const char *parameter, std::array<int, 8>& map) {
        auto values = check_and_cast<cValueArray *>(mib->par(parameter).objectValue());
        if (values->size() != 8)
            throw cRuntimeError("%s requires eight per-NSS MCS maxima", parameter);
        for (int i = 0; i < 8; i++) {
            int value = values->get(i).intValue();
            if (value != -1 && value != 7 && value != 8 && value != 9)
                throw cRuntimeError("%s entries must be -1, 7, 8 or 9", parameter);
            int maximum = -1;
            for (int mcs = 0; mcs <= value && catalogMcs[i][mcs]; mcs++)
                if (mcs >= 7)
                    maximum = mcs;
            map[i] = i < spatialStreamLimit ? maximum : -1;
        }
        if (!isValidVhtMcsMap(map))
            throw cRuntimeError("%s and the VHT catalog must support MCS 0 through 7 at one spatial stream", parameter);
    };
    readMap("vhtRxMcsMap", localVhtCapabilities.rxMaxMcs);
    readMap("vhtTxMcsMap", localVhtCapabilities.txMaxMcs);
    // Intentional limitation of the current packet-level primary-channel PHY:
    // management operates the existing VHT-only profile at 20 MHz. Its catalog
    // is broader, but does not establish primary/secondary channel support.
    // No HT SGI negotiation is claimed by that profile, so 20 MHz uses long GI.
    mib->installLocalVhtCapabilities(localVhtCapabilities, true);
}

void Ieee80211Mac::initializeRadioMode()
{
    const char *initialRadioMode = par("initialRadioMode");
    if (!strcmp(initialRadioMode, "off"))
        radio->setRadioMode(IRadio::RADIO_MODE_OFF);
    else if (!strcmp(initialRadioMode, "sleep"))
        radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
    else if (!strcmp(initialRadioMode, "receiver"))
        radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
    else if (!strcmp(initialRadioMode, "transmitter"))
        radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
    else if (!strcmp(initialRadioMode, "transceiver"))
        radio->setRadioMode(IRadio::RADIO_MODE_TRANSCEIVER);
    else
        throw cRuntimeError("Unknown initialRadioMode");
}

const MacAddress& Ieee80211Mac::isInterfaceRegistered()
{
//    if (!par("multiMac"))
//        return MacAddress::UNSPECIFIED_ADDRESS;
    IInterfaceTable *ift = findModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
    if (!ift)
        return MacAddress::UNSPECIFIED_ADDRESS;
    cModule *interfaceModule = findModuleUnderContainingNode(this);
    if (!interfaceModule)
        throw cRuntimeError("NIC module not found in the host");
    std::string interfaceName = utils::stripnonalnum(interfaceModule->getFullName());
    NetworkInterface *e = ift->findInterfaceByName(interfaceName.c_str());
    if (e)
        return e->getMacAddress();
    return MacAddress::UNSPECIFIED_ADDRESS;
}

void Ieee80211Mac::configureNetworkInterface()
{
    // TODO the mib module should use the mac address from NetworkInterface
    mib->address = networkInterface->getMacAddress();
    networkInterface->setMtu(par("mtu"));
    // capabilities
    networkInterface->setBroadcast(true);
    networkInterface->setMulticast(true);
    networkInterface->setPointToPoint(false);
}

void Ieee80211Mac::handleMessageWhenUp(cMessage *message)
{
    if (message->arrivedOn("mgmtIn")) {
        if (!message->isPacket())
            handleUpperCommand(message);
        else
            handleMgmtPacket(check_and_cast<Packet *>(message));
    }
    else
        LayeredProtocolBase::handleMessageWhenUp(message);
}

void Ieee80211Mac::handleSelfMessage(cMessage *msg)
{
    ASSERT(false);
}

void Ieee80211Mac::handleMgmtPacket(Packet *packet)
{
    const auto& header = makeShared<Ieee80211MgmtHeader>();
    header->setType((Ieee80211FrameType)packet->getTag<Ieee80211SubtypeReq>()->getSubtype());
    header->setReceiverAddress(packet->getTag<MacAddressReq>()->getDestAddress());
    if (mib->mode == Ieee80211Mib::INFRASTRUCTURE && mib->getBssStationData().stationType == Ieee80211Mib::ACCESS_POINT)
        header->setAddress3(mib->getBssData().bssid);
    packet->insertAtFront(header);
    packet->insertAtBack(makeShared<Ieee80211MacTrailer>());
    processUpperFrame(packet, header);
}

void Ieee80211Mac::handleUpperPacket(Packet *packet)
{
    if (mib->mode == Ieee80211Mib::INFRASTRUCTURE && mib->getBssStationData().stationType == Ieee80211Mib::STATION && !mib->getBssStationData().isAssociated) {
        EV << "STA is not associated with an access point, discarding packet " << packet << "\n";
        PacketDropDetails details;
        details.setReason(OTHER_PACKET_DROP);
        emit(packetDroppedSignal, packet, &details);
        delete packet;
        return;
    }
    encapsulate(packet);
    const auto& header = packet->peekAtFront<Ieee80211DataOrMgmtHeader>();
    if (mib->mode == Ieee80211Mib::INFRASTRUCTURE && mib->getBssStationData().stationType == Ieee80211Mib::ACCESS_POINT) {
        auto receiverAddress = header->getReceiverAddress();
        if (!receiverAddress.isMulticast()) {
            auto it = mib->getBssAccessPointData().stations.find(receiverAddress);
            if (it == mib->getBssAccessPointData().stations.end() || it->second != Ieee80211Mib::ASSOCIATED) {
                EV << "STA with MAC address " << receiverAddress << " not associated with this AP, dropping frame\n";
                PacketDropDetails details;
                details.setReason(OTHER_PACKET_DROP);
                emit(packetDroppedSignal, packet, &details);
                delete packet;
                return;
            }
        }
    }
    processUpperFrame(packet, header);
}

void Ieee80211Mac::handleLowerPacket(Packet *packet)
{
    if (rx->lowerFrameReceived(packet)) {
        auto header = packet->peekAtFront<Ieee80211MacHeader>();
        processLowerFrame(packet, header);
    }
    else { // corrupted frame received
        if (mib->qos)
            hcf->corruptedFrameReceived();
        else
            dcf->corruptedFrameReceived();
    }
}

void Ieee80211Mac::handleUpperCommand(cMessage *msg)
{
    if (msg->getKind() == RADIO_C_CONFIGURE) {
        EV_DEBUG << "Passing on command " << msg->getName() << " to physical layer\n";
        if (pendingRadioConfigMsg != nullptr) {
            // merge contents of the old command into the new one, then delete it
            Ieee80211ConfigureRadioCommand *oldConfigureCommand = check_and_cast<Ieee80211ConfigureRadioCommand *>(pendingRadioConfigMsg->getControlInfo());
            Ieee80211ConfigureRadioCommand *newConfigureCommand = check_and_cast<Ieee80211ConfigureRadioCommand *>(msg->getControlInfo());
            if (newConfigureCommand->getChannelNumber() == -1 && oldConfigureCommand->getChannelNumber() != -1)
                newConfigureCommand->setChannelNumber(oldConfigureCommand->getChannelNumber());
            if (std::isnan(newConfigureCommand->getBitrate().get<bps>()) && !std::isnan(oldConfigureCommand->getBitrate().get<bps>()))
                newConfigureCommand->setBitrate(oldConfigureCommand->getBitrate());
            delete pendingRadioConfigMsg;
            pendingRadioConfigMsg = nullptr;
        }

        if (rx->isMediumFree()) { // TODO this should be just the physical channel sense!!!!
            EV_DEBUG << "Sending it down immediately\n";
//            PhyControlInfo *phyControlInfo = dynamic_cast<PhyControlInfo *>(msg->getControlInfo());
//            if (phyControlInfo)
//                phyControlInfo->setAdaptiveSensitivity(true);
            // end dynamic power
            sendDown(msg);
        }
        else {
            // TODO waiting potentially indefinitely?! wtf?!
            EV_DEBUG << "Delaying " << msg->getName() << " until next IDLE or DEFER state\n";
            pendingRadioConfigMsg = msg;
        }
    }
    else {
        throw cRuntimeError("Unrecognized command from mgmt layer: (%s)%s msgkind=%d", msg->getClassName(), msg->getName(), msg->getKind());
    }
}

void Ieee80211Mac::encapsulate(Packet *packet)
{
    packet->addTagIfAbsent<LlcProtocolTag>()->setProtocol(packet->getTag<PacketProtocolTag>()->getProtocol());
    auto macAddressReq = packet->getTag<MacAddressReq>();
    auto destAddress = macAddressReq->getDestAddress();
    const auto& header = makeShared<Ieee80211DataHeader>();
    header->setTransmitterAddress(mib->address);
    if (mib->mode == Ieee80211Mib::INDEPENDENT)
        header->setReceiverAddress(destAddress);
    else if (mib->mode == Ieee80211Mib::INFRASTRUCTURE) {
        if (mib->getBssStationData().stationType == Ieee80211Mib::ACCESS_POINT) {
            header->setFromDS(true);
            header->setAddress3(mib->address);
            header->setReceiverAddress(destAddress);
        }
        else if (mib->getBssStationData().stationType == Ieee80211Mib::STATION) {
            header->setToDS(true);
            header->setReceiverAddress(mib->getBssData().bssid);
            header->setAddress3(destAddress);
        }
        else
            throw cRuntimeError("Unknown station type");
    }
    else
        throw cRuntimeError("Unknown mode");
    if (auto userPriorityReq = packet->findTag<UserPriorityReq>()) {
        // make it a QoS frame, and set TID
        header->setType(ST_DATA_WITH_QOS);
        header->addChunkLength(QOSCONTROL_PART_LENGTH);
        header->setTid(userPriorityReq->getUserPriority());
    }
    packet->insertAtFront(header);
    packet->insertAtBack(makeShared<Ieee80211MacTrailer>());
    auto packetProtocolTag = packet->addTagIfAbsent<PacketProtocolTag>();
    packetProtocolTag->setProtocol(&Protocol::ieee80211Mac);
}

void Ieee80211Mac::decapsulate(Packet *packet)
{
    const auto& header = packet->popAtFront<Ieee80211DataOrMgmtHeader>();
    auto packetProtocolTag = packet->addTagIfAbsent<PacketProtocolTag>();
    if (dynamicPtrCast<const Ieee80211DataHeader>(header))
        packetProtocolTag->setProtocol(llc->getProtocol());
    else if (dynamicPtrCast<const Ieee80211MgmtHeader>(header))
        packetProtocolTag->setProtocol(&Protocol::ieee80211Mgmt);
    auto macAddressInd = packet->addTagIfAbsent<MacAddressInd>();
    if (mib->mode == Ieee80211Mib::INDEPENDENT) {
        macAddressInd->setSrcAddress(header->getTransmitterAddress());
        macAddressInd->setDestAddress(header->getReceiverAddress());
    }
    else if (mib->mode == Ieee80211Mib::INFRASTRUCTURE) {
        if (mib->getBssStationData().stationType == Ieee80211Mib::ACCESS_POINT) {
            macAddressInd->setSrcAddress(header->getTransmitterAddress());
            macAddressInd->setDestAddress(header->getAddress3());
        }
        else if (mib->getBssStationData().stationType == Ieee80211Mib::STATION) {
            macAddressInd->setSrcAddress(header->getAddress3());
            macAddressInd->setDestAddress(header->getReceiverAddress());
        }
        else
            throw cRuntimeError("Unknown station type");
    }
    else
        throw cRuntimeError("Unknown mode");
    if (header->getType() == ST_DATA_WITH_QOS) {
        auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(header);
        int tid = dataHeader->getTid();
        if (tid < 8)
            packet->addTagIfAbsent<UserPriorityInd>()->setUserPriority(tid);
    }
    packet->addTagIfAbsent<InterfaceInd>()->setInterfaceId(networkInterface->getInterfaceId());
    packet->popAtBack<Ieee80211MacTrailer>(B(4));
}

void Ieee80211Mac::receiveSignal(cComponent *source, simsignal_t signalID, intval_t value, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));

    if (signalID == IRadio::receptionStateChangedSignal) {
        rx->receptionStateChanged(static_cast<IRadio::ReceptionState>(value));
    }
    else if (signalID == IRadio::transmissionStateChangedSignal) {
        auto oldTransmissionState = transmissionState;
        transmissionState = static_cast<IRadio::TransmissionState>(value);
        bool transmissionFinished = (oldTransmissionState == IRadio::TRANSMISSION_STATE_TRANSMITTING && transmissionState == IRadio::TRANSMISSION_STATE_IDLE);
        if (transmissionFinished) {
            tx->radioTransmissionFinished();
            EV_DEBUG << "changing radio to receiver mode\n";
            configureRadioMode(IRadio::RADIO_MODE_RECEIVER); // FIXME this is in a very wrong place!!! should be done explicitly from coordination function!
        }
        rx->transmissionStateChanged(transmissionState);
    }
    else if (signalID == IRadio::receivedSignalPartChangedSignal) {
        rx->receivedSignalPartChanged(static_cast<IRadioSignal::SignalPart>(value));
    }
}

void Ieee80211Mac::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));
    // Mode-set application uses the coordinator contract, not notifications.
}

void Ieee80211Mac::applyModeSet(const physicallayer::Ieee80211ModeSet *newModeSet)
{
    Enter_Method_Silent();
    modeSet = const_cast<physicallayer::Ieee80211ModeSet *>(newModeSet);
    updateLocalHtCapabilities(true);
}

void Ieee80211Mac::registerModeSetConsumer(cModule *consumer, Phase phase)
{
    Enter_Method_Silent();
    if (changingModeSet || modeSetInitialized)
        throw cRuntimeError("Mode-set consumers must register before initialization completes");
    if (consumer == nullptr || consumer == this || getContainingNicModule(consumer) != getContainingNicModule(this) ||
            dynamic_cast<physicallayer::IIeee80211ModeSetListener *>(consumer) == nullptr)
        throw cRuntimeError("Mode-set consumer must implement the transition contract");
    if (phase != MANAGEMENT_STATE && phase != DERIVED_STATE)
        throw cRuntimeError("Invalid mode-set consumer phase");
    auto result = modeSetConsumers.emplace(consumer->getId(), phase);
    if (!result.second && result.first->second != phase)
        throw cRuntimeError("Mode-set consumer registered in two phases");
}

void Ieee80211Mac::unregisterModeSetConsumer(cModule *consumer)
{
    Enter_Method_Silent();
    if (changingModeSet)
        throw cRuntimeError("Cannot detach a mode-set consumer during a transition");
    if (consumer == nullptr)
        throw cRuntimeError("Cannot detach a null mode-set consumer");
    if (getContainingNicModule(consumer) != getContainingNicModule(this))
        throw cRuntimeError("Cannot detach a mode-set consumer from another interface");
    modeSetConsumers.erase(consumer->getId());
}

void Ieee80211Mac::beginModeSetChange(const Ieee80211ModeSet *newModeSet)
{
    Enter_Method_Silent();
    if (changingModeSet)
        throw cRuntimeError("Reentrant interface mode-set change");
    if (!modeSetInitialized || !mib->hasPreparedLocalCapabilities())
        throw cRuntimeError("Cannot reconfigure before mode-set initialization completes");
    if (newModeSet == nullptr)
        throw cRuntimeError("Cannot clear the mode set of an IEEE 802.11 interface");
    if (std::none_of(modeSetConsumers.begin(), modeSetConsumers.end(), [](const auto& entry) { return entry.second == MANAGEMENT_STATE; }))
        throw cRuntimeError("Required management mode-set consumer is not registered");
    for (const auto& entry : modeSetConsumers)
        if (getSimulation()->getModule(entry.first) == nullptr)
            throw cRuntimeError("Mode-set consumer was deleted without unregistering");
    pendingModeSet = newModeSet;
    changingModeSet = true;
}

void Ieee80211Mac::completeModeSetChange(const Ieee80211ModeSet *newModeSet)
{
    Enter_Method_Silent();
    if (!changingModeSet || pendingModeSet != newModeSet)
        throw cRuntimeError("Mode-set completion does not match the pending transition");
    if (modeSet != newModeSet) {
        applyModeSet(newModeSet);
        for (auto phase : {MANAGEMENT_STATE, DERIVED_STATE})
            for (const auto& entry : modeSetConsumers)
                if (entry.second == phase)
                    check_and_cast<physicallayer::IIeee80211ModeSetListener *>(
                            getSimulation()->getModule(entry.first))->applyModeSet(newModeSet);
        mib->publishStateChange();
        emit(modesetChangedSignal, const_cast<Ieee80211ModeSet *>(modeSet));
    }
    pendingModeSet = nullptr;
    changingModeSet = false;
}

void Ieee80211Mac::configureRadioMode(IRadio::RadioMode radioMode)
{
    if (radio->getRadioMode() != radioMode) {
        ConfigureRadioCommand *configureCommand = new ConfigureRadioCommand();
        configureCommand->setRadioMode(radioMode);
        auto request = new Request("configureRadioMode", RADIO_C_CONFIGURE);
        request->setControlInfo(configureCommand);
        sendDown(request);
    }
}

void Ieee80211Mac::sendUp(cMessage *msg)
{
    Enter_Method("sendUp(\"%s\")", msg->getName());
    take(msg);
    MacProtocolBase::sendUp(msg);
}

void Ieee80211Mac::sendUpFrame(Packet *frame)
{
    Enter_Method("sendUpFrame(\"%s\")", frame->getName());
    take(frame);
    const auto& header = frame->peekAtFront<Ieee80211DataOrMgmtHeader>();
    decapsulate(frame);
    if (!(header->getType() & 0x30))
        send(frame, "mgmtOut");
    else
        ds->processDataFrame(frame, dynamicPtrCast<const Ieee80211DataHeader>(header));
}

void Ieee80211Mac::sendDownFrame(Packet *frame)
{
    Enter_Method("sendDownFrame(\"%s\")", frame->getName());
    take(frame);
    configureRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
    frame->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ieee80211Mac);
    sendDown(frame);
}

void Ieee80211Mac::sendDownPendingRadioConfigMsg()
{
    if (pendingRadioConfigMsg != nullptr) {
        sendDown(pendingRadioConfigMsg);
        pendingRadioConfigMsg = nullptr;
    }
}

void Ieee80211Mac::cancelManagementTransaction(uint64_t transactionId)
{
    Enter_Method("cancelManagementTransaction");
    if (mib->qos)
        hcf->cancelManagementTransaction(transactionId);
    else
        dcf->cancelManagementTransaction(transactionId);
}

void Ieee80211Mac::processUpperFrame(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& header)
{
    Enter_Method("processUpperFrame(\"%s\")", packet->getName());
    take(packet);
    EV_INFO << "Frame " << packet << " received from higher layer, receiver = " << header->getReceiverAddress() << "\n";
    ASSERT(!header->getReceiverAddress().isUnspecified());
    if (mib->qos)
        hcf->processUpperFrame(packet, header);
    else
        dcf->processUpperFrame(packet, header);
}

void Ieee80211Mac::processLowerFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header)
{
    Enter_Method("processLowerFrame(\"%s\")", packet->getName());
    take(packet);
    if (mib->qos)
        hcf->processLowerFrame(packet, header);
    else
        // TODO what if the received frame is ST_DATA_WITH_QOS? drop?
        dcf->processLowerFrame(packet, header);
}

// FIXME
void Ieee80211Mac::handleStartOperation(LifecycleOperation *operation)
{
    if (!operation)
        return; // do nothing when called from initialize()

    initializeRadioMode();
}

// FIXME
void Ieee80211Mac::handleStopOperation(LifecycleOperation *operation)
{
}

// FIXME
void Ieee80211Mac::handleCrashOperation(LifecycleOperation *operation)
{
}

} // namespace ieee80211
} // namespace inet
