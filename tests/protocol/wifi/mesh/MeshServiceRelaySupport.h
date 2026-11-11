// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TEST_MESHSERVICERELAYSUPPORT_H
#define INET_TEST_MESHSERVICERELAYSUPPORT_H
#include "mesh/MeshServicePeriodSupport.h"
#include "inet/linklayer/ieee80211/mesh/hwmp/Hwmp.h"

class RelayExpiryHcf : public Hcf
{
  public:
    void setExpiry(const char *name,simtime_t deadline) {
        Enter_Method("setExpiry");
        for (int ac=0;ac<AC_NUMCATEGORIES;ac++) {
            auto queue=edca->getEdcaf(static_cast<AccessCategory>(ac))->getPendingQueue();
            for (int i=0;i<queue->getNumPackets();i++) {
                auto packet=queue->getPacket(i);
                if (std::string(packet->getName())!=name) continue;
                edca->getLifetimeHandler()->forgetFrame(packet->getId());
                edca->getLifetimeHandler()->registerFragments({packet},deadline);
                scheduleDataLifetimeTimer(); return;
            }
        }
        throw cRuntimeError("Expected queued sleeping-relay expiry original");
    }
};
namespace relay_expiry_hcf_registration { Define_Module(RelayExpiryHcf); }

class ServiceRelayDriver : public cSimpleModule, public cListener
{
    int delivered=0, dataAir=0, samples=0, success=0, terminals=0, perrs=0, sleepTransitions=0;
    int preqs=0, preps=0, rootPreqs=0, rootRelays=0, ranns=0, confirmations=0, nonneighborRx=0;
    int hopAir[3]={};
    uint32_t meshSequence=0;
    int expiryDrops=0, expiryAir=0, expirySamples=0;
    bool queueExpiry=false;
    simtime_t expiryDeadline=-1;
    bool haveSequence=false, sleepBeforeData=false;
    int which() const { return par("scenario").intValue(); }
    int sender() const { return which()==0 || which()>=3 ? 0 : 2; }
    int target() const { return which()==0 || which()>=3 ? 2 : 0; }
    Hwmp *paths(int i) const { return check_and_cast<Hwmp *>(meshInterface(i)->getSubmodule("pathSelection")); }
    MeshPowerSave *power(int i) const { return check_and_cast<MeshPowerSave *>(meshInterface(i)->getSubmodule("powerSavePolicy")); }
    void at(simtime_t time,int kind) { scheduleAt(time,new cMessage("service-relay",kind)); }
    int node(cComponent *source,const char *name) const {
        for (int i=0;i<3;i++) if (source==meshInterface(i)->getSubmodule(name)) return i;
        return -1;
    }
  protected:
    void initialize() override {
        for (auto signal : {transmissionStartedSignal,receptionEndedSignal,packetSentToUpperSignal,
                Hcf::meshDataAttemptCompletedSignal,Hcf::meshTerminalRecoverySignal,IRadio::radioModeChangedSignal,packetDroppedSignal})
            getSimulation()->getSystemModule()->subscribe(signal,this);
        at(SimTime(1,SIMTIME_MS),0); at(SimTime(15,SIMTIME_MS),1); at(SimTime(100,SIMTIME_MS),2);
        at(SimTime(150,SIMTIME_MS),3); at(SimTime(180,SIMTIME_MS),4);
        at(SimTime(which()==0 || which()>=3 ? 400 : 900,SIMTIME_MS),5); at(SimTime(2,SIMTIME_S),6);
    }
    void receiveSignal(cComponent *source,simsignal_t signal,long value,cObject *) override {
        Enter_Method_Silent();
        if (signal==IRadio::radioModeChangedSignal && node(source,"radio")==1 && value==IRadio::RADIO_MODE_SLEEP) {
            sleepTransitions++;
            if (which()==4 && delivered==1 && !queueExpiry) { queueExpiry=true; at(simTime()+SimTime::fromRaw(1),7); }
        }
    }
    void receiveSignal(cComponent *source,simsignal_t signal,cObject *object,cObject *details) override {
        Enter_Method_Silent();
        if (signal==Hcf::meshTerminalRecoverySignal) { terminals++; return; }
        if (signal==packetDroppedSignal) {
            auto packet=dynamic_cast<Packet *>(object);
            if (packet && std::string(packet->getName())=="sleeping-relay-expiry") {
                ASSERT(source==meshInterface(0)->getSubmodule("mac")->getSubmodule("hcf"));
                ASSERT(check_and_cast<PacketDropDetails *>(details)->getReason()==LIFETIME_EXPIRED && simTime()==expiryDeadline);
                expiryDrops++;
            }
            return;
        }
        if (signal==Hcf::meshDataAttemptCompletedSignal) {
            auto packet=check_and_cast<Packet *>(object);
            if (std::string(packet->getName())=="sleeping-relay-expiry") expirySamples++;
            if (std::string(packet->getName())=="sleeping-relay-data") {
                auto attempt=check_and_cast<MeshDataAttempt *>(details);
                samples++; success+=attempt->acknowledged;
                if (!attempt->acknowledged && attempt->terminalTimeout) terminals++;
            }
            return;
        }
        if (signal==packetSentToUpperSignal) {
            auto packet=dynamic_cast<Packet *>(object);
            int i=node(source,"mac");
            if (i>=0 && packet && std::string(packet->getName())=="sleeping-relay-data") {
                ASSERT(i==target()); delivered++;
                auto addresses=packet->getTag<MacAddressInd>();
                ASSERT(addresses->getSrcAddress()==meshMib(sender())->address && addresses->getDestAddress()==meshMib(target())->address);
            }
            return;
        }
        if (signal==receptionEndedSignal) {
            int i=node(source,"radio"); auto packet=dynamic_cast<Packet *>(details);
            if (i<0 || i==1 || !packet || packet->hasBitError()) return;
            auto header=dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(packet->peekAtFront<Ieee80211MacHeader>());
            if (header && header->getTransmitterAddress()==meshMib(2-i)->address) nonneighborRx++;
            return;
        }
        if (signal!=transmissionStartedSignal) return;
        int i=node(source,"radio"); if (i<0) return;
        auto transmission=check_and_cast<const Ieee80211Transmission *>(object); auto packet=transmission->getPacket();
        if (std::string(packet->getName())=="sleeping-relay-expiry") expiryAir++;
        auto phy=Ieee80211Radio::peekIeee80211PhyHeaderAtFront(packet);
        ASSERT(!transmission->getTxVector().ampdu && !transmission->getTxVector().singleMpdu);
        auto header=packet->peekDataAt<Ieee80211MacHeader>(phy->getChunkLength());
        if (std::string(packet->getName())=="sleeping-relay-data") {
            auto data=packet->peekDataAt<Ieee80211MeshDataHeader>(phy->getChunkLength());
            auto control=packet->peekDataAt<Ieee80211MeshControl>(phy->getChunkLength()+data->getChunkLength());
            ASSERT(i==sender() || i==1);
            ASSERT(data->getReceiverAddress()==meshMib(i==sender() ? 1 : target())->address);
            ASSERT(control->getTtl()==(i==sender() ? 31 : 30));
            if (!haveSequence) { ASSERT(i==sender()); meshSequence=control->getSequenceNumber(); haveSequence=true; }
            ASSERT(control->getSequenceNumber()==meshSequence);
            hopAir[i]++; dataAir++;
            if (which()==3 && i==1 && hopAir[i]==1) emit(registerSignal("relayDataStarted"),1L);
        }
        if (header->getType()!=ST_ACTION) return;
        auto action=packet->peekDataAt<Ieee80211MeshActionFrame>(phy->getChunkLength());
        if (action->getCategory()!=MESH_ACTION || action->getMeshAction()!=MESH_HWMP_PATH_SELECTION) return;
        auto body=packet->peekDataAt<Ieee80211HwmpFrame>(phy->getChunkLength()+header->getChunkLength(),
            phy->getLengthField()-header->getChunkLength()-B(4));
        if (simTime()<SimTime(250,SIMTIME_MS)) return;
        perrs+=body->getPerrsArraySize(); preps+=body->getPrepsArraySize(); ranns+=body->getRannsArraySize();
        for (size_t j=0;j<body->getPreqsArraySize();j++) {
            const auto& preq=body->getPreqs(j); preqs++;
            if (preq.getOriginator()==meshMib(0)->address && preq.getTargetsArraySize()==1
                    && preq.getTargets(0).address.isBroadcast()) {
                rootPreqs+=i==0; rootRelays+=i==1;
            }
            if (which()==2 && i==2 && preq.getOriginator()==meshMib(2)->address
                    && !action->getReceiverAddress().isMulticast()) confirmations++;
        }
    }
    void handleMessage(cMessage *event) override {
        int kind=event->getKind(); delete event;
        if (kind==0) { meshManagement(0)->openPeer(meshMib(1)->address); meshManagement(1)->openPeer(meshMib(0)->address); }
        else if (kind==1) { meshManagement(1)->openPeer(meshMib(2)->address); meshManagement(2)->openPeer(meshMib(1)->address); }
        else if (kind==2) {
            ASSERT(meshMib(0)->isMeshPeerEstablished(meshMib(1)->address) && meshMib(1)->isMeshPeerEstablished(meshMib(2)->address));
            ASSERT(!meshMib(0)->isMeshPeerEstablished(meshMib(2)->address) && !meshMib(2)->isMeshPeerEstablished(meshMib(0)->address));
        }
        else if (kind==3) {
            power(1)->requestMode(meshMib(0)->address,MeshPowerMode::LIGHT_SLEEP);
            power(1)->requestMode(meshMib(2)->address,MeshPowerMode::LIGHT_SLEEP);
        }
        else if (kind==4) {
            power(0)->requestMode(meshMib(1)->address,MeshPowerMode::LIGHT_SLEEP);
            power(2)->requestMode(meshMib(1)->address,MeshPowerMode::LIGHT_SLEEP);
        }
        else if (kind==5) {
            ASSERT(sleepTransitions>=2); sleepBeforeData=true;
            if (which()==0 || which()>=3) ASSERT(!paths(sender())->findPath(meshMib(target())->address));
            else {
                auto path=paths(sender())->findPath(meshMib(target())->address);
                ASSERT(path && path->nextHop==meshMib(1)->address);
            }
            check_and_cast<IMeshForwarding *>(meshInterface(sender())->getSubmodule("mac")->getSubmodule("ds"))
                ->processSourceMsdu(phyMsdu("sleeping-relay-data",meshMib(target())->address));
        }
        else if (kind==7) {
            ASSERT(which()==4 && queueExpiry && delivered==1 && paths(0)->findPath(meshMib(2)->address));
            ASSERT(check_and_cast<IRadio *>(meshInterface(1)->getSubmodule("radio"))->getRadioMode()==IRadio::RADIO_MODE_SLEEP);
            expiryDeadline=simTime()+SimTime(2,SIMTIME_MS);
            check_and_cast<IMeshForwarding *>(meshInterface(0)->getSubmodule("mac")->getSubmodule("ds"))
                ->processSourceMsdu(phyMsdu("sleeping-relay-expiry",meshMib(2)->address));
            check_and_cast<RelayExpiryHcf *>(meshInterface(0)->getSubmodule("mac")->getSubmodule("hcf"))
                ->setExpiry("sleeping-relay-expiry",expiryDeadline);
        }
        else {
            std::cout << "sleeping relay mechanism=" << which() << " delivered=" << delivered << " dataAir=" << dataAir
                << " samples=" << samples << " success=" << success << " sleeps=" << sleepTransitions << " controls="
                << preqs << "/" << preps << "/" << ranns << " directRx=" << nonneighborRx << std::endl;
            ASSERT(sleepBeforeData && sleepTransitions>=2 && delivered==1 && hopAir[sender()]>=1 && hopAir[1]>=1);
            ASSERT(samples==dataAir && nonneighborRx==0);
            if (which()==3) ASSERT(success==1 && terminals==1 && perrs>0 && hopAir[1]==2);
            else ASSERT(success>=2 && terminals==0 && perrs==0);
            if (which()==4) ASSERT(queueExpiry && expiryDrops==1 && expiryAir==0 && expirySamples==0);
            ASSERT(preqs>0 && preps>0);
            if (which()==1) ASSERT(rootPreqs>0 && rootRelays>0);
            if (which()==2) ASSERT(ranns>0 && confirmations>0);
            emit(registerSignal("serviceComplete"),1L);
        }
    }
};
namespace serviceRelayRegistration { Define_Module(ServiceRelayDriver); }
#endif
