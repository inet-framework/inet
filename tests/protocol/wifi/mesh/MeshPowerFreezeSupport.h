// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TEST_MESHPOWERFREEZESUPPORT_H
#define INET_TEST_MESHPOWERFREEZESUPPORT_H
class PowerFreezeDriver : public cSimpleModule, public cListener
{
    bool prepared=false,changed=false,actual=false;
    int sent=0,delivered=0,expired=0,samples=0;
    int64_t retainedId=-1;
    simtime_t plannedStart;
    std::vector<uint8_t> frozen;
    TimMacFixture *mac(){return check_and_cast<TimMacFixture *>(meshInterface(0)->getSubmodule("mac"));}
    Hcf *hcf(){return check_and_cast<Hcf *>(mac()->getSubmodule("hcf"));}
    TimPowerFixture *power(int s){return check_and_cast<TimPowerFixture *>(meshInterface(s)->getSubmodule("powerSavePolicy"));}
    void source(const char *name,int tid){sendDirect(fixtureMsdu(name,meshMib(1)->address,tid),meshInterface(0)->getSubmodule("llc")->gate("lowerLayerOut"));}
    void at(int ms,int kind){scheduleAt(SimTime(ms,SIMTIME_MS),new cMessage("freeze-step",kind));}
    Packet *findOriginal(){
        auto edca=check_and_cast<Edca *>(hcf()->getSubmodule("edca"));
        for(int ac=0;ac<AC_NUMCATEGORIES;++ac){
            auto frames=edca->getEdcaf(static_cast<AccessCategory>(ac))->getInProgressFrames();
            for(int i=0;i<frames->getLength();++i)if(frames->getFrames(i)->getId()==retainedId)return frames->getFrames(i);
        }
        return nullptr;
    }
    void initialize() override{
        mac()->subscribe(registerSignal("freezePrepared"),this);
        mac()->subscribe(registerSignal("packetSentToLower"),this);
        meshInterface(0)->getSubmodule("radio")->subscribe(registerSignal("transmissionStarted"),this);
        meshInterface(1)->getSubmodule("mac")->subscribe(registerSignal("packetSentToUpper"),this);
        hcf()->subscribe(registerSignal("packetDropped"),this);
        hcf()->subscribe(Hcf::meshDataAttemptCompletedSignal,this);
        at(50,0);at(70,1);at(80,2);at(81,3);at(150,4);
    }
    void receiveSignal(cComponent *,simsignal_t signal,cObject *object,cObject *details) override{
        Enter_Method_Silent();
        if(signal==Hcf::meshDataAttemptCompletedSignal){if(simTime()>SimTime(80,SIMTIME_MS))++samples;return;}
        const Packet *packet=dynamic_cast<const Packet *>(object);
        if(signal==registerSignal("transmissionStarted"))packet=check_and_cast<const Ieee80211Transmission *>(object)->getPacket();
        if(!packet)return;
        std::string name=packet->getName();
        if(name=="freeze-training")return;
        if(signal==registerSignal("packetDropped")&&name=="freeze-third"){
            ASSERT(check_and_cast<PacketDropDetails *>(details)->getReason()==LIFETIME_EXPIRED);
            ASSERT(packet->getTag<Ieee80211MeshAggregationTag>()->getPendingSourceIdentity());++expired;return;
        }
        if(name.find("freeze-")!=0)return;
        if(signal==registerSignal("freezePrepared")){
            ASSERT(name=="freeze-second"&&!prepared&&mac()->lastPreparedIfs==SimTime(16,SIMTIME_US));
            ASSERT(!packet->peekAtFront<Ieee80211DataHeader>()->getMoreData());
            prepared=true;retainedId=packet->getId();plannedStart=simTime()+mac()->lastPreparedIfs;
            frozen=packet->peekAtFront<BytesChunk>(packet->getDataLength()-B(4))->getBytes();
            scheduleAt(simTime()+SimTime(1,SIMTIME_NS),new cMessage("inject-after-acceptance",10));
            scheduleAt(simTime()+SimTime(2,SIMTIME_NS),new cMessage("inspect-accepted-copy",11));
        }
        else if(signal==registerSignal("packetSentToLower")){ASSERT(name!="freeze-third");++sent;}
        else if(signal==registerSignal("packetSentToUpper")){ASSERT(name!="freeze-third");++delivered;}
        else if(signal==registerSignal("transmissionStarted")&&name=="freeze-second"){
            ASSERT(changed&&simTime()==plannedStart);
            auto phy=Ieee80211Radio::peekIeee80211PhyHeaderAtFront(packet);
            ASSERT(packet->peekDataAt<BytesChunk>(phy->getChunkLength(),B(frozen.size()))->getBytes()==frozen);
            ASSERT(!packet->peekDataAt<Ieee80211DataHeader>(phy->getChunkLength())->getMoreData());actual=true;
        }
    }
    void handleMessage(cMessage *event) override{
        int kind=event->getKind();delete event;
        if(kind==0)source("freeze-training",0);
        else if(kind==1)power(1)->requestMode(meshMib(0)->address,MeshPowerMode::LIGHT_SLEEP);
        else if(kind==2){mac()->holdData=true;source("freeze-first",6);source("freeze-second",6);}
        else if(kind==3){mac()->holdData=false;power(0)->opportunity(meshMib(1)->address,SimTime(90,SIMTIME_MS));}
        else if(kind==10){
            ASSERT(check_and_cast<ITx *>(mac()->getSubmodule("tx"))->hasTransmission());
            ASSERT(check_and_cast<IRadio *>(meshInterface(0)->getSubmodule("radio"))->getTransmissionState()!=IRadio::TRANSMISSION_STATE_TRANSMITTING);
            mac()->holdThird=true;source("freeze-third",6);
        }
        else if(kind==11){
            auto original=findOriginal();ASSERT(original&&simTime()<plannedStart);
            ASSERT(original->peekAtFront<BytesChunk>(original->getDataLength()-B(4))->getBytes()==frozen);
            std::unique_ptr<Packet> hypothetical(original->dup());
            power(0)->prepareTransmission(hypothetical.get(),{retainedId});
            ASSERT(hypothetical->peekAtFront<Ieee80211DataHeader>()->getMoreData());changed=true;
        }
        else{ASSERT(prepared&&changed&&actual&&sent==2&&delivered==2&&samples==2&&expired==1);emit(registerSignal("timComplete"),1L);}
    }
};
namespace power_freeze_registration { Define_Module(::PowerFreezeDriver); }
#endif
