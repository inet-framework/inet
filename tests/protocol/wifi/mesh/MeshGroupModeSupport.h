// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TEST_MESHGROUPMODESUPPORT_H
#define INET_TEST_MESHGROUPMODESUPPORT_H
#include "mesh/MeshServicePeriodSupport.h"

// Observe the existing owner without changing its decisions.
class GroupModeObserver : public MeshPowerSave
{
  public:
    int matchingCompletions = 0;
    int64_t broadcastId() const {
        auto it = transitions.find(MacAddress::BROADCAST_ADDRESS);
        return it == transitions.end() ? -1 : it->second.originalId;
    }
    bool hasGroupPayload() const { return hasBufferedGroup({}); }
    bool groupDeliveryActive() const { return deliveringGroups; }
    void groupTransmissionComplete(const Packet *packet) override {
        bool matching = broadcastId() == packet->getId();
        MeshPowerSave::groupTransmissionComplete(packet);
        if (matching) {
            ASSERT(broadcastId() == -1);
            matchingCompletions++;
        }
    }
};
namespace group_mode_observer_registration { Define_Module(GroupModeObserver); }

class GroupModeDriver : public cSimpleModule, public cListener
{
    bool requested = false;
    int air = 0, attempts = 0, payloads = 0, recoveries = 0, periods = 0;
    GroupModeObserver *power(int node) const {
        return check_and_cast<GroupModeObserver *>(meshInterface(node)->getSubmodule("powerSavePolicy"));
    }
    void at(simtime_t time, int kind) { scheduleAt(time, new cMessage("group-mode", kind)); }
  protected:
    void initialize() override {
        auto network = getSimulation()->getSystemModule();
        for (auto signal : {transmissionStartedSignal, Hcf::meshDataAttemptCompletedSignal,
                Hcf::meshPayloadTransmittedSignal, Hcf::meshTerminalRecoverySignal,
                MeshPowerSave::meshServicePeriodOutcomeSignal}) network->subscribe(signal, this);
        at(SimTime(1, SIMTIME_MS), 0);
        at(SimTime(60, SIMTIME_MS), 1);
        at(SimTime(80, SIMTIME_MS), 2);
        at(SimTime(130, SIMTIME_MS), 3);
        at(SimTime(1400, SIMTIME_MS), 4);
    }
    void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *) override {
        Enter_Method_Silent();
        if (!requested) return;
        if (signal == Hcf::meshDataAttemptCompletedSignal) { attempts++; return; }
        if (signal == Hcf::meshPayloadTransmittedSignal) { payloads++; return; }
        if (signal == Hcf::meshTerminalRecoverySignal) { recoveries++; return; }
        if (signal == MeshPowerSave::meshServicePeriodOutcomeSignal) { periods++; return; }
        if (signal != transmissionStartedSignal || source != meshInterface(0)->getSubmodule("radio")) return;
        auto packet = check_and_cast<const Ieee80211Transmission *>(object)->getPacket();
        auto phy = Ieee80211Radio::peekIeee80211PhyHeaderAtFront(packet);
        auto header = packet->peekDataAt<Ieee80211MacHeader>(phy->getChunkLength());
        if (header->getType() != ST_QOS_NULL || !header->getReceiverAddress().isMulticast()) return;
        auto data = packet->peekDataAt<Ieee80211MeshDataHeader>(phy->getChunkLength());
        ASSERT(!data->getPowerMgmt() && data->getAckPolicy() == NO_ACK && !data->getRspi());
        air++;
    }
    void handleMessage(cMessage *event) override {
        int kind = event->getKind(); delete event;
        if (kind == 0) {
            meshManagement(0)->openPeer(meshMib(1)->address);
            meshManagement(1)->openPeer(meshMib(0)->address);
        }
        else if (kind == 1) power(0)->requestMode(meshMib(1)->address, MeshPowerMode::LIGHT_SLEEP);
        else if (kind == 2) power(1)->requestMode(meshMib(0)->address, MeshPowerMode::LIGHT_SLEEP);
        else if (kind == 3) {
            auto first = meshMib(0)->findMeshPeerPowerState(meshMib(1)->address);
            auto second = meshMib(1)->findMeshPeerPowerState(meshMib(0)->address);
            auto radio = check_and_cast<IRadio *>(meshInterface(1)->getSubmodule("radio"));
            if (!first || !second || first->local != MeshPowerMode::LIGHT_SLEEP || first->remote != MeshPowerMode::LIGHT_SLEEP
                    || second->local != MeshPowerMode::LIGHT_SLEEP || second->remote != MeshPowerMode::LIGHT_SLEEP
                    || radio->getRadioMode() != IRadio::RADIO_MODE_TRANSCEIVER) {
                at(simTime() + SimTime(1, SIMTIME_MS), 3);
                return;
            }
            ASSERT(!power(0)->hasGroupPayload() && !power(0)->groupDeliveryActive());
            ASSERT(power(0)->getServicePeriods().empty() && power(1)->getServicePeriods().empty());
            requested = true;
            power(0)->requestActiveForAllPeers();
            ASSERT(first->local == MeshPowerMode::ACTIVE && first->remote == MeshPowerMode::LIGHT_SLEEP);
            ASSERT(power(0)->broadcastId() != -1);
        }
        else {
            ASSERT(requested && air == 1 && power(0)->matchingCompletions == 1 && power(0)->broadcastId() == -1);
            ASSERT(meshMib(1)->findMeshPeerPowerState(meshMib(0)->address)->remote == MeshPowerMode::ACTIVE);
            ASSERT(attempts == 0 && payloads == 0 && recoveries == 0 && periods == 0);
            ASSERT(power(0)->getServicePeriods().empty() && power(1)->getServicePeriods().empty());
            emit(registerSignal("serviceComplete"), 1L);
        }
    }
};
namespace group_mode_driver_registration { Define_Module(GroupModeDriver); }
#endif
