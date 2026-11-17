// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TEST_MESHPOWERCOLLISIONSUPPORT_H
#define INET_TEST_MESHPOWERCOLLISIONSUPPORT_H
class PowerCollisionDriver : public cSimpleModule, public cListener
{
    int dataPpdus=0,nullPpdus=0,delivered=0,samples=0;
    std::vector<uint8_t> body;
    PowerHoldMac *mac(){return check_and_cast<PowerHoldMac *>(meshInterface(0)->getSubmodule("mac"));}
    Hcf *hcf(){return check_and_cast<Hcf *>(mac()->getSubmodule("hcf"));}
    AwakePowerFixture *power(){return check_and_cast<AwakePowerFixture *>(meshInterface(0)->getSubmodule("powerSavePolicy"));}
    Edcaf *owner(){return check_and_cast<Edca *>(hcf()->getSubmodule("edca"))->getEdcaf(AC_BE);}
    Packet *original(Ieee80211FrameType type){
        auto frames=owner()->getInProgressFrames();
        for(int i=0;i<frames->getLength();++i)if(frames->getFrames(i)->peekAtFront<Ieee80211MacHeader>()->getType()==type)return frames->getFrames(i);
        return nullptr;
    }
    int retries(Packet *packet){ASSERT(packet);return owner()->getRecoveryProcedure()->getRetryCount(packet,packet->peekAtFront<Ieee80211DataHeader>());}
    void at(simtime_t time,int kind){scheduleAt(time,new cMessage("namespace-live-step",kind));}
    void initialize() override{
        auto radio=meshInterface(0)->getSubmodule("radio");
        radio->subscribe(registerSignal("transmissionStarted"),this);radio->subscribe(registerSignal("transmissionEnded"),this);
        meshInterface(1)->getSubmodule("mac")->subscribe(registerSignal("packetSentToUpper"),this);
        hcf()->subscribe(Hcf::meshDataAttemptCompletedSignal,this);
        at(SimTime(5,SIMTIME_MS),0);at(SimTime(100,SIMTIME_MS),1);at(SimTime(105,SIMTIME_MS),2);
        at(SimTime(106500,SIMTIME_US),3);at(SimTime(108,SIMTIME_MS),4);at(SimTime(110,SIMTIME_MS),5);at(SimTime(140,SIMTIME_MS),6);
    }
    void receiveSignal(cComponent *,simsignal_t signal,cObject *object,cObject *) override{
        Enter_Method_Silent();
        if(signal==Hcf::meshDataAttemptCompletedSignal){++samples;return;}
        if(signal==registerSignal("packetSentToUpper")){
            auto packet=check_and_cast<Packet *>(object);
            ASSERT(std::string(packet->getName())!="mesh-power-mode");
            if(std::string(packet->getName())=="collision-data")++delivered;
            return;
        }
        auto transmission=check_and_cast<const Ieee80211Transmission *>(object);auto packet=transmission->getPacket();
        auto name=std::string(packet->getName());if(name!="collision-data"&&name!="mesh-power-mode")return;
        if(signal==registerSignal("transmissionEnded")){
            if(name=="collision-data"&&dataPpdus==1)mac()->holdData=true;
            if(name=="mesh-power-mode"&&nullPpdus==1)mac()->holdNull=true;
            return;
        }
        auto phy=Ieee80211Radio::peekIeee80211PhyHeaderAtFront(packet);
        auto header=packet->peekDataAt<Ieee80211DataHeader>(phy->getChunkLength());
        ASSERT(header->getSequenceNumber().get()==0&&header->getTid()==0);
        if(name=="collision-data"){
            ASSERT(header->getRetry()==(dataPpdus!=0));
            auto bytes=packet->peekDataAt<BytesChunk>(phy->getChunkLength()+header->getChunkLength(),B(13))->getBytes();
            if(dataPpdus==0)body=bytes;else ASSERT(body==bytes);
            ++dataPpdus;
        }
        else ++nullPpdus;
    }
    void handleMessage(cMessage *message) override{
        int kind=message->getKind();delete message;
        if(kind==0)meshManagement(0)->openPeer(meshMib(1)->address);
        else if(kind==1){
            emit(registerSignal("powerStart"),1L);
            sendDirect(phyMsdu("collision-data",meshMib(1)->address),meshInterface(0)->getSubmodule("llc")->gate("lowerLayerOut"));
        }
        else if(kind==2){ASSERT(dataPpdus==1&&retries(original(ST_DATA_WITH_QOS))==1);power()->requestMode(meshMib(1)->address,MeshPowerMode::DEEP_SLEEP);}
        else if(kind==3){
            auto data=original(ST_DATA_WITH_QOS),null=original(ST_QOS_NULL);
            ASSERT(dataPpdus==1&&nullPpdus==1&&retries(data)==1&&retries(null)==1);
            power()->requestMode(meshMib(1)->address,MeshPowerMode::ACTIVE);
            ASSERT(retries(data)==1);
            mac()->holdNull=false;hcf()->meshPowerEligibilityChanged();
        }
        else if(kind==4){ASSERT(nullPpdus==2&&retries(original(ST_DATA_WITH_QOS))==1);}
        else if(kind==5){mac()->holdData=false;hcf()->meshPowerEligibilityChanged();}
        else{
            ASSERT(dataPpdus==2&&nullPpdus==2&&samples==2&&delivered==1&&!original(ST_DATA_WITH_QOS)&&!original(ST_QOS_NULL));
            ASSERT(check_and_cast<AirtimeLinkMetric *>(meshInterface(0)->getSubmodule("linkMetric"))->getErrorEstimate(meshMib(1)->address)==7.0/64);
            ASSERT(check_and_cast<PowerForwardingFixture *>(mac()->getSubmodule("ds"))->nextIdentity()==1);
            emit(registerSignal("powerComplete"),1L);
        }
    }
};
namespace power_collision_registration { Define_Module(::PowerCollisionDriver); }
#endif
