//
// @authors: Enkhtuvshin Janchivnyambuu
//           Henning Puttnies
//           Peter Danielis
//           University of Rostock, Germany
//

#include "Gptp.h"

#include "GptpPacket_m.h"
#include "inet/clock/servo/PiServoClock.h"
#include "inet/clock/servo/ServoClockBase.h"
#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/clock/ClockUserModuleBase.h"
#include "inet/common/packet/dissector/PacketDissector.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/ethernet/common/Ethernet.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"
#include "inet/linklayer/ieee8021as/GptpPacket_m.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/physicallayer/wired/ethernet/EthernetPhyHeader_m.h"

namespace inet {

Define_Module(Gptp);

simsignal_t Gptp::localTimeSignal = cComponent::registerSignal("localTime");
simsignal_t Gptp::timeDifferenceSignal = cComponent::registerSignal("timeDifference");
simsignal_t Gptp::gmRateRatioSignal = cComponent::registerSignal("gmRateRatio");
simsignal_t Gptp::receivedRateRatioSignal = cComponent::registerSignal("receivedRateRatio");
simsignal_t Gptp::neighborRateRatioSignal = cComponent::registerSignal("neighborRateRatio");
simsignal_t Gptp::peerDelaySignal = cComponent::registerSignal("peerDelay");
simsignal_t Gptp::residenceTimeSignal = cComponent::registerSignal("residenceTime");
simsignal_t Gptp::correctionFieldIngressSignal = cComponent::registerSignal("correctionFieldIngress");
simsignal_t Gptp::correctionFieldEgressSignal = cComponent::registerSignal("correctionFieldEgress");
simsignal_t Gptp::gptpSyncStateChanged = cComponent::registerSignal("gptpSyncStateChanged");
simsignal_t Gptp::gmIdSignal = cComponent::registerSignal("gmId");

// MAC address:
//   01-80-C2-00-00-0E for Announce and Signaling messages, for Sync, Follow_Up,
//   Pdelay_Req, Pdelay_Resp, and Pdelay_Resp_Follow_Up messages (ieee 802.1as-2020, 10.5.3 and 11.3.4)
const MacAddress Gptp::GPTP_MULTICAST_ADDRESS("01:80:C2:00:00:0E");
std::map<std::string, std::string> inet::Gptp::clockIdentityToFullPath;

// EtherType:
//   0x8809 for TimeSync (ieee 802.1as-2020, 13.3.1.2)
//   0x88F7 for Announce and Signaling messages, for Sync, Follow_Up,
//   Pdelay_Req, Pdelay_Resp, and Pdelay_Resp_Follow_Up messages

Gptp::~Gptp()
{
    if (selfMsgDelayReq)
        cancelAndDeleteClockEvent(selfMsgDelayReq);
    if (selfMsgSync)
        cancelAndDeleteClockEvent(selfMsgSync);
    if (selfMsgAnnounce)
        cancelAndDeleteClockEvent(selfMsgAnnounce);
    for (auto &reqAnswerEvent : reqAnswerEvents)
        cancelAndDeleteClockEvent(reqAnswerEvent);
    for (auto &announce : receivedAnnounces)
        delete announce.second;
    for (auto &announceTimeout : announceTimeouts)
        cancelAndDeleteClockEvent(announceTimeout.second);
    if (selfMsgSyncTimeout)
        cancelAndDeleteClockEvent(selfMsgSyncTimeout);

    delete bestAnnounce;
}

void Gptp::initialize(int stage)
{
    ClockUserModuleBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        gptpNodeType = static_cast<GptpNodeType>(cEnum::get("GptpNodeType", "inet")->resolve(par("gptpNodeType")));
        useNrr = par("useNrr");
        gmRateRatioCalculationMethod = static_cast<GmRateRatioCalculationMethod>(
            cEnum::get("GmRateRatioCalculationMethod", "inet")->resolve(par("gmRateRatioCalculationMethod")));
        if (!useNrr && gmRateRatioCalculationMethod == GmRateRatioCalculationMethod::NRR) {
            throw cRuntimeError(
                "Parameter inconsistency: useNrr=false and gmRateRatioCalculationMethod=NEIGHBOR_RATE_RATIO");
        }

        domainNumber = par("domainNumber");
        syncInterval = par("syncInterval");
        pDelayReqProcessingTime = par("pDelayReqProcessingTime");
        std::hash<std::string> strHash;
        clockIdentity = strHash(getFullPath());
        clockIdentityToFullPath[std::to_string(clockIdentity)] = getFullPath();

        if (gptpNodeType == BMCA_NODE) {
            initBmca();
        }
        sendAnnounceImmediately = par("sendAnnounceImmediately");
    }
    if (stage == INITSTAGE_LINK_LAYER) {
        meanLinkDelay = 0;
        syncIngressTimestampLast = -1;
        correctionField = par("correctionField");
        gmRateRatio = 1.0;
        registerProtocol(Protocol::gptp, gate("socketOut"), gate("socketIn"));

        interfaceTable.reference(this, "interfaceTableModule", true);

        initPorts();
        if (gptpNodeType != BMCA_NODE) {
            scheduleMessageOnTopologyChange();
        }
        else {
            selfMsgAnnounce = new ClockEvent("selfMsgAnnounce", GPTP_SELF_MSG_ANNOUNCE);
            announceInterval = par("announceInterval");
            scheduleClockEventAfter(par("announceInitialOffset"), selfMsgAnnounce);
        }

        // We cast to cComponent, because clock can also be a MultiClock which does not extend from ClockBase,
        //  but also implements the clockJumpSignal
        auto servoClock = check_and_cast<cComponent *>(clock.get());
        servoClock->subscribe(ClockBase::timeJumpedSignal, this);

        WATCH(meanLinkDelay);
    }
}

void Gptp::initPorts()
{
    const char *str = par("slavePort");
    if (*str) {
        if (gptpNodeType == MASTER_NODE)
            throw cRuntimeError("Parameter inconsistency: MASTER_NODE with slave port");
        if (gptpNodeType == BMCA_NODE)
            throw cRuntimeError("Parameter inconsistency: BMCA_NODE with slave port");
        auto nic = CHK(interfaceTable->findInterfaceByName(str));
        slavePortId = nic->getInterfaceId();
        nic->subscribe(transmissionStartedSignal, this);
        nic->subscribe(receptionStartedSignal, this);
        nic->subscribe(receptionEndedSignal, this);
    }
    else if (gptpNodeType != MASTER_NODE && gptpNodeType != BMCA_NODE)
        throw cRuntimeError("Parameter error: Missing slave port for %s", par("gptpNodeType").stringValue());

    auto masterPortsVector = check_and_cast<cValueArray *>(par("masterPorts").objectValue())->asStringVector();
    auto bmcaPortsVector = check_and_cast<cValueArray *>(par("bmcaPorts").objectValue())->asStringVector();
    if (masterPortsVector.empty() && gptpNodeType != SLAVE_NODE && gptpNodeType != BMCA_NODE)
        throw cRuntimeError("Parameter error: Missing any master port for %s", par("gptpNodeType").stringValue());
    if (!masterPortsVector.empty() && (gptpNodeType == BMCA_NODE || gptpNodeType == SLAVE_NODE))
        throw cRuntimeError("Parameter inconsistency: %s with master ports", par("gptpNodeType").stringValue());

    if (bmcaPortsVector.empty() && gptpNodeType == BMCA_NODE)
        throw cRuntimeError("Parameter error: Missing any bmca port for %s", par("gptpNodeType").stringValue());
    if (!bmcaPortsVector.empty() && gptpNodeType != BMCA_NODE)
        throw cRuntimeError("Parameter inconsistency: %s with bmca ports", par("gptpNodeType").stringValue());

    for (const auto &p : masterPortsVector) {
        auto nic = CHK(interfaceTable->findInterfaceByName(p.c_str()));
        int portId = nic->getInterfaceId();
        if (portId == slavePortId)
            throw cRuntimeError("Parameter error: the port '%s' specified both "
                                "master and slave port",
                                p.c_str());
        masterPortIds.insert(portId);
        nic->subscribe(transmissionStartedSignal, this);
        nic->subscribe(receptionStartedSignal, this);
        nic->subscribe(receptionEndedSignal, this);
    }

    for (const auto &p : bmcaPortsVector) {
        auto nic = CHK(interfaceTable->findInterfaceByName(p.c_str()));
        int portId = nic->getInterfaceId();
        bmcaPortIds.insert(portId);
        nic->subscribe(transmissionStartedSignal, this);
        nic->subscribe(receptionStartedSignal, this);
        nic->subscribe(receptionEndedSignal, this);
    }

    if (slavePortId != -1) {
        auto networkInterface = interfaceTable->getInterfaceById(slavePortId);
        if (!networkInterface->matchesMulticastMacAddress(GPTP_MULTICAST_ADDRESS))
            networkInterface->addMulticastMacAddress(GPTP_MULTICAST_ADDRESS);
    }
    for (auto id : masterPortIds) {
        auto networkInterface = interfaceTable->getInterfaceById(id);
        if (!networkInterface->matchesMulticastMacAddress(GPTP_MULTICAST_ADDRESS))
            networkInterface->addMulticastMacAddress(GPTP_MULTICAST_ADDRESS);
    }
    for (auto id : bmcaPortIds) {
        auto networkInterface = interfaceTable->getInterfaceById(id);
        if (!networkInterface->matchesMulticastMacAddress(GPTP_MULTICAST_ADDRESS))
            networkInterface->addMulticastMacAddress(GPTP_MULTICAST_ADDRESS);
    }
}

void Gptp::scheduleMessageOnTopologyChange()
{
    if (gptpNodeType == BMCA_NODE) {
        // Cancel old sync and pdelay timers first
        if (selfMsgSync) {
            cancelAndDeleteClockEvent(selfMsgSync);
            selfMsgSync = nullptr;
        }
        if (selfMsgDelayReq) {
            cancelAndDeleteClockEvent(selfMsgDelayReq);
            selfMsgDelayReq = nullptr;
        }
        if (selfMsgSyncTimeout) {
            cancelAndDeleteClockEvent(selfMsgSyncTimeout);
            selfMsgSyncTimeout = nullptr;
        }
    }

    /* Only grandmaster in the domain can initialize the synchronization message
     * periodically so below condition checks whether it is grandmaster and then
     * schedule first sync message */
    if (isGM()) {
        // Schedule Sync message to be sent
        selfMsgSync = new ClockEvent("selfMsgSync", GPTP_SELF_MSG_SYNC);

        clocktime_t scheduleSync = par("syncInitialOffset");
        preciseOriginTimestamp = clock->getClockTime() + scheduleSync;
        scheduleClockEventAfter(scheduleSync, selfMsgSync);
        changeSyncState(SYNCED);
    }
    else {
        selfMsgSyncTimeout = new ClockEvent("selfMsgSyncTimeout", GPTP_SELF_MSG_SYNC_TIMEOUT);
        scheduleClockEventAfter(par("syncTimeout"), selfMsgSyncTimeout);
    }
    if (slavePortId != -1) {
        // Schedule Pdelay_Req message is sent by slave port
        // without depending on node type which is grandmaster or bridge
        selfMsgDelayReq = new ClockEvent("selfMsgPdelay", GPTP_SELF_MSG_PDELAY_REQ);
        pdelayInterval = par("pdelayInterval");
        scheduleClockEventAfter(par("pdelayInitialOffset"), selfMsgDelayReq);
    }
}

void Gptp::initBmca()
{
    // TODO: harcoded for now
    localPriorityVector.grandmasterPriority1 = par("grandmasterPriority1");
    localPriorityVector.grandmasterClockQuality.clockClass = 248;
    localPriorityVector.grandmasterClockQuality.clockAccuracy = 0;
    localPriorityVector.grandmasterClockQuality.offsetScaledLogVariance = 0;
    localPriorityVector.grandmasterPriority2 = 0;
    localPriorityVector.grandmasterIdentity = clockIdentity;
    localPriorityVector.stepsRemoved = 0;
}

void Gptp::handleSelfMessage(cMessage *msg)
{
    switch (msg->getKind()) {
    case GPTP_SELF_MSG_SYNC:
        // masterport:
        ASSERT(selfMsgSync == msg);
        sendSync();

        /* Schedule next Sync message at next sync interval
         * Grand master always works at simulation time */
        scheduleClockEventAfter(syncInterval, selfMsgSync);
        break;

    case GPTP_SELF_REQ_ANSWER_KIND:
        // masterport:
        sendPdelayResp(check_and_cast<GptpReqAnswerEvent *>(msg));
        delete msg;
        break;

    case GPTP_SELF_MSG_PDELAY_REQ:
        // slaveport:
        sendPdelayReq();
        scheduleClockEventAfter(pdelayInterval, selfMsgDelayReq);
        break;
    case GPTP_SELF_MSG_ANNOUNCE:
        sendAnnounce();
        break;
    case GPTP_SELF_MSG_ANNOUNCE_TIMEOUT:
        handleAnnounceTimeout(msg);
        break;
    case GPTP_SELF_MSG_SYNC_TIMEOUT:
        handleSyncTimeout(msg);
        break;
    default:
        throw cRuntimeError("Unknown self message (%s)%s, kind=%d", msg->getClassName(), msg->getName(),
                            msg->getKind());
    }
}

void Gptp::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        handleSelfMessage(msg);
    }
    else {
        Packet *packet = check_and_cast<Packet *>(msg);
        auto gptp = packet->peekAtFront<GptpBase>();
        auto gptpMessageType = gptp->getMessageType();
        auto incomingNicId = packet->getTag<InterfaceInd>()->getInterfaceId();
        int incomingDomainNumber = gptp->getDomainNumber();

        if (incomingDomainNumber != domainNumber) {
            EV_ERROR << "Message " << msg->getClassAndFullName() << " arrived with foreign domainNumber "
                     << incomingDomainNumber << ", dropped\n";
            PacketDropDetails details;
            details.setReason(NOT_ADDRESSED_TO_US);
            emit(packetDroppedSignal, packet, &details);
        }
        else if (gptpMessageType == GPTPTYPE_ANNOUNCE && bmcaPortIds.find(incomingNicId) != bmcaPortIds.end()) {
            processAnnounce(packet, check_and_cast<const GptpAnnounce *>(gptp.get()));
        }
        else if (incomingNicId == slavePortId) {
            // slave port
            switch (gptpMessageType) {
            case GPTPTYPE_SYNC:
                processSync(packet, check_and_cast<const GptpSync *>(gptp.get()));
                break;
            case GPTPTYPE_FOLLOW_UP:
                processFollowUp(packet, check_and_cast<const GptpFollowUp *>(gptp.get()));
                break;
            case GPTPTYPE_PDELAY_RESP:
                processPdelayResp(packet, check_and_cast<const GptpPdelayResp *>(gptp.get()));
                break;
            case GPTPTYPE_PDELAY_RESP_FOLLOW_UP:
                processPdelayRespFollowUp(packet, check_and_cast<const GptpPdelayRespFollowUp *>(gptp.get()));
                break;
            default:
                if (gptpNodeType != BMCA_NODE) {
                    throw cRuntimeError("Unknown gPTP packet type: %d", (int)(gptpMessageType));
                }
                else {
                    // In BMCA mode, packets might be still in transmission when switching port state
                    // Ignore and throw warning
                    EV_WARN << "Message " << msg->getClassAndFullName() << " arrived on slave port " << incomingNicId
                            << ", dropped\n";
                }
            }
        }
        else if (masterPortIds.find(incomingNicId) != masterPortIds.end()) {
            // master port
            if (gptpMessageType == GPTPTYPE_PDELAY_REQ) {
                processPdelayReq(packet, check_and_cast<const GptpPdelayReq *>(gptp.get()));
            }
            else {
                if (gptpNodeType != BMCA_NODE) {
                    throw cRuntimeError("Unaccepted gPTP type: %d", (int)(gptpMessageType));
                }
                else {
                    // In BMCA mode, packets might be still in transmission when switching port state
                    // Ignore and throw warning
                    EV_WARN << "Message " << msg->getClassAndFullName() << " arrived on master port " << incomingNicId
                            << ", dropped\n";
                }
            }
        }
        else {
            // passive port
            EV_ERROR << "Message " << msg->getClassAndFullName() << " arrived on passive port " << incomingNicId
                     << ", dropped\n";
        }
        delete msg;
    }
}

void Gptp::sendPacketToNIC(Packet *packet, int portId)
{
    auto networkInterface = interfaceTable->getInterfaceById(portId);
    EV_INFO << "Sending " << packet << " to output interface = " << networkInterface->getInterfaceName() << ".\n";
    packet->addTag<InterfaceReq>()->setInterfaceId(portId);
    packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::gptp);
    packet->addTag<DispatchProtocolInd>()->setProtocol(&Protocol::gptp);
    auto protocol = networkInterface->getProtocol();
    if (protocol != nullptr)
        packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(protocol);
    else
        packet->removeTagIfPresent<DispatchProtocolReq>();
    send(packet, "socketOut");
}

void Gptp::sendAnnounce()
{
    for (auto port : bmcaPortIds) {
        if (port == slavePortId || passivePortIds.find(port) != passivePortIds.end())
            continue;

        auto packet = new Packet("GptpAnnounce");
        packet->addTag<MacAddressReq>()->setDestAddress(GPTP_MULTICAST_ADDRESS);

        auto gptp = makeShared<GptpAnnounce>();
        gptp->setDomainNumber(domainNumber);
        gptp->setOriginTimestamp(clock->getClockTime());
        gptp->setSequenceId(sequenceId++);

        if (bestAnnounce == nullptr ||
            bestAnnounce->getPriorityVector().grandmasterIdentity == localPriorityVector.grandmasterIdentity)
        {
            gptp->setPriorityVector(localPriorityVector);
        }
        else {
            auto newPriorityVector = bestAnnounce->getPriorityVector();
            newPriorityVector.stepsRemoved++;
            gptp->setPriorityVector(newPriorityVector);
        }

        PortIdentity portId;
        portId.clockIdentity = clockIdentity;
        portId.portNumber = port;

        gptp->setSourcePortIdentity(portId);
        packet->insertAtFront(gptp);

        sendPacketToNIC(packet, port);
    }

    rescheduleClockEventAfter(announceInterval, selfMsgAnnounce);
}

void Gptp::sendSync()
{
    auto packet = new Packet("GptpSync");
    packet->addTag<MacAddressReq>()->setDestAddress(GPTP_MULTICAST_ADDRESS);
    auto gptp = makeShared<GptpSync>();
    gptp->setDomainNumber(domainNumber);
    /* OriginTimestamp always get Sync departure time from grand master */
    if (isGM()) {
        preciseOriginTimestamp = clock->getClockTime();
    }
    // gptp->setOriginTimestamp(CLOCKTIME_ZERO);

    gptp->setSequenceId(sequenceId++);
    // Correction field for Sync message is zero for two-step mode
    // See Table 11-6 in IEEE 802.1AS-2020
    // Change when implementing CMLDS
    gptp->setCorrectionField(CLOCKTIME_ZERO);
    packet->insertAtFront(gptp);

    for (auto port : masterPortIds)
        sendPacketToNIC(packet->dup(), port);
    delete packet;

    // The sendFollowUp(portId) called by receiveSignal(), when GptpSync sent
}

void Gptp::sendFollowUp(int portId, const GptpSync *sync, const clocktime_t &syncEgressTimestampOwn)
{
    auto packet = new Packet("GptpFollowUp");
    packet->addTag<MacAddressReq>()->setDestAddress(GPTP_MULTICAST_ADDRESS);
    auto gptp = makeShared<GptpFollowUp>();
    gptp->setDomainNumber(domainNumber);
    gptp->setPreciseOriginTimestamp(preciseOriginTimestamp);
    gptp->setSequenceId(sync->getSequenceId());

    clocktime_t residenceTime;
    if (isGM()) {
        residenceTime = syncEgressTimestampOwn - preciseOriginTimestamp;
        gptp->setCorrectionField(residenceTime);
    }
    else if (gptpNodeType == BRIDGE_NODE || gptpNodeType == BMCA_NODE) {
        if (syncIngressTimestamp == -1) {
            // There is a rare case when a SYNC message is in transmission, while BMCA selects a new GM, the followup
            // message contains a wrong timestamp.
            // In this case we simply drop the message.
            delete packet;
            return;
        }

        residenceTime = syncEgressTimestampOwn - syncIngressTimestamp;
        // meanLinkDelay and residence time are in the local time base
        // In the correctionField we need to express it in the grandmaster's time base
        // Thus, we need to multiply the meanLinkDelay and residenceTime with the gmRateRatio
        auto newCorrectionField = correctionField + gmRateRatio * (meanLinkDelay + residenceTime);
        gptp->setCorrectionField(newCorrectionField);
    }
    emit(residenceTimeSignal, residenceTime.asSimTime());
    emit(correctionFieldEgressSignal, gptp->getCorrectionField().asSimTime());
    gptp->getFollowUpInformationTLVForUpdate().setRateRatio(gmRateRatio);
    packet->insertAtFront(gptp);

    EV_INFO << "############## SEND FOLLOW_UP ################################" << endl;
    EV_INFO << "Correction Field              - " << gptp->getCorrectionField() << endl;
    EV_INFO << "gmRateRatio                   - " << gmRateRatio << endl;
    EV_INFO << "meanLinkDelay                 - " << meanLinkDelay << endl;
    EV_INFO << "residenceTime                 - " << residenceTime << endl;

    sendPacketToNIC(packet, portId);
}

void Gptp::processAnnounce(Packet *packet, const GptpAnnounce *announce)
{
    if (announce->getPriorityVector().stepsRemoved >= 255) {
        // IEEE 1588-2019 9.3.2.5 d)
        EV_WARN << "Announce message dropped because stepsRemoved is 255" << endl;
        return;
    }
    if (announce->getPriorityVector().grandmasterIdentity == clockIdentity) {
        EV_WARN << "Announce message dropped because grandmasterIdentity is own clockIdentity - "
                   "why does this even happen?"
                << endl;
        return;
    }

    auto incomingNicId = packet->getTag<InterfaceInd>()->getInterfaceId();
    if (receivedAnnounces.find(incomingNicId) != receivedAnnounces.end()) {
        delete receivedAnnounces[incomingNicId];
    }
    receivedAnnounces[incomingNicId] = announce->dup();

    clocktime_t gptpAnnounceTimeoutTime = par("announceTimeout");
    if (announceTimeouts.find(incomingNicId) != announceTimeouts.end()) {
        rescheduleClockEventAfter(gptpAnnounceTimeoutTime, announceTimeouts[incomingNicId]);
    }
    else {
        auto announceTimeoutEvent = new ClockEvent("gptpAnnounceTimeout", GPTP_SELF_MSG_ANNOUNCE_TIMEOUT);
        auto nicIdPar = announceTimeoutEvent->addPar("nicId").setLongValue(incomingNicId);
        announceTimeouts[incomingNicId] = announceTimeoutEvent;
        scheduleClockEventAfter(gptpAnnounceTimeoutTime, announceTimeoutEvent);
    }

    executeBmca();
}

void Gptp::executeBmca()
{
    PortIdentity ownPortIdentity;
    ownPortIdentity.clockIdentity = clockIdentity;
    ownPortIdentity.portNumber = 0;

    // IEEE 1588-2019 Table 29
    auto ownAnnounce = new GptpAnnounce();
    ownAnnounce->setPriorityVector(localPriorityVector);
    ownAnnounce->setSourcePortIdentity(ownPortIdentity);

    auto bestAnnounceCurr = ownAnnounce;
    auto bestAnnounceReceiverIdentityCurr = ownPortIdentity;

    for (const auto &announceEntry : receivedAnnounces) {
        auto a = announceEntry.second;
        auto aReceiverIdentity = PortIdentity();
        aReceiverIdentity.clockIdentity = clockIdentity;
        aReceiverIdentity.portNumber = announceEntry.first;

        switch (compareAnnounceMessages(a, bestAnnounceCurr, aReceiverIdentity, bestAnnounceReceiverIdentityCurr)) {
        case A_BETTER_THAN_B_BY_TOPOLOGY:
        case A_BETTER_THAN_B:
            // If a is better or better by topology, then a becomes the new best Announce
            bestAnnounceCurr = announceEntry.second;
            bestAnnounceReceiverIdentityCurr = aReceiverIdentity;
            break;
        case B_BETTER_THAN_A_BY_TOPOLOGY:
        case B_BETTER_THAN_A:
            // If bestAnnounceCurr is better or better by topology, then nothing changes
            break;
        }
    }

    auto fullGmPath = clockIdentityToFullPath[std::to_string(bestAnnounceCurr->getPriorityVector().grandmasterIdentity)];

    EV_INFO << "############## BMCA ################################" << endl;
    EV_INFO << "Choosing from " << receivedAnnounces.size() << " Announces" << endl;
    EV_INFO << "Selected Grandmaster Identity      - " << bestAnnounceCurr->getPriorityVector().grandmasterIdentity
            << endl;
    EV_INFO << "Selected Grandmaster Name          - "
            << clockIdentityToFullPath[std::to_string(bestAnnounceCurr->getPriorityVector().grandmasterIdentity)]
            << endl;
    EV_INFO << "Selected Grandmaster Priority1     - "
            << (int)bestAnnounceCurr->getPriorityVector().grandmasterPriority1 << endl;
    EV_INFO << "Selected Slave Port Identity       - " << bestAnnounceReceiverIdentityCurr.portNumber << endl;

    if (bestAnnounce == bestAnnounceCurr) {
        // Received an Announce message, but best clock is still the same
    }
    else if (bestAnnounce && bestAnnounce->getPriorityVector() == bestAnnounceCurr->getPriorityVector() &&
             slavePortId == bestAnnounceReceiverIdentityCurr.portNumber)
    {
        // Same priority vector but newer Announce, no topology change because priority vector is the same
        // Store the new Announce
        delete bestAnnounce;
        bestAnnounce = bestAnnounceCurr->dup();
    }
    else {
        // Topology change, update ports and send Announce
        bool newInfo = true;

        if (bestAnnounceCurr->getPriorityVector().grandmasterIdentity != clockIdentity) {
            // We are not the GM
            auto oldSlavePortId = slavePortId;

            slavePortId = bestAnnounceReceiverIdentityCurr.portNumber;
            masterPortIds.clear();
            passivePortIds.clear();
            for (const auto &portId : bmcaPortIds) {
                if (portId != slavePortId) {
                    masterPortIds.insert(portId);
                }
            }
            auto masterName = interfaceTable->getInterfaceById(slavePortId)
                                  ->getRxTransmissionChannel()
                                  ->getSourceGate()
                                  ->getOwnerModule()
                                  ->getFullName();
            getContainingNode(this)->bubble(("My master is " + std::string(masterName)).c_str());

            auto gmId = getModuleByPath(fullGmPath.c_str())->getId();
            emit(gmIdSignal, gmId);

            // In this case we have a new master we need to reset stuff for everything to function properly
            resetGptpAfterMasterChange();
        }
        else {
            // We are the GM:
            getContainingNode(this)->bubble("I'm GM");
            if (slavePortId == -1) {
                // We were already GM, no need to send Announce again
                // Should only happen in the bootup phase
                // (otherwise priority vector would be the same and earlier case would apply)
                newInfo = true;
            }
            masterPortIds = bmcaPortIds;
            passivePortIds.clear();
            slavePortId = -1;

            emit(gmIdSignal, getId());
        }

        delete bestAnnounce;
        bestAnnounce = bestAnnounceCurr->dup();

        if (newInfo && sendAnnounceImmediately) {
            sendAnnounce();
        }
        scheduleMessageOnTopologyChange();
    }

    // Calculate ports states (based on IEEE 1588-2019 Figure 33)
    masterPortIds.clear();
    passivePortIds.clear();
    for (const auto &portId : bmcaPortIds) {
        if (portId != slavePortId) {
            masterPortIds.insert(portId);
        }
    }
    if (slavePortId != -1) {
        // Don't do this calculation if we're selected as the GM => We assume all gPTP enabled ports are masters
        for (const auto &portId : masterPortIds) {
            GptpAnnounce *portAnnounce = nullptr;
            if (auto it = receivedAnnounces.find(portId); it != receivedAnnounces.end()) {
                portAnnounce = it->second;
            }

            PortIdentity receiverPortIdentity;
            receiverPortIdentity.clockIdentity = clockIdentity;
            receiverPortIdentity.portNumber = portId;

            // Standard 1588-2019 defines 1 to 127, but 0 is reserved and unused anyway.
            // The standard thus does not define the behavior for 0, so we just use <= 127
            if (localPriorityVector.grandmasterClockQuality.clockClass <= 127) {
                if (portAnnounce == nullptr) {
                    continue;
                }
                auto compareLocalToPort =
                    compareAnnounceMessages(ownAnnounce, portAnnounce, ownPortIdentity, receiverPortIdentity);

                if (compareLocalToPort == B_BETTER_THAN_A || compareLocalToPort == B_BETTER_THAN_A_BY_TOPOLOGY) {
                    passivePortIds.insert(portId);
                }
                continue;
            }

            // Clock class >= 128
            auto compareLocalToBest =
                compareAnnounceMessages(ownAnnounce, bestAnnounce, ownPortIdentity, bestAnnounceReceiverIdentityCurr);

            if (compareLocalToBest == A_BETTER_THAN_B || compareLocalToBest == A_BETTER_THAN_B_BY_TOPOLOGY) {
                // Local better than best => Master
                continue;
            }

            if (bestAnnounceReceiverIdentityCurr.portNumber == portId) {
                // => Slave (already set above)
                continue;
            }

            if (portAnnounce == nullptr) {
                continue;
            }

            auto compareBestToPort = compareAnnounceMessages(bestAnnounce, portAnnounce,
                                                             bestAnnounceReceiverIdentityCurr, receiverPortIdentity);
            if (compareBestToPort == A_BETTER_THAN_B_BY_TOPOLOGY) {
                passivePortIds.insert(portId);
            }
        }
        for (const auto &passivePortId : passivePortIds) {
            masterPortIds.erase(passivePortId);
        }
    }

    delete ownAnnounce;
}

Gptp::BmcaPriorityVectorComparisonResult Gptp::compareAnnounceMessages(GptpAnnounce *a, GptpAnnounce *b,
                                                                       PortIdentity aReceiverIdentity,
                                                                       PortIdentity bReceiverIdentity)
{
    auto aVec = a->getPriorityVector();
    auto bVec = b->getPriorityVector();

    // IEEE 1588-2019 Figure 34
    if (aVec.grandmasterIdentity == bVec.grandmasterIdentity) {
        // Special cases Figure 35

        // Compare steps removed (|A-B| >=2)
        if (aVec.stepsRemoved > bVec.stepsRemoved + 1) {
            return B_BETTER_THAN_A;
        }

        else if (aVec.stepsRemoved + 1 < bVec.stepsRemoved) {
            return A_BETTER_THAN_B;
        }

        // Compare steps removed |A-B| <= 1
        if (aVec.stepsRemoved > bVec.stepsRemoved) {
            if (aReceiverIdentity == a->getSourcePortIdentity()) {
                throw cRuntimeError("Error-1: Indicates that one of the PTP messages was "
                                    "transmitted and received on the same PTP Port.\n"
                                    "(See IEEE 1588-2019 Figure 35 NOTE 2)");
            }
            else if (aReceiverIdentity < a->getSourcePortIdentity()) {
                return B_BETTER_THAN_A;
            }
            else {
                // aReceiverIdentity > a->getSourcePortIdentity()
                return B_BETTER_THAN_A_BY_TOPOLOGY;
            }
        }
        else if (aVec.stepsRemoved < bVec.stepsRemoved) {
            if (bReceiverIdentity == b->getSourcePortIdentity()) {
                throw cRuntimeError("Error-1: Indicates that one of the PTP messages was "
                                    "transmitted and received on the same PTP Port.\n"
                                    "(See IEEE 1588-2019 Figure 35 NOTE 2)");
            }
            else if (bReceiverIdentity < b->getSourcePortIdentity()) {
                return A_BETTER_THAN_B;
            }
            else {
                return A_BETTER_THAN_B_BY_TOPOLOGY;
            }
        }

        // Compare Identities of senders
        if (a->getSourcePortIdentity() > b->getSourcePortIdentity()) {
            return B_BETTER_THAN_A_BY_TOPOLOGY;
        }
        else if (a->getSourcePortIdentity() < b->getSourcePortIdentity()) {
            return A_BETTER_THAN_B_BY_TOPOLOGY;
        }

        // Compare port number of receivers
        if (aReceiverIdentity.portNumber > bReceiverIdentity.portNumber) {
            return B_BETTER_THAN_A_BY_TOPOLOGY;
        }
        else if (aReceiverIdentity.portNumber < bReceiverIdentity.portNumber) {
            return A_BETTER_THAN_B_BY_TOPOLOGY;
        }
        else {
            throw cRuntimeError("Error-2: indicates that the PTP messages are duplicates or that they are "
                                "an earlier and later PTP message from the same Grandmaster PTP Instance.\n"
                                "(See IEEE 1588-2019 Figure 35 NOTE 2)");
        }
    }
    else if (aVec > bVec) {
        return B_BETTER_THAN_A;
    }
    else if (aVec < bVec) {
        return A_BETTER_THAN_B;
    }
    else {
        throw cRuntimeError("Unknown case in BMCA -- should never happen");
    }
}

void Gptp::sendPdelayResp(GptpReqAnswerEvent *req)
{
    reqAnswerEvents.erase(req);

    int portId = req->getPortId();
    auto packet = new Packet("GptpPdelayResp");
    packet->addTag<MacAddressReq>()->setDestAddress(GPTP_MULTICAST_ADDRESS);
    auto gptp = makeShared<GptpPdelayResp>();

    // Correction field for Pdelay_Resp contains fractional nanoseconds
    // according to the standard, we do not need this in INET.
    // See Table 11-6 in IEEE 802.1AS-2020
    gptp->setCorrectionField(CLOCKTIME_ZERO);

    gptp->setDomainNumber(domainNumber);
    gptp->setRequestingPortIdentity(req->getSourcePortIdentity());
    gptp->setSequenceId(req->getSequenceId());
    gptp->setRequestReceiptTimestamp(req->getIngressTimestamp()); // t2
    packet->insertAtFront(gptp);
    sendPacketToNIC(packet, portId);
    // The sendPdelayRespFollowUp(portId) called by receiveSignal(), when
    // GptpPdelayResp sent
}

void Gptp::sendPdelayRespFollowUp(int portId, const GptpPdelayResp *resp)
{
    auto packet = new Packet("GptpPdelayRespFollowUp");
    packet->addTag<MacAddressReq>()->setDestAddress(GPTP_MULTICAST_ADDRESS);
    auto gptp = makeShared<GptpPdelayRespFollowUp>();
    gptp->setDomainNumber(domainNumber);

    // Correction field for Pdelay_Resp contains fractional nanoseconds
    // according to the standard, we do not need this in INET.
    // See Table 11-6 in IEEE 802.1AS-2020
    gptp->setCorrectionField(CLOCKTIME_ZERO);

    // We can set this to now, because this function is called directly
    // after the transmissionStarted signal for the pdelayResp packet
    // is received
    auto now = clock->getClockTime();
    gptp->setResponseOriginTimestamp(now); // t3

    gptp->setRequestingPortIdentity(resp->getRequestingPortIdentity());
    gptp->setSequenceId(resp->getSequenceId());
    packet->insertAtFront(gptp);
    sendPacketToNIC(packet, portId);
}

void Gptp::sendPdelayReq()
{
    ASSERT(slavePortId != -1);
    auto packet = new Packet("GptpPdelayReq");
    packet->addTag<MacAddressReq>()->setDestAddress(GPTP_MULTICAST_ADDRESS);
    auto gptp = makeShared<GptpPdelayReq>();
    gptp->setDomainNumber(domainNumber);

    // Correction field for Pdelay_Req message is zero for two-step mode
    // See Table 11-6 in IEEE 802.1AS-2020
    gptp->setCorrectionField(CLOCKTIME_ZERO);

    // save and send IDs
    PortIdentity portId;
    portId.clockIdentity = clockIdentity;
    portId.portNumber = slavePortId;
    gptp->setSourcePortIdentity(portId);
    lastSentPdelayReqSequenceId = sequenceId++;
    gptp->setSequenceId(lastSentPdelayReqSequenceId);
    packet->insertAtFront(gptp);
    rcvdPdelayResp = false;
    //    sendReqStartTimestamp = clock->getClockTime();
    sendPacketToNIC(packet, slavePortId);
}

void Gptp::processSync(Packet *packet, const GptpSync *gptp)
{
    rcvdGptpSync = true;
    lastReceivedGptpSyncSequenceId = gptp->getSequenceId();
    syncIngressTimestamp = packet->getTag<GptpIngressTimeInd>()->getArrivalClockTime();
}

void Gptp::processFollowUp(Packet *packet, const GptpFollowUp *gptp)
{
    // check: is received the GptpSync for this GptpFollowUp?
    if (!rcvdGptpSync) {
        EV_WARN << "GptpFollowUp arrived without GptpSync, dropped";
        return;
    }
    // verify IDs
    if (gptp->getSequenceId() != lastReceivedGptpSyncSequenceId) {
        EV_WARN << "GptpFollowUp arrived with invalid sequence ID, dropped";
        return;
    }

    preciseOriginTimestamp = gptp->getPreciseOriginTimestamp();
    correctionField = gptp->getCorrectionField();
    receivedRateRatio = gptp->getFollowUpInformationTLV().getRateRatio();

    emit(correctionFieldIngressSignal, correctionField.asSimTime());

    synchronize();

    EV_INFO << "############## FOLLOW_UP ################################" << endl;
    EV_INFO << "RECEIVED TIME AFTER SYNC  - " << newLocalTimeAtTimeSync << endl;
    EV_INFO << "ORIGIN TIME SYNC          - " << preciseOriginTimestamp << endl;
    EV_INFO << "CORRECTION FIELD          - " << correctionField << endl;
    EV_INFO << "PROPAGATION DELAY         - " << meanLinkDelay << endl;
    EV_INFO << "receivedRateRatio         - " << receivedRateRatio << endl;

    rcvdGptpSync = false;

    // Send a request to send Sync message
    // through other gptp Ethernet interfaces
    if (gptpNodeType == BRIDGE_NODE || gptpNodeType == BMCA_NODE)
        sendSync();
}

void Gptp::synchronize()
{
    /************** Time synchronization *****************************************
     * Local time is adjusted using peer delay, correction field, residence time *
     * and packet transmission time based departure time of Sync message from GM *
     *****************************************************************************/
    simtime_t now = simTime();
    clocktime_t oldLocalTimeAtTimeSync = clock->getClockTime();
    emit(timeDifferenceSignal, CLOCKTIME_AS_SIMTIME(oldLocalTimeAtTimeSync) - now);

    clocktime_t residenceTime = oldLocalTimeAtTimeSync - syncIngressTimestamp;

    ASSERT(!isGM());

    calculateGmRatio();

    // preciseOriginTimestamp and correctionField are in the grandmaster's time base
    // meanLinkDelay and residence time are in the local time base
    // Thus, we need to multiply the meanLinkDelay and residenceTime with the gmRateRatio
    clocktime_t newTime = preciseOriginTimestamp + correctionField + gmRateRatio * (meanLinkDelay + residenceTime);

    auto servoClock = check_and_cast<ServoClockBase *>(clock.get());
    servoClock->adjustClockTo(newTime);
    //    EV_INFO << "newOscillatorCompensation " << newOscillatorCompensation << endl;

    newLocalTimeAtTimeSync = clock->getClockTime();

    updateSyncStateAndRescheduleSyncTimeout(servoClock);

    /************** Rate ratio calculation *************************************
     * It is calculated based on interval between two successive Sync messages *
     ***************************************************************************/

    EV_INFO << "############## SYNC #####################################" << endl;
    EV_INFO << "LOCAL TIME BEFORE SYNC     - " << oldLocalTimeAtTimeSync << endl;
    EV_INFO << "LOCAL TIME AFTER SYNC      - " << newLocalTimeAtTimeSync << endl;
    EV_INFO << "CALCULATED NEW TIME        - " << newTime << endl;
    if (servoClock->referenceClockModule != nullptr) {
        auto referenceClockTime = servoClock->referenceClockModule->getClockTime();
        auto diffReferenceToOldLocal = oldLocalTimeAtTimeSync - referenceClockTime;
        auto diffReferenceToNewTime = newTime - referenceClockTime;
        EV_INFO << "REFERENCE CLOCK TIME       - " << referenceClockTime << endl;
        EV_INFO << "DIFF REFERENCE TO OLD TIME - " << diffReferenceToOldLocal << endl;
        EV_INFO << "DIFF REFERENCE TO NEW TIME - " << diffReferenceToNewTime << endl;
    }
    EV_INFO << "CURRENT SIMTIME            - " << now << endl;
    EV_INFO << "ORIGIN TIME SYNC           - " << preciseOriginTimestamp << endl;
    EV_INFO << "PREV ORIGIN TIME SYNC      - " << preciseOriginTimestampLast << endl;
    EV_INFO << "SYNC INGRESS TIME          - " << syncIngressTimestamp << endl;
    EV_INFO << "SYNC INGRESS TIME LAST     - " << syncIngressTimestampLast << endl;
    EV_INFO << "RESIDENCE TIME             - " << residenceTime << endl;
    EV_INFO << "CORRECTION FIELD           - " << correctionField << endl;
    EV_INFO << "PROPAGATION DELAY          - " << meanLinkDelay << endl;
    EV_INFO << "TIME DIFFERENCE TO SIMTIME - " << CLOCKTIME_AS_SIMTIME(newLocalTimeAtTimeSync) - now << endl;
    EV_INFO << "NEIGHBOR RATE RATIO        - " << neighborRateRatio << endl;
    EV_INFO << "RECIEVED RATE RATIO        - " << receivedRateRatio << endl;
    EV_INFO << "GM RATE RATIO              - " << gmRateRatio << endl;

    syncIngressTimestampLast = syncIngressTimestamp;
    preciseOriginTimestampLast = preciseOriginTimestamp;

    emit(receivedRateRatioSignal, receivedRateRatio);
    emit(gmRateRatioSignal, gmRateRatio);
    emit(localTimeSignal, CLOCKTIME_AS_SIMTIME(newLocalTimeAtTimeSync));
    emit(timeDifferenceSignal, CLOCKTIME_AS_SIMTIME(newLocalTimeAtTimeSync) - now);
}

void Gptp::updateSyncStateAndRescheduleSyncTimeout(const ServoClockBase *servoClock)
{
    switch (servoClock->getClockState()) {
    case ServoClockBase::INIT:
        changeSyncState(UNSYNCED);
        break;
    case ServoClockBase::SYNCED:
        changeSyncState(SYNCED);
        break;
    }

    if (selfMsgSyncTimeout->isScheduled()) {
        rescheduleClockEventAfter(par("syncTimeout"), selfMsgSyncTimeout);
    }
    else if (!isGM()) {
        scheduleClockEventAfter(par("syncTimeout"), selfMsgSyncTimeout);
    }
}

void Gptp::changeSyncState(SyncState state, bool resetClock)
{
    auto prevSyncState = syncState;
    syncState = state;
    if (prevSyncState != syncState) {
        emit(gptpSyncStateChanged, syncState);
    }

    auto servoClock = dynamic_cast<ServoClockBase *>(clock.get());
    if (resetClock && state == UNSYNCED && servoClock != nullptr && par("resetClockStateOnSyncLoss").boolValue()) {
        servoClock->resetClockState();
    }
}

void Gptp::calculateGmRatio()
{
    switch (gmRateRatioCalculationMethod) {
    case NONE:
        gmRateRatio = 1.0;
        break;
    case NRR:
        gmRateRatio = receivedRateRatio * neighborRateRatio;
        break;
    case DIRECT:
        peerSentTimeSync = preciseOriginTimestamp + correctionField;
        if (syncIngressTimestampLast != -1) {
            gmRateRatio = (peerSentTimeSync - peerSentTimeSyncLast) / (syncIngressTimestamp - syncIngressTimestampLast);
        }
        peerSentTimeSyncLast = peerSentTimeSync;
        break;
    }
}

void Gptp::resetGptpAfterMasterChange()
{
    // Reset GPTP variables
    neighborRateRatio = 1.0;
    rcvdGptpSync = false;
    rcvdPdelayResp = false;
    meanLinkDelay = CLOCKTIME_ZERO;
    syncIngressTimestamp = -1;
    syncIngressTimestampLast = -1;

    changeSyncState(UNSYNCED, true);
}

void Gptp::processPdelayReq(Packet *packet, const GptpPdelayReq *gptp)
{
    auto resp = new GptpReqAnswerEvent("selfMsgPdelayResp", GPTP_SELF_REQ_ANSWER_KIND);
    resp->setPortId(packet->getTag<InterfaceInd>()->getInterfaceId());
    resp->setIngressTimestamp(packet->getTag<GptpIngressTimeInd>()->getArrivalClockTime()); // t2
    resp->setSourcePortIdentity(gptp->getSourcePortIdentity());
    resp->setSequenceId(gptp->getSequenceId());

    reqAnswerEvents.insert(resp);

    scheduleClockEventAfter(pDelayReqProcessingTime, resp);
}

void Gptp::processPdelayResp(Packet *packet, const GptpPdelayResp *gptp)
{
    // verify IDs
    if (gptp->getRequestingPortIdentity().clockIdentity != clockIdentity ||
        gptp->getRequestingPortIdentity().portNumber != slavePortId)
    {
        EV_WARN << "GptpPdelayResp arrived with invalid PortIdentity, dropped";
        return;
    }
    if (gptp->getSequenceId() != lastSentPdelayReqSequenceId) {
        EV_WARN << "GptpPdelayResp arrived with invalid sequence ID, dropped";
        return;
    }

    rcvdPdelayResp = true;

    if (pDelayRespIngressTimestampSetStart == -1) {
        pDelayRespIngressTimestampSetStart = pDelayRespIngressTimestamp; // t4 last
    }

    pDelayRespIngressTimestamp = packet->getTag<GptpIngressTimeInd>()->getArrivalClockTime(); // t4 now
    pDelayReqIngressTimestamp = gptp->getRequestReceiptTimestamp();                           // t2
}

void Gptp::processPdelayRespFollowUp(Packet *packet, const GptpPdelayRespFollowUp *gptp)
{
    if (!rcvdPdelayResp) {
        EV_WARN << "GptpPdelayRespFollowUp arrived without GptpPdelayResp, dropped";
        return;
    }
    // verify IDs
    if (gptp->getRequestingPortIdentity().clockIdentity != clockIdentity ||
        gptp->getRequestingPortIdentity().portNumber != slavePortId)
    {
        EV_WARN << "GptpPdelayRespFollowUp arrived with invalid PortIdentity, dropped";
        return;
    }
    if (gptp->getSequenceId() != lastSentPdelayReqSequenceId) {
        EV_WARN << "GptpPdelayRespFollowUp arrived with invalid sequence ID, dropped";
        return;
    }

    if (pDelayRespEgressTimestampSetStart == -1) {
        pDelayRespEgressTimestampSetStart = pDelayRespEgressTimestamp; // t3 last
    }

    pDelayRespEgressTimestamp = gptp->getResponseOriginTimestamp(); // t3 now

    // Note, that the standard defines the usage of the correction field
    // for the following two calculations.
    // However, these contain fractional nanoseconds, which we do not
    // use in INET.
    // See 11.2.19.3.3 computePdelayRateRatio() in IEEE 802.1AS-2020
    clocktime_t prevRespRegress = -1;
    clocktime_t prevRespIngress = -1;

    if (nrrCalculationSetCurrent == nrrCalculationSetMaximum) {
        neighborRateRatio = (pDelayRespEgressTimestamp - pDelayRespEgressTimestampSetStart) /
                            (pDelayRespIngressTimestamp - pDelayRespIngressTimestampSetStart);
        pDelayRespEgressTimestampSetStart = -1;
        pDelayRespIngressTimestampSetStart = -1;
        nrrCalculationSetCurrent = 0;
    }
    else {
        nrrCalculationSetCurrent++;
    }

    if (!useNrr) {
        neighborRateRatio = 1.0;
    }

    // See 11.2.19.3.4 computePropTime() and Figure11-1 in IEEE 802.1AS-2020
    auto t4 = pDelayRespIngressTimestamp;
    auto t1 = pDelayReqEgressTimestamp;

    auto t2 = pDelayReqIngressTimestamp;
    auto t3 = pDelayRespEgressTimestamp;

    auto meanLinkDelayInResponderTimebase = (neighborRateRatio * (t4 - t1) - (t3 - t2)) / 2;

    // Regarding NOTE 1 in 11.2.19.3.4, we need to device by the nrr to get the meanLinkDelay in the current time base
    meanLinkDelay = meanLinkDelayInResponderTimebase / neighborRateRatio;

    EV_INFO << "GM RATE RATIO               - " << gmRateRatio << endl;
    EV_INFO << "NEIGHBOR RATE RATIO         - " << neighborRateRatio << endl;
    EV_INFO << "pDelayReqEgressTimestamp    - " << pDelayReqEgressTimestamp << endl;
    EV_INFO << "pDelayReqIngressTimestamp   - " << pDelayReqIngressTimestamp << endl;
    EV_INFO << "pDelayRespEgressTimestamp   - " << pDelayRespEgressTimestamp << endl;
    EV_INFO << "pDelayRespIngressTimestamp  - " << pDelayRespIngressTimestamp << endl;
    EV_INFO << "PEER DELAY                  - " << meanLinkDelay << endl;

    emit(neighborRateRatioSignal, neighborRateRatio);
    emit(peerDelaySignal, CLOCKTIME_AS_SIMTIME(meanLinkDelay));
}

const GptpBase *Gptp::extractGptpHeader(Packet *packet)
{
    PacketDissector::ChunkFinder chunkFinder(&Protocol::gptp);
    PacketDissector packetDissector(ProtocolDissectorRegistry::getInstance(), chunkFinder);
    packetDissector.dissectPacket(packet);
    const auto &chunk = staticPtrCast<const GptpBase>(chunkFinder.getChunk());
    return chunk != nullptr ? chunk.get() : nullptr;
}

void Gptp::receiveSignal(cComponent *source, simsignal_t simSignal, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(simSignal));

    auto clockBase = check_and_cast<cComponent *>(clock.get());

    if (simSignal == ClockBase::timeJumpedSignal && obj == clockBase) {
        auto clockJumpDetails = check_and_cast<ServoClockBase::ClockJumpDetails *>(details);
        handleClockJump(clockJumpDetails);
    }

    if (simSignal != receptionStartedSignal && simSignal != transmissionStartedSignal &&
        simSignal != receptionEndedSignal)
        return;

    auto ethernetSignal = check_and_cast<cPacket *>(obj);
    auto packet = check_and_cast_nullable<Packet *>(ethernetSignal->getEncapsulatedPacket());
    if (!packet)
        return;

    auto gptp = extractGptpHeader(packet);
    if (!gptp)
        return;

    if (gptp->getDomainNumber() != domainNumber)
        return;

    if (simSignal == receptionStartedSignal) {
        auto transmissionId = ethernetSignal->getTransmissionId();
        auto ingressTime = clock->getClockTime();
        ingressTimeMap[transmissionId] = ingressTime;
        // Save ingress time in map
    }
    else if (simSignal == receptionEndedSignal) {
        packet->addTagIfAbsent<GptpIngressTimeInd>()->setArrivalClockTime(
            ingressTimeMap[ethernetSignal->getTransmissionId()]);
        ingressTimeMap.erase(ethernetSignal->getTransmissionId());
        // Read ingress time from map
        // Ad tag to packet
    }
    else if (simSignal == transmissionStartedSignal)
        handleTransmissionStartedSignal(gptp, source);
}

void Gptp::handleAnnounceTimeout(cMessage *pMessage)
{
    int nicId = pMessage->par("nicId").longValue();
    auto it = receivedAnnounces.find(nicId);
    if (it == receivedAnnounces.end()) {
        throw cRuntimeError("Received announce timeout for unknown NIC %d", nicId);
    }

    delete it->second;
    receivedAnnounces.erase(it);
    executeBmca();
}

void Gptp::handleSyncTimeout(cMessage *pMessage) { changeSyncState(UNSYNCED, true); }

void Gptp::handleTransmissionStartedSignal(const GptpBase *gptp, cComponent *source)
{
    int portId = getContainingNicModule(check_and_cast<cModule *>(source))->getInterfaceId();

    switch (gptp->getMessageType()) {
    case GPTPTYPE_PDELAY_RESP: {
        auto gptpResp = check_and_cast<const GptpPdelayResp *>(gptp);
        sendPdelayRespFollowUp(portId, gptpResp);
        break;
    }
    case GPTPTYPE_SYNC: {
        auto gptpSync = check_and_cast<const GptpSync *>(gptp);
        sendFollowUp(portId, gptpSync, clock->getClockTime());
        break;
    }
    case GPTPTYPE_PDELAY_REQ:
        if (portId == slavePortId) {
            pDelayReqEgressTimestamp = clock->getClockTime();
        }
        break;
    default:
        break;
    }
}

void Gptp::handleClockJump(ServoClockBase::ClockJumpDetails *clockJumpDetails)
{
    EV_INFO << "############## Adjust local timestamps #################################" << endl;
    EV_INFO << "BEFORE:" << endl;
    EV_INFO << "SYNC INGRESS TIME          - " << syncIngressTimestamp << endl;
    EV_INFO << "SYNC INGRESS TIME LAST     - " << syncIngressTimestampLast << endl;
    EV_INFO << "PDELAY REQ EGRESS TIME      - " << pDelayReqEgressTimestamp << endl;
    EV_INFO << "PDELAY RESP INGRESS TIME    - " << pDelayRespIngressTimestamp << endl;
    EV_INFO << "PDELAY RESP INGRESS TIME SET- " << pDelayRespIngressTimestampSetStart << endl;

    auto timeDiff = clockJumpDetails->newClockTime - clockJumpDetails->oldClockTime;
    adjustLocalTimestamp(syncIngressTimestamp, timeDiff);
    adjustLocalTimestamp(syncIngressTimestampLast, timeDiff);
    adjustLocalTimestamp(pDelayReqEgressTimestamp, timeDiff);
    adjustLocalTimestamp(pDelayRespIngressTimestamp, timeDiff);
    adjustLocalTimestamp(pDelayRespIngressTimestampSetStart, timeDiff);
    // NOTE: Do not pDelayReqIngressTimestamp and pDelayRespEgressTimestamp, because they are based on neighbor clock

    EV_INFO << "AFTER:" << endl;
    EV_INFO << "SYNC INGRESS TIME          - " << syncIngressTimestamp << endl;
    EV_INFO << "SYNC INGRESS TIME LAST     - " << syncIngressTimestampLast << endl;
    EV_INFO << "PDELAY REQ EGRESS TIME      - " << pDelayReqEgressTimestamp << endl;
    EV_INFO << "PDELAY RESP INGRESS TIME    - " << pDelayRespIngressTimestamp << endl;
    EV_INFO << "PDELAY RESP INGRESS TIME SET- " << pDelayRespIngressTimestampSetStart << endl;

    // We also need to adjust the timestamps of scheduled pDelayResp packets
    for (auto &reqAnswerEvent : reqAnswerEvents) {
        auto ingressTimestamp = reqAnswerEvent->getIngressTimestamp();
        adjustLocalTimestamp(ingressTimestamp, timeDiff);
        reqAnswerEvent->setIngressTimestamp(ingressTimestamp);
    }

    // This is a very special case, that only occurs when a clock jump occurs between the receptionStarted and
    // receptionEnded signal.
    //
    // You can see this in action in the SteppingClock showcase (At t=9s we do not notify the gPTP module about the
    // clock jump, this leads to an incorrect peer delay calculation). We want to allow the gPTP module to still work
    // correctly in this case, so we make this adjustment here.
    for (auto &entry : ingressTimeMap) {
        EV_INFO << " Before Ingress time: " << entry.first << " - " << entry.second << endl;
        adjustLocalTimestamp(entry.second, timeDiff);
        EV_INFO << " After Ingress time: " << entry.first << " - " << entry.second << endl;
    }
    // NOTE: There is a special case this does not solve. If the time jump would occur between the reception ended
    // and the processing of the packet in the gPTP module the same problem as described above would occur.
    // However, this would require a processing delay between the reception ended and the processing of the packet in
    // the gPTP module, which to my knowledge is currently not configurable in the INET modules.
    // Should this behavior change in the future, this timestamping adjustment mechanism needs to be changed as well to
    // somehow also adjust the timestamps already attaches as tags to the gPTP packets.
}
void Gptp::handleParameterChange(const char *name)
{
    // TODO: Maybe one could make more paremeters mutable (e.g. syncInterval might be interesting for some people)

    // Compare if parameter is grandmasterPriority1
    if (!strcmp(name, "grandmasterPriority1")) {
        // Update local priority vector
        localPriorityVector.grandmasterPriority1 = par("grandmasterPriority1");
        executeBmca();
    }
}

} // namespace inet