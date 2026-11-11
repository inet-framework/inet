// SPDX-License-Identifier: LGPL-3.0-or-later

#include "inet/linklayer/ieee80211/mac/Ieee80211MeshMac.h"
#include "inet/linklayer/ieee80211/mac/contract/Ieee80211MeshAggregationTag_m.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211SubtypeTag_m.h"
#include "inet/linklayer/ieee80211/mac/contract/IRtsPolicy.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MeshFrame_m.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtFrame_m.h"
#include "inet/linklayer/ieee80211/mesh/contract/IMeshForwarding.h"
#include "inet/linklayer/common/UserPriorityTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/ieee80211/llc/IIeee80211Llc.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/IIeee80211ChannelAccess.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Tag_m.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Radio.h"

namespace inet {
namespace ieee80211 {

Define_Module(Ieee80211MeshMac);

Ieee80211MeshMac::~Ieee80211MeshMac()
{
    cancelAndDelete(radioReview);
}

void Ieee80211MeshMac::initialize(int stage)
{
    Ieee80211Mac::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        meshManagement.reference(this, "meshManagementModule", true);
        meshPowerSave.reference(this, "meshPowerSaveModule", false);
        radioReview = new cMessage("meshRadioReview");
        WATCH(powerManaged);
        WATCH(awakeIntent);
    }
    if (stage == INITSTAGE_LINK_LAYER) {
        auto physical = dynamic_cast<const physicallayer::IIeee80211ChannelAccess *>(radio.get());
        mib->configureMeshPhyCapabilities(par("phyProfile").stdstringValue(), modeSet,
            physical ? physical->getOperatingChannel() : nullptr,
            std::min(radio->getAntenna()->getNumAntennas(), modeSet->getMaximumNumberOfSpatialStreams()));
    }
    if (stage == INITSTAGE_LAST && (!mib->qos || mib->mode != Ieee80211Mib::MESH))
        throw cRuntimeError("Mesh requires a QoS mesh interface");
    if (stage == INITSTAGE_LAST) {
        check_and_cast<cModule *>(radio.get())->subscribe(physicallayer::Ieee80211Radio::primaryCcaChangedSignal, this);
        requestRadioReview(); // Public radio timing is consumed after all initialization stages.
        auto radioModule = check_and_cast<cModule *>(radio.get());
        bool high = mib->getMeshPhyProfile() != "ofdm";
        if (strcmp(par("modeSet").stringValue(), high ? "ac-complete" : "a") != 0
                || (radioModule->hasPar("bandName") && strcmp(radioModule->par("bandName").stringValue(), "5.9 GHz") == 0))
            throw cRuntimeError("Mesh requires its selected5GHz OFDM or HT/VHT catalog outside EPD-required channels");
        auto service = hcf->getSubmodule("originatorMacDataService");
        auto rts = check_and_cast<IRtsPolicy *>(hcf->getSubmodule("rtsPolicy"));
        if (hcf->getSubmodule("rateControl") || service->getSubmodule("fragmentationPolicy")
                || (!high && (service->getSubmodule("msduAggregationPolicy") || service->getSubmodule("mpduAggregationPolicy")
                    || hcf->par("isBlockAckSupported").boolValue()))
                || rts->getRtsThreshold() <= (high ? 65535 : modeSet->getMode(0)->getMpduMaxLength()))
            throw cRuntimeError("Mesh requires fixed rates, no fragmentation, HT/VHT for aggregation/Block Ack, and RTS disabled");
        if (service->getSubmodule("mpduAggregationPolicy") && (!hcf->par("isBlockAckSupported").boolValue()
                || strcmp(hcf->par("blockAckVariant").stringValue(), "compressed") != 0))
            throw cRuntimeError("Mesh A-MPDU requires immediate Compressed Block Ack");
    }
}

void Ieee80211MeshMac::handleMgmtPacket(Packet *packet)
{
    auto subtype = packet->getTag<Ieee80211SubtypeReq>()->getSubtype();
    Ptr<Ieee80211MgmtHeader> header;
    if (subtype == ST_ACTION) {
        auto action = makeShared<Ieee80211MeshActionFrame>();
        auto body = packet->peekAtFront<Chunk>();
        if (dynamicPtrCast<const Ieee80211MeshPeeringConfirmFrame>(body)) {
            action->setCategory(MESH_SELF_PROTECTED_ACTION);
            action->setMeshAction(MESH_PEERING_CONFIRM);
        }
        else if (dynamicPtrCast<const Ieee80211MeshPeeringOpenFrame>(body)) {
            action->setCategory(MESH_SELF_PROTECTED_ACTION);
            action->setMeshAction(MESH_PEERING_OPEN);
        }
        else if (dynamicPtrCast<const Ieee80211MeshPeeringCloseFrame>(body)) {
            action->setCategory(MESH_SELF_PROTECTED_ACTION);
            action->setMeshAction(MESH_PEERING_CLOSE);
        }
        else {
            action->setCategory(MESH_ACTION);
            action->setMeshAction(dynamicPtrCast<const Ieee80211MeshLinkMetricFrame>(body)
                    ? MESH_LINK_METRIC_REPORT : MESH_HWMP_PATH_SELECTION);
        }
        header = action;
    }
    else {
        header = makeShared<Ieee80211MgmtHeader>();
        header->setType(static_cast<Ieee80211FrameType>(subtype));
    }
    header->setReceiverAddress(packet->getTag<MacAddressReq>()->getDestAddress());
    // IEEE Std 802.11-2024, 9.3.3.1 c1/c4v: Probe Request identifies
    // the intended recipient (or wildcard); other mesh management uses the TA.
    header->setAddress3(subtype == ST_PROBEREQUEST ? header->getReceiverAddress() : mib->address);
    packet->insertAtFront(header);
    packet->insertAtBack(makeShared<Ieee80211MacTrailer>());
    requestRadioReview();
    processUpperFrame(packet, header);
}

void Ieee80211MeshMac::unsupportedData(Packet *packet)
{
    PacketDropDetails details;
    details.setReason(OTHER_PACKET_DROP);
    emit(packetDroppedSignal, packet, &details);
    delete packet;
}

void Ieee80211MeshMac::handleUpperPacket(Packet *packet)
{
    if (auto forwarding = dynamic_cast<IMeshForwarding *>(ds.getNullable()))
        forwarding->processSourceMsdu(packet);
    else
        unsupportedData(packet);
}

void Ieee80211MeshMac::sendMeshData(Packet *packet)
{
    Enter_Method("sendMeshData");
    take(packet); // The direct handoff consumes ownership, including refusal.
    requestRadioReview();
    auto header = packet->peekAtFront<Ieee80211DataHeader>();
    if (mib->getLocalMeshPhyCapabilities().htPresent
            && !header->getAMsduPresent()
            && packet->getDataLength() - header->getChunkLength() - B(10) > B(2304)) {
        unsupportedData(packet);
        return;
    }
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ieee80211Mac);
    processUpperFrame(packet, header);
}

void Ieee80211MeshMac::deliverMeshMsdu(Packet *packet, MacAddress source, MacAddress destination, int tid)
{
    Enter_Method("deliverMeshMsdu");
    take(packet);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(llc->getProtocol());
    auto addresses = packet->addTagIfAbsent<MacAddressInd>();
    addresses->setSrcAddress(source);
    addresses->setDestAddress(destination);
    packet->addTagIfAbsent<UserPriorityInd>()->setUserPriority(tid);
    packet->addTagIfAbsent<InterfaceInd>()->setInterfaceId(networkInterface->getInterfaceId());
    sendUp(packet);
}

void Ieee80211MeshMac::handleLowerMessage(cMessage *message)
{
    // Supply the known interface role before LayeredProtocolBase emits the
    // received-packet observation. Address roles and QoS bit 8 alone are ambiguous.
    if (auto packet = dynamic_cast<Packet *>(message)) {
        auto mode = packet->findTag<physicallayer::Ieee80211ModeInd>();
        if (mode && mode->getTxVector().ampdu) {
            hcf->receiveAmpdu(packet);
            return;
        }
        if (!rx->prepareReceivedFrame(packet)) {
            Ieee80211Mac::handleLowerPacket(packet);
            return;
        }
        auto first = packet->peekAtFront<Chunk>();
        auto data = dynamicPtrCast<const Ieee80211DataHeader>(first);
        // Inspect only fixed Frame Control bits for a raw input. Parsing a
        // nondata Action header here could consume its variable body and FCS.
        if (!data && packet->getDataLength() >= B(24)
                && (packet->peekAtFront<BytesChunk>(B(1))->getBytes()[0] & 0x0C) == 0x08)
            data = packet->peekAtFront<Ieee80211MeshDataHeader>(b(-1),
                    Chunk::PF_ALLOW_NULLPTR | Chunk::PF_ALLOW_INCORRECT | Chunk::PF_ALLOW_INCOMPLETE);
        if (data && data->isCorrect() && data->isComplete() && !dynamicPtrCast<const Ieee80211MeshDataHeader>(first)) {
            auto mesh = makeShared<Ieee80211MeshDataHeader>();
            mesh->Ieee80211DataHeader::operator=(*data);
            // The cached received bytes remain identical, including reserved bits,
            // so the subsequent normal FCS check still sees the on-air representation.
            packet->popAtFront(data->getChunkLength());
            packet->insertAtFront(mesh);
        }
    }
    Ieee80211Mac::handleLowerMessage(message);
}

void Ieee80211MeshMac::sendUpFrame(Packet *packet)
{
    Enter_Method("sendUpFrame");
    take(packet);
    auto header = packet->peekAtFront<Ieee80211MacHeader>();
    if (header->getReceiverAddress().isMulticast() && meshPowerSave && meshPowerSave->isEnabled())
        if (auto addresses = dynamicPtrCast<const Ieee80211TwoAddressHeader>(header))
            meshPowerSave->groupFrameReceived(addresses->getTransmitterAddress(), header->getMoreData());
    if (header->getFrameType() != 0) {
        auto mesh = dynamicPtrCast<const Ieee80211MeshDataHeader>(header);
        if (header->getType() == ST_QOS_NULL && (!mesh || !isDataReceptionSupported(mesh)
                || packet->getDataLength() != mesh->getChunkLength() + B(4)
                || !mesh->getFromDS() || mesh->getToDS() == mesh->getReceiverAddress().isMulticast())) {
            unsupportedData(packet);
            return;
        }
        if (mesh && meshPowerSave && meshPowerSave->isEnabled())
            meshPowerSave->receivePowerFields(packet, mesh);
        if (header->getType() == ST_QOS_NULL) {
            // Ordinary MAC ACK has completed. Null has no payload identity or
            // LLC body; peer learning and physical ACK are separate contracts.
            delete packet;
            return;
        }
        auto data = dynamicPtrCast<const Ieee80211DataHeader>(header);
        bool oversizedNative = data && mib->getMeshPhyProfile() != "ofdm" && !data->getAMsduPresent()
            && packet->getDataLength() > data->getChunkLength() + B(6 + 2304 + 4);
        // Ordinary MAC admission/ACK has completed. Reject an unsupported
        // native MSDU before forwarding, LLC delivery or mesh-state refresh.
        // A-MSDU limits belong to its separate aggregation consumer.
        if (data && isDataReceptionSupported(data) && !oversizedNative && ds.getNullable())
            ds.getNullable()->processDataFrame(packet, data);
        else
            unsupportedData(packet);
        return;
    }
    packet->popAtBack<Ieee80211MacTrailer>(B(4));
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ieee80211Mgmt);
    send(packet, "mgmtOut");
}

void Ieee80211MeshMac::prepareFrameTransmission(Packet *packet, simtime_t ifs,
        const physicallayer::IIeee80211Mode *mode)
{
    meshManagement->prepareManagementTransmission(packet, ifs, mode);
}

void Ieee80211MeshMac::frameTransmissionComplete(const Packet *packet)
{
    meshManagement->managementTransmissionComplete(packet);
}

void Ieee80211MeshMac::frameTransmissionComplete(const Packet *packet, simtime_t start)
{
    auto mode = packet->findTag<physicallayer::Ieee80211ModeReq>();
    if (!mode || !mode->getTxVector().ampdu) frameTransmissionComplete(packet);
    if (meshPowerSave && meshPowerSave->isEnabled())
        meshPowerSave->ppduTransmissionComplete(packet, start);
    requestRadioReview();
}

void Ieee80211MeshMac::prepareFrameContent(Packet *packet, const std::set<int64_t>& deliveryUnit)
{
    meshManagement->prepareManagementContent(packet);
    auto mesh = dynamicPtrCast<const Ieee80211MeshDataHeader>(packet->peekAtFront<Ieee80211MacHeader>());
    if (mesh && mesh->getReceiverAddress().isMulticast()
            && (mesh->getType() == ST_DATA_WITH_QOS || mesh->getType() == ST_QOS_NULL)
            && mesh->getAckPolicy() != NO_ACK) {
        // IEEE Std 802.11-2024, Table 9-13: selected non-GCR group
        // transmission requests no acknowledgment. Canonicalize local and
        // relayed headers before FCS, independently of power-save activation.
        auto updated = packet->removeAtFront<Ieee80211MeshDataHeader>();
        updated->setAckPolicy(NO_ACK);
        packet->insertAtFront(updated);
    }
    if (meshPowerSave && meshPowerSave->isEnabled())
        meshPowerSave->prepareTransmission(packet, deliveryUnit);
}

void Ieee80211MeshMac::finalizeDataFrame(Packet *packet)
{
    auto source = packet->findTag<Ieee80211MeshAggregationTag>();
    if (!source || !source->getPendingSourceIdentity())
        return; // Committed/transit work needs no allocator, including after provider removal.
    auto forwarding = dynamic_cast<IMeshForwarding *>(ds.getNullable());
    if (!forwarding)
        throw cRuntimeError("Cannot commit a pending mesh source without its allocator");
    forwarding->finalizeSourceFrame(packet);
}

void Ieee80211MeshMac::discardPendingMeshSources()
{
    Enter_Method("discardPendingMeshSources");
    check_and_cast<Hcf *>(getSubmodule("hcf"))->discardPendingMeshSources();
}

bool Ieee80211MeshMac::isFrameTransmissionEligible(const Packet *packet, simtime_t plannedStart) const
{
    if (defersTransmissionStart() && !isRadioReadyForTransmission(plannedStart)) return false;
    auto source = packet->findTag<Ieee80211MeshAggregationTag>();
    if (source && source->getPendingSourceIdentity()) {
        auto forwarding = dynamic_cast<IMeshForwarding *>(ds.getNullable());
        if (!forwarding || !forwarding->isSourceActive())
            return false;
    }
    // IEEE Std 802.11-2024, 14.14.3.1: pending traffic waits behind the TBTT beacon.
    if (!meshManagement->isMeshManagementActive())
        return false;
    if (packet->peekAtFront<Ieee80211MacHeader>()->getType() == ST_BEACON)
        return true;
    if (meshPowerSave && !meshPowerSave->isTransmissionEligible(packet, plannedStart))
        return false;
    return !meshManagement->isBeaconPending() && plannedStart < meshManagement->getNextTbtt();
}

bool Ieee80211MeshMac::isDataReceptionSupported(const Ptr<const Ieee80211DataHeader>& header) const
{
    auto mesh = dynamicPtrCast<const Ieee80211MeshDataHeader>(header);
    if (mesh && header->getType() == ST_QOS_NULL)
        return !mesh->getMeshControlPresent() && !mesh->getAMsduPresent() && !mesh->getMoreFragments() && mesh->getFragmentNumber() == 0;
    return mesh && header->getType() == ST_DATA_WITH_QOS && mesh->getMeshControlPresent()
        && !header->getMoreFragments() && header->getFragmentNumber() == 0
        && (!header->getAMsduPresent() || (mib->getLocalMeshPhyCapabilities().htPresent && header->getToDS() && header->getFromDS()));
}

bool Ieee80211MeshMac::defersTransmissionStart() const
{
    return powerManaged || (meshPowerSave && meshPowerSave->isEnabled());
}

bool Ieee80211MeshMac::isTransmissionReady() const
{
    if (!defersTransmissionStart()) return true;
    auto state = radio->getRadioModeState();
    return state && !state->transition && state->mode == physicallayer::IRadio::RADIO_MODE_TRANSCEIVER
        && radio->getTransmissionState() == physicallayer::IRadio::TRANSMISSION_STATE_IDLE
        && radio->getReceptionState() != physicallayer::IRadio::RECEPTION_STATE_RECEIVING;
}

bool Ieee80211MeshMac::isRadioReadyForTransmission(simtime_t start) const
{
    if (!defersTransmissionStart()) return true;
    if (start < simTime()) return false;
    auto state = radio->getRadioModeState();
    if (!state) return false;
    using Radio = physicallayer::IRadio;
    if (state->transition)
        return state->transition->targetMode == Radio::RADIO_MODE_TRANSCEIVER && state->transition->endTime <= start;
    if (state->mode == Radio::RADIO_MODE_TRANSCEIVER)
        return start > simTime() || isTransmissionReady();
    auto duration = radio->getRadioModeSwitchingTime(state->mode, Radio::RADIO_MODE_TRANSCEIVER);
    return duration && simTime() + *duration <= start;
}

std::optional<physicallayer::IRadio::RadioModeState> Ieee80211MeshMac::getMeshRadioState() const
{
    return radio->getRadioModeState();
}

std::optional<simtime_t> Ieee80211MeshMac::getMeshRadioSwitchingTime(
        physicallayer::IRadio::RadioMode from, physicallayer::IRadio::RadioMode to) const
{
    return radio->getRadioModeSwitchingTime(from, to);
}

bool Ieee80211MeshMac::hasMeshRadioActivity() const
{
    return tx->hasTransmission() || hcf->hasActiveExchange()
        || radio->getReceptionState() == physicallayer::IRadio::RECEPTION_STATE_RECEIVING
        || radio->getTransmissionState() == physicallayer::IRadio::TRANSMISSION_STATE_TRANSMITTING;
}

bool Ieee80211MeshMac::isMeshPhyIdle() const
{
    using Radio = physicallayer::IRadio;
    auto mode = radio->getRadioMode();
    if (mode != Radio::RADIO_MODE_RECEIVER && mode != Radio::RADIO_MODE_TRANSCEIVER) return false;
    if (radio->getTransmissionState() == Radio::TRANSMISSION_STATE_TRANSMITTING) return false;
    auto channel = dynamic_cast<const physicallayer::IIeee80211ChannelAccess *>(radio.get());
    return channel && channel->hasContiguousAllocation() ? !channel->isPrimaryChannelBusy()
        : radio->getReceptionState() == Radio::RECEPTION_STATE_IDLE;
}

void Ieee80211MeshMac::requestRadioReview()
{
    Enter_Method("requestRadioReview");
    if (radioReview && defersTransmissionStart() && operationalState == OPERATING && !radioReview->isScheduled())
        scheduleAt(simTime(), radioReview);
}

void Ieee80211MeshMac::handleSelfMessage(cMessage *message)
{
    if (message == radioReview) reviewRadioState();
    else Ieee80211Mac::handleSelfMessage(message);
}

void Ieee80211MeshMac::reviewRadioState()
{
    if (meshPowerSave && meshPowerSave->isEnabled()) {
        powerManaged = true;
        meshPowerSave->reviewAwakeState();
    }
    if (!powerManaged || radioReview->isScheduled()) return;
    awakeIntent = !meshPowerSave || meshPowerSave->requiresAwake() || tx->hasTransmission() || hcf->hasActiveExchange();
    auto state = radio->getRadioModeState();
    if (!state || state->transition) return; // Completion==now still waits for its actual publication.
    auto target = awakeIntent ? physicallayer::IRadio::RADIO_MODE_TRANSCEIVER : physicallayer::IRadio::RADIO_MODE_SLEEP;
    if (state->mode != target) {
        radio->setRadioMode(target); // Only this deferred continuation issues policy commands.
        return;
    }
    if (isTransmissionReady()) tx->resumePendingTransmission();
    hcf->meshPowerEligibilityChanged();
}

void Ieee80211MeshMac::configureRadioMode(physicallayer::IRadio::RadioMode mode)
{
    if (defersTransmissionStart()) requestRadioReview();
    else Ieee80211Mac::configureRadioMode(mode);
}

void Ieee80211MeshMac::receiveSignal(cComponent *source, simsignal_t signal, intval_t value, cObject *details)
{
    if (meshPowerSave && meshPowerSave->isEnabled()) meshPowerSave->radioObservationChanged();
    Ieee80211Mac::receiveSignal(source, signal, value, details);
    if (defersTransmissionStart()) requestRadioReview();
}

void Ieee80211MeshMac::receiveSignal(cComponent *source, simsignal_t signal, bool value, cObject *details)
{
    receiveSignal(source, signal, static_cast<intval_t>(value), details);
}

void Ieee80211MeshMac::handleStartOperation(LifecycleOperation *operation)
{
    Ieee80211Mac::handleStartOperation(operation);
    requestRadioReview();
}

void Ieee80211MeshMac::handleStopOperation(LifecycleOperation *operation)
{
    cancelEvent(radioReview);
    Ieee80211Mac::handleStopOperation(operation);
}

void Ieee80211MeshMac::handleCrashOperation(LifecycleOperation *operation)
{
    cancelEvent(radioReview);
    Ieee80211Mac::handleCrashOperation(operation);
}

} // namespace ieee80211
} // namespace inet
