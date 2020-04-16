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

#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/common/UserPriorityTag_m.h"
#include "inet/linklayer/ieee80211/llc/IIeee80211Llc.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Mac.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211SubtypeTag_m.h"
#include "inet/linklayer/ieee80211/mac/Rx.h"
#include "inet/linklayer/ieee80211/mac/contract/IContention.h"
#include "inet/linklayer/ieee80211/mac/contract/IFrameSequence.h"
#include "inet/linklayer/ieee80211/mac/contract/IRx.h"
#include "inet/linklayer/ieee80211/mac/contract/ITx.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211ControlInfo_m.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211Tag_m.h"

namespace inet {
namespace ieee80211 {

using namespace inet::physicallayer;

Define_Module(Ieee80211Mac);


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
        mib = getModuleFromPar<Ieee80211Mib>(par("mibModule"), this);
        mib->qos = par("qosStation");
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
        emit(modesetChangedSignal, modeSet);
        if (isUp())
            initializeRadioMode();
        rx = check_and_cast<IRx *>(getSubmodule("rx"));
        tx = check_and_cast<ITx *>(getSubmodule("tx"));
        dcf = check_and_cast<Dcf *>(getSubmodule("dcf"));
        hcf = check_and_cast_nullable<Hcf *>(getSubmodule("hcf"));
        if (mib->qos && !hcf)
            throw cRuntimeError("Missing hcf module, required for QoS");
    }
}

void Ieee80211Mac::initializeRadioMode() {
    const char *initialRadioMode = par("initialRadioMode");
    if(!strcmp(initialRadioMode, "off"))
        radio->setRadioMode(IRadio::RADIO_MODE_OFF);
    else if(!strcmp(initialRadioMode, "sleep"))
        radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
    else if(!strcmp(initialRadioMode, "receiver"))
        radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
    else if(!strcmp(initialRadioMode, "transmitter"))
        radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
    else if(!strcmp(initialRadioMode, "transceiver"))
        radio->setRadioMode(IRadio::RADIO_MODE_TRANSCEIVER);
    else
        throw cRuntimeError("Unknown initialRadioMode");
}

const MacAddress& Ieee80211Mac::isInterfaceRegistered()
{
    // if (!par("multiMac"))
    //    return MacAddress::UNSPECIFIED_ADDRESS;
    IInterfaceTable *ift = findModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
    if (!ift)
        return MacAddress::UNSPECIFIED_ADDRESS;
    cModule *interfaceModule = findModuleUnderContainingNode(this);
    if (!interfaceModule)
        throw cRuntimeError("NIC module not found in the host");
    std::string interfaceName = utils::stripnonalnum(interfaceModule->getFullName());
    InterfaceEntry *e = ift->findInterfaceByName(interfaceName.c_str());
    if (e)
        return e->getMacAddress();
    return MacAddress::UNSPECIFIED_ADDRESS;
}

void Ieee80211Mac::configureInterfaceEntry()
{
    //TODO the mib module should use the mac address from InterfaceEntry
    mib->address = interfaceEntry->getMacAddress();
    interfaceEntry->setMtu(par("mtu"));
    // capabilities
    interfaceEntry->setBroadcast(true);
    interfaceEntry->setMulticast(true);
    interfaceEntry->setPointToPoint(false);
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
    if (mib->mode == Ieee80211Mib::INFRASTRUCTURE && mib->bssStationData.stationType == Ieee80211Mib::ACCESS_POINT)
        header->setAddress3(mib->bssData.bssid);
    packet->insertAtFront(header);
    packet->insertAtBack(makeShared<Ieee80211MacTrailer>());
    processUpperFrame(packet, header);
}

void Ieee80211Mac::handleUpperPacket(Packet *packet)
{
    if (mib->mode == Ieee80211Mib::INFRASTRUCTURE && mib->bssStationData.stationType == Ieee80211Mib::STATION && !mib->bssStationData.isAssociated) {
        EV << "STA is not associated with an access point, discarding packet " << packet << "\n";
        PacketDropDetails details;
        details.setReason(OTHER_PACKET_DROP);
        emit(packetDroppedSignal, packet, &details);
        delete packet;
        return;
    }
    encapsulate(packet);
    const auto& header = packet->peekAtFront<Ieee80211DataOrMgmtHeader>();
    if (mib->mode == Ieee80211Mib::INFRASTRUCTURE && mib->bssStationData.stationType == Ieee80211Mib::ACCESS_POINT) {
        auto receiverAddress = header->getReceiverAddress();
        if (!receiverAddress.isMulticast()) {
            auto it = mib->bssAccessPointData.stations.find(receiverAddress);
            if (it == mib->bssAccessPointData.stations.end() || it->second != Ieee80211Mib::ASSOCIATED) {
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
            if (std::isnan(newConfigureCommand->getBitrate().get()) && !std::isnan(oldConfigureCommand->getBitrate().get()))
                newConfigureCommand->setBitrate(oldConfigureCommand->getBitrate());
            delete pendingRadioConfigMsg;
            pendingRadioConfigMsg = nullptr;
        }

        if (rx->isMediumFree()) {    // TODO: this should be just the physical channel sense!!!!
            EV_DEBUG << "Sending it down immediately\n";
            // PhyControlInfo *phyControlInfo = dynamic_cast<PhyControlInfo *>(msg->getControlInfo());
            // if (phyControlInfo)
            // phyControlInfo->setAdaptiveSensitivity(true);
            // end dynamic power
            sendDown(msg);
        }
        else {
            // TODO: waiting potentially indefinitely?! wtf?!
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
    auto macAddressReq = packet->getTag<MacAddressReq>();
    auto destAddress = macAddressReq->getDestAddress();
    const auto& header = makeShared<Ieee80211DataHeader>();
    header->setTransmitterAddress(mib->address);
    if (mib->mode == Ieee80211Mib::INDEPENDENT)
        header->setReceiverAddress(destAddress);
    else if (mib->mode == Ieee80211Mib::INFRASTRUCTURE) {
        if (mib->bssStationData.stationType == Ieee80211Mib::ACCESS_POINT) {
            header->setFromDS(true);
            header->setAddress3(mib->address);
            header->setReceiverAddress(destAddress);
        }
        else if (mib->bssStationData.stationType == Ieee80211Mib::STATION) {
            header->setToDS(true);
            header->setReceiverAddress(mib->bssData.bssid);
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
        if (mib->bssStationData.stationType == Ieee80211Mib::ACCESS_POINT) {
            macAddressInd->setSrcAddress(header->getTransmitterAddress());
            macAddressInd->setDestAddress(header->getAddress3());
        }
        else if (mib->bssStationData.stationType == Ieee80211Mib::STATION) {
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
    packet->addTagIfAbsent<InterfaceInd>()->setInterfaceId(interfaceEntry->getInterfaceId());
    packet->popAtBack<Ieee80211MacTrailer>(B(4));
}

void Ieee80211Mac::receiveSignal(cComponent *source, simsignal_t signalID, intval_t value, cObject *details)
{
    Enter_Method_Silent("receiveSignal");
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
            configureRadioMode(IRadio::RADIO_MODE_RECEIVER); // FIXME: this is in a very wrong place!!! should be done explicitly from coordination function!
        }
        rx->transmissionStateChanged(transmissionState);
    }
    else if (signalID == IRadio::receivedSignalPartChangedSignal) {
        rx->receivedSignalPartChanged(static_cast<IRadioSignal::SignalPart>(value));
    }
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
    Enter_Method_Silent("sendUp(\"%s\")", msg->getName());
    take(msg);
    MacProtocolBase::sendUp(msg);
}

void Ieee80211Mac::sendUpFrame(Packet *frame)
{
    Enter_Method_Silent("sendUpFrame(\"%s\")", frame->getName());
    const auto& header = frame->peekAtFront<Ieee80211DataOrMgmtHeader>();
    decapsulate(frame);
    if (!(header->getType() & 0x30))
        send(frame, "mgmtOut");
    else
        ds->processDataFrame(frame, dynamicPtrCast<const Ieee80211DataHeader>(header));
}

void Ieee80211Mac::sendDownFrame(Packet *frame)
{
    Enter_Method_Silent("sendDownFrame(\"%s\")", frame->getName());
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

void Ieee80211Mac::processUpperFrame(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& header)
{
    Enter_Method_Silent("processUpperFrame(\"%s\")", packet->getName());
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
    Enter_Method_Silent("processLowerFrame(\"%s\")", packet->getName());
    take(packet);
    if (mib->qos)
        hcf->processLowerFrame(packet, header);
    else
        // TODO: what if the received frame is ST_DATA_WITH_QOS? drop?
        dcf->processLowerFrame(packet, header);
}

// FIXME
void Ieee80211Mac::handleStartOperation(LifecycleOperation *operation)
{
    if (!operation)
        return;    // do nothing when called from initialize()

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
