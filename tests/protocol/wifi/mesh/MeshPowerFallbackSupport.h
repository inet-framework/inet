// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TEST_MESHPOWERFALLBACKSUPPORT_H
#define INET_TEST_MESHPOWERFALLBACKSUPPORT_H
class PowerFallbackDriver : public cSimpleModule, public cListener
{
    int actual=0, delivered=0, counts=0;
    std::vector<uint32_t> identities;
    bool armed=false;
    PowerHoldMac *mac(){return check_and_cast<PowerHoldMac *>(meshInterface(0)->getSubmodule("mac"));}
    Hcf *hcf(){return check_and_cast<Hcf *>(mac()->getSubmodule("hcf"));}
    AwakePowerFixture *power(){return check_and_cast<AwakePowerFixture *>(meshInterface(0)->getSubmodule("powerSavePolicy"));}
    PowerForwardingFixture *forwarding(){return check_and_cast<PowerForwardingFixture *>(mac()->getSubmodule("ds"));}
    void source(const char *name,int size){
        sendDirect(phyMsdu(name,meshMib(1)->address,size-7),meshInterface(0)->getSubmodule("llc")->gate("lowerLayerOut"));
    }
    void at(int ms,int kind){scheduleAt(SimTime(ms,SIMTIME_MS),new cMessage("fallback-step",kind));}
    void initialize() override {
        mac()->subscribe(registerSignal("packetSentToLower"),this);
        meshInterface(1)->getSubmodule("mac")->subscribe(registerSignal("packetSentToUpper"),this);
        hcf()->subscribe(Hcf::meshPayloadTransmittedSignal,this);
        at(5,0);at(50,1);at(100,2);at(108,6);at(115,3);at(116,4);at(140,5);
    }
    void receiveSignal(cComponent *source,simsignal_t signal,cObject *object,cObject *details) override {
        Enter_Method_Silent();
        auto packet=dynamic_cast<Packet *>(object);if(!packet||!armed||std::string(packet->getName()).find("fallback-power-")!=0)return;
        if(signal==registerSignal("packetSentToUpper")){++delivered;return;}
        if(signal==Hcf::meshPayloadTransmittedSignal){
            ASSERT(check_and_cast<MeshPayloadTransmission *>(details)->msduCount==1);++counts;return;
        }
        if(source==mac()){
            auto header=packet->peekAtFront<Ieee80211MeshDataHeader>();
            ASSERT(!header->getAMsduPresent()&&header->getPowerMgmt()&&header->getMeshPowerSaveLevel());
            ASSERT(header->getMoreData()==(actual==0));
            ASSERT(packet->getTag<Ieee80211ModeReq>()->getMode()->getHtMcsIndex()==0);
            identities.push_back(packet->peekDataAt<Ieee80211MeshControl>(header->getChunkLength())->getSequenceNumber());
            ++actual;
        }
    }
    void handleMessage(cMessage *event) override {
        int kind=event->getKind();delete event;
        if(kind==0)meshManagement(0)->openPeer(meshMib(1)->address);
        else if(kind==1)source("fallback-training",7);
        else if(kind==2){emit(registerSignal("powerStart"),1L);power()->requestMode(meshMib(1)->address,MeshPowerMode::DEEP_SLEEP);}
        else if(kind==6){check_and_cast<AwakePowerFixture *>(meshInterface(1)->getSubmodule("powerSavePolicy"))->requestMode(meshMib(0)->address,MeshPowerMode::LIGHT_SLEEP);}
        else if(kind==3){
            ASSERT(meshMib(0)->findMeshPeerPowerState(meshMib(1)->address)->local==MeshPowerMode::DEEP_SLEEP);
            mac()->holdData=true;source("fallback-power-1",1000);source("fallback-power-2",1000);
        }
        else if(kind==4){
            auto edca=check_and_cast<Edca *>(hcf()->getSubmodule("edca"));
            auto queue=edca->getEdcaf(AC_BE)->getPendingQueue();
            ASSERT(queue->getNumPackets()==2&&forwarding()->nextIdentity()==1);
            mac()->holdData=false;
            power()->opportunity(meshMib(1)->address,SimTime(130,SIMTIME_MS));
            auto service=check_and_cast<IOriginatorQosMacDataService *>(hcf()->getSubmodule("originatorMacDataService"));
            auto preview=service->prepareFrame(queue,true);
            ASSERT(preview&&preview->aggregate&&preview->sourceIds.size()==2);
            ASSERT(forwarding()->nextIdentity()==1&&queue->getNumPackets()==2);
            preview.reset();armed=true;hcf()->meshPowerEligibilityChanged();
        }
        else{
            ASSERT(actual==2&&delivered==2&&counts==2&&identities.size()==2);
            ASSERT(identities[0]==1&&identities[1]==2&&forwarding()->nextIdentity()==3);
            emit(registerSignal("powerComplete"),1L);
        }
    }
};
namespace power_fallback_registration { Define_Module(::PowerFallbackDriver); }
#endif
