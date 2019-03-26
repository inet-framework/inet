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

#include "UDPApplication.h"
#include "inet/applications/base/ApplicationPacket_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/TagBase_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include <vector>
#include "CloudTask_m.h"
#include "Tks.h"
#include <iostream>
#include <string>

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

Define_Module(UDPApplication);

int UDPApplication::counter;
int UDPApplication::recv_counter=0;
int UDPApplication::task_generated=0;

simsignal_t UDPApplication::sentPkSignal = SIMSIGNAL_NULL;
simsignal_t UDPApplication::rcvdPkSignal = SIMSIGNAL_NULL;

void UDPApplication::initialize(int stage)
{
    UdpBasicApp::initialize(stage);
    EV_INFO<<"*************UDP APPLICATION LAYER 2***************"<< endl;

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

    //const char *destAddrs = par("destAddresses");
    //const char *destAddrs = "192.168.0.3";
    const char *destAddrs = "Schedular";

    this->myAddr = L3AddressResolver().resolve(this->getParentModule()->getFullPath().c_str());
    EV<<"My ADDRESS IS "<<this->myAddr<<endl;
    cStringTokenizer tokenizer(destAddrs);
    setMyID();
    //ev<<"Id "<<this->myId<<" IP: "<<myAddr<<endl;
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
        //scheduleAt(startTime, timerMsg);
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
 UDPApplication::UDPApplication()
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
 }

void UDPApplication::setMyID()
{
    std::string strr = (std::string )this->getFullPath();
    const char *c = strr.c_str();
    cStringTokenizer tokenizer(c,".");
    const char *token;
    int cp=0;
    while ((token = tokenizer.nextToken()) != NULL){
        if(cp==1)  {
            this->myId = token;
        }
        cp++;
    }
}

void UDPApplication::finish()
{
    recordScalar("packets sent", numSent);
    recordScalar("packets received", numReceived);
    filepointer.close();
    delayptr.close();
    std::cout<<"Total task generated "<<task_generated<<endl;
    //tasks_list_.~vector();
}

void UDPApplication::DisplayListofDestIP()
{

    int k = destAddresses.size();
    for(int i=0;i<k;i++)
    {
        EV_INFO<<"Dest IP List at : "<<this->myId<<" : "<< destAddresses[i]<<endl;
    }
}

Packet *UDPApplication::createCloudTaskPacket()
{

    char msgName[32];
    sprintf(msgName, "Task-%d", ++counter);
    auto  tsk = makeShared<CloudTask>();
    tsk->setPacketId(counter);
    //tsk->setSrcProcId(this->getIndex());
    string srcaddress = this->myAddr.str();
    const char *srcA = srcaddress.data();
    tsk->setSrcaddress(srcA);
    tsk->setChunkLength(B(par("messageLength").intValue()));

    auto pkt = new Packet(msgName);
    pkt->setSrcProcId(this->getIndex());
    pkt->setTimestamp(simTime());// added later

    double prob = dblrand();
    tsk->setMpis_(prob * 1000);
    tsk->setSize_(0.1);
    tsk->setDeadline_(1); //  SIM DURATION
    tsk->setOutput_(0);
    tsk->setIntercom_(0);
    tsk->setCurrProcRate_(0);
    tsk->setExecutedSince_(0);
    tsk->setIsUser(1);
    task_generated++;
    pkt->insertAtFront(tsk);

    pkt->addTag<CreationTimeTag>()->setCreationTime(simTime());

    return pkt;
}

Packet *UDPApplication::createPacket()
{
    char msgName[32];
    //sprintf(msgName, "UDPBAppData-%d", counter++);


    sprintf(msgName, "Data-%d", counter++);
    //cPacket *payload = new cPacket(msgName);

    Packet *packet = new Packet(msgName);
    //payload->setByteLength(1048576);// in bytes
    //payload->addBitLength(8388608); // this mean 1MB data

    const auto& payload = makeShared<ApplicationPacket>();
    payload->setChunkLength(B(par("messageLength")));
    payload->setSequenceNumber(numSent);
    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
    packet->insertAtBack(payload);


    packet->setSrcProcId(this->getIndex());
    //packet->setByteLength(par("messageLength").longValue());
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

void UDPApplication::displayAttachParameters(Packet *pk)
{
    if(pk->hasPar("size_"))
    {
        long ss = pk->par("size_").longValue();
        EV_INFO <<"SIZE VALUE IS "<<ss<<endl;
    }

    if(pk->hasPar("mips_"))
    {
        long se = pk->par("mips_").longValue();
        EV_INFO <<"MIPS VALUE IS "<<se<<endl;
    }

}

Packet * UDPApplication::attachTaskParameters(Packet *pk)
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

void UDPApplication::sendPacket()
{

    //cPacket *payload = createPacket();
    auto payload = createCloudTaskPacket();
    //cPacket *payload = createCloudTaskPacket();
    L3Address destAddr = chooseDestAddr();
    EV_INFO <<"**********************************Packet Generated*****************************"<<endl;
    EV_INFO <<"Sim_Time=" <<simTime()<< "  Src :" << this->myAddr <<"  Destination : "<<destAddr<< " TaskNum "<<counter<<endl;
    EV_INFO <<"********************************************************************************"<<endl;

    emit(sentPkSignal, payload);

    socket.sendTo(payload, destAddr, destPort);

    numSent++;
}



void UDPApplication::processStart() {
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

void UDPApplication::handleMessageWhenUp(cMessage *msg) {
    if (hasGUI()) {
        char buf[140];
        // sprintf(buf, "rcvd: %d pks\nsent: %d pks", numReceived, numSent);
        sprintf(buf, "rcvd: %d pks, sent: %d pks \n eConsumed (kW*h): %e", numReceived, numSent,(eConsumed_/1000));
        getDisplayString().setTagArg("t", 0, buf);
        EV<<buf<<endl;
    }
    UdpBasicApp::handleMessageWhenUp(msg);
}

void UDPApplication::socketDataArrived(UdpSocket *socket, Packet *packet)
{
    // process incoming packet

    recv_counter++;
    std::cout<<"casting to cloud message"<<endl;
    const auto &tlk = packet->peekAtFront<CloudTask>();

    auto l3Addresses = packet->getTag<L3AddressInd>();
    L3Address srcAddr = l3Addresses->getSrcAddress();
    L3Address destAddr = l3Addresses->getDestAddress();

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
    EV_INFO<<"DELAY : " <<simTime() - packet->getTimestamp()<<endl;
    delayptr << packet->getId() << " , "<< simTime() - packet->getTimestamp()<<endl;
    std::cout<<"delay "<<simTime() - packet->getTimestamp()<<endl;
    EV_INFO<<"******************************************************************************"<<endl;
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
    processPacket(packet);
    /* error if I uncomment this function I guess it is when to remove
     *      *            * from the task list ------ > */
    updateTskList();
     /* update energy */
    updateTskComputingRates();
    eUpdate(); /* update energy for the last interval */
    setCurrentConsumption(); /* update current energy consumption */
    if (hasGUI()) {
        bubble("Recv");
    }
}

void UDPApplication::processPacket(Packet *pk)
{

    emit(rcvdPkSignal, pk);
    EV << "RECEIVED PACKET : " << UdpSocket::getReceivedPacketInfo(pk) << endl;
    delete pk;
    numReceived++;
    std::cout<<"NUM RECEIVED "<<numReceived<<endl;

}

void UDPApplication::updateTskList()
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
           //   EV_INFO<<"Computing output is "<<(*iter)->getOutput()<<endl;
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

void UDPApplication::updateTskComputingRates()
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

void UDPApplication::setComputingRate()
{
    eDVFS_enabled_ = false;
    EV_INFO<<"VALUE OF eDVFS_enabled "<< eDVFS_enabled_<<endl;
    /* DVFS enabled */
    if (eDVFS_enabled_) {
        /* Max requested rate times the number of active taks */
        current_mips_ = getMostUrgentTaskRate()*tasks_list_.size();
    }
    else {
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

void UDPApplication::nextEvent(cloudTask * schdtk)
{
    EV_INFO<<"Default value of status is "<<status_<<endl;
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

void UDPApplication::_sched(double delay, cMessage *msg)
{
    simtime_t del = delay;
    simtime_t schTime = del + simTime();
    scheduleAt(schTime, msg);
    EV_INFO<<"Event schedule at "<<schTime<<endl;
}

void UDPApplication::_cancel(cMessage *cmg)
{
    cloudTask *tp = check_and_cast<cloudTask *>(cmg);
    EV_INFO<<"event cancelled "<<tp->getDeadline()<<endl;
    cancelEvent(cmg);
}

void UDPApplication::eUpdate()
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

void UDPApplication::setCurrentConsumption()
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

double UDPApplication::getMostUrgentTaskRate()
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

double UDPApplication::getCurrentLoad()
{
    EV_INFO<<"Value of current_mpis"<<current_mips_<<endl;
    EV_INFO<<"Value of nominal_mpis"<<nominal_mips_<<endl;
    currentLoad_ = (double)current_mips_/(double)nominal_mips_;
    return currentLoad_;

}


}
}
