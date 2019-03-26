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

#include "UDPComputeNode.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/applications/base/ApplicationPacket_m.h"
#include <vector>
#include "CloudTask_m.h"
#include "Tks.h"
#include <iostream>
#include <string>
#include <fstream>

using namespace std;
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

Define_Module(UDPComputeNode);

int UDPComputeNode::counter;
int UDPComputeNode::recv_counter=0;

double UDPComputeNode::totalEnergy = 0;
int UDPComputeNode::task_completed=0;

simsignal_t UDPComputeNode::sentPkSignal = SIMSIGNAL_NULL;
simsignal_t UDPComputeNode::rcvdPkSignal = SIMSIGNAL_NULL;

void UDPComputeNode::initialize(int stage)
{

    EV_INFO<<"*************** Compute Node Init"<< endl;
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

    localPort = par("localPort");
    destPort = par("destPort");


    const char *destAddrs = par("destAddresses");
    //const char *destAddrs = "192.168.0.3";
    //const char *destAddrs = "ServerNode[15]";

    this->myAddr = L3AddressResolver().resolve(this->getParentModule()->getFullPath().c_str());
    EV<<"My ADDRESS IS "<<this->myAddr<<endl;
    setMyID();
    //EV_INFO<<"Id "<<this->myId<<" IP: "<<myAddr<<endl;
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

void UDPComputeNode::processStart() {
    UdpBasicApp::processStart();
    if (destAddresses.empty())
    {
        EV_INFO<<this->getFullName()<<"DESTINATION ADDRESS IS EMPTY "<<endl;;
        return;
    }
    else
    {
        DisplayListofDestIP();
    }
}

UDPComputeNode::UDPComputeNode()
 {
     EV_INFO<<"CONST CALLED"<<endl;
     nominal_mips_ = 10000;
     eNominalrate_ = 130.0;
     currentLoad_ = 0;
     eConsumed_ = 0.0;          /* total W of energy consumed */
     eNominalrate_ = 130.0;           /* nominal consumption rate at full load at max CPU frequency */
     eCurrentConsumption_ = 0.0;        /* current consumption rate */
     eDVFS_enabled_ = true;
     eDNS_enabled_ = false;
     CPU_MIPS=500;
     tasks_list_.clear();
     typeQ= UDPComputeNode::SJF;
 }

UDPComputeNode::~UDPComputeNode() {
    while (!tasks_list_.empty()) {
        cancelAndDelete(tasks_list_.back());
        tasks_list_.pop_back();
    }
}

void UDPComputeNode::setMyID() {
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

void UDPComputeNode::finish()
{
    recordScalar("packets sent", numSent);
    recordScalar("packets received", numReceived);
    filepointer.close();
    delayptr.close();

    EV<<"Energy consumed "<<eConsumed_<<endl;
    EV<<"Energy consumed max "<<energyCountStats.getMax()<<endl;
    EV<<"Energy consumed min "<<energyCountStats.getMin()<<endl;
    EV<<"Energy consumed std "<<energyCountStats.getStddev()<<endl;
    energyCountStats.recordAs("EnergyConsumed");
    ofstream ftr("engy.txt");
    ftr<<totalEnergy<<endl;
    ftr.close();
    cout<<"Total energy consumed is "<<totalEnergy<<endl;
    cout<<"Total task Completed: "<<task_completed<<endl;

    //tasks_list_.~vector();
}


void UDPComputeNode::DisplayListofDestIP()
{

    int k = destAddresses.size();
    for(int i=0;i<k;i++)
    {
        EV_INFO<<"Dest IP List at : "<<this->myId<<" : "<< destAddresses[i]<<endl;
    }
}

Packet *UDPComputeNode::CopyCloudTaskPacket(int cter, int srcid,double mip,double deadline, double execSince, double procRate)
{
    char msgName[32];
    sprintf(msgName, "ACK-%d", cter);
    auto tsk = makeShared<CloudTask>();//(msgName);
    auto pkt = new Packet(msgName);
    pkt->setSrcProcId(this->getIndex());
    tsk->setPacketId(cter);
    string srcaddress = this->myAddr.str();
    const char *srcA = srcaddress.data();
    tsk->setSrcaddress(srcA);
    tsk->setChunkLength(B(par("messageLength").intValue()));
    tsk->setMpis_(mip);
    tsk->setSize_(0.1);
    tsk->setDeadline_(deadline); // START TIME + SIM DURATION
    tsk->setOutput_(0);
    tsk->setIntercom_(0);
    tsk->setCurrProcRate_(procRate);
    tsk->setExecutedSince_(execSince);
    tsk->setIsUser(0); // from compute node
    pkt->setTimestamp(simTime());// added later
    pkt->insertAtFront(tsk);
    return pkt;
}

Packet *UDPComputeNode::createCloudTaskPacket()
{

    char msgName[32];
    sprintf(msgName, "ACK-%d", ++counter);
   // CloudTask *tsk = new CloudTask(msgName);
    //tsk->setSrcProcId(this->getIndex());
    auto tsk = makeShared<CloudTask>();//(msgName);
    auto pkt = new Packet(msgName);
    pkt->setSrcProcId(this->getIndex());
    tsk->setPacketId(counter);
    string srcaddress = this->myAddr.str();
    const char *srcA = srcaddress.data();
    tsk->setSrcaddress(srcA);
    tsk->setChunkLength(B(par("messageLength").intValue()));
    //tsk->setByteLength(par("messageLength").longValue());
    tsk->setMpis_(1000);
    tsk->setSize_(0.1);
    tsk->setDeadline_(0 + 1000); // START TIME + SIM DURATION
    tsk->setOutput_(0);
    tsk->setIntercom_(0);
    tsk->setCurrProcRate_(0);
    tsk->setExecutedSince_(0);
    //tsk->setTimestamp(simTime());// added later
    tsk->setIsUser(0); // from compute node
    pkt->setTimestamp(simTime());// added later
    pkt->insertAtFront(tsk);
    return pkt;
}

Packet *UDPComputeNode::createPacket()
{
    char msgName[32];
    sprintf(msgName, "Data-%d", ++counter);
    //cPacket *payload = new cPacket(msgName);
    auto packet = new Packet(msgName);
    //payload->setByteLength(1048576);// in bytes
    //payload->addBitLength(8388608); // this mean 1MB data

    auto payload = makeShared<ApplicationPacket>();
    packet->setSrcProcId(this->getIndex());

    payload->setChunkLength(B(par("messageLength")));
    //payload->setByteLength(par("messageLength").longValue());

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

    packet->insertAtFront(payload);

    return packet;
}

void UDPComputeNode::displayAttachParameters(Packet *pk)
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

Packet * UDPComputeNode::attachTaskParameters(Packet *pk)
{
    pk->addPar("size_");
    pk->par("size_").setLongValue(1);
    pk->addPar("mips_");
    pk->par("mips_").setLongValue(11);
    pk->addPar("deadline_");
    pk->par("deadline_").setLongValue(2);
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

void UDPComputeNode::sendPacket(L3Address destAddr)
{
    //cPacket *payload = createPacket();
      auto  payload = createCloudTaskPacket();
      //cPacket *payload = createCloudTaskPacket();
     // L3Address destAddr = chooseDestAddr();
      EV_INFO<<"**********************************Packet Generated*****************************"<<endl;
      EV_INFO<<"Sim_Time=" <<simTime()<< "  Src :" << this->myAddr <<"  Destination : "<<destAddr<< " TaskNum "<<counter<<endl;
      EV_INFO<<"********************************************************************************"<<endl;
      emit(sentPkSignal, payload);
      socket.sendTo(payload, destAddr, destPort);
      numSent++;
}

void UDPComputeNode::sendPacket()
{

    //cPacket *payload = createPacket();
    auto  payload = createCloudTaskPacket();
    //cPacket *payload = createCloudTaskPacket();
    L3Address destAddr = chooseDestAddr();
    EV_INFO<<"**********************************Packet Generated*****************************"<<endl;
    EV_INFO<<"Sim_Time=" <<simTime()<< "  Src :" << this->myAddr <<"  Destination : "<<destAddr<< " TaskNum "<<counter<<endl;
    EV_INFO<<"********************************************************************************"<<endl;
    emit(sentPkSignal, payload);
    socket.sendTo(payload, destAddr, destPort);

    numSent++;
}


void UDPComputeNode::handleMessageWhenUp(cMessage *msg)
{
    if (hasGUI())
    {
        char buf[140];
        sprintf(buf, "rcvd: %d pks, sent: %d pks \n eConsumed (kW*h): %e", numReceived, numSent,(eConsumed_/1000));
        getDisplayString().setTagArg("t", 0, buf);
        bubble("Packet received");
    }
    if (msg->isSelfMessage()) {
        auto clod = dynamic_cast<cloudTask *>(msg);
        if (nullptr != clod ) {
            bool inList = false;
            bool isCopy = true;
            for (auto elem : tasks_list_) {
                if (elem->getID() == clod->getID())
                    inList = true;
                if (elem == clod)
                    isCopy = false;
            }
            if (!inList) {
                delete msg; // if is not in the queue, delete it
                return;
            }
            EV<<"SelfMesg received at ComputeNode and SIze is "<<tasks_list_.size()<<endl;
       //     std::cout<<"Current Time "<<simTime().dbl()<<endl;
            if(typeQ==UDPComputeNode::SJF)
            {
                EV<<" SJF Executing "<<endl;
                updateTskList();
         //       std::cout<<"updateTskList completed "<<endl;
                /* update energy */
                updateTskComputingRates();
                eUpdate(); /* update energy for the last interval */
                setCurrentConsumption(); /* update current energy consumption */
                totalEnergy+=eConsumed_;
            }
            if (isCopy)
                delete msg; // is a copy of a msg in the queue
            return;
        }
        ASSERT(msg == selfMsg);
        switch (selfMsg->getKind()) {
            case START:
                processStart();
                break;

            case SEND:
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

void UDPComputeNode::socketDataArrived(UdpSocket *socket, Packet *packet)
{
    numReceived++;
    const auto & tlk = packet->peekAtFront<CloudTask>();
    //CloudTask *tlk = check_and_cast<CloudTask *>(msg);
    recv_counter++;
    EV<<"   "<<tlk->getPacketId()<<" Receiver IP "<< myAddr<<endl;
    cloudTask *tpk = new cloudTask();
    tpk->setMIPS(tlk->getMpis_());
    tpk->setID(tlk->getPacketId());
    tpk->setSize(tlk->getSize_());
    tpk->setIntercom(tlk->getIntercom_());
    tpk->setExecTime(simTime().dbl());
    simtime_t d = simTime();
    tpk->setComputingRate(tlk->getCurrProcRate_(),d.dbl());
    tpk->setDeadline(tlk->getDeadline_());
    if(tasks_list_.empty()) {
        double _deadline = tpk->getDeadline();
        EV<<"First packet is scheduled at "<<_deadline<< " node IP "<<myAddr<<endl;
        scheduleAt(simTime().dbl()+_deadline, tpk);
    }
    ntasks_++;
    tasks_list_.push_back(tpk);  // add to the active tasks links
    EV<<"Queue size is "<<tasks_list_.size()<< " at Node IP "<<myAddr<<endl;
#if 0
   //   return;

      /* BELOW CODE NOT IN USE */
      /*
      cMessage *pqt = PK(msg);
      UDPDataIndication *ctrl = check_and_cast<UDPDataIndication *>(pqt->getControlInfo());
      L3Address srcAddr = ctrl->getSrcAddr();
      L3Address destAddr = ctrl->getDestAddr();

      EV_INFO<<"**********************************Packet Received*****************************"<<endl;
      std::cout<<"Packet received"<<endl;
      EV_INFO<<"Packet Received Number "<<recv_counter<<endl;
      EV_INFO<<"Sim_Time=" <<simTime()<< "  Src :" << srcAddr <<"  Destination : "<<destAddr<<endl;
      EV_INFO<< tlk->getMpis_()<<endl;
      EV_INFO<< tlk->getSize_()<<endl;
      EV_INFO<< tlk->getIntercom_()<<endl;
      EV_INFO<< tlk->getExecutedSince_()<<endl;
      EV_INFO<< tlk->getCurrProcRate_()<<endl;
      EV_INFO<< tlk->getDeadline_()<<endl;
      EV_INFO<<"DELAY : " <<simTime() - tlk->getTimestamp()<<endl;
      delayptr << tlk->getId() << " , "<< simTime() - tlk->getTimestamp()<<endl;
      std::cout<<"delay "<<simTime() - tlk->getTimestamp()<<endl;
      EV_INFO<<"******************************************************************************"<<endl;


      bool energyModule = true;
      if(energyModule)
      {
          cloudTask *tpk = new cloudTask();
          tpk->setMIPS(tlk->getMpis_());
          tpk->setID(tlk->getPacketId());
          tpk->setSize(tlk->getSize_());
          tpk->setIntercom(tlk->getIntercom_());

          tpk->setExecTime(tlk->getExecutedSince_());
          simtime_t d = simTime();
          tpk->setComputingRate(tlk->getCurrProcRate_(),d.dbl());
          tpk->setDeadline(tlk->getDeadline_());

          ntasks_++;
          tasks_list_.push_back(tpk);  // add to the active tasks links
          tlk->setExecutedSince_(simTime().dbl());
                /* error if I uncomment this function I guess it is when to remove
                 * from the task list ------ > */
     //     std::cout<<"Before updateTskList completed "<<endl;*/
    /*
          updateTskList();
   //       std::cout<<"updateTskList completed "<<endl;
          /* update energy */
      /*    updateTskComputingRates();
          eUpdate(); /* update energy for the last interval */
    /*      setCurrentConsumption(); /* update current energy consumption */

    totalEnergy+=eConsumed_;
    //  }

    //  energyCountStats.collect(eConsumed_);
   //   energyCountVector.record(eConsumed_);

      /*  replying back code need to shift into a function */
    //  sendPacket();
     // string srcaddress = this->myAddr.str();
      //const char *srcA = srcaddress.data();

      //L3Address sendbackAddr = srcAddr;
      //sendPacket(sendbackAddr);

      /* ********************************************** */
//       hopCountVector.record(ctrl->getTtl());
//      hopCountStats.collect(ctrl->getTtl());
#else
    totalEnergy+=eConsumed_;
#endif
    processPacket(packet);
    if (hasGUI()) {
        char buf[140];
        sprintf(buf, "rcvd: %d pks, sent: %d pks \n eConsumed (kW*h): %e", numReceived, numSent,(eConsumed_/1000));
        getDisplayString().setTagArg("t", 0, buf);
        bubble("Packet received");
    }
}

void UDPComputeNode::processPacket(Packet *pk)
{
  //  numReceived++;
    emit(rcvdPkSignal, pk);
    EV << "Received Packet \t : " << UdpSocket::getReceivedPacketInfo(pk) << endl;
    delete pk;
  //  std::cout<<"Num Received "<<numReceived<<endl;
}

void UDPComputeNode::updateTskList() {
    /* update task computing rates to see which tasks are completed */
    updateTskComputingRates();
    /* remove completed tasks from the execution list */
    /*  for task display purpose */
    for (const auto &elem : tasks_list_) {
        EV_INFO << "Tsk id inside queue is: " << elem->getID() << endl;
    }
    /*
     for (ite = tasks_list_.begin(); ite != tasks_list_.end();)
     {
         EV_INFO<<"Tsk id inside queue is: "<<(*ite)->getID()<<endl;
         ite++;
     }
     */

    if (!tasks_list_.empty()) {
        EV << "Inside removing section................!" << endl;
        for (auto iter = tasks_list_.begin(); iter != tasks_list_.end();) {

            auto tsk = (*iter);
            /* task should be completed and remove it from the list */
            if (tsk->getMIPS() <= 1) {

                EV << "Removing Task....!" << tsk->getID() << endl;
                cout << "Task removed #" << tsk->getID() << " MIPS # " << tsk->getMIPS() << endl;
                iter = tasks_list_.erase(iter);
                delete tsk;
                task_completed++;
            }
            else {
                EV << "Task ID: " << tsk->getID() << " and Mips " << tsk->getMIPS() << endl;
                ++iter;
            }
        }
    }
    /* set server computing rate */
    setComputingRate();
    /* compute next deadline */
    double nextDeadline = DBL_MAX;

    // added if task is not empty schedule event

    if (!tasks_list_.empty()) {
        EV << "SEcond check for empty" << endl;
        for (const auto &elem : tasks_list_) {
            if (nextDeadline > elem->getDeadline()) {
                nextDeadline = elem->execTime();
            }
        }

        bool NS2Model = true;
        /* reschedule next update */
        if (nextDeadline != DBL_MAX) {
            for (auto & elem : tasks_list_) {
                EV << "Searching for Next task" << endl;
                if (NS2Model) {
                    if (nextDeadline == elem->execTime()) {
                        cloudTask *tpk = new cloudTask();
                        tpk->setMIPS(elem->getMIPS());
                        tpk->setID(elem->getID());
                        //      tpk->setSize((*iter)->getSize());
                        //      tpk->setIntercom((*iter)->getIntercom());
                        tpk->setExecTime(elem->execTime());

                        simtime_t dtime = simTime();
                        tpk->setComputingRate(elem->getComputingRate(), dtime.dbl());
                        tpk->setDeadline(elem->getDeadline());
                        nextEvent(tpk, simTime().dbl() + nextDeadline);
                        break;
                    }
                }                        // typeQ ends
                else if (!NS2Model) {

                }
            }
        }
    }
    /* Update energy */
    eUpdate(); /* Update energy for the last interval */
    setCurrentConsumption(); /* Update current energy consumption */
}

void UDPComputeNode::updateTskComputingRates()
{
    for (auto & elem : tasks_list_)
    {
        /* each task with then update mips left */
        simtime_t d = simTime();
        std::cout<<"Executed Since "<< elem->execTime()<<" current time "<<d<<" GetExecTime "<<elem->getExecTime()<<endl;
        elem->setComputingRate((double)current_mips_/tasks_list_.size(), d.dbl());
        //  std::cout<<"Computing rate is set to "<<(*iter)->getComputingRate()<<"\n";
     }
}

void UDPComputeNode::setComputingRate() {
    /* DVFS enabled */
    if (eDVFS_enabled_) {
        /* Max requested rate times the number of active taks */
        current_mips_ = getMostUrgentTaskRate() * tasks_list_.size();
    }
    else {
        /* no energy saving */
        if (tasks_list_.size() != 0)
            current_mips_ = nominal_mips_;
        else
            current_mips_ = 0;
    }

    /* new computing rate, report it to tasks */
    updateTskComputingRates();
}

void UDPComputeNode::nextEvent(cloudTask * schdtk, double delay) {

    if (status_ == EVENT_PENDING) {
        EV_INFO << "EVENT PENDING........! " << endl;
        if (schdtk->isScheduled())
            _cancel(schdtk);
        status_ = EVENT_IDLE;
    }

    // event_.handler_ = this;
    //event_.time_ = Scheduler::instance().clock();
    //  cMessage *pkt = check_and_cast<cMessage *>(schdtk);
    _sched(delay, schdtk);
    status_ = EVENT_PENDING;

}

void UDPComputeNode::_sched(double delay, cMessage *msg)
{
   // simtime_t del = delay;
    //simtime_t schTime = del + simTime();
    simtime_t schTime = simTime();
    EV_INFO<<"Next event schedule at :"<<schTime<<endl;
    scheduleAt(delay, msg);


}
void UDPComputeNode::_cancel(cMessage *cmg)
{
    cloudTask *tp = check_and_cast<cloudTask *>(cmg);
    EV_INFO<<"event cancelled "<<tp->getDeadline()<<endl;
    cancelEvent(cmg);

}
void UDPComputeNode::eUpdate() /* complete impl*/
{
    /* Get time spent since last update */
     double etime = (simTime().dbl() - eLastUpdateTime_)/3600;   /* time in hours */
     eConsumed_ += etime * eCurrentConsumption_;
     eLastUpdateTime_ = simTime().dbl();
}

void UDPComputeNode::setCurrentConsumption()  /* complete impl*/
{
    /* Compute idle server consumption */
      double eIdleConsumption = eNominalrate_*2/3;

      /* if DNS is enabled no energy is consumed with zero load */
      if ((getCurrentLoad() == 0) && (eDNS_enabled_)) {
        eCurrentConsumption_ = 0;
         return;
      }

      /* if DVFS is enabled energy consumed is scaled with the frequency */
      if (eDVFS_enabled_) {
        double f = getCurrentLoad();    /* frequency component */
        eCurrentConsumption_ = eIdleConsumption + eNominalrate_ * f*f*f / 3;
        return;
      }

      /* Compute load dependant energy consumption component */
      double eLoadComponent = (eNominalrate_ - eIdleConsumption) * getCurrentLoad();
      eCurrentConsumption_ = eIdleConsumption + eLoadComponent;

}

double UDPComputeNode::getMostUrgentTaskRate()
{
      /* Compute highest MIPS/deadline ratio */
     double maxrate = 0.0;

     /* remove completed tasks from the execution list */
     for (const auto &elem : tasks_list_)
     {
       /* task should be completed and remove it from the list */
       double rate = (double)elem->getMIPS()/(double)elem->getDeadline();
       if (rate > maxrate) maxrate = rate;
     }
     return maxrate;
}

double UDPComputeNode::getCurrentLoad()
{
    EV_INFO<<"Value of current_mpis"<<current_mips_<<endl;
    EV_INFO<<"Value of nominal_mpis"<<nominal_mips_<<endl;
    currentLoad_ = (double)current_mips_/(double)nominal_mips_;
    return currentLoad_;

}


}
}
