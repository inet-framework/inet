//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// VRRPv2 (RFC 3768) ported from the ANSAINET project.
// Original authors: Petr Vitek, Vladimir Vesely (Brno University of Technology).
//

#include "inet/routing/vrrp/VrrpVirtualRouter.h"

#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/routing/vrrp/Vrrp.h"

namespace inet {
namespace vrrp {

Define_Module(VrrpVirtualRouter);

simsignal_t VrrpVirtualRouter::vrStateSignal = registerSignal("vrState");
simsignal_t VrrpVirtualRouter::sentAdvertisementSignal = registerSignal("sentAdvertisement");
simsignal_t VrrpVirtualRouter::recvAdvertisementSignal = registerSignal("recvAdvertisement");
simsignal_t VrrpVirtualRouter::sentArpSignal = registerSignal("sentArp");

VrrpVirtualRouter::~VrrpVirtualRouter()
{
    cancelAndDelete(masterDownTimer);
    cancelAndDelete(adverTimer);
    cancelAndDelete(broadcastTimer);
    cancelAndDelete(preemptionTimer);
    cancelAndDelete(initCheckTimer);
}

void VrrpVirtualRouter::initialize(int stage)
{
    SimpleModule::initialize(stage);

    if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        vrrp = check_and_cast<Vrrp *>(getParentModule());
        ift = vrrp->getInterfaceTable();

        loadConfig();

        ie = ift->getInterfaceById(interfaceId);
        if (!ie)
            throw cRuntimeError("VRRP group %d refers to unknown interface id %d", vrid, interfaceId);

        WATCH(state);
        WATCH(priority);
        WATCH(own);
        WATCH(advertisementIntervalActual);

        createVirtualMac();

        vforwarder = new VirtualForwarder();
        vforwarder->setMacAddress(virtualMac);
        for (auto& addr : virtualIps)
            vforwarder->addIpAddress(addr);
        ie->addVirtualForwarder(vforwarder);

        ie->getProtocolDataForUpdate<Ipv4InterfaceData>()->joinMulticastGroup(multicastIp);

        setOwn();
        stateInitialize();
    }
}

void VrrpVirtualRouter::preDelete(cComponent *root)
{
    cancelTimer(masterDownTimer);
    cancelTimer(adverTimer);
    cancelTimer(broadcastTimer);
    cancelTimer(preemptionTimer);
    cancelTimer(initCheckTimer);
    if (vforwarder && ie) {
        ie->removeVirtualForwarder(vforwarder);
        delete vforwarder;
        vforwarder = nullptr;
    }
}

//
// CONFIGURATION
//

void VrrpVirtualRouter::loadConfig()
{
    vrid = par("vrid");
    interfaceId = par("interfaceId");
    version = par("version");
    priority = par("priority");
    priorityOwner = par("priorityOwner");
    ttl = par("timeToLive");
    arpType = par("arpType");
    preemption = par("preempt");
    learn = par("learn");
    description = par("description").stringValue();
    arpDelay = par("arpDelay").doubleValue();
    virtualMacOui = par("virtualMacOui").stringValue();
    multicastIp = Ipv4Address(par("multicastAddress").stringValue());

    advertisementInterval = (int)par("advertiseInterval").doubleValue();
    advertisementIntervalActual = advertisementInterval;

    preemptTimerInit = par("preemptDelay").doubleValue();
    preemptionDelay = preemptTimerInit > 0;

    // virtualIps: space-separated list; the first one is the primary/group address
    virtualIps.clear();
    cStringTokenizer tokenizer(par("virtualIps").stringValue());
    while (tokenizer.hasMoreTokens())
        virtualIps.push_back(Ipv4Address(tokenizer.nextToken()));
    if (virtualIps.empty())
        throw cRuntimeError("VRRP group %d has no virtual IP address configured", vrid);
    primaryIp = virtualIps.front();
}

void VrrpVirtualRouter::createVirtualMac()
{
    virtualMac.setAddress(virtualMacOui.c_str());
    virtualMac.setAddressByte(5, vrid);
}

void VrrpVirtualRouter::setOwn()
{
    if (primaryIp == ie->getProtocolData<Ipv4InterfaceData>()->getIPAddress()) {
        own = true;
        priority = priorityOwner;
    }
    else
        own = false;
}

//
// STATE MACHINE (RFC 3768, section 6.4)
//

void VrrpVirtualRouter::stateInitialize()
{
    state = INITIALIZE;
    if (!ie->isUp()) {
        scheduleInitCheck();
        return;
    }

    emit(vrStateSignal, (long)INITIALIZE);

    if (priority == priorityOwner) {
        sendAdvertisement();
        scheduleBroadcastTimer();
        adverTimerInit = getAdvertisementInterval();
        EV_INFO << "VRRP group " << vrid << " on " << ie->getInterfaceName() << ": Initialize -> Master\n";
        stateMaster(INIT);
    }
    else {
        masterDownTimerInit = getMasterDownInterval();
        EV_INFO << "VRRP group " << vrid << " on " << ie->getInterfaceName() << ": Initialize -> Backup\n";
        stateBackup(INIT);
    }
}

void VrrpVirtualRouter::stateBackup(VrrpPhase phase)
{
    if (!ie->isUp()) {
        stateInitialize();
        return;
    }
    else if (phase == INIT) {
        state = BACKUP;
        emit(vrStateSignal, (long)BACKUP);
        vforwarder->setEnabled(false);
        cancelPreemptionTimer();
        stateBackup(TIMER_START);
    }
    else if (phase == TIMER_START) {
        scheduleMasterDownTimer();
    }
    else if (phase == TIMER_END) {
        vforwarder->setEnabled(true);
        sendAdvertisement();
        scheduleBroadcastTimer();
        adverTimerInit = getAdvertisementInterval();
        EV_INFO << "VRRP group " << vrid << " on " << ie->getInterfaceName() << ": Backup -> Master\n";
        stateMaster(INIT);
    }
    else if (phase == STOP) {
        cancelMasterDownTimer();
    }
}

void VrrpVirtualRouter::stateMaster(VrrpPhase phase)
{
    if (!ie->isUp()) {
        stateInitialize();
        return;
    }
    else if (phase == INIT) {
        state = MASTER;
        advertisementIntervalActual = advertisementInterval;
        emit(vrStateSignal, (long)MASTER);
        vforwarder->setEnabled(true);
        stateMaster(TIMER_START);
    }
    else if (phase == TIMER_START) {
        scheduleAdverTimer();
    }
    else if (phase == TIMER_END) {
        sendAdvertisement();
        adverTimerInit = getAdvertisementInterval();
        stateMaster(TIMER_START);
    }
    else if (phase == STOP) {
        cancelAdverTimer();
        priority = 0;
        sendAdvertisement();
        EV_INFO << "VRRP group " << vrid << " on " << ie->getInterfaceName() << ": Master -> Initialize\n";
    }
}

//
// TIMERS
//

void VrrpVirtualRouter::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
        handleTimer(msg);
    else
        throw cRuntimeError("VrrpVirtualRouter received an unexpected message '%s' (it has no gates)", msg->getName());
}

void VrrpVirtualRouter::handleTimer(cMessage *msg)
{
    switch ((VrrpTimerKind)msg->getKind()) {
        case ADVERTISE:
            stateMaster(TIMER_END);
            break;
        case MASTERDOWN:
            if (!preemptionDelay)
                stateBackup(TIMER_END);
            break;
        case BROADCAST:
            sendBroadcast();
            break;
        case PREEMPTION:
            stateBackup(TIMER_END);
            break;
        case INITCHECK:
            stateInitialize();
            break;
        default:
            throw cRuntimeError("Unknown VRRP timer kind %d", msg->getKind());
    }
}

void VrrpVirtualRouter::cancelTimer(cMessage *&timer)
{
    if (timer) {
        cancelAndDelete(timer);
        timer = nullptr;
    }
}

void VrrpVirtualRouter::scheduleAdverTimer()
{
    cancelAdverTimer();
    adverTimer = new cMessage("AdverTimer", ADVERTISE);
    scheduleAfter(adverTimerInit, adverTimer);
}

void VrrpVirtualRouter::cancelAdverTimer() { cancelTimer(adverTimer); }

void VrrpVirtualRouter::scheduleMasterDownTimer()
{
    cancelMasterDownTimer();
    masterDownTimer = new cMessage("MasterDownTimer", MASTERDOWN);
    scheduleAfter(masterDownTimerInit, masterDownTimer);
}

void VrrpVirtualRouter::cancelMasterDownTimer() { cancelTimer(masterDownTimer); }

void VrrpVirtualRouter::schedulePreemptionTimer()
{
    if (preemptionTimer == nullptr) {
        preemptionTimer = new cMessage("PreemptionDelay", PREEMPTION);
        scheduleAfter(preemptTimerInit, preemptionTimer);
    }
}

void VrrpVirtualRouter::cancelPreemptionTimer() { cancelTimer(preemptionTimer); }

void VrrpVirtualRouter::scheduleBroadcastTimer()
{
    cancelBroadcastTimer();
    broadcastTimer = new cMessage("BroadcastDelay", BROADCAST);
    scheduleAfter(arpDelay, broadcastTimer);
}

void VrrpVirtualRouter::cancelBroadcastTimer() { cancelTimer(broadcastTimer); }

void VrrpVirtualRouter::scheduleInitCheck()
{
    if (initCheckTimer == nullptr)
        initCheckTimer = new cMessage("InitCheck", INITCHECK);
    scheduleAfter((double)advertisementInterval / 4, initCheckTimer);
}

//
// ADVERTISEMENTS
//

void VrrpVirtualRouter::sendAdvertisement()
{
    auto adv = makeShared<VrrpAdvertisement>();
    adv->setVersion(version);
    adv->setType(VRRP_ADVERTISEMENT);
    adv->setVrid(vrid);
    adv->setPriority(priority);
    adv->setCountIpAddrs(virtualIps.size());
    adv->setAdverInt(advertisementIntervalActual);
    adv->setChecksum(0); // computed by the serializer when serialized
    adv->setAddressesArraySize(virtualIps.size());
    for (size_t i = 0; i < virtualIps.size(); i++)
        adv->setAddresses(i, virtualIps[i]);
    adv->setChunkLength(B(8 + 4 * virtualIps.size()));

    vrrp->sendAdvertisement(this, adv);
    emit(sentAdvertisementSignal, 1L);
}

void VrrpVirtualRouter::sendBroadcast()
{
    // TODO send gratuitous ARP for each virtual IP (implemented in V5)
}

bool VrrpVirtualRouter::validateAdvertisement(const Ptr<const VrrpAdvertisement>& adv, int receivedTtl)
{
    if (receivedTtl != 255) {
        EV_WARN << "VRRP advertisement with TTL " << receivedTtl << " != 255, discarding\n";
        return false;
    }
    if ((int)adv->getVersion() != version) {
        EV_WARN << "VRRP advertisement with version " << (int)adv->getVersion() << " != " << version << ", discarding\n";
        return false;
    }
    if (adv->getAddressesArraySize() == virtualIps.size()) {
        for (size_t i = 0; i < adv->getAddressesArraySize(); i++)
            if (adv->getAddresses(i) != virtualIps[i]) {
                EV_WARN << "VRRP advertisement with mismatching virtual address " << adv->getAddresses(i) << ", discarding\n";
                return false;
            }
    }
    if (advertisementIntervalActual != (int)adv->getAdverInt() && !learn) {
        EV_WARN << "VRRP advertisement interval mismatch (mine=" << advertisementIntervalActual
                << " received=" << (int)adv->getAdverInt() << "), discarding\n";
        return false;
    }
    return true;
}

void VrrpVirtualRouter::processReceivedAdvertisement(const Ptr<const VrrpAdvertisement>& adv, Ipv4Address srcAddr, int receivedTtl)
{
    Enter_Method("processReceivedAdvertisement");

    if (!validateAdvertisement(adv, receivedTtl))
        return;

    if (state == BACKUP)
        handleAdvertisementBackup(adv, srcAddr);
    else if (state == MASTER)
        handleAdvertisementMaster(adv, srcAddr);

    emit(recvAdvertisementSignal, 1L);
}

void VrrpVirtualRouter::handleAdvertisementBackup(const Ptr<const VrrpAdvertisement>& adv, Ipv4Address srcAddr)
{
    if ((int)adv->getPriority() == 0) {
        masterDownTimerInit = getSkewTime();
        stateBackup(TIMER_START);
        return;
    }
    else if (preemption == false || (int)adv->getPriority() > priority) {
        masterDownTimerInit = getMasterDownInterval();
        stateBackup(TIMER_START);
    }
    else if ((int)adv->getPriority() <= priority) {
        // remain Backup if same priority but higher master IP address
        if ((int)adv->getPriority() == priority
                && srcAddr > ie->getProtocolData<Ipv4InterfaceData>()->getIPAddress()) {
            masterDownTimerInit = getMasterDownInterval();
            stateBackup(TIMER_START);
            return;
        }
        // become new Master (after the optional preemption delay)
        if (preemptionDelay)
            schedulePreemptionTimer();
        // else let the master-down interval expire
    }
}

void VrrpVirtualRouter::handleAdvertisementMaster(const Ptr<const VrrpAdvertisement>& adv, Ipv4Address srcAddr)
{
    if ((int)adv->getPriority() == 0) {
        sendAdvertisement();
        adverTimerInit = getAdvertisementInterval();
    }
    else if (((int)adv->getPriority() > priority)
            || ((int)adv->getPriority() == priority
                    && srcAddr > ie->getProtocolData<Ipv4InterfaceData>()->getIPAddress())) {
        cancelAdverTimer();
        masterDownTimerInit = getMasterDownInterval();
        if (learn)
            advertisementIntervalActual = adv->getAdverInt();
        EV_INFO << "VRRP group " << vrid << " on " << ie->getInterfaceName() << ": Master -> Backup\n";
        stateBackup(INIT);
        return;
    }
    // else discard the advertisement

    stateMaster(TIMER_START);
}

//
// DISPLAY
//

std::string VrrpVirtualRouter::getStateName(VrrpState state) const
{
    switch (state) {
        case INITIALIZE: return "Initialize";
        case BACKUP: return "Backup";
        case MASTER: return "Master";
        case SHUTDOWN: return "Shutdown";
        default: return "?";
    }
}

void VrrpVirtualRouter::refreshDisplay() const
{
    getDisplayString().setTagArg("t", 0, (getStateName(state) + " (prio " + std::to_string(priority) + ")").c_str());
}

} // namespace vrrp
} // namespace inet
