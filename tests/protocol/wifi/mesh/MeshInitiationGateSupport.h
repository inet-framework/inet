// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TEST_MESHINITIATIONGATESUPPORT_H
#define INET_TEST_MESHINITIATIONGATESUPPORT_H

#include "mesh/MeshServicePeriodSupport.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/SignalTag_m.h"

class InitiationPowerObserver : public MeshPowerSave
{
  public:
    simtime_t ownWindowEnd() const { return std::max(ownAwEnd,postAwEnd); }
    simtime_t peerWindowEnd(MacAddress peer) const {
        auto it=awakeUntil.find(peer); return it==awakeUntil.end() ? SIMTIME_ZERO : it->second;
    }
    bool initiationAllowed(MacAddress peer,simtime_t time) const { return canInitiateTo(peer,time); }
};
namespace initiation_power_registration { Define_Module(InitiationPowerObserver); }

class InitiationHoldMac : public Ieee80211MeshMac
{
  public:
    bool isFrameTransmissionEligible(const Packet *packet,simtime_t start) const override {
        auto driver=getSimulation()->getSystemModule()->getSubmodule("driver");
        if (getContainingNode(this)->getIndex()==0 && std::string(packet->getName())=="mesh-period-ending"
                && !driver->par("releaseEnding").boolValue()) return false;
        return Ieee80211MeshMac::isFrameTransmissionEligible(packet,start);
    }
};
namespace initiation_mac_registration { Define_Module(InitiationHoldMac); }

class InitiationGateDriver : public cSimpleModule, public cListener
{
    bool submitted=false, queued=false, observedQuiet=false, nextBeacon=false;
    int reverseTriggers=0, reverseAir=0, reverseDelivered=0;
    int endingAir=0, ownerConfirmed=0, recipientConfirmed=0;
    int crossingReception=0;
    bool endingOutside() const { return par("scenario").intValue()==44; }
    bool crossing() const { return par("scenario").intValue()==45; }
    InitiationPowerObserver *power(int i) const { return check_and_cast<InitiationPowerObserver *>(meshInterface(i)->getSubmodule("powerSavePolicy")); }
    bool role(int node,MeshPeriodDirection direction) const {
        for (const auto& view : power(node)->getServicePeriods())
            if (view.peer==meshMib(1-node)->address && view.direction==direction) return true;
        return false;
    }
    void at(simtime_t when,int kind) { scheduleAt(when,new cMessage("initiation-gate",kind)); }
    void trigger() {
        auto header=makeShared<Ieee80211MeshDataHeader>();
        header->setType(ST_QOS_NULL); header->setChunkLength(B(32));
        header->setToDS(true); header->setFromDS(true);
        header->setReceiverAddress(meshMib(1)->address); header->setTransmitterAddress(meshMib(0)->address);
        header->setAddress3(meshMib(1)->address); header->setAddress4(meshMib(0)->address);
        header->setAckPolicy(NORMAL_ACK); header->setRspi(false); header->setEosp(false);
        auto packet=new Packet("gate-opening",header);
        packet->insertAtBack(makeShared<Ieee80211MacTrailer>());
        packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::ieee80211Mac);
        check_and_cast<IMeshDataMac *>(meshInterface(0)->getSubmodule("mac"))->sendMeshData(packet);
    }
  protected:
    void initialize() override {
        for (auto signal : {transmissionStartedSignal,receptionEndedSignal,packetSentToUpperSignal,MeshPowerSave::meshServicePeriodOutcomeSignal})
            getSimulation()->getSystemModule()->subscribe(signal,this);
        at(SimTime(1,SIMTIME_MS),0); at(SimTime(60,SIMTIME_MS),1); at(SimTime(80,SIMTIME_MS),2);
        at(SimTime(2200,SIMTIME_MS),9);
    }
    void receiveSignal(cComponent *source,simsignal_t signal,cObject *object,cObject *details) override {
        Enter_Method_Silent();
        if (signal==MeshPowerSave::meshServicePeriodOutcomeSignal) {
            auto fact=check_and_cast<MeshPowerSave::PeriodOutcome *>(object);
            if (fact->outcome==MeshPeriodOutcome::CONFIRMED) {
                ownerConfirmed+=source==power(0) && fact->direction==MeshPeriodDirection::OWNER;
                recipientConfirmed+=source==power(1) && fact->direction==MeshPeriodDirection::RECIPIENT;
            }
            return;
        }
        if (signal==packetSentToUpperSignal && source==meshInterface(0)->getSubmodule("mac")) {
            auto packet=dynamic_cast<Packet *>(object);
            if (packet && std::string(packet->getName())=="gate-reverse") reverseDelivered++;
            return;
        }
        if (signal==transmissionStartedSignal && source==meshInterface(1)->getSubmodule("radio")) {
            auto packet=check_and_cast<const Ieee80211Transmission *>(object)->getPacket();
            if (queued && std::string(packet->getName())=="mesh-period-trigger") { ASSERT(observedQuiet && nextBeacon); reverseTriggers++; }
            if (std::string(packet->getName())=="gate-reverse") { ASSERT(observedQuiet && nextBeacon); reverseAir++; }
            return;
        }
        if (signal==transmissionStartedSignal && source==meshInterface(0)->getSubmodule("radio") && endingOutside()) {
            auto packet=check_and_cast<const Ieee80211Transmission *>(object)->getPacket();
            if (std::string(packet->getName())=="mesh-period-ending") {
                ASSERT(simTime()>power(1)->ownWindowEnd() && role(1,MeshPeriodDirection::RECIPIENT));
                endingAir++;
            }
            return;
        }
        if (signal!=receptionEndedSignal) return;
        auto packet=dynamic_cast<Packet *>(details); if (!packet || packet->hasBitError()) return;
        auto header=packet->peekAtFront<Ieee80211MacHeader>();
        if (!submitted && source==meshInterface(0)->getSubmodule("radio") && header->getType()==ST_BEACON
                && simTime()>SimTime(150,SIMTIME_MS)) {
            submitted=true;
            if (crossing()) {
                auto end=power(1)->ownWindowEnd();
                ASSERT(end>simTime()+SimTime(70,SIMTIME_US));
                at(end-SimTime(70,SIMTIME_US),3);
            }
            else at(simTime()+SimTime::fromRaw(1),3);
        }
        if (source==meshInterface(1)->getSubmodule("radio") && std::string(packet->getName())=="gate-opening") {
            if (crossing()) {
                auto signalTime=packet->findTag<SignalTimeInd>();
                std::cout << "gate crossing observed tag=" << bool(signalTime) << " start="
                    << (signalTime ? signalTime->getStartTime() : SimTime(-1))
                    << " end=" << simTime() << " ownEnd=" << power(1)->ownWindowEnd() << std::endl;
                ASSERT(signalTime && signalTime->getStartTime()<power(1)->ownWindowEnd()
                    && simTime()>power(1)->ownWindowEnd());
                crossingReception++; at(simTime()+SimTime::fromRaw(1),6); return;
            }
            auto end=std::max({power(0)->ownWindowEnd(),power(1)->ownWindowEnd(),power(0)->peerWindowEnd(meshMib(1)->address),power(1)->peerWindowEnd(meshMib(0)->address)});
            at(std::max(end,simTime())+SimTime(1,SIMTIME_MS),4);
        }
        if (queued && observedQuiet && source==meshInterface(1)->getSubmodule("radio") && header->getType()==ST_BEACON) {
            auto beacon=packet->peekAtFront<Ieee80211TwoAddressHeader>();
            if (beacon->getTransmitterAddress()==meshMib(0)->address) nextBeacon=true;
        }
    }
    void handleMessage(cMessage *event) override {
        int kind=event->getKind(); delete event;
        if (kind==0) { meshManagement(0)->openPeer(meshMib(1)->address); meshManagement(1)->openPeer(meshMib(0)->address); }
        else if (kind==1) power(0)->requestMode(meshMib(1)->address,MeshPowerMode::DEEP_SLEEP);
        else if (kind==2) power(1)->requestMode(meshMib(0)->address,MeshPowerMode::DEEP_SLEEP);
        else if (kind==3) { ASSERT(power(0)->initiationAllowed(meshMib(1)->address,simTime())); trigger(); }
        else if (kind==4) {
            ASSERT(role(0,MeshPeriodDirection::OWNER) && role(1,MeshPeriodDirection::RECIPIENT));
            ASSERT(simTime()>power(0)->ownWindowEnd() && simTime()>power(1)->peerWindowEnd(meshMib(0)->address));
            if (endingOutside()) {
                ASSERT(simTime()>power(1)->ownWindowEnd());
                int64_t endingId=-1;
                auto buffer=check_and_cast<IMeshPowerBuffer *>(meshInterface(0)->getSubmodule("mac")->getSubmodule("hcf"));
                for (const auto& frame : buffer->getMeshBufferedFrames())
                    if (frame.receiver==meshMib(1)->address && frame.kind==MeshBufferedFrame::Kind::POWER_CONTROL) {
                        ASSERT(endingId==-1); endingId=frame.id;
                    }
                ASSERT(endingId!=-1 && simTime()>power(0)->peerWindowEnd(meshMib(1)->address));
                MeshPowerTxMember prospective{endingId,meshMib(1)->address,ST_QOS_NULL,0,true,true,7};
                auto before=power(0)->getServicePeriods();
                auto end=power(0)->peerWindowEnd(meshMib(1)->address);
                ASSERT(end>SIMTIME_ZERO && power(0)->canTransmitSet({prospective},end-SimTime::fromRaw(1)));
                ASSERT(!power(0)->canTransmitSet({prospective},end));
                ASSERT(!power(0)->canTransmitSet({prospective},simTime()));
                ASSERT(!power(0)->authorizeTransmissionStart({prospective},simTime()));
                prospective.rspi=false;
                ASSERT(power(0)->canTransmitSet({prospective},simTime()));
                auto after=power(0)->getServicePeriods();
                ASSERT(before.size()==after.size() && before.front().cohort==after.front().cohort
                    && before.front().spent==after.front().spent && before.front().ending==after.front().ending);
                par("releaseEnding").setBoolValue(true);
                buffer->meshPowerEligibilityChanged();
                return;
            }
            ASSERT(!power(1)->initiationAllowed(meshMib(0)->address,simTime()));
            fixturePath(1,0,0,SimTime(3,SIMTIME_S)); fixturePrecursor(0,0,1,SimTime(3,SIMTIME_S));
            queued=true;
            check_and_cast<IMeshForwarding *>(meshInterface(1)->getSubmodule("mac")->getSubmodule("ds"))
                ->processSourceMsdu(phyMsdu("gate-reverse",meshMib(0)->address));
            at(simTime()+SimTime(2,SIMTIME_MS),5);
        }
        else if (kind==5) {
            ASSERT(!role(1,MeshPeriodDirection::OWNER) && !role(0,MeshPeriodDirection::RECIPIENT));
            ASSERT(reverseTriggers==0 && reverseAir==0 && reverseDelivered==0 && !nextBeacon);
            observedQuiet=true;
        }
        else if (kind==6) {
            ASSERT(crossing() && crossingReception==1 && role(1,MeshPeriodDirection::RECIPIENT));
            meshManagement(0)->closePeer(meshMib(1)->address);
            meshManagement(1)->closePeer(meshMib(0)->address);
            emit(registerSignal("serviceComplete"),1L);
        }
        else {
            if (endingOutside()) {
                ASSERT(submitted && endingAir==1 && ownerConfirmed==1 && recipientConfirmed==1);
                ASSERT(!role(0,MeshPeriodDirection::OWNER) && !role(1,MeshPeriodDirection::RECIPIENT));
                emit(registerSignal("serviceComplete"),1L); return;
            }
            ASSERT(submitted && queued && observedQuiet && nextBeacon && reverseDelivered==1 && reverseAir==1);
            par("releaseEnding").setBoolValue(true);
            check_and_cast<IMeshPowerBuffer *>(meshInterface(0)->getSubmodule("mac")->getSubmodule("hcf"))->meshPowerEligibilityChanged();
            emit(registerSignal("serviceComplete"),1L);
        }
    }
};
namespace initiation_driver_registration { Define_Module(InitiationGateDriver); }
#endif
