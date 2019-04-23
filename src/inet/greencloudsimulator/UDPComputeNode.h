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

#ifndef COMPUTINGNODE_H_
#define COMPUTINGNODE_H_


#include <fstream>
#include <vector>
#include "inet/common/INETDefs.h"

#include "inet/applications/udpapp/UdpBasicApp.h"
#include "inet/common/packet/Packet.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/applications/base/ApplicationBase.h"

#include <string>
#include "cloudTask.h"
#include "CloudTask_m.h"
#include <fstream>

namespace inet {
namespace greencloudsimulator {


class INET_API UDPComputeNode : public UdpBasicApp
{
public:
    enum QueueType
    {
    FCFS,
    PRIORITY,
    SJF

    };
private:
    simsignal_t esignal;
    static double totalEnergy;
    QueueType typeQ;
    double CPU_MIPS;


  protected:
    cHistogram energyCountStats;
    cOutVector energyCountVector;

    std::ofstream filepointer;
    std::ofstream delayptr;

    std::vector <cloudTask*> tasks_list_;        /* execution list */
    int current_mips_;      /* current load of the server in mips */
    int nominal_mips_;      /* maximum computing capacility of the server at the maximum frequency */
    int status_;
    double eLastUpdateTime_;
    double eConsumed_;          /* total W of energy consumed */
    double eNominalrate_;           /* nominal consumption rate at full load at max CPU frequency */
    double eCurrentConsumption_;        /* current consumption rate */
    bool eDVFS_enabled_;
    int eDNS_enabled_;
    enum EventStatus { EVENT_IDLE, EVENT_PENDING, EVENT_HANDLING };

    /* Stats */
    int ntasks_;
    double currentLoad_;
    cMessage *event;
    //Event event_;
    L3Address myAddr;

    static int counter; // counter for generating a global number for each packet
    static int mipsCounter;
    static int recv_counter;
    static int task_completed;

   // statistics
    int numSent;
    int numReceived;
    std::string myId; // to store a unique id

    static simsignal_t sentPkSignal;
    static simsignal_t rcvdPkSignal;

    // chooses random destination address

    virtual Packet *createPacket();
    virtual Packet *createCloudTaskPacket();
    virtual Packet *CopyCloudTaskPacket(int counter, int srcid,double mip,double deadline, double execSince, double procRate);
    virtual Packet *attachTaskParameters(Packet *pk);
    virtual void displayAttachParameters(Packet *pk);
    virtual void sendPacket() override;
    virtual void processPacket(Packet *msg) override;

  protected:
    //virtual int numInitStages() const {return 4;}
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void finish() override;
    virtual void setMyID();
    virtual void DisplayListofDestIP();
    virtual void updateTskList();
    void updateTskComputingRates();
    void setComputingRate();
    //void nextEvent(double delay);
    void nextEvent(cloudTask * schdtk, double delay);
    void eUpdate();
    void setCurrentConsumption();
    double getMostUrgentTaskRate();
    void computeCurrentLoad();  /* updates current load of the server after task is received or its execution completed */
    double getCurrentLoad();
    void _sched(double delay, cMessage *msg);
    void _cancel(cMessage *cmg);
    void sendPacket(L3Address destAddr);

    virtual void processStart() override;

public:
    UDPComputeNode();
    virtual ~UDPComputeNode();
    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;

};

}
}
#endif /* ComputingNode */
