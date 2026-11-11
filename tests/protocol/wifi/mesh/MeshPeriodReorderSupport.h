// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TEST_MESHPERIODREORDERSUPPORT_H
#define INET_TEST_MESHPERIODREORDERSUPPORT_H
#include "mesh/MeshSelectivePeriodSupport.h"

class PeriodMemberFact : public cObject
{
  public:
    int sequence;
    bool duplicate;
    int64_t opening;
    PeriodMemberFact(int sequence,bool duplicate,int64_t opening) : sequence(sequence),duplicate(duplicate),opening(opening) {}
};

class ReorderPowerObserver : public SelectivePowerObserver
{
  public:
    void recipientMemberReceived(const Packet *packet,const Ptr<const Ieee80211MeshDataHeader>& header,bool duplicate) override {
        MeshPowerSave::recipientMemberReceived(packet,header,duplicate);
        if (getContainingNode(this)->getIndex()!=1 || header->getType()!=ST_DATA_WITH_QOS
                || std::string(packet->getName()).find("service-payload")!=0) return;
        auto period=findPeriod(header->getTransmitterAddress(),MeshPeriodDirection::RECIPIENT);
        ASSERT(period && period->ending && header->getEosp());
        PeriodMemberFact fact(header->getSequenceNumber().get(),duplicate,period->openingId);
        emit(registerSignal("periodMember"),&fact);
    }
    void originalAcknowledged(const Packet *packet) override {
        MeshPowerSave::originalAcknowledged(packet);
        if (getContainingNode(this)->getIndex()!=1 || std::string(packet->getName())!="mesh-period-trigger") return;
        auto header=packet->peekAtFront<Ieee80211MeshDataHeader>();
        if (header->getRspi() && header->getEosp()) emit(registerSignal("reorderRecipientOpened"),1L);
    }
};
namespace reorderPowerRegistration { Define_Module(ReorderPowerObserver); }

class PeriodReorderMac : public EndingResponseMac
{
  public:
    bool isFrameTransmissionEligible(const Packet *packet,simtime_t start) const override {
        if (getContainingNode(this)->getIndex()==0 && std::string(packet->getName()).find("service-payload")==0
                && getSimulation()->getSystemModule()->getSubmodule("driver")->par("holdRetries").boolValue()) return false;
        return EndingResponseMac::isFrameTransmissionEligible(packet,start);
    }
};
namespace periodReorderMacRegistration { Define_Module(PeriodReorderMac); }

inline void permuteReceivedPeriodMembers(Packet *packet,bool retry)
{
    auto phy=Ieee80211Radio::peekIeee80211PhyHeaderAtFront(packet);
    auto mode=packet->getTag<Ieee80211ModeInd>();
    Ieee80211AmpduLayout layout;
    ASSERT(findAmpdu(packet,phy->getChunkLength(),phy->getLengthField(),mode->getTxVector().format==Ieee80211TxVector::VHT,layout));
    ASSERT(layout.members.size()==3);
    std::vector<std::vector<uint8_t>> members;
    for (const auto& member : layout.members) {
        ASSERT(member.length==layout.members.front().length);
        members.push_back(packet->peekDataAt<BytesChunk>(phy->getChunkLength()+member.offset,member.length)->getBytes());
    }
    // RC2 keeps the last received SC. Initial order ends in slot1; retry
    // starts with that same SC to exercise its recognized-duplicate verdict.
    const int initial[]={2,0,1}, repeated[]={1,0,2};
    const int *order=retry ? repeated : initial;
    for (size_t i=0;i<3;i++) packet->replaceDataAt(makeShared<BytesChunk>(members[order[i]]),
        phy->getChunkLength()+layout.members[i].offset,layout.members[i].length);
}

class PeriodReorderDriver : public ServicePeriodDriver
{
    std::vector<int> sequence,observed;
    std::vector<int64_t> originalIds;
    int64_t firstRecipient=-1, secondRecipient=-1;
    int accepted=0, duplicates=0, repeatedMembers=0, retrievalAcks=0, terminal=0;
  protected:
    void initialize() override {
        ServicePeriodDriver::initialize(); at(SimTime(50,SIMTIME_MS),10);
        auto network=getSimulation()->getSystemModule();
        network->subscribe(registerSignal("periodMember"),this);
        network->subscribe(registerSignal("reorderRecipientOpened"),this);
        network->subscribe(Hcf::meshTerminalRecoverySignal,this);
    }
    void receiveSignal(cComponent *source,simsignal_t signal,long value,cObject *details) override {
        Enter_Method_Silent();
        if (signal==registerSignal("reorderRecipientOpened")) {
            ASSERT(value==1 && aggregateAir==1 && accepted==3 && delivered==3 && retrievalAcks==0);
            retrievalAcks++; at(simTime()+SimTime::fromRaw(1),11); return;
        }
        ServicePeriodDriver::receiveSignal(source,signal,value,details);
    }
    void receiveSignal(cComponent *source,simsignal_t signal,cObject *object,cObject *details) override {
        Enter_Method_Silent();
        if (signal==Hcf::meshTerminalRecoverySignal) { terminal++; return; }
        if (signal==registerSignal("periodMember")) {
            ASSERT(source==power(1)); auto fact=check_and_cast<PeriodMemberFact *>(object);
            if (aggregateAir==1) {
                ASSERT(!fact->duplicate);
                ASSERT(aggregateAir==1 && accepted<3 && !sequence.empty());
                const int order[]={2,0,1}; ASSERT(fact->sequence==sequence.at(order[accepted]));
                if (accepted==0) { ASSERT(delivered==0); firstRecipient=fact->opening; }
                ASSERT(fact->opening==firstRecipient); observed.push_back(fact->sequence); accepted++;
            }
            else {
                ASSERT(aggregateAir==2 && retrievalAcks==1 && delivered==3 && repeatedMembers<3);
                const int order[]={1,0,2};
                ASSERT(fact->duplicate==(repeatedMembers==0));
                if (repeatedMembers==0) { secondRecipient=fact->opening; ASSERT(secondRecipient!=firstRecipient); }
                ASSERT(fact->opening==secondRecipient && fact->sequence==sequence.at(order[repeatedMembers]));
                duplicates+=fact->duplicate; repeatedMembers++;
            }
            return;
        }
        ServicePeriodDriver::receiveSignal(source,signal,object,details);
    }
    void observedAggregate(const Ieee80211Transmission *transmission) override {
        auto packet=transmission->getPacket(); auto phy=Ieee80211Radio::peekIeee80211PhyHeaderAtFront(packet);
        Ieee80211AmpduLayout layout;
        ASSERT(findAmpdu(packet,phy->getChunkLength(),phy->getLengthField(),transmission->getTxVector().format==Ieee80211TxVector::VHT,layout));
        const auto& ids=check_and_cast<ReorderPowerObserver *>(power(0))->selected();
        ASSERT(layout.members.size()==3 && ids.size()==3);
        for (size_t i=0;i<3;i++) {
            auto header=packet->peekDataAt<Ieee80211MeshDataHeader>(phy->getChunkLength()+layout.members[i].offset);
            ASSERT(header->getRetry()==(aggregateAir==2));
            if (aggregateAir==1) sequence.push_back(header->getSequenceNumber().get());
            else ASSERT(header->getSequenceNumber().get()==sequence.at(i));
        }
        if (aggregateAir==1) {
            originalIds=ids; par("holdRetries").setBoolValue(true);
        }
        else {
            ASSERT(aggregateAir==2 && ids==originalIds && retrievalAcks==1);
            bool found=false;
            for (const auto& period : power(0)->getServicePeriods()) if (period.direction==MeshPeriodDirection::OWNER) {
                ASSERT(period.ending && period.spent==3 && period.cohort==std::set<int64_t>(ids.begin(),ids.end())); found=true;
            }
            ASSERT(found);
        }
    }
    void handleMessage(cMessage *event) override {
        int kind=event->getKind();
        if (kind==10) {
            delete event; fixturePath(0,1,1,SimTime(2,SIMTIME_S)); fixturePrecursor(1,1,0,SimTime(2,SIMTIME_S));
            auto forwarding=check_and_cast<IMeshForwarding *>(meshInterface(0)->getSubmodule("mac")->getSubmodule("ds"));
            forwarding->processSourceMsdu(phyMsdu("stamp-prime",meshMib(1)->address));
            forwarding->processSourceMsdu(phyMsdu("stamp-sentinel",meshMib(1)->address)); return;
        }
        if (kind==11) {
            delete event; par("holdRetries").setBoolValue(false);
            originator()->meshPowerEligibilityChanged(); return;
        }
        ServicePeriodDriver::handleMessage(event);
        if (kind==3) check_and_cast<EndingResponseHcf *>(originator())->expireSentinel();
    }
    void finishAssertions() override {
        ASSERT(accepted==3 && duplicates==1 && repeatedMembers==3 && retrievalAcks==1 && terminal==0 && aggregateAir==2);
        ASSERT(sampleOrder=="000111" && nChecks==2 && efChecks==6);
    }
};
namespace periodReorderDriverRegistration { Define_Module(PeriodReorderDriver); }
#endif
