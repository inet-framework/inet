// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TEST_MESHPOWERORIGINALFACTSSUPPORT_H
#define INET_TEST_MESHPOWERORIGINALFACTSSUPPORT_H
#include "inet/linklayer/ieee80211/mac/aggregation/Ieee80211Ampdu.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211DataSequenceTag_m.h"
class PowerFactsFixture : public AwakePowerFixture
{
    bool live(const Packet *packet) {
        auto owner=dynamic_cast<Hcf *>(buffers.get());
        auto edca=check_and_cast<Edca *>(owner->getSubmodule("edca"));
        for(int ac=0;ac<AC_NUMCATEGORIES;++ac)
            if(edca->getEdcaf(static_cast<AccessCategory>(ac))->getInProgressFrames()->containsFrame(packet))return true;
        return false;
    }
  public:
    std::set<int64_t> acknowledged,retired;
    void originalAcknowledged(const Packet *packet) override {
        if(std::string(packet->getName()).find("original-fact-")==0){
            ASSERT(live(packet)&&acknowledged.insert(packet->getId()).second);
            for(const auto& frame:buffers->getMeshBufferedFrames())ASSERT(frame.id!=packet->getId());
        }
        AwakePowerFixture::originalAcknowledged(packet);
    }
    void originalRetired(const Packet *packet) override {
        if(std::string(packet->getName()).find("original-fact-")==0){
            ASSERT(!live(packet)&&retired.insert(packet->getId()).second);
            for(const auto& frame:buffers->getMeshBufferedFrames())ASSERT(frame.id!=packet->getId());
        }
        AwakePowerFixture::originalRetired(packet);
    }
};
namespace power_facts_registration { Define_Module(::PowerFactsFixture); }

class PowerOriginalFactsDriver : public cSimpleModule, public cListener
{
    int ppdus=0,samples=0,delivered=0,bars=0;
    int outsideSent=0,outsideDelivered=0;
    bool aggregateMore=false;
    bool ready=false;
    double errorBeforeOutside=0;
    PowerHoldMac *mac(){return check_and_cast<PowerHoldMac *>(meshInterface(0)->getSubmodule("mac"));}
    Hcf *hcf(){return check_and_cast<Hcf *>(mac()->getSubmodule("hcf"));}
    void source(const char *name,int tid=0){
        auto packet=phyMsdu(name,meshMib(1)->address);packet->addTagIfAbsent<UserPriorityReq>()->setUserPriority(tid);
        sendDirect(packet,meshInterface(0)->getSubmodule("llc")->gate("lowerLayerOut"));
    }
    void at(int ms,int kind){scheduleAt(SimTime(ms,SIMTIME_MS),new cMessage("original-step",kind));}
    void initialize() override {
        hcf()->subscribe(Hcf::meshDataAttemptCompletedSignal,this);
        hcf()->subscribe(Hcf::meshPayloadTransmittedSignal,this);
        mac()->subscribe(registerSignal("packetSentToLower"),this);
        meshInterface(1)->getSubmodule("mac")->subscribe(registerSignal("packetSentToUpper"),this);
        meshInterface(0)->getSubmodule("radio")->subscribe(registerSignal("transmissionStarted"),this);
        meshInterface(0)->getSubmodule("radio")->subscribe(registerSignal("transmissionEnded"),this);
        at(5,0);at(50,1);at(70,5);at(75,7);at(80,2);at(81,3);at(120,6);at(150,4);
    }
    void receiveSignal(cComponent *source,simsignal_t signal,cObject *object,cObject *) override {
        Enter_Method_Silent();
        if(signal==registerSignal("transmissionEnded")){
            auto packet=check_and_cast<const Ieee80211Transmission *>(object)->getPacket();
            if(std::string(packet->getName())=="mesh-power-mode"&&simTime()<SimTime(80,SIMTIME_MS))mac()->holdNull=true;
            return;
        }
        if(!ready)return;
        if(signal==registerSignal("transmissionStarted")){
            auto transmission=check_and_cast<const Ieee80211Transmission *>(object);
            if(transmission->getTxVector().ampdu){
                auto packet=transmission->getPacket();auto phy=Ieee80211Radio::peekIeee80211PhyHeaderAtFront(packet);
                Ieee80211AmpduLayout layout;ASSERT(findAmpdu(packet,phy->getChunkLength(),phy->getLengthField(),false,layout));
                ASSERT(layout.members.size()==4);
                for(const auto& member:layout.members){
                    auto bytes=packet->peekDataAt<BytesChunk>(phy->getChunkLength()+member.offset,member.length)->getBytes();
                    ASSERT(bytes[1]&0x20); // Independent FC More Data bit, outside the selected PPDU.
                }
                aggregateMore=true;
            }
            return;
        }
        auto packet=dynamic_cast<Packet *>(object);if(!packet)return;
        if(std::string(packet->getName())=="outside-selected"){
            if(signal==registerSignal("packetSentToLower")){ASSERT(!packet->peekAtFront<Ieee80211DataHeader>()->getMoreData());++outsideSent;}
            if(signal==registerSignal("packetSentToUpper"))++outsideDelivered;
            return;
        }
        auto mode=packet->findTag<Ieee80211ModeReq>();
        if(signal==registerSignal("packetSentToLower")&&(!mode||!mode->getTxVector().ampdu)
                &&packet->peekAtFront<Ieee80211MacHeader>()->getType()==ST_BLOCKACK_REQ)++bars;
        if(std::string(packet->getName()).find("original-fact-")!=0)return;
        if(signal==Hcf::meshDataAttemptCompletedSignal)++samples;
        if(signal==Hcf::meshPayloadTransmittedSignal)++ppdus;
        if(signal==registerSignal("packetSentToUpper"))++delivered;
    }
    void handleMessage(cMessage *event) override {
        int kind=event->getKind();delete event;
        if(kind==0)meshManagement(0)->openPeer(meshMib(1)->address);
        else if(kind==1)source("facts-training");
        else if(kind==5)check_and_cast<AwakePowerFixture *>(meshInterface(1)->getSubmodule("powerSavePolicy"))->requestMode(meshMib(0)->address,MeshPowerMode::LIGHT_SLEEP);
        else if(kind==7){emit(registerSignal("nullStart"),1L);check_and_cast<AwakePowerFixture *>(meshInterface(0)->getSubmodule("powerSavePolicy"))->requestMode(meshMib(1)->address,MeshPowerMode::DEEP_SLEEP);}
        else if(kind==2){
            ready=true;mac()->holdData=true;mac()->holdOutside=true;emit(registerSignal("powerStart"),1L);
            for(int i=0;i<4;++i)source(("original-fact-"+std::to_string(i)).c_str());
            source("outside-selected",6);
        }
        else if(kind==3){
            mac()->holdData=false;
            check_and_cast<AwakePowerFixture *>(meshInterface(0)->getSubmodule("powerSavePolicy"))->opportunity(meshMib(1)->address,SimTime(145,SIMTIME_MS));
            check_and_cast<AwakePowerFixture *>(meshInterface(1)->getSubmodule("powerSavePolicy"))->opportunity(meshMib(0)->address,SimTime(145,SIMTIME_MS));
        }
        else if(kind==6){
            ASSERT(aggregateMore&&check_and_cast<PowerFactsFixture *>(meshInterface(0)->getSubmodule("powerSavePolicy"))->acknowledged.size()==4);
            auto policy=check_and_cast<AwakePowerFixture *>(meshInterface(0)->getSubmodule("powerSavePolicy"));
            ASSERT(policy->getRequestedMode(meshMib(1)->address)==MeshPowerMode::DEEP_SLEEP);
            ASSERT(meshMib(0)->findMeshPeerPowerState(meshMib(1)->address)->local==MeshPowerMode::ACTIVE);
            auto edca=check_and_cast<Edca *>(hcf()->getSubmodule("edca"));bool retainedNull=false;
            for(int ac=0;ac<AC_NUMCATEGORIES;++ac){
                auto owner=edca->getEdcaf(static_cast<AccessCategory>(ac));auto frames=owner->getInProgressFrames();
                for(int i=0;i<frames->getLength();++i){auto packet=frames->getFrames(i);
                    if(packet->peekAtFront<Ieee80211MacHeader>()->getType()==ST_QOS_NULL){
                        ASSERT(!packet->findTag<Ieee80211DataSequenceTag>());
                        ASSERT(owner->getRecoveryProcedure()->getRetryCount(packet,packet->peekAtFront<Ieee80211DataHeader>())==1);retainedNull=true;
                    }
                }
            }
            ASSERT(retainedNull); // Real empty-transfer DELBA has not consumed SNS5 control.
            errorBeforeOutside=check_and_cast<AirtimeLinkMetric *>(meshInterface(0)->getSubmodule("linkMetric"))->getErrorEstimate(meshMib(1)->address);
            ASSERT(errorBeforeOutside==(par("lostBlockAck").boolValue()?1695.0/4096:0));
            policy->requestMode(meshMib(1)->address,MeshPowerMode::ACTIVE);mac()->holdNull=false;
            mac()->holdOutside=false;hcf()->meshPowerEligibilityChanged();
        }
        else{
            auto power=check_and_cast<PowerFactsFixture *>(meshInterface(0)->getSubmodule("powerSavePolicy"));
            std::cout<<"original facts ack="<<power->acknowledged.size()<<" retired="<<power->retired.size()
                <<" ppdus="<<ppdus<<" samples="<<samples<<" delivered="<<delivered<<" bars="<<bars<<endl;
            ASSERT(power->acknowledged.size()==4&&power->retired==power->acknowledged);
            ASSERT(ppdus==1&&samples==4&&delivered==4);
            ASSERT(aggregateMore&&outsideSent==1&&outsideDelivered==1);
            ASSERT(check_and_cast<AirtimeLinkMetric *>(meshInterface(0)->getSubmodule("linkMetric"))->getErrorEstimate(meshMib(1)->address)==errorBeforeOutside*0.875);
            ASSERT(bars==(par("lostBlockAck").boolValue()?1:0));
            emit(registerSignal("powerComplete"),1L);
        }
    }
};
namespace power_facts_registration { Define_Module(::PowerOriginalFactsDriver); }
#endif
