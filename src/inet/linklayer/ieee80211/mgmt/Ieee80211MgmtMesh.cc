// SPDX-License-Identifier: LGPL-3.0-or-later

#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtMesh.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211HtMgmtElements.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211VhtMgmtElements.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/IIeee80211ChannelAccess.h"

#include <algorithm>
#include <limits>
#include <sstream>

#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/ieee80211/IIeee80211Mac.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211SubtypeTag_m.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211BeaconInterval.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/SignalTag_m.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211OfdmMode.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Tag_m.h"

namespace inet::ieee80211 {

using namespace physicallayer;

Define_Module(Ieee80211MgmtMesh);
simsignal_t Ieee80211MgmtMesh::meshPeeringStateChangedSignal = registerSignal("meshPeeringStateChanged");
simsignal_t Ieee80211MgmtMesh::meshManagementDiscardedSignal = registerSignal("meshManagementDiscarded");
simsignal_t Ieee80211MgmtMesh::meshBeaconTransmissionEndSignal = registerSignal("meshBeaconTransmissionEnd");

Ieee80211MgmtMesh::~Ieee80211MgmtMesh()
{
    if (myIface && myIface->isSubscribed(modesetChangedSignal, this))
        myIface->unsubscribe(modesetChangedSignal, this);
    if (myIface && myIface->isSubscribed(packetDroppedSignal, this))
        myIface->unsubscribe(packetDroppedSignal, this);
    cancelAndDelete(beaconTimer);
    for (auto& entry : peers)
        cancelAndDelete(entry.second.timer);
}

void Ieee80211MgmtMesh::initialize(int stage)
{
    Ieee80211MgmtBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        linkMetric.reference(this,"linkMetricModule",true);
        linkTiming.reference(this,"linkTimingModule",true);
        pathSelection.reference(this,"pathSelectionModule",false);
        meshPowerSave.reference(this,"meshPowerSaveModule",false);
        mib->mode = Ieee80211Mib::MESH;
        MeshConfiguration configuration;
        if (par("pathSelectionMetricId").intValue() != 1 && par("pathSelectionMetricId").intValue() != 2)
            throw cRuntimeError("Mesh pathSelectionMetricId must be 1 or 2");
        configuration.pathSelectionMetricId = par("pathSelectionMetricId");
        mib->configureMeshProfile(par("meshId").stdstringValue(), configuration);
        beaconInterval = normalizeIeee80211BeaconInterval(par("beaconInterval"));
        retryTimeout = par("retryTimeout");
        confirmTimeout = par("confirmTimeout");
        holdingTimeout = par("holdingTimeout");
        for (auto timeout : {retryTimeout, confirmTimeout, holdingTimeout})
            if (timeout < SimTime(1, SIMTIME_MS) || timeout > SimTime(255, SIMTIME_MS))
                throw cRuntimeError("Mesh peering timeouts must be in [1,255] ms");
        maxRetries = par("maxRetries");
        maxPeerings = par("maxPeerings");
        dtimPeriod = par("dtimPeriod");
        autoPeer = par("autoPeer");
        auto window = simtime_t(par("meshAwakeWindow"));
        if (maxRetries < 0 || maxRetries > 16 || maxPeerings < 1 || maxPeerings > 2007
                || dtimPeriod < 1 || dtimPeriod > 255 || window < SIMTIME_ZERO
                || window > SimTime(65535LL * 1024, SIMTIME_US) || simtime_t(par("initialTsf")) < SIMTIME_ZERO)
            throw cRuntimeError("Invalid mesh peering, beacon or TSF policy");
        awakeWindow = window.inUnit(SIMTIME_US) / 1024;
        beaconTimer = new cMessage("meshBeaconTimer");
        // Receive events at the exact timeout instant precede timer expiry.
        beaconTimer->setSchedulingPriority(1);
        WATCH_EXPR("beaconPending", isBeaconPending());
        WATCH(totalSlowing);
        WATCH(maxClockDrift);
        WATCH(nextTbttTsf);
        WATCH_EXPR("peerStates", describePeers());
    }
    if (stage == INITSTAGE_LAST && !modeSet)
        throw cRuntimeError("Open mesh peering requires a prepared PHY catalog");
}

void Ieee80211MgmtMesh::start()
{
    if (!myIface->isSubscribed(modesetChangedSignal, this))
        myIface->subscribe(modesetChangedSignal, this);
    if (!myIface->isSubscribed(packetDroppedSignal, this))
        myIface->subscribe(packetDroppedSignal, this);
    tsfOrigin = simTime() - simtime_t(par("initialTsf"));
    totalSlowing = lastSlowingDuration = maxClockDrift = SIMTIME_ZERO;
    lastSlowingStart = simTime();
    beaconNumber = 0;
    pendingBeaconTreeId = -1;
    auto initial = simtime_t(par("initialTsf"));
    auto periods = initial.raw() / beaconInterval.raw();
    nextTbttTsf = (periods + (initial.raw() % beaconInterval.raw() != 0)) * beaconInterval;
    beaconNumber = periods + (initial.raw() % beaconInterval.raw() != 0);
    scheduleNextBeacon();
}

void Ieee80211MgmtMesh::stop()
{
    cancelEvent(beaconTimer);
    pendingBeaconTreeId = -1;
    for (auto& entry : peers) {
        if (entry.second.state != IDLE)
            setState(entry.first,entry.second,IDLE);
    }
    neighbors.clear();
    if (myIface->isSubscribed(modesetChangedSignal, this))
        myIface->unsubscribe(modesetChangedSignal, this);
    if (myIface->isSubscribed(packetDroppedSignal, this))
        myIface->unsubscribe(packetDroppedSignal, this);
}

void Ieee80211MgmtMesh::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    if (signal == packetDroppedSignal) {
        Enter_Method("beacon packet drop");
        auto packet = dynamic_cast<Packet *>(object);
        bool currentBeacon = isMeshManagementActive() && packet && packet->getTreeId() == pendingBeaconTreeId;
        if (currentBeacon)
            pendingBeaconTreeId = -1;
        if (currentBeacon && meshPowerSave && meshPowerSave->isEnabled())
            meshPowerSave->timingTransmissionRetired();
    }
    else
        Ieee80211MgmtBase::receiveSignal(source, signal, object, details);
}

void Ieee80211MgmtMesh::handleCommand(int, cObject *control)
{
    delete control;
    throw cRuntimeError("Mesh management uses openPeer/closePeer; infrastructure agent commands are unsupported");
}

void Ieee80211MgmtMesh::handleMessageWhenUp(cMessage *message)
{
    if (message->arrivedOn("macIn"))
        processManagement(check_and_cast<Packet *>(message));
    else
        Ieee80211MgmtBase::handleMessageWhenUp(message);
}

void Ieee80211MgmtMesh::handleMessageWhenDown(cMessage *message)
{
    auto packet = dynamic_cast<Packet *>(message);
    if (!message->isSelfMessage() && packet && message->arrivedOn("macIn"))
        dropManagementFrame(packet);
    else
        Ieee80211MgmtBase::handleMessageWhenDown(message);
}

void Ieee80211MgmtMesh::dropManagementFrame(Packet *packet)
{
    emit(meshManagementDiscardedSignal, 1L);
    Ieee80211MgmtBase::dropManagementFrame(packet);
}

void Ieee80211MgmtMesh::processManagement(Packet *packet)
{
    auto header = packet->popAtFront<Ieee80211MgmtHeader>();
    numMgmtFramesReceived++;
    if (auto action = dynamicPtrCast<const Ieee80211MeshActionFrame>(header)) {
        if (action->getCategory() == MESH_SELF_PROTECTED_ACTION)
            processPeering(packet, action);
        else if (action->getCategory() == MESH_ACTION && action->getMeshAction() == MESH_LINK_METRIC_REPORT)
            processLinkMetric(packet, action);
        else if (action->getCategory() == MESH_ACTION && action->getMeshAction() == MESH_HWMP_PATH_SELECTION)
            processHwmpFrame(packet, action);
        else
            dropManagementFrame(packet);
    }
    else if (header->getType() == ST_BEACON || header->getType() == ST_PROBERESPONSE)
        processBeacon(packet, header, header->getType() == ST_PROBERESPONSE);
    else if (header->getType() == ST_PROBEREQUEST)
        processProbeRequest(packet, header);
    else
        dropManagementFrame(packet);
}

void Ieee80211MgmtMesh::handleTimer(cMessage *message)
{
    if (message == beaconTimer) {
        if (!isBeaconPending()) {
            sendBeacon();
        }
        beaconNumber++;
        nextTbttTsf += beaconInterval;
        scheduleNextBeacon();
        return;
    }
    for (const auto& entry : peers) {
        if (entry.second.timer == message) {
            auto state = entry.second.state;
            event(entry.first, state == HOLDING ? HOLDING_EXPIRED
                    : state == CNF_RCVD ? CONFIRM_EXPIRED : RETRY_EXPIRED);
            return;
        }
    }
    throw cRuntimeError("Unknown mesh management timer");
}

void Ieee80211MgmtMesh::sendHwmp(const Ptr<Ieee80211HwmpFrame>& body, MacAddress receiver)
{
    Enter_Method("sendHwmp");
    if (isMeshManagementActive())
        sendBody(body,ST_ACTION,receiver);
}

void Ieee80211MgmtMesh::processHwmpFrame(Packet *packet, const Ptr<const Ieee80211MeshActionFrame>& header)
{
    if (!pathSelection || !mib->isMeshPeerEstablished(header->getTransmitterAddress()) || packet->getDataLength() == b(0)) {
        dropManagementFrame(packet);
        return;
    }
    auto body = packet->peekAtFront<Ieee80211HwmpFrame>(packet->getDataLength(),Chunk::PF_ALLOW_INCORRECT | Chunk::PF_ALLOW_INCOMPLETE);
    if (body->isIncorrect() || body->isIncomplete()) {
        dropManagementFrame(packet);
        return;
    }
    pathSelection->processHwmp(body,header->getTransmitterAddress(),header->getReceiverAddress());
    delete packet;
}

void Ieee80211MgmtMesh::processLinkMetric(Packet *packet, const Ptr<const Ieee80211MeshActionFrame>& header)
{
    if (!mib->isMeshPeerEstablished(header->getTransmitterAddress()) || header->getReceiverAddress() != mib->address
            || packet->getDataLength() == b(0)) {
        dropManagementFrame(packet);
        return;
    }
    auto report = packet->peekAtFront<Ieee80211MeshLinkMetricFrame>(packet->getDataLength(),
        Chunk::PF_ALLOW_INCORRECT | Chunk::PF_ALLOW_INCOMPLETE);
    if (report->isIncorrect() || report->isIncomplete()) {
        dropManagementFrame(packet);
        return;
    }
    if (report->getRequest()) {
        // IEEE Std 802.11-2024, 14.9.3: reply with this station's outgoing cost.
        auto reply = makeShared<Ieee80211MeshLinkMetricFrame>();
        reply->setMetric(linkMetric->getMetric(header->getTransmitterAddress()));
        reply->setChunkLength(B(7));
        sendBody(reply,ST_ACTION,header->getTransmitterAddress());
    }
    // Local policy permitted by14.9.3: remote reports do not replace local samples.
    delete packet;
}

bool Ieee80211MgmtMesh::matchesProfile(const Ieee80211MeshFrame& body) const
{
    // IEEE Std 802.11-2024, 14.3.3/14.3.4: formation and capability are not profile selectors.
    const auto& id = mib->getMeshId();
    if (!body.getMeshIdPresent() || body.getMeshIdArraySize() != id.size() || !body.getMeshConfigurationPresent())
        return false;
    for (size_t i = 0; i < id.size(); i++)
        if (body.getMeshId(i) != uint8_t(id[i]))
            return false;
    const auto& c = body.getMeshConfiguration();
    const auto& local = mib->getMeshConfiguration();
    return c.pathSelectionProtocolId == local.pathSelectionProtocolId
        && c.pathSelectionMetricId == local.pathSelectionMetricId && c.congestionControlId == local.congestionControlId
        && c.synchronizationId == local.synchronizationId && c.authenticationId == local.authenticationId;
}

bool Ieee80211MgmtMesh::compatibleRates(const Ieee80211RateSetState& rates) const
{
    const auto& local = mib->getLocalRateSet();
    // 14.3.4 requires an identical basic set, not merely a nonempty intersection.
    if (!rates.basic.known || !local.basic.known || rates.basic.legacyRates != local.basic.legacyRates)
        return false;
    for (auto rate : local.basic.legacyRates)
        if (rates.supported.legacyRates.count(rate) == 0)
            return false;
    return true;
}

bool Ieee80211MgmtMesh::decodePeerPhy(const Ieee80211MeshFrame& body, Ieee80211PeerPhyCapabilities& phy) const
{
    const auto& local = mib->getLocalMeshPhyCapabilities();
    if (body.getHtCapabilitiesPresent() != body.getHtOperationPresent()
            || body.getVhtCapabilitiesPresent() != body.getVhtOperationPresent()
            || body.getHtCapabilitiesPresent() != local.htPresent || body.getVhtCapabilitiesPresent() != local.vhtPresent)
        return false;
    if (!local.htPresent)
        return true;
    auto physical = dynamic_cast<const physicallayer::IIeee80211ChannelAccess *>(myIface->getSubmodule("radio"));
    if (!physical || !physical->hasContiguousAllocation())
        return false;
    try {
        phy.ht = makeHtCapabilities(body.getHtCapabilities());
        phy.htOperation = makeHtOperation(physical->getOperatingChannel()->getBand(), body.getHtOperation());
        if (phy.htOperation.primaryChannel != local.htOperation.primaryChannel
                || phy.htOperation.operatingChannelWidth != local.htOperation.operatingChannelWidth
                || phy.htOperation.secondaryChannelOffset != local.htOperation.secondaryChannelOffset
                || phy.htOperation.protectionMode != Ieee80211HtProtectionMode::NO_PROTECTION
                || phy.htOperation.basicMcsSupported != local.htOperation.basicMcsSupported
                || !supportsBasicHtMcsSet(phy.ht, local.htOperation)
                || (local.htOperation.operatingChannelWidth == MHz(40)
                    && (!phy.ht.supportedChannelWidths.count(MHz(40)) || body.getHtOperation().obssNonHtStasPresent)))
            return false;
        phy.htPresent = true;
        if (local.vhtPresent) {
            phy.vht = makeVhtCapabilities(body.getVhtCapabilities());
            const auto& wire = body.getVhtOperation();
            const auto& operation = local.vhtOperation;
            auto expected = makeVhtOperationElement(operation);
            if (wire.channelWidth != expected.channelWidth || wire.centerFrequencySegment0 != expected.centerFrequencySegment0
                    || wire.centerFrequencySegment1 != expected.centerFrequencySegment1 || wire.basicMcsMap != expected.basicMcsMap
                    || !supportsBasicVhtMcs(phy.vht, operation.basicMcsMap)
                    || (operation.width == MHz(160) && !phy.vht.width160)
                    || body.getHtOperation().channelCenterFrequencySegment2 != 0)
                return false;
            phy.vhtOperation = operation;
            phy.vhtPresent = true;
        }
        return true;
    }
    catch (const cRuntimeError&) {
        return false; // A malformed/unsupported advertisement never mutates committed state.
    }
}

Ieee80211RateSetState Ieee80211MgmtMesh::readPeeringRates(const Ieee80211MeshPeeringOpenFrame& body) const
{
    Ieee80211RateSetState rates;
    rates.supported.known = rates.operational.known = rates.basic.known = true;
    auto add = [&](uint8_t encoded) {
        auto value = encoded & 0x7F;
        if ((encoded & 0x80) && value >= 122) {
            const auto& local = mib->getLocalMeshPhyCapabilities();
            if (!((value == 127 && local.htPresent) || (value == 126 && local.vhtPresent)))
                rates.basic.known = false;
            return; // Table9-131 requirement selectors are not legacy rates.
        }
        if (value == 0) {
            rates.basic.known = false;
            return;
        }
        auto rate = Mbps(value * 0.5);
        rates.supported.legacyRates.insert(rate);
        rates.operational.legacyRates.insert(rate);
        if (encoded & 0x80)
            rates.basic.legacyRates.insert(rate);
    };
    for (size_t i = 0; i < body.getSupportedRatesArraySize(); i++)
        add(body.getSupportedRates(i));
    for (size_t i = 0; i < body.getExtendedSupportedRatesArraySize(); i++)
        add(body.getExtendedSupportedRates(i));
    return rates;
}

void Ieee80211MgmtMesh::processBeacon(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header, bool response)
{
    if (packet->getDataLength() == b(0)) {
        dropManagementFrame(packet);
        return;
    }
    const int flags = Chunk::PF_ALLOW_INCORRECT | Chunk::PF_ALLOW_INCOMPLETE;
    Ptr<const Ieee80211BeaconFrame> body = response
        ? staticPtrCast<const Ieee80211BeaconFrame>(packet->peekAtFront<Ieee80211ProbeResponseFrame>(packet->getDataLength(), flags))
        : packet->peekAtFront<Ieee80211BeaconFrame>(packet->getDataLength(), flags);
    if (body->isIncorrect() || body->isIncomplete()) {
        dropManagementFrame(packet);
        return;
    }
    auto peer = header->getTransmitterAddress();
    bool compatible = body->isCorrect() && body->isComplete() && matchesProfile(*body)
        && !(body->getCapabilityInformation() & (MESH_EPD_CAPABILITY | 3));
    auto rates = makeRateSet(body->getSupportedRates(), body->getExtendedSupportedRatesPresent(), body->getExtendedSupportedRates());
    compatible &= compatibleRates(rates);
    Ieee80211PeerPhyCapabilities phy;
    compatible &= decodePeerPhy(*body, phy);
    auto hasSelector = [&](const auto& element) {
        for (int i = 0; i < element.numRates; i++)
            if (element.basicRate[i] && element.rate[i] >= 61) {
                int selector = int(element.rate[i] * 2);
                const auto& local = mib->getLocalMeshPhyCapabilities();
                if (!((selector == 127 && local.htPresent) || (selector == 126 && local.vhtPresent)))
                    return true;
            }
        return false;
    };
    compatible &= !hasSelector(body->getSupportedRates())
        && (!body->getExtendedSupportedRatesPresent() || !hasSelector(body->getExtendedSupportedRates()));
    if (!compatible || peer.isMulticast() || peer.isUnspecified() || peer == mib->address) {
        dropManagementFrame(packet);
        return;
    }
    updateNeighborTiming(packet, body, peer);
    if (meshPowerSave && meshPowerSave->isEnabled() && mib->isMeshPeerEstablished(peer)) {
        auto tim = body->getTim();
        auto aid = getPeerAid(peer); // Assigned by the reporting peer, not our local AID.
        int byte = aid / 8 - (tim.bitmapControl & 0xfe);
        bool individual = body->getTimPresent() && aid && byte >= 0 && byte < tim.bitmapLength
            && (tim.partialVirtualBitmap[byte] & (1 << (aid % 8)));
        meshPowerSave->beaconReceived(peer,
            body->getTimPresent() && tim.dtimCount == 0 && (tim.bitmapControl & 1), individual,
            body->getMeshAwakeWindowPresent() ? SimTime(int64_t(body->getMeshAwakeWindow()) * 1024, SIMTIME_US) : SIMTIME_ZERO);
    }
    if (autoPeer && getPeerState(peer) == IDLE && (body->getMeshConfiguration().capability & 1))
        openPeer(peer);
    delete packet;
}

void Ieee80211MgmtMesh::processProbeRequest(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header)
{
    if (packet->getDataLength() > b(0)) {
        auto body = packet->peekAtFront<Ieee80211ProbeRequestFrame>(packet->getDataLength(),
                Chunk::PF_ALLOW_INCORRECT | Chunk::PF_ALLOW_INCOMPLETE);
        if (body->isCorrect() && body->isComplete() && body->getMeshIdPresent()) {
            bool match = body->getMeshIdArraySize() == 0;
            if (body->getMeshIdArraySize() == mib->getMeshId().size()) {
                match = true;
                for (size_t i = 0; i < body->getMeshIdArraySize(); i++)
                    match &= body->getMeshId(i) == uint8_t(mib->getMeshId()[i]);
            }
            if (match)
                sendBeacon(true, header->getTransmitterAddress());
        }
    }
    delete packet;
}

Ieee80211MgmtMesh::Peer& Ieee80211MgmtMesh::requirePeer(MacAddress address)
{
    auto& peer = peers[address];
    if (!peer.timer) {
        peer.timer = new cMessage("meshPeerTimer");
        peer.timer->setSchedulingPriority(1);
    }
    return peer;
}

void Ieee80211MgmtMesh::createInstance(Peer& peer)
{
    if (peer.localLinkId != 0)
        return;
    // Monotonic allocation avoids immediate reuse of a recently retired Link ID.
    for (int attempts = 0; attempts < 65535; attempts++) {
        auto candidate = nextLinkId++;
        if (candidate == 0)
            continue;
        bool used = false;
        for (const auto& entry : peers)
            used |= entry.second.localLinkId == candidate;
        if (!used) {
            peer.localLinkId = candidate;
            peer.retries = 0;
            return;
        }
    }
    throw cRuntimeError("No unused mesh Link ID");
}

uint16_t Ieee80211MgmtMesh::allocateAid() const
{
    for (uint16_t aid = 1; aid <= 2007; aid++) {
        bool used = false;
        for (const auto& entry : peers)
            used |= entry.second.localAid == aid;
        if (!used)
            return aid;
    }
    throw cRuntimeError("No unused mesh AID");
}

void Ieee80211MgmtMesh::arm(Peer& peer, simtime_t timeout)
{
    cancelEvent(peer.timer);
    scheduleAfter(timeout, peer.timer);
}

void Ieee80211MgmtMesh::setState(MacAddress address, Peer& peer, PeerState state)
{
    bool wasEstablished = peer.state == ESTAB;
    if (peer.state != state && state == ESTAB)
        linkMetric->resetPeer(address);
    peer.state = state;
    if (state == ESTAB && !wasEstablished)
        mib->establishMeshPeer(address, peer.rates, peer.phy);
    if (state == IDLE) {
        cancelEvent(peer.timer);
        peer.localLinkId = peer.localAid = peer.peerAid = 0;
        peer.peerLinkId.reset();
        peer.rates = {};
        peer.phy = {};
        peer.retries = 0;
    }
    if (wasEstablished && state != ESTAB)
        retirePeer(address);
    Ieee80211Mib::MeshPeerChange details(address);
    emit(meshPeeringStateChangedSignal, long(state), &details);
    if (meshPowerSave && meshPowerSave->isEnabled())
        meshPowerSave->managementTimingChanged();
}

void Ieee80211MgmtMesh::retirePeer(MacAddress address)
{
    bool changed = mib->withdrawMeshPeer(address);
    if (meshPowerSave)
        meshPowerSave->peerRemoved(address);
    linkMetric->resetPeer(address);
    resetPeerAssociationContext(address);
    if (changed)
        mib->publishMeshPeerRemoval(address);
}

void Ieee80211MgmtMesh::openPeer(MacAddress address)
{
    Enter_Method("openPeer");
    if (!isMeshManagementActive() || address.isUnspecified() || address.isMulticast() || address == mib->address)
        return;
    auto& peer = requirePeer(address);
    if (peer.state == IDLE)
        event(address, ACTIVE_OPEN);
}

void Ieee80211MgmtMesh::closePeer(MacAddress address)
{
    Enter_Method("closePeer");
    auto it = peers.find(address);
    if (isMeshManagementActive() && it != peers.end())
        event(address, CANCEL, RC_MESH_PEERING_CANCELLED);
}

void Ieee80211MgmtMesh::event(MacAddress address, Event stimulus, uint16_t reason)
{
    // IEEE Std 802.11-2024, 14.5.6--14.5.11/Table 14-2. One timer per instance:
    // the state identifies whether its pending event is retry, confirm or holding.
    auto& peer = requirePeer(address);
    if (peer.state == HOLDING) {
        if (stimulus == HOLDING_EXPIRED || stimulus == CLOSE_ACCEPT)
            setState(address, peer, IDLE);
        else if (stimulus == OPEN_ACCEPT || stimulus == CONFIRM_ACCEPT
                || stimulus == OPEN_REJECT || stimulus == CONFIRM_REJECT)
            sendPeering(address, MESH_PEERING_CLOSE, peer.closeReason);
        return;
    }
    if (peer.state == IDLE) {
        if (stimulus == ACTIVE_OPEN || stimulus == OPEN_ACCEPT || stimulus == REQUEST_REJECT) {
            createInstance(peer);
            if (stimulus == REQUEST_REJECT) {
                sendPeering(address, MESH_PEERING_CLOSE, reason);
                setState(address, peer, IDLE);
                return;
            }
            sendPeering(address, MESH_PEERING_OPEN);
            if (stimulus == OPEN_ACCEPT)
                sendPeering(address, MESH_PEERING_CONFIRM);
            arm(peer, retryTimeout);
            setState(address, peer, stimulus == ACTIVE_OPEN ? OPN_SNT : OPN_RCVD);
        }
        return;
    }
    bool terminal = stimulus == CANCEL || stimulus == CLOSE_ACCEPT || stimulus == OPEN_REJECT
        || stimulus == CONFIRM_REJECT || stimulus == CONFIRM_EXPIRED;
    if (stimulus == RETRY_EXPIRED && peer.retries >= maxRetries) {
        terminal = true;
        reason = RC_MESH_MAX_RETRIES;
    }
    if (terminal) {
        cancelEvent(peer.timer);
        if (stimulus == CANCEL)
            reason = RC_MESH_PEERING_CANCELLED;
        if (stimulus == CLOSE_ACCEPT)
            reason = RC_MESH_CLOSE_RCVD;
        if (stimulus == CONFIRM_EXPIRED)
            reason = RC_MESH_CONFIRM_TIMEOUT;
        peer.closeReason = reason;
        sendPeering(address, MESH_PEERING_CLOSE, reason);
        arm(peer, holdingTimeout);
        setState(address, peer, HOLDING);
    }
    else if (stimulus == OPEN_ACCEPT) {
        sendPeering(address, MESH_PEERING_CONFIRM);
        if (peer.state == CNF_RCVD) {
            cancelEvent(peer.timer);
            setState(address, peer, ESTAB);
        }
        else if (peer.state == OPN_SNT)
            setState(address, peer, OPN_RCVD);
    }
    else if (stimulus == CONFIRM_ACCEPT) {
        if (peer.state == OPN_RCVD) {
            cancelEvent(peer.timer);
            setState(address, peer, ESTAB);
        }
        else if (peer.state == OPN_SNT) {
            arm(peer, confirmTimeout);
            setState(address, peer, CNF_RCVD);
        }
    }
    else if (stimulus == RETRY_EXPIRED && (peer.state == OPN_SNT || peer.state == OPN_RCVD)) {
        sendPeering(address, MESH_PEERING_OPEN);
        peer.retries++;
        arm(peer, retryTimeout);
    }
}

void Ieee80211MgmtMesh::processPeering(Packet *packet, const Ptr<const Ieee80211MeshActionFrame>& header)
{
    auto address = header->getTransmitterAddress();
    if (address.isMulticast() || address.isUnspecified() || header->getReceiverAddress() != mib->address
            || packet->getDataLength() == b(0)) {
        dropManagementFrame(packet);
        return;
    }
    int flags = Chunk::PF_ALLOW_INCORRECT | Chunk::PF_ALLOW_INCOMPLETE;
    auto action = header->getMeshAction();
    Ptr<const Ieee80211MeshFrame> body;
    if (action == MESH_PEERING_OPEN)
        body = packet->peekAtFront<Ieee80211MeshPeeringOpenFrame>(packet->getDataLength(), flags);
    else if (action == MESH_PEERING_CONFIRM)
        body = packet->peekAtFront<Ieee80211MeshPeeringConfirmFrame>(packet->getDataLength(), flags);
    else if (action == MESH_PEERING_CLOSE)
        body = packet->peekAtFront<Ieee80211MeshPeeringCloseFrame>(packet->getDataLength(), flags);
    if (!body || body->isIncorrect() || body->isIncomplete() || !body->getMeshPeeringPresent()
            || body->getMeshPeering().protocolId != 0 || body->getMeshPeering().chosenPmkPresent) {
        dropManagementFrame(packet);
        return;
    }
    // IEEE Std 802.11-2024, 14.4.5: an unsecured peer silently discards
    // AMPE or MIC elements even when the advertised peering protocol is MPM.
    for (size_t i = 0; i + 2 <= body->getOpaqueMeshElementsArraySize(); ) {
        auto id = body->getOpaqueMeshElements(i);
        size_t length = body->getOpaqueMeshElements(i + 1);
        if (id == 139 || id == 140 || length > body->getOpaqueMeshElementsArraySize() - i - 2) {
            dropManagementFrame(packet);
            return;
        }
        i += 2 + length;
    }
    if (auto confirm = dynamicPtrCast<const Ieee80211MeshPeeringConfirmFrame>(body)) {
        if (confirm->getAssociationId() < 1 || confirm->getAssociationId() > 2007) {
            dropManagementFrame(packet);
            return;
        }
    }
    const auto& peering = body->getMeshPeering();
    auto it = peers.find(address);
    if (action != MESH_PEERING_OPEN && (it == peers.end() || !peering.peerLinkIdPresent
            || peering.peerLinkId != it->second.localLinkId)) {
        dropManagementFrame(packet);
        return;
    }
    if (it != peers.end() && it->second.peerLinkId && *it->second.peerLinkId != peering.localLinkId) {
        dropManagementFrame(packet);
        return;
    }
    if (action == MESH_PEERING_CLOSE) {
        bool sameId = body->getMeshIdPresent() && body->getMeshIdArraySize() == mib->getMeshId().size();
        for (size_t i = 0; sameId && i < body->getMeshIdArraySize(); i++)
            sameId &= body->getMeshId(i) == uint8_t(mib->getMeshId()[i]);
        if (sameId)
            event(address, CLOSE_ACCEPT, RC_MESH_CLOSE_RCVD);
        delete packet;
        return;
    }
    auto open = staticPtrCast<const Ieee80211MeshPeeringOpenFrame>(body);
    auto rates = readPeeringRates(*open);
    bool compatible = matchesProfile(*body) && compatibleRates(rates)
        && !(open->getCapabilityInformation() & MESH_EPD_CAPABILITY);
    Ieee80211PeerPhyCapabilities phy;
    compatible &= decodePeerPhy(*body, phy);
    std::optional<MeshLinkTiming> candidateTiming;
    if (compatible && mib->getMeshPhyProfile() != "ofdm") {
        try {
            candidateTiming = linkTiming->previewMeshLinkTiming(address, rates, phy);
        }
        catch (const cRuntimeError& error) {
            EV_WARN << "Rejecting incompatible fixed mesh PHY: " << error.what() << endl;
            dropManagementFrame(packet);
            return;
        }
    }
    if (compatible && mib->getMeshPhyProfile() == "ofdm" && !linkMetric->supportsPeerRates(address,rates)) {
        // Supported-profile policy: retain a committed relationship unchanged
        // rather than publish rates that invalidate its configured fixed PHY.
        dropManagementFrame(packet);
        return;
    }
    auto& peer = requirePeer(address);
    if (!compatible) {
        if (!peer.peerLinkId)
            peer.peerLinkId = peering.localLinkId;
        event(address, action == MESH_PEERING_CONFIRM ? CONFIRM_REJECT
                : peer.state == IDLE ? REQUEST_REJECT : OPEN_REJECT,
                action == MESH_PEERING_CONFIRM ? RC_MESH_INCONSISTENT_PARAMETERS : RC_MESH_CONFIGURATION_POLICY_VIOLATION);
        delete packet;
        return;
    }
    if (peer.state == HOLDING) {
        event(address, action == MESH_PEERING_OPEN ? OPEN_ACCEPT : CONFIRM_ACCEPT);
        delete packet;
        return;
    }
    size_t active = 0;
    for (const auto& entry : peers)
        active += entry.second.localAid != 0;
    if (action == MESH_PEERING_OPEN && peer.localAid == 0 && active >= size_t(maxPeerings)) {
        peer.peerLinkId = peering.localLinkId;
        event(address, peer.state == IDLE ? REQUEST_REJECT : OPEN_REJECT, RC_MESH_MAX_PEERS);
        delete packet;
        return;
    }
    peer.peerLinkId = peering.localLinkId;
    bool changedEstablishedRates = peer.state == ESTAB && (peer.rates != rates || peer.phy != phy);
    if (candidateTiming && changedEstablishedRates) {
        auto current = linkTiming->getMeshLinkTiming(address);
        if (current.dataMode != candidateTiming->dataMode || current.ackMode != candidateTiming->ackMode)
            linkMetric->resetPeer(address);
        else if (peer.phy.htPresent != phy.htPresent || peer.phy.vhtPresent != phy.vhtPresent
                || peer.phy.ht.maxAmsduLength != phy.ht.maxAmsduLength || peer.phy.vht.maxMpduLength != phy.vht.maxMpduLength)
            linkMetric->resetAggregation(address);
    }
    peer.rates = rates;
    peer.phy = phy;
    if (changedEstablishedRates)
        mib->establishMeshPeer(address,rates,phy);
    if (action == MESH_PEERING_OPEN && peer.localAid == 0)
        peer.localAid = allocateAid();
    if (action == MESH_PEERING_CONFIRM) {
        auto confirm = staticPtrCast<const Ieee80211MeshPeeringConfirmFrame>(body);
        peer.peerAid = confirm->getAssociationId();
    }
    event(address, action == MESH_PEERING_OPEN ? OPEN_ACCEPT : CONFIRM_ACCEPT);
    delete packet;
}

void Ieee80211MgmtMesh::setMeshElements(const Ptr<Ieee80211MeshFrame>& body) const
{
    body->setMeshIdPresent(true);
    body->setMeshIdArraySize(mib->getMeshId().size());
    for (size_t i = 0; i < mib->getMeshId().size(); i++)
        body->setMeshId(i, mib->getMeshId()[i]);
    body->setMeshConfigurationPresent(true);
    auto configuration = mib->getMeshConfiguration();
    size_t count = 0;
    for (const auto& entry : peers)
        count += entry.second.state == ESTAB;
    configuration.formationInfo = std::min(size_t(63), count) << 1;
    body->setMeshConfiguration(configuration);
    if (dynamicPtrCast<const Ieee80211MeshPeeringCloseFrame>(body))
        return;
    const auto& phy = mib->getLocalMeshPhyCapabilities();
    if (phy.htPresent) {
        body->setHtCapabilitiesPresent(true);
        body->setHtCapabilities(makeHtCapabilitiesElement(phy.ht));
        body->setHtOperationPresent(true);
        auto radio = dynamic_cast<const physicallayer::IIeee80211ChannelAccess *>(myIface->getSubmodule("radio"));
        body->setHtOperation(makeHtOperationElement(radio->getOperatingChannel()->getBand(), phy.htOperation));
    }
    if (phy.vhtPresent) {
        body->setVhtCapabilitiesPresent(true);
        body->setVhtCapabilities(makeVhtCapabilitiesElement(phy.vht));
        body->setVhtOperationPresent(true);
        body->setVhtOperation(makeVhtOperationElement(phy.vhtOperation));
    }
}

void Ieee80211MgmtMesh::sendBody(const Ptr<Ieee80211MeshFrame>& body, Ieee80211FrameType subtype, MacAddress destination)
{
    body->setChunkLength(body->getChunkLength() + B((body->getHtCapabilitiesPresent() ? 28 : 0)
        + (body->getHtOperationPresent() ? 24 : 0) + (body->getVhtCapabilitiesPresent() ? 14 : 0)
        + (body->getVhtOperationPresent() ? 7 : 0)));
    auto packet = new Packet(body->getClassName(), body);
    packet->addTag<MacAddressReq>()->setDestAddress(destination);
    packet->addTag<Ieee80211SubtypeReq>()->setSubtype(subtype);
    if (subtype == ST_BEACON)
        pendingBeaconTreeId = packet->getTreeId();
    sendDown(packet);
}

void Ieee80211MgmtMesh::sendPeering(MacAddress address, uint8_t action, uint16_t reason)
{
    auto& peer = peers.at(address);
    Ptr<Ieee80211MeshFrame> frame;
    B length(0);
    if (action == MESH_PEERING_CLOSE) {
        frame = makeShared<Ieee80211MeshPeeringCloseFrame>();
        setMeshElements(frame);
        frame->setMeshConfigurationPresent(false);
        length = B(2 + mib->getMeshId().size());
    }
    else {
        Ptr<Ieee80211MeshPeeringOpenFrame> open;
        if (action == MESH_PEERING_CONFIRM) {
            auto confirm = makeShared<Ieee80211MeshPeeringConfirmFrame>();
            confirm->setAssociationId(peer.localAid);
            open = confirm;
            length += B(2);
        }
        else
            open = makeShared<Ieee80211MeshPeeringOpenFrame>();
        open->setCapabilityInformation(1 << 9); // QoS, EPD disabled.
        open->setSupportedRatesArraySize(supportedRates.numRates);
        for (int i = 0; i < supportedRates.numRates; i++)
            open->setSupportedRates(i, uint8_t(supportedRates.rate[i] * 2) | (supportedRates.basicRate[i] ? 0x80 : 0));
        open->setExtendedSupportedRatesArraySize(extendedSupportedRates.numRates);
        for (int i = 0; i < extendedSupportedRates.numRates; i++)
            open->setExtendedSupportedRates(i, uint8_t(extendedSupportedRates.rate[i] * 2)
                    | (extendedSupportedRates.basicRate[i] ? 0x80 : 0));
        setMeshElements(open);
        length += B(2 + 2 + supportedRates.numRates + 2 + mib->getMeshId().size() + 9);
        if (extendedSupportedRates.numRates)
            length += B(2 + extendedSupportedRates.numRates);
        frame = open;
    }
    MeshPeering peering;
    peering.localLinkId = peer.localLinkId;
    peering.peerLinkIdPresent = action != MESH_PEERING_OPEN && peer.peerLinkId.has_value();
    peering.peerLinkId = peer.peerLinkId.value_or(0);
    peering.reasonCodePresent = action == MESH_PEERING_CLOSE;
    peering.reasonCode = reason;
    frame->setMeshPeeringPresent(true);
    frame->setMeshPeering(peering);
    frame->setChunkLength(length + B(6 + 2 * peering.peerLinkIdPresent + 2 * peering.reasonCodePresent));
    sendBody(frame, ST_ACTION, address);
}

void Ieee80211MgmtMesh::sendBeacon(bool response, MacAddress destination)
{
    Ptr<Ieee80211BeaconFrame> frame = response
        ? staticPtrCast<Ieee80211BeaconFrame>(makeShared<Ieee80211ProbeResponseFrame>()) : makeShared<Ieee80211BeaconFrame>();
    frame->setBeaconInterval(beaconInterval);
    frame->setCapabilityInformation(1 << 9);
    setSupportedRateElements(frame);
    setMeshElements(frame);
    B length = B(12 + 2) + getSupportedRateElementsLength(frame) + B(2 + mib->getMeshId().size() + 9);
    if (!response) {
        MeshTim tim;
        tim.dtimPeriod = dtimPeriod;
        tim.dtimCount = (dtimPeriod - beaconNumber % dtimPeriod) % dtimPeriod;
        frame->setTimPresent(true);
        frame->setTim(tim);
        length += B(6);
        if (tim.dtimCount == 0) {
            frame->setMeshAwakeWindowPresent(true);
            frame->setMeshAwakeWindow(awakeWindow);
            length += B(4);
        }
    }
    frame->setChunkLength(length);
    sendBody(frame, response ? ST_PROBERESPONSE : ST_BEACON, destination);
}

simtime_t Ieee80211MgmtMesh::tsfAt(simtime_t time) const
{
    // A slowdown pauses counting; it never steps the TSF backwards.
    auto currentPause = std::max(SIMTIME_ZERO, std::min(lastSlowingDuration, time - lastSlowingStart));
    auto value = time - tsfOrigin - (totalSlowing - lastSlowingDuration) - currentPause;
    return std::max(SIMTIME_ZERO, value);
}

uint64_t Ieee80211MgmtMesh::getTsfAt(simtime_t time) const
{
    return tsfAt(time).inUnit(SIMTIME_US);
}

void Ieee80211MgmtMesh::scheduleNextBeacon()
{
    cancelEvent(beaconTimer);
    auto pauseRemaining = std::max(SIMTIME_ZERO, lastSlowingStart + lastSlowingDuration - simTime());
    auto localNow = tsfAt(simTime());
    scheduleAt(simTime() + pauseRemaining + std::max(SIMTIME_ZERO, nextTbttTsf - localNow), beaconTimer);
    if (meshPowerSave && meshPowerSave->isEnabled())
        meshPowerSave->managementTimingChanged();
}

void Ieee80211MgmtMesh::applyClockSlowing()
{
    // IEEE Std 802.11-2024, 14.14.2.2.3: retain residual drift beyond this period's budget.
    auto adjustment = std::min(maxClockDrift, beaconInterval * 0.0004);
    maxClockDrift -= adjustment;
    lastSlowingStart = simTime();
    lastSlowingDuration = adjustment;
    totalSlowing += adjustment;
    scheduleNextBeacon();
}

void Ieee80211MgmtMesh::prepareManagementTransmission(Packet *packet, simtime_t ifs, const IIeee80211Mode *mode)
{
    Enter_Method("prepareManagementTransmission");
    if (!isMeshManagementActive())
        return;
    auto header = packet->peekAtFront<Ieee80211MacHeader>();
    if (header->getType() != ST_BEACON && header->getType() != ST_PROBERESPONSE)
        return;
    auto ofdm = dynamic_cast<const Ieee80211OfdmMode *>(mode);
    if (!ofdm)
        throw cRuntimeError("Mesh timestamp reference requires the OFDM reference PHY");
    if (header->getType() == ST_BEACON)
        applyClockSlowing();
    auto fixed = packet->removeAtFront<Ieee80211MgmtHeader>();
    auto body = packet->removeAtFront<Ieee80211BeaconFrame>();
    body->setHasTimestamp(true);
    body->setTimestamp(getTsfAt(simTime() + ifs + ofdm->getDataSymbolStart(fixed->getChunkLength())));
    packet->insertAtFront(body);
    packet->insertAtFront(fixed);
}

void Ieee80211MgmtMesh::prepareManagementContent(Packet *packet)
{
    Enter_Method("prepareManagementContent");
    if (!isMeshManagementActive() || !meshPowerSave || !meshPowerSave->isEnabled()) return;
    auto header = packet->peekAtFront<Ieee80211MacHeader>();
    if (header->getType() != ST_BEACON && header->getType() != ST_PROBERESPONSE) return;
    // Inventory is a query over complete retained originals. Take its snapshot
    // before temporarily removing this original's header/body for mutation.
    auto original = packet->peekDataAt<Ieee80211BeaconFrame>(header->getChunkLength());
    auto tim = meshPowerSave->getTim(original->getTim().dtimCount, original->getTim().dtimPeriod, {packet->getId()});
    auto fixed = packet->removeAtFront<Ieee80211MgmtHeader>();
    auto body = packet->removeAtFront<Ieee80211BeaconFrame>();
    auto configuration = body->getMeshConfiguration();
    configuration.capability = (configuration.capability & ~64) | (mib->getMeshGroupPowerSaveLevel() ? 64 : 0);
    body->setMeshConfiguration(configuration);
    if (fixed->getType() == ST_BEACON) {
        auto oldTim = body->getTim();
        auto oldLength = B(5 + oldTim.bitmapLength + (body->getMeshAwakeWindowPresent() ? 4 : 0));
        body->setTim(tim);
        bool window = meshPowerSave->needsAwakeWindow(tim);
        body->setMeshAwakeWindowPresent(window);
        body->setMeshAwakeWindow(window ? awakeWindow : 0);
        body->setChunkLength(body->getChunkLength() - oldLength + B(5 + tim.bitmapLength + (window ? 4 : 0)));
    }
    else {
        auto oldLength = body->getMeshAwakeWindowPresent() ? B(4) : B(0);
        body->setMeshAwakeWindowPresent(true);
        body->setMeshAwakeWindow(awakeWindow);
        body->setChunkLength(body->getChunkLength() - oldLength + B(4));
    }
    packet->insertAtFront(body);
    packet->insertAtFront(fixed);
}

void Ieee80211MgmtMesh::managementTransmissionComplete(const Packet *packet)
{
    Enter_Method("managementTransmissionComplete");
    if (!isMeshManagementActive())
        return;
    if (packet->peekAtFront<Ieee80211MacHeader>()->getType() == ST_BEACON) {
        if (packet->getTreeId() == pendingBeaconTreeId)
            pendingBeaconTreeId = -1;
        auto beacon = packet->peekDataAt<Ieee80211BeaconFrame>(B(24));
        if (meshPowerSave && meshPowerSave->isEnabled() && beacon->getTim().dtimCount == 0)
            meshPowerSave->dtimTransmissionComplete();
        emit(meshBeaconTransmissionEndSignal, simTime().dbl());
    }
    auto type = packet->peekAtFront<Ieee80211MacHeader>()->getType();
    if (meshPowerSave && meshPowerSave->isEnabled() && (type == ST_BEACON || type == ST_PROBERESPONSE)) {
        auto body = packet->peekDataAt<Ieee80211BeaconFrame>(B(24));
        // IEEE Std 802.11-2024, 14.15.6: the advertised AW starts at actual TX end.
        meshPowerSave->timingTransmissionComplete(body->getMeshAwakeWindowPresent()
            ? SimTime(int64_t(body->getMeshAwakeWindow()) * 1024, SIMTIME_US) : SIMTIME_ZERO);
    }
}

void Ieee80211MgmtMesh::updateNeighborTiming(Packet *packet, const Ptr<const Ieee80211BeaconFrame>& body, MacAddress address)
{
    // IEEE Std 802.11-2024, 14.14.2.2.2--3 and 11.1.3.9: reference the symbol,
    // not the later MAC delivery of a complete PPDU.
    auto time = packet->findTag<SignalTimeInd>();
    auto mode = packet->findTag<Ieee80211ModeInd>();
    auto ofdm = mode ? dynamic_cast<const Ieee80211OfdmMode *>(mode->getMode()) : nullptr;
    if (!time || !ofdm)
        return;
    auto& neighbor = neighbors[address];
    neighbor.beaconInterval = body->getBeaconInterval();
    if (body->getMeshConfiguration().capability & (1 << 5)) {
        neighbor.offsetValid = false;
        return;
    }
    auto reference = time->getStartTime() + ofdm->getDataSymbolStart(B(24));
    uint64_t difference = body->getTimestamp() - getTsfAt(reference);
    int64_t offset = difference <= uint64_t(INT64_MAX) ? int64_t(difference) : -int64_t(~difference) - 1;
    if (neighbor.offsetValid && neighbor.offset > offset) {
        uint64_t drift = uint64_t(neighbor.offset) - uint64_t(offset);
        if (drift <= uint64_t(SimTime::getMaxTime().inUnit(SIMTIME_US)))
            maxClockDrift = std::max(maxClockDrift, SimTime(drift, SIMTIME_US));
    }
    neighbor.offset = offset;
    neighbor.offsetValid = true;
}

Ieee80211MgmtMesh::PeerState Ieee80211MgmtMesh::getPeerState(MacAddress address) const
{
    auto it = peers.find(address);
    return it == peers.end() ? IDLE : it->second.state;
}

uint16_t Ieee80211MgmtMesh::getLocalLinkId(MacAddress address) const
{
    auto it = peers.find(address);
    return it == peers.end() ? 0 : it->second.localLinkId;
}

uint16_t Ieee80211MgmtMesh::getPeerLinkId(MacAddress address) const
{
    auto it = peers.find(address);
    return it == peers.end() ? 0 : it->second.peerLinkId.value_or(0);
}

uint16_t Ieee80211MgmtMesh::getLocalAid(MacAddress address) const
{
    auto it = peers.find(address);
    return it == peers.end() ? 0 : it->second.localAid;
}

uint16_t Ieee80211MgmtMesh::getPeerAid(MacAddress address) const
{
    auto it = peers.find(address);
    return it == peers.end() ? 0 : it->second.peerAid;
}

simtime_t Ieee80211MgmtMesh::getPeerDeadline(MacAddress address) const
{
    auto it = peers.find(address);
    return it == peers.end() || !it->second.timer->isScheduled() ? SIMTIME_ZERO : it->second.timer->getArrivalTime();
}

bool Ieee80211MgmtMesh::getNeighborOffset(MacAddress address, int64_t& offset) const
{
    auto it = neighbors.find(address);
    if (it == neighbors.end() || !it->second.offsetValid)
        return false;
    offset = it->second.offset;
    return true;
}

simtime_t Ieee80211MgmtMesh::getNextTbtt() const
{
    return beaconTimer && beaconTimer->isScheduled() ? beaconTimer->getArrivalTime() : SIMTIME_ZERO;
}

MeshWakeSchedule Ieee80211MgmtMesh::getMeshWakeSchedule() const
{
    MeshWakeSchedule result;
    result.nextTbtt = getNextTbtt();
    result.beaconInterval = beaconInterval;
    result.ordinal = beaconNumber;
    result.beaconPending = isBeaconPending();
    for (const auto& entry : peers)
        result.peeringPending |= entry.second.state != IDLE && entry.second.state != ESTAB;
    return result;
}

simtime_t Ieee80211MgmtMesh::getOwnTbtt(uint64_t ordinal) const
{
    auto target = nextTbttTsf;
    if (ordinal > beaconNumber)
        target += beaconInterval * (ordinal - beaconNumber);
    auto pause = std::max(SIMTIME_ZERO, lastSlowingStart + lastSlowingDuration - simTime());
    return simTime() + pause + std::max(SIMTIME_ZERO, target - tsfAt(simTime()));
}

std::optional<simtime_t> Ieee80211MgmtMesh::getPeerTbtt(MacAddress peer, simtime_t notBefore) const
{
    auto it = neighbors.find(peer);
    if (it == neighbors.end() || !it->second.offsetValid || it->second.beaconInterval <= 0)
        return {};
    auto maximum = SimTime::getMaxTime().inUnit(SIMTIME_US);
    if (it->second.offset > maximum || it->second.offset < -maximum)
        return {};
    auto offset = SimTime(it->second.offset, SIMTIME_US);
    auto local = tsfAt(notBefore);
    if ((offset > 0 && local > SimTime::getMaxTime() - offset) || (offset < 0 && local < -offset))
        return {};
    auto peerNow = local + offset;
    auto interval = it->second.beaconInterval;
    int64_t ordinal = peerNow.raw() / interval.raw() + (peerNow.raw() % interval.raw() != 0);
    if (ordinal > SimTime::getMaxTime().raw() / interval.raw())
        return {};
    auto target = SimTime::fromRaw(ordinal * interval.raw()) - offset;
    auto pause = std::max(SIMTIME_ZERO, lastSlowingStart + lastSlowingDuration - simTime());
    return simTime() + pause + std::max(SIMTIME_ZERO, target - tsfAt(simTime()));
}

std::string Ieee80211MgmtMesh::describePeers() const
{
    std::ostringstream result;
    for (const auto& entry : peers)
        result << entry.first << ": state=" << entry.second.state << " localLink=" << entry.second.localLinkId
            << " peerLink=" << entry.second.peerLinkId.value_or(0) << " AID=" << entry.second.localAid << "\n";
    return result.str();
}

void Ieee80211MgmtMesh::handleAuthenticationFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>&) { dropManagementFrame(packet); }
void Ieee80211MgmtMesh::handleDeauthenticationFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>&) { dropManagementFrame(packet); }
void Ieee80211MgmtMesh::handleAssociationRequestFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>&) { dropManagementFrame(packet); }
void Ieee80211MgmtMesh::handleAssociationResponseFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>&) { dropManagementFrame(packet); }
void Ieee80211MgmtMesh::handleReassociationRequestFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>&) { dropManagementFrame(packet); }
void Ieee80211MgmtMesh::handleReassociationResponseFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>&) { dropManagementFrame(packet); }
void Ieee80211MgmtMesh::handleDisassociationFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>&) { dropManagementFrame(packet); }
void Ieee80211MgmtMesh::handleBeaconFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header) { processBeacon(packet, header, false); }
void Ieee80211MgmtMesh::handleProbeRequestFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header) { processProbeRequest(packet, header); }
void Ieee80211MgmtMesh::handleProbeResponseFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header) { processBeacon(packet, header, true); }

} // namespace inet::ieee80211
