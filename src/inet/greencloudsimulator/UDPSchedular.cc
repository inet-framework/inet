//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "UDPSchedular.h"
#include "inet/applications/base/ApplicationPacket_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include <vector>
#include "CloudTask_m.h"
#include "Tks.h"
#include <iostream>
//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004,2011 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

namespace inet {
namespace greencloudsimulator {

Define_Module(UDPSchedular);

int UDPSchedular::counter;
int UDPSchedular::recv_counter=0;

simsignal_t UDPSchedular::sentPkSignal = SIMSIGNAL_NULL;
simsignal_t UDPSchedular::rcvdPkSignal = SIMSIGNAL_NULL;

void UDPSchedular::initialize(int stage)
{
    EV_INFO<<"*************UDP UDPSchedular LAYER 2***************"<< endl;
    UdpBasicApp::initialize(stage);
    // because of L3AddressResolver, we need to wait until interfaces are registered,
    // address auto-assignment takes place etc.
    if (stage != 3)
        return;

//    filepointer.open("eConsumedServer11.csv");
  //  delayptr.open("1Mbps-8nodes.csv");
    //mipsCounter =0;
    counter = 0;
    numSent = 0;
    numReceived = 0;
    WATCH(numSent);
    WATCH(numReceived);
    sentPkSignal = registerSignal("sentPk");
    rcvdPkSignal = registerSignal("rcvdPk");
    esignal = registerSignal("arrival");

   // EV_INFO<<"address received are "<<destAddrs<<endl;
    //const char *destAddrs = "192.168.0.1";
    //const char *destAddrs = "ServerNode[15]";
    this->myAddr = L3AddressResolver().resolve(this->getParentModule()->getFullPath().c_str());

    DisplayVectorTable();
    L3Address addr = RoundRobin();
    EV_INFO<<"The first node available is :"<<addr<<endl;
    setMyID();
    //EV_INFO<<"Id "<<this->myId<<" IP: "<<myAddr<<endl;

    //string str(L3AddressResolver().resolve());

    //string str = this->getParentModule()->getFullPath().c_str();
    string sst= myAddr.str();
    EV_INFO<<"MY IP In string "<<sst;

    nominal_mips_ = 10000;
    eNominalrate_ = 130.0;
    currentLoad_ = 0;
    eConsumed_ = 0.0;          /* total W of energy consumed */
    eNominalrate_ = 130.0;           /* nominal consumption rate at full load at max CPU frequency */
    eCurrentConsumption_ = 0.0;        /* current consumption rate */
    eDVFS_enabled_ = true;
    eDNS_enabled_ = true;

    //if(this->getIndex()==0)
    //{
    setMyID();
    //if(this->myId=="TskGen[0]")
    //{
        EV_INFO<<"Scheduling INIT  tsk"<<endl;
   //     scheduleAt(startTime, timerMsg);
    //}
        filepointer<< this->getFullPath()<<"\n";
    EV_INFO<<"********************"<<endl;
    EV_INFO<<this->getFullPath()<<" My id "<<this->myId<<endl;
    EV_INFO<<"********************"<<endl;
//     EV_INFO<<this->getFullName() << "........$$$$$ SCHEDULED CMESSAGE AT "<<size() << " "<<startTime<<endl;
    //}
    //sendPacket();
    status_ = EVENT_IDLE;



}

void UDPSchedular::processStart() {
    UdpBasicApp::processStart();

    for (auto elem : destAddresses) {
        if (elem != myAddr)
            vlist.push_back(IPMapper(elem,0,elem));
    }
    if (destAddresses.empty()) {
        throw cRuntimeError("DESTINATION ADDRESS IS EMPTY ");
        EV_INFO<<this->getFullName()<<"DESTINATION ADDRESS IS EMPTY "<<endl;;
        return;
    }
    else  {
        EV_INFO<<"-------------------vector table------------------------------"<<endl;
        DisplayListofDestIP();
        EV_INFO<<"------------------------- table------------------------------"<<endl;
    }
}

void UDPSchedular::DisplayVectorTable()
{
    EV_INFO<<"-------------------Scheduling Table------------------------------"<<endl;
    for(std::vector<IPMapper>::iterator it=vlist.begin();it!=vlist.end();++it)
    {
        it->display();
    }
    EV_INFO<<"------------------- table------------------------------"<<endl;

}

L3Address UDPSchedular::FCFS_ComputeNode()
{
    if(!vlist.empty()) {
        int temp= vlist_indexer;
        vlist_indexer++;
        vlist_indexer = vlist_indexer % vlist.size();
        return vlist[temp].getComputingNodeIP();

    }
    return L3Address();
}


L3Address UDPSchedular::RoundRobin()
{
    for(const auto & elem : vlist) {
        if(elem.getTskID()==0) {
            return elem.getComputingNodeIP();
        }
    }
    return L3Address();
}


void UDPSchedular::ResetVirtualTable()
{
    for( auto & elem : vlist) {
        EV_INFO<<"Updating table values"<<endl;
        elem.setTskID(0);
        //vlist[i].setSrcID(dest);
    }
//    for(int i=0;i<vlist.size();i++)
//    {
//        EV_INFO<<"Updating table values"<<endl;
//        vlist[i].setTskID(0);
//        //vlist[i].setSrcID(dest);
//    }
}
bool UDPSchedular::UpdateVirtualTable(L3Address src, int taskId, L3Address dest)
{
    for(auto & elem : vlist) {
        if(0 == elem.getTskID()) {
            EV_INFO<<"Updating table values"<<endl;
            elem.setTskID(taskId);
            elem.setSrcID(dest);
            return true;
        }
    }
    return false;
}

UDPSchedular::UDPSchedular() {
    EV_INFO << "CONST CALLED" << endl;
    nominal_mips_ = 10000;
    eNominalrate_ = 130.0;
    currentLoad_ = 0;
    eConsumed_ = 0.0; /* total W of energy consumed */
    eNominalrate_ = 130.0; /* nominal consumption rate at full load at max CPU frequency */
    eCurrentConsumption_ = 0.0; /* current consumption rate */
    eDVFS_enabled_ = true;
    eDNS_enabled_ = false;
    queue_type = UDPSchedular::SJF;
    scheduling_task.clear();
    vlist_indexer = 0;
}

void UDPSchedular::setMyID() {
    std::string strr = (std::string) this->getFullPath();
    const char *c = strr.c_str();
    cStringTokenizer tokenizer(c, ".");
    const char *token;
    int cp = 0;
    while ((token = tokenizer.nextToken()) != NULL) {
        if (cp == 1) {
            this->myId = token;

        }
        cp++;
    }
}

void UDPSchedular::finish()
{
    recordScalar("packets sent", numSent);
    recordScalar("packets received", numReceived);
    filepointer.close();
    delayptr.close();
    //tasks_list_.~vector();
}

void UDPSchedular::DisplayListofDestIP()
{

    int k = destAddresses.size();
    for(int i=0;i<k;i++)
    {
        EV_INFO<<"Dest IP List at : "<< destAddresses[i]<<endl;
    }
}

Packet *UDPSchedular::createCloudTaskPacket()
{

    char msgName[32];
    sprintf(msgName, "T-%d", ++counter);
    auto tsk = makeShared<CloudTask>();
    auto packet = new Packet(msgName);
    packet->setSrcProcId(this->getIndex());
    tsk->setChunkLength(B(par("messageLength").intValue()));
    tsk->setMpis_(1000);
    tsk->setSize_(0.1);
    tsk->setDeadline_(1); // START TIME + SIM DURATION
    tsk->setOutput_(0);
    tsk->setIntercom_(0);
    tsk->setCurrProcRate_(0);
    tsk->setExecutedSince_(0);
    packet->setTimestamp(simTime());// added later
    packet->insertAtFront(tsk);
    return packet;
}

Packet *UDPSchedular::CopyCloudTaskPacket(Packet *prev)
{
    char msgName[32];
    sprintf(msgName, "T-%d", ++counter);

    const auto &prevTsk = prev->peekAtFront<CloudTask>();

    auto tsk = makeShared<CloudTask>();
    auto packet = new Packet(msgName);

    tsk->setPacketId(prevTsk->getPacketId());/* packet id contain the counter */
    packet->setSrcProcId(prev->getSrcProcId());
    tsk->setChunkLength(B(par("messageLength").intValue()));
    tsk->setMpis_(prevTsk->getMpis_());
    tsk->setSize_(prevTsk->getSize_());
    tsk->setDeadline_(prevTsk->getDeadline_()); // START TIME + SIM DURATION
    tsk->setOutput_(0);
    tsk->setIntercom_(0);
    tsk->setCurrProcRate_(0);
    tsk->setExecutedSince_(0);
    packet->setTimestamp(simTime());// added later
    packet->insertAtFront(tsk);
    return packet;
}

Packet *UDPSchedular::createPacket()
{
    char msgName[32];
    //sprintf(msgName, "UDPBAppData-%d", counter++);

    sprintf(msgName, "T-%d", ++counter);
    //cPacket *payload = new cPacket(msgName);
    Packet *packet = new Packet(msgName);
    const auto& payload = makeShared<ApplicationPacket>();
    payload->setChunkLength(B(par("messageLength")));
    payload->setSequenceNumber(numSent);
    packet->insertAtFront(payload);
    //payload->setByteLength(1048576);// in bytes
    //payload->addBitLength(8388608); // this mean 1MB data

    packet->setSrcProcId(this->getIndex());

    packet->addPar("size_");
    packet->par("size_").setLongValue(1);
    packet->addPar("mips_");
    packet->par("mips_").setLongValue(11);
    packet->addPar("deadline_");
    packet->par("deadline_").setLongValue(2);
    packet->addPar("output_");
    packet->par("output_").setLongValue(22);
    packet->addPar("intercom_");
    packet->par("intercom_").setLongValue(222);
    packet->addPar("currProcRate_");
    packet->par("currProcRate_").setLongValue(33);
    packet->addPar("executedSince_");
    packet->par("executedSince_").setLongValue(333);

    return packet;
}

void UDPSchedular::displayAttachParameters(Packet *pk)
{
    if(pk->hasPar("size_"))
    {
        long ss = pk->par("size_").longValue();
        EV_INFO<<"SIZE VALUE IS "<<ss<<endl;
    }

    if(pk->hasPar("mips_"))
    {
        long se = pk->par("mips_").longValue();
        EV_INFO<<"MIPS VALUE IS "<<se<<endl;
    }

}

Packet * UDPSchedular::attachTaskParameters(Packet *pk)
{
    pk->addPar("size_");
    pk->par("size_").setLongValue(1);
    pk->addPar("mips_");
    pk->par("mips_").setLongValue(11);
    pk->addPar("deadline_");
    pk->par("deadline_").setLongValue(1);
    pk->addPar("output_");
    pk->par("output_").setLongValue(22);
    pk->addPar("intercom_");
    pk->par("intercom_").setLongValue(222);
    pk->addPar("currProcRate_");
    pk->par("currProcRate_").setLongValue(33);
    pk->addPar("executedSince_");
    pk->par("executedSince_").setLongValue(333);

    return pk;

}

void UDPSchedular::sendPacket()
{
    //cPacket *payload = createPacket();
    auto payload = createCloudTaskPacket();
    //cPacket *payload = createCloudTaskPacket();
    L3Address destAddr = chooseDestAddr();
    EV_INFO<<"**********************************Packet Generated*****************************"<<endl;
    EV_INFO<<"Sim_Time=" <<simTime()<< "  Src :" << this->myAddr <<"  Destination : "<<destAddr<< " TaskNum "<<counter<<endl;
    EV_INFO<<"********************************************************************************"<<endl;
    emit(sentPkSignal, payload);
    socket.sendTo(payload, destAddr, destPort);
    numSent++;
}


void UDPSchedular::PacketConversion(Packet *msg) {

    if (scheduling_task.empty()) {
        simtime_t d = simTime() + par("scheduleDelay");  //5  //par("sendInterval").doubleValue();
        scheduleAt(d, msg);
    }
    ntasks_++;
    auto packet = check_and_cast<Packet *>(msg);
    const auto & tsk = packet->peekAtFront<CloudTask>();
    if (tsk == nullptr)
        throw cRuntimeError("Header error");
    scheduling_task.push_back(packet);  // add to the active tasks links

}


void UDPSchedular::DecreasingTimeAlgo()
{
    if(!scheduling_task.empty())
    {
        double mips_required=-1;
        int index=-1;

        for(int i = 0 ; i < (int)scheduling_task.size();i++)
        {
            const auto & tmpt_ = scheduling_task[i]->peekAtFront<CloudTask>();
            //CloudTask *tmpt_ = check_and_cast<CloudTask *>(scheduling_task[t]);
            if(mips_required < tmpt_->getMpis_())
            {
                mips_required = tmpt_->getMpis_();
                index = i;
            }
        }
        cout<<"Queue Size at Schedular is "<<scheduling_task.size()<<endl;

        auto packet_ = scheduling_task[index];
        auto payload = CopyCloudTaskPacket(packet_);

        L3Address freeNode = FCFS_ComputeNode();
        L3Address localAddress = L3AddressResolver().resolve("127.0.0.1");
        if(freeNode==localAddress || freeNode.isUnspecified())
        {
             cout<<"IP Address list is empty ";
             exit(0);
        }
        const auto &aux = payload->peekAtFront<CloudTask>();
        cout<<"Schedular sending Task # "<<aux->getPacketId()<<" MIPS "<< aux->getMpis_() <<" to compute Node IP# "<<freeNode<<endl;
        emit(sentPkSignal, payload);
        cout<<"Task sending to Node "<<freeNode<<endl;
        socket.sendTo(payload, freeNode, destPort);
        numSent++;
        scheduling_task.erase(scheduling_task.begin() + index); /* remove first task from the queue"*/

        if(!scheduling_task.empty())
        {
            simtime_t d = simTime() + par("scheduleDelay"); // 5; //par("sendInterval").doubleValue();
            auto sch_task = scheduling_task[0];
            const auto & tsk = sch_task->peekAtFront<CloudTask>();
            if (tsk == nullptr)
                throw cRuntimeError("Header error");
            // CloudTask *sch_task = check_and_cast<CloudTask *>(scheduling_task[0]);
            scheduleAt(d, sch_task);
        }
    }

}
void UDPSchedular::ShortestJobFirst()
{
    if(!scheduling_task.empty())
    {
        double mips_required=DBL_MAX;
        int index=-1;

        for(int t=0;t<scheduling_task.size();t++)
        {
            auto packet = scheduling_task[t];
            const auto & tmpt_ = packet->peekAtFront<CloudTask>();
            if(mips_required > tmpt_->getMpis_())
            {
                mips_required = tmpt_->getMpis_();
                index =t;
            }

        }
        cout<<"Queue Size at Schedular is "<<scheduling_task.size()<<endl;
        auto packet_ = scheduling_task[index];
        auto payload = CopyCloudTaskPacket(packet_);

        L3Address freeNode = FCFS_ComputeNode();
        L3Address localAddress = L3AddressResolver().resolve("127.0.0.1");
        if(freeNode==localAddress || freeNode.isUnspecified())
        {
             cout<<"IP Address list is empty ";
             exit(0);
        }

        const auto &aux = payload->peekAtFront<CloudTask>();
        cout<<"Schedular sending Task # "<<aux->getPacketId()<<" MIPS "<< aux->getMpis_() <<" to compute Node IP# "<<freeNode<<endl;
        emit(sentPkSignal, payload);
        cout<<"Task sending to Node "<<freeNode<<endl;
        socket.sendTo(payload, freeNode, destPort);
        numSent++;
        if (index >=0) {
            delete scheduling_task[index];
            scheduling_task.erase(scheduling_task.begin() + index); /* remove first task from the queue"*/
        }

        if(!scheduling_task.empty())
        {
            simtime_t d = simTime() + par("scheduleDelay"); // 5;//par("sendInterval").doubleValue();
            auto sch_task = scheduling_task[0];
            scheduleAt(d, sch_task);
        }
    }

}

void UDPSchedular::LastInFirstOut()
{
    if(!scheduling_task.empty())
    {
        auto packet_ = scheduling_task[scheduling_task.size()-1];
        auto payload = CopyCloudTaskPacket(packet_);

        L3Address freeNode = FCFS_ComputeNode();
        L3Address localAddress = L3AddressResolver().resolve("127.0.0.1");
        if(freeNode==localAddress || freeNode.isUnspecified())
        {
             cout<<"IP Address list is empty ";
             exit(0);
        }
        const auto &aux = payload->peekAtFront<CloudTask>();
        cout<<"Schedular sending Task # "<<aux->getPacketId()<<" to compute Node IP# "<<freeNode<<endl;
        emit(sentPkSignal, payload);
        cout<<"Task sending to Node "<<freeNode<<endl;
        socket.sendTo(payload, freeNode, destPort);
        numSent++;
        scheduling_task.erase(scheduling_task.begin()); /* remove first task from the queue"*/

        if(!scheduling_task.empty())
        {
            simtime_t d = simTime() + par("lastInFirstOutDelay");// 1;//par("sendInterval").doubleValue();
            auto sch_task = scheduling_task[0];
            scheduleAt(d, sch_task);
        }
    }

}


void UDPSchedular::FirstComeFirstServe()
{
    if(!scheduling_task.empty())
    {
        auto packet_ = scheduling_task[0];
        auto payload = CopyCloudTaskPacket(packet_);

        L3Address freeNode = FCFS_ComputeNode();
        L3Address localAddress = L3AddressResolver().resolve("127.0.0.1");
        if(freeNode==localAddress || freeNode.isUnspecified())
        {
             cout<<"IP Address list is empty ";
             exit(0);
        }
        const auto &aux = payload->peekAtFront<CloudTask>();
        cout<<"Schedular sending Task # "<<aux->getPacketId()<<" to compute Node IP# "<<freeNode<<endl;
        emit(sentPkSignal, payload);
        cout<<"Task sending to Node "<<freeNode<<endl;
        socket.sendTo(payload, freeNode, destPort);
        numSent++;
        scheduling_task.erase(scheduling_task.end()); /* remove first task from the queue"*/

        if(!scheduling_task.empty())
        {
            simtime_t d = simTime() + par("lastInFirstOutDelay"); // 1;//par("sendInterval").doubleValue();
            auto sch_task = scheduling_task[0];
            scheduleAt(d, sch_task);
        }
    }
}


void UDPSchedular::socketDataArrived(UdpSocket *socket, Packet *packet)
{
    // process incoming packet
    recv_counter++;
    PacketConversion(packet);
}

#if 0
void UDPSchedular::handleMessage(cMessage *msg)
{
    // http://inet.omnetpp.org/doc/INET/neddoc/index.html

     std::cout<<" Schedular Handle Message Function "<<endl;
     if (hasGUI())
       {
           char buf[40];
           sprintf(buf, "rcvd: %d pks, sent: %d pks \n eConsumed (kW*h): %e", numReceived, numSent,(eConsumed_/1000));
           getDisplayString().setTagArg("t", 0, buf);

       }
    if (msg->isSelfMessage())
    {
        if(queue_type==UDPSchedular::FCFS)
        {
            FirstComeFirstServe();
        }
        else if(queue_type==UDPSchedular::SJF)
        {
            ShortestJobFirst();
        }
        else if(queue_type==UDPSchedular::DTA)
        {
            DecreasingTimeAlgo();
        }
        else if(queue_type==UDPSchedular::LIFO)
        {
            LastInFirstOut();
        }


        return;
        sendPacket();
        simtime_t d = simTime() + par("sendInterval").doubleValue();
        if ((stopTime == 0 || d < stopTime))
        {
                scheduleAt(d, msg);

        }

        else
            delete msg;

    }
    else if (msg->getKind() == UDP_I_DATA)
    {
        recv_counter++;

        PacketConversion(msg);
        return;



/* BELOW CODE NOT IN USE */
        CloudTask * tlk = check_and_cast<CloudTask *>(msg);
        if(tlk->getIsUser())
        {
     //       EV_INFO<<"Src node address is "<<tlk->getSrcaddress()<<endl;
            rtsk.push(msg);
            L3Address freeNode = RoundRobin();
    //        EV_INFO<<"Free node available is :: "<<freeNode<<endl;
            L3Address localAddress = L3AddressResolver().resolve("127.0.0.1");
            if(freeNode==localAddress || freeNode.isUnspecified())
            {
                bubble("No Compt ");
     //           EV_INFO<<"No node available, Queue size is :"<<rtsk.size()<<endl;
                ResetVirtualTable();
            }
             freeNode = RoundRobin();
      //       EV_INFO<<"Scheduling packet :: "<<freeNode<<endl;
             cMessage *pp = rtsk.front();
             CloudTask *tlk = check_and_cast<CloudTask *>(pp);
             //cMessage *pqt = PK(pp);
             UDPDataIndication *ctrl = check_and_cast<UDPDataIndication *>(pp->getControlInfo());
             L3Address srcAddr = ctrl->getSrcAddr();

             L3Address destAddr = ctrl->getDestAddr();
             bubble("Compt ");
             if(UpdateVirtualTable(srcAddr,1,destAddr))
             {
                    rtsk.pop();
                    CloudTask *payload = createCloudTaskPacket();
                    socket.sendTo(payload, freeNode, destPort);
                    DisplayVectorTable();
             }
             else
             {

      //              EV_INFO<<"Queue size is :"<<rtsk.size()<<endl;
             }

        }
        else
        {
            DisplayVectorTable();
            for(std::vector<IPMapper>::iterator it=vlist.begin();it!=vlist.end();++it)
            {
                       if(it->getComputingNodeIP().str()==tlk->getSrcaddress())
                       {
        //                   EV_INFO<<"Setting tsk id 0"<<endl;
                           it->setTskID(0);
                           break;

                       }

            }
            DisplayVectorTable();
          //  EV_INFO<<" Compute node reply received....!"<<tlk->getSrcaddress()<<endl;
        }

          //  CloudTask *tt = rtsk.pop();

         //


//        return;



        cMessage *pqt = PK(msg);
        UDPDataIndication *ctrl = check_and_cast<UDPDataIndication *>(pqt->getControlInfo());
        L3Address srcAddr = ctrl->getSrcAddr();
        L3Address destAddr = ctrl->getDestAddr();

   //     EV_INFO<<"Schedular:: Received Packet "<<endl;
   //     EV_INFO<<"Sim_Time=" <<simTime()<< "  Src :" << srcAddr <<"  Destination : "<<destAddr<<endl;
   //     delayptr << tlk->getId() << " , "<< simTime() - tlk->getTimestamp()<<endl;
   //     std::cout<<"delay "<<simTime() - tlk->getTimestamp()<<endl;
   //     EV_INFO<<"******************************************************************************"<<endl;
        cloudTask *tpk = new cloudTask();
        tpk->setMIPS(tlk->getMpis_());
        tpk->setSize(tlk->getSize_());
        tpk->setIntercom(tlk->getIntercom_());
        tpk->setExecTime(tlk->getExecutedSince_());
        simtime_t d = simTime();
        tpk->setComputingRate(tlk->getCurrProcRate_(),d.dbl());
        tpk->setDeadline(tlk->getDeadline_());
        ntasks_++;
        tasks_list_.push_back(tpk);  // add to the active tasks links
       // tlk->setExecutedSince_(simTime().dbl());
        processPacket(PK(msg));

        /* error if I uncomment this function I guess it is when to remove
         * from the task list ------ > */updateTskList();
        /* update energy */
        updateTskComputingRates();
        eUpdate(); /* update energy for the last interval */
        setCurrentConsumption(); /* update current energy consumption */
        if (hasGUI())
        {
            bubble("Recv");
        }



 //       hopCountVector.record(ctrl->getTtl());
  //      hopCountStats.collect(ctrl->getTtl());
    }
    else if (msg->getKind() == UDP_I_ERROR)
    {
        EV << "Ignoring UDP error report\n";
        delete msg;
    }
    else
    {
        error("Unrecognized message (%s)%s", msg->getClassName(), msg->getName());
    }


}
#endif

void UDPSchedular::handleMessageWhenUp(cMessage *msg)
{
    if (hasGUI())  {
        char buf[140];
        sprintf(buf, "rcvd: %d pks, sent: %d pks \n eConsumed (kW*h): %e", numReceived, numSent,(eConsumed_/1000));
        getDisplayString().setTagArg("t", 0, buf);
    }
    if (msg->isSelfMessage()) {

        auto pkt = dynamic_cast<Packet *> (msg);
        auto clod = dynamic_cast<cloudTask *>(msg);
        if (clod != nullptr || pkt != nullptr) {
            if(queue_type==UDPSchedular::FCFS)
            {
                FirstComeFirstServe();
            }
            else if(queue_type==UDPSchedular::SJF)
            {
                ShortestJobFirst();
            }
            else if(queue_type==UDPSchedular::DTA)
            {
                DecreasingTimeAlgo();
            }
            else if(queue_type==UDPSchedular::LIFO)
            {
                LastInFirstOut();
            }
            return;
        }
        ASSERT(msg == selfMsg);
        switch (selfMsg->getKind()) {
            case START:
                processStart();
                break;

            case SEND:
                return; // not send
                processSend();
                break;

            case STOP:
                processStop();
                break;

            default:
                throw cRuntimeError("Invalid kind %d in self message", (int)selfMsg->getKind());
        }
    }
    else
        socket.processMessage(msg);
}


void UDPSchedular::processPacket(Packet *pk)
{
    numReceived++;
    emit(rcvdPkSignal, pk);
    EV << "ECEIVED PACKET : " << UdpSocket::getReceivedPacketInfo(pk) << endl;
    delete pk;

    std::cout<<"NUM RECEIVED "<<numReceived<<endl;

}

void UDPSchedular::updateTskList()
{
    std::vector<cloudTask*>::iterator iter;
    //std::cout<<"Inside updateTsklIST"<<tasks_list_.size();
    /* update task computing rates to see which tasks are completed */
    updateTskComputingRates();
   // std::cout<<"after updateTskCR";
    //std::cout<<"^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^update tskList "<<endl;
    /* remove completed tasks from the execution list */
    if(!tasks_list_.empty())
    {
    for (iter = tasks_list_.begin(); iter != tasks_list_.end();)
    {
        std::cout<<tasks_list_.size() << "MPIS is "<<(*iter)->getMIPS()<<"\n";
      /* task should be completed and remove it from the list */
      if ((*iter)->getMIPS() <= 1) {
          /*
        if ((agent_) && ((*iter)->getOutput() != 0)) {
            //agent_->send((*iter)->getOutput());   // generate output if any required by the task
        }  OLD CODE CHANGED IN OMNET++*/
          //if((*iter)->getOutput() !=0)
          //{
           //   ev<<"Computing output is "<<(*iter)->getOutput()<<endl;
          //}
          std::cout<<"ERasing tasks \n";
        tasks_list_.erase(iter);
      } else {
        iter++;
      }
    }
    }
    //std::cout<<"99999999999999999999999999999999999update tskList "<<endl;
    /* set server computing rate */
    setComputingRate();

    /* compute next deadline */
    double nextDeadline = DBL_MAX;
    bool chk = false; // added if task is not empty schedule event
    cloudTask *schdTask = new cloudTask();

    if(!tasks_list_.empty())
    {
    for (iter = tasks_list_.begin(); iter != tasks_list_.end(); iter++)
    {
        if (nextDeadline > (*iter)->getDeadline())
        {
            nextDeadline = (*iter)->execTime();

                    schdTask->setMIPS((*iter)->getMIPS());
                    schdTask->setSize((*iter)->getSize());
                    schdTask->setIntercom((*iter)->getIntercom());
                    schdTask->setExecTime((*iter)->execTime());
                    simtime_t d = simTime();
                    schdTask->setComputingRate((*iter)->getComputingRate(),d.dbl());
                    schdTask->setDeadline((*iter)->getDeadline());
                    chk = true;


        }
    }
    }

    /* reschedule next update */
    if (nextDeadline != DBL_MAX)
    {
        nextEvent(schdTask);
    }

    /* Update energy */
    eUpdate();            /* Update energy for the last interval */
    setCurrentConsumption();  /* Update current energy consumption */


}

void UDPSchedular::updateTskComputingRates()
{
    std::vector<cloudTask*>::iterator iter;

      for (iter = tasks_list_.begin(); iter != tasks_list_.end(); iter++)
      {
        /* each task with then update mips left */
          simtime_t d = simTime();
        (*iter)->setComputingRate((double)current_mips_/tasks_list_.size(), d.dbl());
      //  std::cout<<"Computing rate is set to "<<(*iter)->getComputingRate()<<"\n";
      }
}

void UDPSchedular::setComputingRate()
{
    eDVFS_enabled_ = false;
    EV_INFO <<"VALUE OF eDVFS_enabled "<< eDVFS_enabled_<<endl;
      /* DVFS enabled */
      if (eDVFS_enabled_) {
        /* Max requested rate times the number of active taks */
        current_mips_ = getMostUrgentTaskRate()*tasks_list_.size();
      } else {
        /* no energy saving */
        if (tasks_list_.size() != 0)
            current_mips_ = nominal_mips_;
        else
            current_mips_ = 0;
      }
      EV_INFO<<"Current computing rate is "<<current_mips_<<endl;
      /* new computing rate, report it to tasks */
      updateTskComputingRates();
}

void UDPSchedular::nextEvent(cloudTask * schdtk)
{
    EV_INFO <<"Default value of status is "<<status_<<endl;
    if (status_ == EVENT_PENDING) {
        EV_INFO<<"EVENT PENDING........! "<<endl;
        if(schdtk->isScheduled())
            _cancel(schdtk);
       status_ = EVENT_IDLE;
     }

    // event_.handler_ = this;
     //event_.time_ = Scheduler::instance().clock();

     _sched(schdtk->getDeadline(),schdtk);
     status_ = EVENT_PENDING;
     EV_INFO<<"nextEvent value of status is "<<status_<<endl;
}

void UDPSchedular::_sched(double delay, cMessage *msg)
{
    simtime_t del = delay;
    simtime_t schTime = del + simTime();
    scheduleAt(schTime, msg);
    EV_INFO<<"Event schedule at "<<schTime<<endl;

}
void UDPSchedular::_cancel(cMessage *cmg)
{
    cloudTask *tp = check_and_cast<cloudTask *>(cmg);
    EV_INFO<<"event cancelled "<<tp->getDeadline()<<endl;
    cancelEvent(cmg);

}
void UDPSchedular::eUpdate()
{
    /* Get time spent since last update */
     double etime = (simTime().dbl() - eLastUpdateTime_)/3600;   /* time in hours */
     eConsumed_ += etime * eCurrentConsumption_;
     eLastUpdateTime_ = simTime().dbl();
     EV_INFO<<"*****************************"<<endl;
     EV_INFO<<"eConsumed_"<<eConsumed_<<endl;
     EV_INFO<<" LastUpdateTime_"<<eLastUpdateTime_<<endl;
     EV_INFO<<"Current Consumption "<<eCurrentConsumption_<<endl;
     emit(esignal,(eConsumed_/1000));
     EV_INFO<<"*****************************"<<endl;
}

void UDPSchedular::setCurrentConsumption()
{
    /* Compute idle server consumption */
      double eIdleConsumption = eNominalrate_*2/3;

      /* if DNS is enabled no energy is consumed with zero load */
      if ((getCurrentLoad() == 0) && (eDNS_enabled_)) {
        eCurrentConsumption_ = 0;
        EV_INFO<<"1 eCurrentConsumption_"<<  eCurrentConsumption_<<endl;
        return;
      }

      /* if DVFS is enabled energy consumed is scaled with the frequency */
      if (eDVFS_enabled_) {
        double f = getCurrentLoad();    /* frequency component */
        EV_INFO<<" getCurrentLoad "<<f<<endl;
        EV_INFO<<" eIdleConsumption "<<eIdleConsumption<<endl;
        EV_INFO<<" eNominalrate_ "<<eNominalrate_<<endl;

        eCurrentConsumption_ = eIdleConsumption + eNominalrate_ * f*f*f / 3;
        EV_INFO<<"2 eCurrentConsumption_"<<  eCurrentConsumption_<<endl;
        return;
      }

      /* Compute load dependant energy consumption component */
      double eLoadComponent = (eNominalrate_ - eIdleConsumption) * getCurrentLoad();
      eCurrentConsumption_ = eIdleConsumption + eLoadComponent;
      EV_INFO<<"3 eCurrentConsumption_"<<  eCurrentConsumption_<<endl;
}

double UDPSchedular::getMostUrgentTaskRate()
{
    std::vector<cloudTask*>::iterator iter;

     /* Compute highest MIPS/deadline ratio */
     double maxrate = 0.0;

     /* remove completed tasks from the execution list */
     for (iter = tasks_list_.begin(); iter != tasks_list_.end(); iter++)
     {
       /* task should be completed and remove it from the list */
       double rate = (double)(*iter)->getMIPS()/(double)(*iter)->getDeadline();
       if (rate > maxrate) maxrate = rate;
     }

     return maxrate;
}

double UDPSchedular::getCurrentLoad()
{
    EV_INFO<<"Value of current_mpis"<<current_mips_<<endl;
    EV_INFO<<"Value of nominal_mpis"<<nominal_mips_<<endl;
    currentLoad_ = (double)current_mips_/(double)nominal_mips_;
    return currentLoad_;

}


}
}
