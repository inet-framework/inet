// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TEST_MESHPOWERBUFFERSUPPORT_H
#define INET_TEST_MESHPOWERBUFFERSUPPORT_H

class PowerBufferDriver : public cSimpleModule, public cListener
{
    bool ready=false;
    int overflow=0, expired=0, sent=0, samples=0;
    std::set<int64_t> retained;
    int scenario() const { return par("scenario"); }
    Hcf *hcf() { return check_and_cast<Hcf *>(meshInterface(0)->getSubmodule("mac")->getSubmodule("hcf")); }
    TimPowerFixture *power(int s) { return check_and_cast<TimPowerFixture *>(meshInterface(s)->getSubmodule("powerSavePolicy")); }
    void source(const char *name,int dest) {
        sendDirect(fixtureMsdu(name,meshMib(dest)->address),meshInterface(0)->getSubmodule("llc")->gate("lowerLayerOut"));
    }
    void at(simtime_t time,int kind,int priority=0) { auto m=new cMessage("buffer-step",kind);m->setSchedulingPriority(priority);scheduleAt(time,m); }
    std::set<int64_t> payloadIds() {
        std::set<int64_t> result;
        for(const auto& frame:hcf()->getMeshBufferedFrames())
            if(frame.kind==MeshBufferedFrame::Kind::PAYLOAD) result.insert(frame.id);
        return result;
    }
    void initialize() override {
        hcf()->subscribe(registerSignal("packetDropped"),this);
        hcf()->subscribe(Hcf::meshDataAttemptCompletedSignal,this);
        meshInterface(0)->getSubmodule("mac")->subscribe(registerSignal("packetSentToLower"),this);
        at(SimTime(50,SIMTIME_MS),0);at(SimTime(70,SIMTIME_MS),1);at(SimTime(80,SIMTIME_MS),2);
        at(SimTime(81,SIMTIME_MS),3);at(SimTime(82,SIMTIME_MS),4);
        at(scenario()==3?SimTime(5080,SIMTIME_MS):SimTime(100,SIMTIME_MS),5,-1);
        at((scenario()==3?SimTime(5080,SIMTIME_MS):SimTime(100,SIMTIME_MS))+SimTime(1,SIMTIME_NS),6);
        at(scenario()==3?SimTime(5150,SIMTIME_MS):SimTime(110,SIMTIME_MS),7);
    }
    void receiveSignal(cComponent *source,simsignal_t signal,cObject *object,cObject *details) override {
        Enter_Method_Silent(); if(!ready)return;
        if(signal==Hcf::meshDataAttemptCompletedSignal) {++samples;return;}
        auto packet=dynamic_cast<Packet *>(object);if(!packet)return;
        auto name=std::string(packet->getName());
        if(source==hcf()&&signal==registerSignal("packetDropped")) {
            auto drop=check_and_cast<PacketDropDetails *>(details);
            if(drop->getReason()==QUEUE_OVERFLOW) {ASSERT(name=="buffer-overflow-new");++overflow;}
            if(drop->getReason()==LIFETIME_EXPIRED) ++expired;
        }
        if(signal==registerSignal("packetSentToLower") &&
                (name.find("buffer-")==0||name.find("Ieee80211MeshLinkMetricFrame")!=std::string::npos))++sent;
    }
    void handleMessage(cMessage *message) override {
        int kind=message->getKind();delete message;
        int count=scenario()==1?64:2;
        if(kind==0){source("training-b",1);source("training-c",2);}
        else if(kind==1){
            power(1)->requestMode(meshMib(0)->address,MeshPowerMode::DEEP_SLEEP);
            power(2)->requestMode(meshMib(0)->address,MeshPowerMode::DEEP_SLEEP);
        }
        else if(kind==2){
            ASSERT(meshMib(0)->findMeshPeerPowerState(meshMib(1)->address)->remote==MeshPowerMode::DEEP_SLEEP);
            ASSERT(meshMib(0)->findMeshPeerPowerState(meshMib(2)->address)->remote==MeshPowerMode::DEEP_SLEEP);
            ready=true;
            if(scenario()==3)check_and_cast<TimManagementFixture *>(meshManagement(0))->queueReport(meshMib(1)->address);
            else for(int i=0;i<count;++i)source(("buffer-old-"+std::to_string(i)).c_str(),1+i%2);
        }
        else if(kind==3){
            if(scenario()!=3){
                retained=payloadIds();ASSERT(retained.size()==size_t(count));
                b length(0);for(const auto& frame:hcf()->getMeshBufferedFrames())if(retained.count(frame.id))length+=frame.length;
                ASSERT(length==B(49*count));
                source("buffer-overflow-new",2);
            }
        }
        else if(kind==4){if(scenario()!=3){ASSERT(overflow==1&&payloadIds()==retained);}}
        else if(kind==5){
            ASSERT(sent==0&&samples==0&&expired==0);
            if(scenario()==3){for(const auto& frame:hcf()->getMeshBufferedFrames())ASSERT(frame.kind!=MeshBufferedFrame::Kind::MANAGEMENT);}
            else ASSERT(payloadIds().empty()); // exact deadline before lifetime timer dispatch
        }
        else if(kind==6){
            ASSERT(expired==(scenario()==3?1:count));
            if(scenario()==3){
                check_and_cast<TimManagementFixture *>(meshManagement(0))->queueReport(meshMib(1)->address);
                power(0)->opportunity(meshMib(1)->address,SimTime(5140,SIMTIME_MS));
            }
        }
        else {
            ASSERT(samples==0&&sent==(scenario()==3?1:0));
            if(scenario()!=3)ASSERT(payloadIds().empty()&&overflow==1);
            emit(registerSignal("timComplete"),1L);
        }
    }
};
namespace power_buffer_registration { Define_Module(::PowerBufferDriver); }
#endif
