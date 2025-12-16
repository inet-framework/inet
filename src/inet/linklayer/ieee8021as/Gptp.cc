//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Inspired by work of Enkhtuvshin Janchivnyambuu, Henning Puttnies, Peter Danielis at University of Rostock, Germany
//

#include "inet/linklayer/ieee8021as/Gptp.h"

#include "inet/clock/model/SettableClock.h"
#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/packet/dissector/PacketDissector.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

Define_Module(Gptp);

simsignal_t Gptp::gmRateRatioChangedSignal = cComponent::registerSignal("gmRateRatioChanged");
simsignal_t Gptp::neighborRateRatioChangedSignal = cComponent::registerSignal("neighborRateRatioChanged");
simsignal_t Gptp::pdelayChangedSignal = cComponent::registerSignal("pdelayChanged");

// MAC address:
//   01-80-C2-00-00-02 for TimeSync (ieee 802.1as-2020, 13.3.1.2)
//   01-80-C2-00-00-0E for Announce and Signaling messages, for Sync, Follow_Up, Pdelay_Req, Pdelay_Resp, and Pdelay_Resp_Follow_Up messages
const MacAddress Gptp::GPTP_MULTICAST_ADDRESS("01:80:C2:00:00:0E");

// EtherType:
//   0x8809 for TimeSync (ieee 802.1as-2020, 13.3.1.2)
//   0x88F7 for Announce and Signaling messages, for Sync, Follow_Up, Pdelay_Req, Pdelay_Resp, and Pdelay_Resp_Follow_Up messages

Gptp::~Gptp()
{
    cancelAndDeleteClockEvent(pdelayTimer);
    cancelAndDeleteClockEvent(syncTimer);
}

void Gptp::initialize(int stage)
{
    ClockUserModuleBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        interfaceTable.reference(this, "interfaceTableModule", true);
        clockServo.reference(this, "clockServoModule", false);
        auto settableClock = dynamic_cast<SettableClock *>(clock.get());
        localClock = settableClock != nullptr ? settableClock->getUnderlyingClock() : clock;
        gptpNodeType = static_cast<GptpNodeType>(cEnum::get("GptpNodeType", "inet")->resolve(par("gptpNodeType")));
        domainNumber = par("domainNumber");
        syncInterval = par("syncInterval");
        pdelayInterval = par("pdelayInterval");
        pdelaySmoothingFactor = par("pdelaySmoothingFactor");
        clockIdentity = std::hash<std::string>()(getFullPath());
    }
    if (stage == INITSTAGE_LINK_LAYER) {
        registerProtocol(Protocol::gptp, gate("socketOut"), gate("socketIn"));
        // configure slave port
        const char *str = par("slavePort");
        if (*str) {
            if (gptpNodeType == MASTER_NODE)
                throw cRuntimeError("Parameter inconsistency: MASTER_NODE with slave port");
            auto nic = CHK(interfaceTable->findInterfaceByName(str));
            slavePortId = nic->getInterfaceId();
            nic->subscribe(transmissionStartedSignal, this);
            nic->subscribe(transmissionEndedSignal, this);
            nic->subscribe(receptionStartedSignal, this);
            nic->subscribe(receptionEndedSignal, this);
            auto networkInterface = interfaceTable->getInterfaceById(slavePortId);
            if (!networkInterface->matchesMulticastMacAddress(GPTP_MULTICAST_ADDRESS))
                networkInterface->addMulticastMacAddress(GPTP_MULTICAST_ADDRESS);
        }
        else
            if (gptpNodeType != MASTER_NODE)
                throw cRuntimeError("Parameter error: Missing slave port for %s", par("gptpNodeType").stringValue());
        // configure master ports
        auto v = check_and_cast<cValueArray *>(par("masterPorts").objectValue())->asStringVector();
        if (v.empty() and gptpNodeType != SLAVE_NODE)
            throw cRuntimeError("Parameter error: Missing any master port for %s", par("gptpNodeType").stringValue());
        for (const auto& p : v) {
            auto nic = CHK(interfaceTable->findInterfaceByName(p.c_str()));
            int portId = nic->getInterfaceId();
            if (portId == slavePortId)
                throw cRuntimeError("Parameter error: the port '%s' specified both master and slave port", p.c_str());
            masterPortIds.insert(portId);
            nic->subscribe(transmissionStartedSignal, this);
            nic->subscribe(transmissionEndedSignal, this);
            nic->subscribe(receptionStartedSignal, this);
            nic->subscribe(receptionEndedSignal, this);
            auto networkInterface = interfaceTable->getInterfaceById(portId);
            if (!networkInterface->matchesMulticastMacAddress(GPTP_MULTICAST_ADDRESS))
                networkInterface->addMulticastMacAddress(GPTP_MULTICAST_ADDRESS);
        }
        if (gptpNodeType == MASTER_NODE) {
            syncTimer = new ClockEvent("SyncTimer");
            scheduleClockEventAfter(par("syncInitialOffset"), syncTimer);
        }
        if (slavePortId != -1) {
            pdelayTimer = new ClockEvent("PdelayTimer");
            scheduleClockEventAfter(par("pdelayInitialOffset"), pdelayTimer);
        }
        WATCH(nextSequenceId);
        WATCH(gmRateRatio);
        WATCH(neighborRateRatio);
        WATCH(pdelay);
    }
}

void Gptp::handleMessage(cMessage *msg)
{
    if (syncTimer == msg) {
        startSyncProcesses();
        scheduleClockEventAfter(syncInterval, syncTimer);
    }
    else if (pdelayTimer == msg) {
        startPdelayMeasurementProcess();
        scheduleClockEventAfter(pdelayInterval, pdelayTimer);
    }
    else {
        Packet *packet = check_and_cast<Packet *>(msg);
        auto gptpMessage = packet->peekAtFront<GptpBase>();
        auto gptpMessageType = gptpMessage->getMessageType();
        auto portId = packet->getTag<InterfaceInd>()->getInterfaceId();
        if (gptpMessage->getDomainNumber() != domainNumber) {
            EV_ERROR << "Ignoring " << msg->getClassAndFullName() << " message because the domain number is unknown" << EV_FIELD(gptpMessage) << EV_ENDL;
            PacketDropDetails details;
            details.setReason(NOT_ADDRESSED_TO_US);
            emit(packetDroppedSignal, packet, &details);
        }
        else if (portId == slavePortId) {
            switch (gptpMessageType) {
                case GPTPTYPE_SYNC: {
                    auto sync = check_and_cast<const GptpSync *>(gptpMessage.get());
                    processSync(packet, sync);
                    if (gptpNodeType == BRIDGE_NODE)
                        forwardSync(sync);
                    break;
                }
                case GPTPTYPE_FOLLOW_UP:
                    processFollowUp(packet, check_and_cast<const GptpFollowUp *>(gptpMessage.get()));
                    if (gptpNodeType == SLAVE_NODE)
                        syncReceiverProcess = SyncReceiverProcess();
                    else if (gptpNodeType == BRIDGE_NODE) {
                        for (auto portId : masterPortIds) {
                            auto& process = syncSenderProcesses[portId];
                            if (process.state == SyncSenderProcess::State::SYNC_TRANSMISSION_STARTED || process.state == SyncSenderProcess::State::SYNC_TRANSMISSION_ENDED)
                                sendFollowUp(portId);
                        }
                    }
                    break;
                case GPTPTYPE_PDELAY_RESP:
                    processPdelayResp(packet, check_and_cast<const GptpPdelayResp *>(gptpMessage.get()));
                    break;
                case GPTPTYPE_PDELAY_RESP_FOLLOW_UP:
                    processPdelayRespFollowUp(packet, check_and_cast<const GptpPdelayRespFollowUp *>(gptpMessage.get()));
                    break;
                default:
                    throw cRuntimeError("Unknown gPTP message type: %d", (int)(gptpMessageType));
            }
        }
        else if (masterPortIds.find(portId) != masterPortIds.end()) {
            if (gptpMessageType == GPTPTYPE_PDELAY_REQ)
                processPdelayReq(packet, check_and_cast<const GptpPdelayReq *>(gptpMessage.get()));
            else
                throw cRuntimeError("Unknown gPTP message type: %d", (int)(gptpMessageType));
        }
        else
            EV_ERROR << "Ignoring " << msg->getClassAndFullName() << " message because it arrived on a passive port" << EV_FIELD(gptpMessage) << EV_FIELD(portId) << EV_ENDL;
        delete msg;
    }
}

void Gptp::startSyncProcesses()
{
    ASSERT(gptpNodeType == MASTER_NODE);
    for (auto portId : masterPortIds)
        startSyncProcess(portId);
    nextSequenceId++;
}

void Gptp::startSyncProcess(int portId) {
    ASSERT(gptpNodeType == MASTER_NODE);
    syncSenderProcesses[portId] = SyncSenderProcess();
    sendSync(portId);
}

void Gptp::startPdelayMeasurementProcess()
{
    pdelayMeasurementRequesterProcesses[slavePortId] = PdelayMeasurementRequesterProcess();
    sendPdelayReq();
    nextSequenceId++;
}

void Gptp::synchronize()
{
    ASSERT(gptpNodeType != MASTER_NODE);
    ASSERT(syncReceiverProcess.state == SyncReceiverProcess::State::FOLLOW_UP_RECEPTION_ENDED);
    // at this point we have just received the follow-up message and we have already received the sync message before
    // we want to calculate what the slave clock should have read at the reception of the first bit of the sync message
    //
    // the slave clock time is calculated as 't1 + correctionField + pdelay' where
    //  - t1 is the master clock time at the transmission of the first bit of the sync message
    //  - t1 is conveyed to the slave via the preciseOriginTimestamp field of the follow-up message
    //  - correctionField is the accumulated delay that a sync message experienced between the master’s transmission and the slave’s reception, excluding propagation delay, which is handled separately
    //    for a simple master-slave point-to-point topology without time-aware bridges, the correction field is typically zero
    //  - pdelay is the estimated signal propagation delay from the master to the slave, typically determined via the pdelay mechanism
    //
    // t2 is the actual slave clock time at the reception of the first bit of the sync message
    //
    // offset = t2 - (t1 + correctionField + pdelay)
    // offset is the time difference in the slave clock time compared to the ideal value (the one matching the master's clock the most) at the reception of the first bit of the sync message
    //
    // either the slave clock is updated or a clock servo is configured with this offset
    clocktime_t t1 = syncReceiverProcess.followUp->getPreciseOriginTimestamp();
    clocktime_t t2 = syncReceiverProcess.syncReceptionStartSynchronized;
    ASSERT(t1 >= 0);
    ASSERT(t2 >= 0);
    clocktime_t correctionField = syncReceiverProcess.followUp->getCorrectionField();
    ASSERT(pdelay >= 0);
    clocktime_t clockTimeDifference = t2 - (t1 + correctionField + pdelay);
    ppm rateDifference = ppm(gmRateRatio.toPpm());
    EV_INFO << "Updating clock time" << EV_FIELD(clockTimeDifference) << EV_FIELD(rateDifference) << EV_ENDL;
    clockServo->adjustClockForDifference(clockTimeDifference, rateDifference);
}

void Gptp::sendSync(int portId)
{
    ASSERT(gptpNodeType != SLAVE_NODE);
    auto& process = syncSenderProcesses[portId];
    ASSERT(process.state == SyncSenderProcess::State::INITIALIZED);
    auto sync = makeShared<GptpSync>();
    sync->setDomainNumber(domainNumber);
    sync->setSequenceId(nextSequenceId);
    sync->setCorrectionField(0);
    process.sync = sync->dup();
    auto packet = new Packet("GptpSync");
    packet->addTag<MacAddressReq>()->setDestAddress(GPTP_MULTICAST_ADDRESS);
    packet->insertAtFront(sync);
    sendPacketToNic(packet, portId);
    // The sendFollowUp(portId) called by receiveSignal(), when GptpSync sent
}

void Gptp::sendFollowUp(int portId)
{
    ASSERT(gptpNodeType != SLAVE_NODE);
    auto& process = syncSenderProcesses[portId];
    ASSERT(process.state == SyncSenderProcess::State::SYNC_TRANSMISSION_STARTED || process.state == SyncSenderProcess::State::SYNC_TRANSMISSION_ENDED);
    ASSERT(gptpNodeType == MASTER_NODE || syncReceiverProcess.state == SyncReceiverProcess::State::COMPLETED);
    auto followUp = makeShared<GptpFollowUp>();
    followUp->setDomainNumber(domainNumber);
    if (gptpNodeType == MASTER_NODE) {
        followUp->setSequenceId(process.sync->getSequenceId());
        followUp->setCorrectionField(0);
        followUp->setPreciseOriginTimestamp(process.syncTransmissionStartSynchronized);
    }
    else if (gptpNodeType == BRIDGE_NODE) {
        followUp->setSequenceId(syncReceiverProcess.sync->getSequenceId());
        clocktime_t upstreamTxTime = syncReceiverProcess.syncReceptionStartUnsychronized - pdelay / neighborRateRatio;
        clocktime_t residenceTime = process.syncTransmissionStartUnsychronized - upstreamTxTime;
        ASSERT(residenceTime >= 0);
        ASSERT(pdelay >= 0);
        followUp->setCorrectionField(syncReceiverProcess.followUp->getCorrectionField() + gmRateRatio * residenceTime);
        followUp->setPreciseOriginTimestamp(syncReceiverProcess.followUp->getPreciseOriginTimestamp());
    }
    followUp->getFollowUpInformationTLVForUpdate().setRateRatio(gmRateRatio.toDouble());
    process.followUp = followUp->dup();
    auto packet = new Packet("GptpFollowUp");
    packet->addTag<MacAddressReq>()->setDestAddress(GPTP_MULTICAST_ADDRESS);
    packet->insertAtFront(followUp);
    sendPacketToNic(packet, portId);
}

void Gptp::sendPdelayReq()
{
    ASSERT(gptpNodeType != MASTER_NODE);
    auto& process = pdelayMeasurementRequesterProcesses[slavePortId];
    ASSERT(process.state == PdelayMeasurementRequesterProcess::State::INITIALIZED);
    auto pdelayReq = makeShared<GptpPdelayReq>();
    pdelayReq->setDomainNumber(domainNumber);
    pdelayReq->setCorrectionField(CLOCKTIME_ZERO);
    PortIdentity portIdentity;
    portIdentity.clockIdentity = clockIdentity;
    portIdentity.portNumber = slavePortId;
    pdelayReq->setSourcePortIdentity(portIdentity);
    pdelayReq->setSequenceId(nextSequenceId);
    process.pdelayReq = pdelayReq->dup();
    auto packet = new Packet("GptpPdelayReq");
    packet->addTag<MacAddressReq>()->setDestAddress(GPTP_MULTICAST_ADDRESS);
    packet->insertAtFront(pdelayReq);
    sendPacketToNic(packet, slavePortId);
}

void Gptp::sendPdelayResp(int portId)
{
    ASSERT(gptpNodeType != SLAVE_NODE);
    auto& process = pdelayMeasurementResponderProcesses[portId];
    ASSERT(process.state == PdelayMeasurementResponderProcess::State::PDELAY_REQ_RECEPTION_ENDED);
    auto pdelayResp = makeShared<GptpPdelayResp>();
    pdelayResp->setDomainNumber(domainNumber);
    pdelayResp->setRequestingPortIdentity(process.pdelayReq->getSourcePortIdentity());
    pdelayResp->setSequenceId(process.pdelayReq->getSequenceId());
    pdelayResp->setRequestReceiptTimestamp(process.pdelayReqReceptionStartUnsychronized);
    process.pdelayResp = pdelayResp->dup();
    auto packet = new Packet("GptpPdelayResp");
    packet->addTag<MacAddressReq>()->setDestAddress(GPTP_MULTICAST_ADDRESS);
    packet->insertAtFront(pdelayResp);
    sendPacketToNic(packet, portId);
    // The sendPdelayRespFollowUp(portId) called by receiveSignal(), when GptpPdelayResp sent
}

void Gptp::sendPdelayRespFollowUp(int portId, const GptpPdelayResp *resp)
{
    ASSERT(gptpNodeType != SLAVE_NODE);
    auto& process = pdelayMeasurementResponderProcesses[portId];
    ASSERT(process.state == PdelayMeasurementResponderProcess::State::PDELAY_RESP_TRANSMISSION_ENDED);
    auto pdelayRespFollowUp = makeShared<GptpPdelayRespFollowUp>();
    pdelayRespFollowUp->setDomainNumber(domainNumber);
    pdelayRespFollowUp->setResponseOriginTimestamp(process.pdelayRespTransmissionStartUnsychronized);
    pdelayRespFollowUp->setRequestingPortIdentity(resp->getRequestingPortIdentity());
    pdelayRespFollowUp->setSequenceId(resp->getSequenceId());
    process.pdelayRespFollowUp = pdelayRespFollowUp->dup();
    auto packet = new Packet("GptpPdelayRespFollowUp");
    packet->addTag<MacAddressReq>()->setDestAddress(GPTP_MULTICAST_ADDRESS);
    packet->insertAtFront(pdelayRespFollowUp);
    sendPacketToNic(packet, portId);
}

void Gptp::forwardSync(const GptpSync *receivedSync)
{
    ASSERT(gptpNodeType == BRIDGE_NODE);
    ASSERT(syncReceiverProcess.state == SyncReceiverProcess::State::SYNC_RECEPTION_ENDED);
    for (auto portId : masterPortIds) {
        syncSenderProcesses[portId] = SyncSenderProcess();
        forwardSync(receivedSync, portId);
    }
}

void Gptp::forwardSync(const GptpSync *receivedSync, int portId)
{
    ASSERT(gptpNodeType == BRIDGE_NODE);
    auto& process = syncSenderProcesses[portId];
    ASSERT(process.state == SyncSenderProcess::State::INITIALIZED);
    auto sentSync = makeShared<GptpSync>();
    *sentSync = *receivedSync;
    auto packet = new Packet("GptpSync");
    packet->addTag<MacAddressReq>()->setDestAddress(GPTP_MULTICAST_ADDRESS);
    packet->insertAtFront(sentSync);
    sendPacketToNic(packet, portId);
}

void Gptp::processSync(Packet *packet, const GptpSync *sync)
{
    ASSERT(gptpNodeType != MASTER_NODE);
    ASSERT(syncReceiverProcess.state == SyncReceiverProcess::State::SYNC_RECEPTION_ENDED);
    EV_INFO << "Processing Sync message" << EV_FIELD(sync) << EV_ENDL;
    syncReceiverProcess.sync = sync->dup();
}

void Gptp::processFollowUp(Packet *packet, const GptpFollowUp *followUp)
{
    ASSERT(gptpNodeType != MASTER_NODE);
    if (followUp->getSequenceId() != syncReceiverProcess.sync->getSequenceId())
        EV_WARN << "Ignoring FollowUp message because sequence ID is unknown" << EV_FIELD(followUp) << EV_ENDL;
    else {
        ASSERT(syncReceiverProcess.state == SyncReceiverProcess::State::FOLLOW_UP_RECEPTION_ENDED);
        EV_INFO << "Processing FollowUp message" << EV_FIELD(followUp) << EV_ENDL;
        syncReceiverProcess.followUp = followUp->dup();
        computeGmRateRatio();
        synchronize();
        syncReceiverProcess.state = SyncReceiverProcess::State::COMPLETED;
    }
}

void Gptp::processPdelayReq(Packet *packet, const GptpPdelayReq *pdelayReq)
{
    ASSERT(gptpNodeType != SLAVE_NODE);
    int portId = packet->getTag<InterfaceInd>()->getInterfaceId();
    auto& process = pdelayMeasurementResponderProcesses[portId];
    ASSERT(process.state == PdelayMeasurementResponderProcess::State::PDELAY_REQ_RECEPTION_ENDED);
    process.pdelayReq = pdelayReq->dup();
    sendPdelayResp(portId);
}

void Gptp::processPdelayResp(Packet *packet, const GptpPdelayResp *pdelayResp)
{
    ASSERT(gptpNodeType != MASTER_NODE);
    auto& process = pdelayMeasurementRequesterProcesses[slavePortId];
    ASSERT(process.state == PdelayMeasurementRequesterProcess::State::PDELAY_RESP_RECEPTION_ENDED);
    if (pdelayResp->getRequestingPortIdentity().clockIdentity != clockIdentity || pdelayResp->getRequestingPortIdentity().portNumber != slavePortId)
        EV_WARN << "Ignoring PdelayResp message because port identity is unknown" << EV_FIELD(pdelayResp) << EV_ENDL;
    else if (pdelayResp->getSequenceId() != process.pdelayReq->getSequenceId())
        EV_WARN << "Ignoring PdelayResp message because sequence ID is unknown" << EV_FIELD(pdelayResp) << EV_ENDL;
    else {
        EV_INFO << "Processing PdelayResp message" << EV_FIELD(pdelayResp) << EV_ENDL;
        process.pdelayResp = pdelayResp->dup();
    }
}

void Gptp::processPdelayRespFollowUp(Packet *packet, const GptpPdelayRespFollowUp *pdelayRespFollowUp)
{
    ASSERT(gptpNodeType != MASTER_NODE);
    auto& process = pdelayMeasurementRequesterProcesses[slavePortId];
    ASSERT(process.state == PdelayMeasurementRequesterProcess::State::PDELAY_RESP_FOLLOW_UP_RECEPTION_ENDED);
    if (pdelayRespFollowUp->getRequestingPortIdentity().clockIdentity != clockIdentity || pdelayRespFollowUp->getRequestingPortIdentity().portNumber != slavePortId)
        EV_WARN << "Ignoring PdelayRespFollowUp message because port identity is unknown" << EV_FIELD(pdelayRespFollowUp) << EV_ENDL;
    else if (pdelayRespFollowUp->getSequenceId() != process.pdelayReq->getSequenceId())
        EV_WARN << "Ignoring PdelayRespFollowUp message because sequence ID is unknown" << EV_FIELD(pdelayRespFollowUp) << EV_ENDL;
    else {
        EV_INFO << "Processing PdelayRespFollowUp message" << EV_FIELD(pdelayRespFollowUp) << EV_ENDL;
        process.pdelayRespFollowUp = pdelayRespFollowUp->dup();
        computeNeighborRateRatio();
        computePropTime();
        process.state = PdelayMeasurementRequesterProcess::State::COMPLETED;
    }
}

void Gptp::computeGmRateRatio()
{
    ASSERT(syncReceiverProcess.state == SyncReceiverProcess::State::FOLLOW_UP_RECEPTION_ENDED);
    ASSERT(pdelay >= 0);
    clocktime_t sourceTime = syncReceiverProcess.followUp->getPreciseOriginTimestamp() + syncReceiverProcess.followUp->getCorrectionField() + pdelay;
    clocktime_t localTime = syncReceiverProcess.syncReceptionStartUnsychronized;
    if (previousSourceTime != -1 && previousLocalTime != -1) {
        // IEEE 802.1AS-2020 Section 10.2.11.2.1, index N is previous, 0 is last
        clocktime_t sourceTimeDifference = sourceTime - previousSourceTime;
        clocktime_t localTimeDifference = localTime - previousLocalTime;
        // TODO: we could be more accurate here by not doing the double division but directly calculating the e_q63 in the ClockTimeScale
        gmRateRatio = ClockTimeScale::fromRatio(sourceTimeDifference, localTimeDifference);
        ppm gmDriftRate = ppm(gmRateRatio.toPpm());
        EV_INFO << "Updating GM rate ratio" << EV_FIELD(gmRateRatio) << EV_FIELD(gmDriftRate) << EV_FIELD(sourceTimeDifference) << EV_FIELD(localTimeDifference) << EV_ENDL;
        emit(gmRateRatioChangedSignal, gmRateRatio.toDouble());
    }
    previousSourceTime = sourceTime;
    previousLocalTime = localTime;
}

void Gptp::computeNeighborRateRatio()
{
    auto& process = pdelayMeasurementRequesterProcesses[slavePortId];
    ASSERT(process.state == PdelayMeasurementRequesterProcess::State::PDELAY_RESP_FOLLOW_UP_RECEPTION_ENDED);
    // NOTE: the standard defines the usage of the correction field for the following two timestamps
    // however, these contain fractional nanoseconds, which we do not use in INET
    clocktime_t correctedResponderEventTimestamp = process.pdelayRespFollowUp->getResponseOriginTimestamp();
    clocktime_t pdelayRespEventIngressTimestamp = process.pdelayRespReceptionStartUnsychronized;
    if (previousCorrectedResponderEventTimestamp != -1 && previousPdelayRespEventIngressTimestamp != -1) {
        // IEEE 802.1AS-2020 Section 11.2.19.3.3, index N is previous, 0 is last
        clocktime_t correctedResponderEventTimestampDifference = correctedResponderEventTimestamp - previousCorrectedResponderEventTimestamp;
        clocktime_t pdelayRespEventIngressTimestampDifference = pdelayRespEventIngressTimestamp - previousPdelayRespEventIngressTimestamp;
        // TODO: we could be more accurate here by not doing the double division but directly calculating the e_q63 in the ClockTimeScale
        neighborRateRatio = ClockTimeScale::fromRatio(correctedResponderEventTimestampDifference, pdelayRespEventIngressTimestampDifference);
        ppm neighborDriftRate = ppm(neighborRateRatio.toPpm());
        EV_INFO << "Updating neighbor rate ratio" << EV_FIELD(neighborRateRatio) << EV_FIELD(neighborDriftRate) << EV_FIELD(correctedResponderEventTimestampDifference) << EV_FIELD(pdelayRespEventIngressTimestampDifference) << EV_ENDL;
        emit(neighborRateRatioChangedSignal, neighborRateRatio.toDouble());
    }
    previousCorrectedResponderEventTimestamp = correctedResponderEventTimestamp;
    previousPdelayRespEventIngressTimestamp = pdelayRespEventIngressTimestamp;
}

void Gptp::computePropTime()
{
    auto& process = pdelayMeasurementRequesterProcesses[slavePortId];
    ASSERT(process.state == PdelayMeasurementRequesterProcess::State::PDELAY_RESP_FOLLOW_UP_RECEPTION_ENDED);
    clocktime_t t1 = process.pdelayReqTransmissionStartUnsychronized;
    clocktime_t t2 = process.pdelayResp->getRequestReceiptTimestamp();
    clocktime_t t3 = process.pdelayRespFollowUp->getResponseOriginTimestamp();
    clocktime_t t4 = process.pdelayRespReceptionStartUnsychronized;
    ASSERT(t1 >= 0);
    ASSERT(t2 >= 0);
    ASSERT(t3 >= 0);
    ASSERT(t4 >= 0);
    ASSERT(t4 >= t1);
    ASSERT(t3 >= t2);
    // IEEE 802.1AS-2020 Section 11.2.19.3.4
    clocktime_t currentPdelay = (neighborRateRatio * (t4 - t1) - (t3 - t2)) / 2;
    ASSERT(currentPdelay >= 0);
    if (pdelay == -1)
        pdelay = currentPdelay;
    else
        pdelay = pdelaySmoothingFactor * currentPdelay + (1 - pdelaySmoothingFactor) * pdelay;
    EV_INFO << "Updating pdelay" << EV_FIELD(pdelay) << EV_ENDL;
    emit(pdelayChangedSignal, CLOCKTIME_AS_SIMTIME(pdelay));
}

void Gptp::sendPacketToNic(Packet *packet, int portId)
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

const GptpBase *Gptp::extractGptpMessage(Packet *packet)
{
    PacketDissector::ChunkFinder chunkFinder(&Protocol::gptp);
    PacketDissector packetDissector(ProtocolDissectorRegistry::getInstance(), chunkFinder);
    packetDissector.dissectPacket(packet);
    const auto& chunk = staticPtrCast<const GptpBase>(chunkFinder.getChunk());
    return chunk != nullptr ? chunk.get() : nullptr;
}

bool Gptp::isAllSyncSenderProcessesCompleted() const
{
    for (auto portId : masterPortIds) {
        const auto& process = syncSenderProcesses.at(portId);
        if (process.state != SyncSenderProcess::State::COMPLETED)
            return false;
    }
    return true;
}

void Gptp::receiveSignal(cComponent *source, simsignal_t simSignal, cObject *object, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(simSignal));
    auto ethernetSignal = check_and_cast<cPacket *>(object);
    auto packet = check_and_cast_nullable<Packet *>(ethernetSignal->getEncapsulatedPacket());
    if (packet == nullptr)
        return;
    auto gptpMessage = extractGptpMessage(packet);
    if (gptpMessage == nullptr || gptpMessage->getDomainNumber() != domainNumber)
        return;
    int portId = getContainingNicModule(check_and_cast<cModule*>(source))->getInterfaceId();
    if (simSignal == receptionStartedSignal) {
        switch (gptpMessage->getMessageType()) {
            case GPTPTYPE_PDELAY_REQ: {
                auto& process = pdelayMeasurementResponderProcesses[portId];
                process = PdelayMeasurementResponderProcess();
                ASSERT(process.state == PdelayMeasurementResponderProcess::State::INITIALIZED);
                process.state = PdelayMeasurementResponderProcess::State::PDELAY_REQ_RECEPTION_STARTED;
                process.pdelayReqReceptionStartUnsychronized = localClock->getClockTime();
                break;
            }
            case GPTPTYPE_PDELAY_RESP: {
                auto& process = pdelayMeasurementRequesterProcesses[portId];
                ASSERT(process.state == PdelayMeasurementRequesterProcess::State::PDELAY_REQ_TRANSMISSION_ENDED);
                process.state = PdelayMeasurementRequesterProcess::State::PDELAY_RESP_RECEPTION_STARTED;
                process.pdelayRespReceptionStartUnsychronized = localClock->getClockTime();
                break;
            }
            case GPTPTYPE_PDELAY_RESP_FOLLOW_UP: {
                auto& process = pdelayMeasurementRequesterProcesses[portId];
                ASSERT(process.state == PdelayMeasurementRequesterProcess::State::PDELAY_RESP_RECEPTION_ENDED);
                process.state = PdelayMeasurementRequesterProcess::State::PDELAY_RESP_FOLLOW_UP_RECEPTION_STARTED;
                break;
            }
            case GPTPTYPE_SYNC: {
                ASSERT(syncReceiverProcess.state == SyncReceiverProcess::State::INITIALIZED);
                syncReceiverProcess.state = SyncReceiverProcess::State::SYNC_RECEPTION_STARTED;
                syncReceiverProcess.syncReceptionStartSynchronized = clock->getClockTime();
                syncReceiverProcess.syncReceptionStartUnsychronized = localClock->getClockTime();
                break;
            }
            case GPTPTYPE_FOLLOW_UP: {
                ASSERT(syncReceiverProcess.state == SyncReceiverProcess::State::SYNC_RECEPTION_ENDED);
                syncReceiverProcess.state = SyncReceiverProcess::State::FOLLOW_UP_RECEPTION_STARTED;
                break;
            }
            default: throw cRuntimeError("Unknown gPTP message");
        }
    }
    else if (simSignal == receptionEndedSignal) {
        switch (gptpMessage->getMessageType()) {
            case GPTPTYPE_PDELAY_REQ: {
                auto& process = pdelayMeasurementResponderProcesses[portId];
                if (process.state != PdelayMeasurementResponderProcess::State::PDELAY_REQ_RECEPTION_STARTED)
                    throw cRuntimeError("Invalid process state, the network interface module must emit receptionStarted signal");
                process.state = PdelayMeasurementResponderProcess::State::PDELAY_REQ_RECEPTION_ENDED;
                break;
            }
            case GPTPTYPE_PDELAY_RESP: {
                auto& process = pdelayMeasurementRequesterProcesses[portId];
                if (process.state != PdelayMeasurementRequesterProcess::State::PDELAY_RESP_RECEPTION_STARTED)
                    throw cRuntimeError("Invalid process state, the network interface module must emit receptionStarted signal");
                process.state = PdelayMeasurementRequesterProcess::State::PDELAY_RESP_RECEPTION_ENDED;
                break;
            }
            case GPTPTYPE_PDELAY_RESP_FOLLOW_UP: {
                auto& process = pdelayMeasurementRequesterProcesses[portId];
                if (process.state != PdelayMeasurementRequesterProcess::State::PDELAY_RESP_FOLLOW_UP_RECEPTION_STARTED)
                    throw cRuntimeError("Invalid process state, the network interface module must emit receptionStarted signal");
                process.state = PdelayMeasurementRequesterProcess::State::PDELAY_RESP_FOLLOW_UP_RECEPTION_ENDED;
                break;
            }
            case GPTPTYPE_SYNC: {
                if (syncReceiverProcess.state != SyncReceiverProcess::State::SYNC_RECEPTION_STARTED)
                    throw cRuntimeError("Invalid process state, the network interface module must emit receptionStarted signal");
                syncReceiverProcess.state = SyncReceiverProcess::State::SYNC_RECEPTION_ENDED;
                break;
            }
            case GPTPTYPE_FOLLOW_UP: {
                if (syncReceiverProcess.state != SyncReceiverProcess::State::FOLLOW_UP_RECEPTION_STARTED)
                    throw cRuntimeError("Invalid process state, the network interface module must emit receptionStarted signal");
                syncReceiverProcess.state = SyncReceiverProcess::State::FOLLOW_UP_RECEPTION_ENDED;
                break;
            }
            default: throw cRuntimeError("Unknown gPTP message");
        }
    }
    else if (simSignal == transmissionStartedSignal) {
        switch (gptpMessage->getMessageType()) {
            case GPTPTYPE_PDELAY_REQ: {
                auto& process = pdelayMeasurementRequesterProcesses[portId];
                ASSERT(process.state == PdelayMeasurementRequesterProcess::State::INITIALIZED);
                process.state = PdelayMeasurementRequesterProcess::State::PDELAY_REQ_TRANSMISSION_STARTED;
                process.pdelayReqTransmissionStartUnsychronized = localClock->getClockTime();
                break;
            }
            case GPTPTYPE_PDELAY_RESP: {
                auto& process = pdelayMeasurementResponderProcesses[portId];
                ASSERT(process.state == PdelayMeasurementResponderProcess::State::PDELAY_REQ_RECEPTION_ENDED);
                process.state = PdelayMeasurementResponderProcess::State::PDELAY_RESP_TRANSMISSION_STARTED;
                process.pdelayRespTransmissionStartUnsychronized = localClock->getClockTime();
                break;
            }
            case GPTPTYPE_PDELAY_RESP_FOLLOW_UP: {
                auto& process = pdelayMeasurementResponderProcesses[portId];
                ASSERT(process.state == PdelayMeasurementResponderProcess::State::PDELAY_RESP_TRANSMISSION_ENDED);
                process.state = PdelayMeasurementResponderProcess::State::PDELAY_RESP_FOLLOW_UP_TRANSMISSION_STARTED;
                break;
            }
            case GPTPTYPE_SYNC: {
                auto& process = syncSenderProcesses[portId];
                ASSERT(process.state == SyncSenderProcess::State::INITIALIZED);
                process.state = SyncSenderProcess::State::SYNC_TRANSMISSION_STARTED;
                process.syncTransmissionStartUnsychronized = localClock->getClockTime();
                process.syncTransmissionStartSynchronized = clock->getClockTime();
                if (gptpNodeType == MASTER_NODE)
                    sendFollowUp(portId);
                else if (gptpNodeType == BRIDGE_NODE) {
                    if (syncReceiverProcess.state == SyncReceiverProcess::State::COMPLETED)
                        sendFollowUp(portId);
                }
                break;
            }
            case GPTPTYPE_FOLLOW_UP: {
                auto& process = syncSenderProcesses[portId];
                ASSERT(process.state == SyncSenderProcess::State::SYNC_TRANSMISSION_ENDED);
                process.state = SyncSenderProcess::State::FOLLOW_UP_TRANSMISSION_STARTED;
                break;
            }
            default: throw cRuntimeError("Unknown gPTP message");
        }
    }
    else if (simSignal == transmissionEndedSignal) {
        switch (gptpMessage->getMessageType()) {
            case GPTPTYPE_PDELAY_REQ: {
                auto& process = pdelayMeasurementRequesterProcesses[portId];
                if (process.state != PdelayMeasurementRequesterProcess::State::PDELAY_REQ_TRANSMISSION_STARTED)
                    throw cRuntimeError("Invalid process state, the network interface module must emit transmissionStarted signal");
                process.state = PdelayMeasurementRequesterProcess::State::PDELAY_REQ_TRANSMISSION_ENDED;
                break;
            }
            case GPTPTYPE_PDELAY_RESP: {
                auto& process = pdelayMeasurementResponderProcesses[portId];
                if (process.state != PdelayMeasurementResponderProcess::State::PDELAY_RESP_TRANSMISSION_STARTED)
                    throw cRuntimeError("Invalid process state, the network interface module must emit transmissionStarted signal");
                process.state = PdelayMeasurementResponderProcess::State::PDELAY_RESP_TRANSMISSION_ENDED;
                auto gptpResp = check_and_cast<const GptpPdelayResp*>(gptpMessage);
                sendPdelayRespFollowUp(portId, gptpResp);
                break;
            }
            case GPTPTYPE_PDELAY_RESP_FOLLOW_UP: {
                auto& process = pdelayMeasurementResponderProcesses[portId];
                if (process.state != PdelayMeasurementResponderProcess::State::PDELAY_RESP_FOLLOW_UP_TRANSMISSION_STARTED)
                    throw cRuntimeError("Invalid process state, the network interface module must emit transmissionStarted signal");
                process.state = PdelayMeasurementResponderProcess::State::PDELAY_RESP_FOLLOW_UP_TRANSMISSION_ENDED;
                // immediately complete the process
                process.state = PdelayMeasurementResponderProcess::State::COMPLETED;
                break;
            }
            case GPTPTYPE_SYNC: {
                auto& process = syncSenderProcesses[portId];
                if (process.state != SyncSenderProcess::State::SYNC_TRANSMISSION_STARTED)
                    throw cRuntimeError("Invalid process state, the network interface module must emit transmissionStarted signal");
                process.state = SyncSenderProcess::State::SYNC_TRANSMISSION_ENDED;
                break;
            }
            case GPTPTYPE_FOLLOW_UP: {
                auto& process = syncSenderProcesses[portId];
                if (process.state != SyncSenderProcess::State::FOLLOW_UP_TRANSMISSION_STARTED)
                    throw cRuntimeError("Invalid process state, the network interface module must emit transmissionStarted signal");
                process.state = SyncSenderProcess::State::FOLLOW_UP_TRANSMISSION_ENDED;
                // immediately complete the process
                process.state = SyncSenderProcess::State::COMPLETED;
                if (gptpNodeType == BRIDGE_NODE) {
                    if (isAllSyncSenderProcessesCompleted())
                        syncReceiverProcess = SyncReceiverProcess();
                }
                break;
            }
            default: throw cRuntimeError("Unknown gPTP message");
        }
    }
    else
        throw cRuntimeError("Unknown signal");
}

} // namespace inet
