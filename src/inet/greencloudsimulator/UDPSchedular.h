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

#ifndef UDPAPPLICATION_H_
#define UDPAPPLICATION_H_

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
#include "IPMapper.h"
#include <queue>

namespace inet {
namespace greencloudsimulator {


class INET_API UDPSchedular : public UdpBasicApp
{
public:
    enum queueType
    {
        FCFS,
        SJF,
        DTA,
        LIFO
    };
/* DTA stands for decresing time algorithm */
private:
    simsignal_t esignal;
    queueType queue_type;

  protected:
    std::ofstream filepointer;
    std::ofstream delayptr;

    std::vector<IPMapper> vlist;
    int vlist_indexer;

    std::queue<cMessage *> rtsk;

    std::vector <cloudTask*> tasks_list_;        /* execution list */
    std::vector <Packet*> scheduling_task;        /* execution list */
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

    // statistics
     int numSent;
    int numReceived;
    std::string myId; // to store a unique id

    static simsignal_t sentPkSignal;
    static simsignal_t rcvdPkSignal;

    // chooses random destination address

    virtual Packet *createPacket();
    virtual Packet *createCloudTaskPacket();
    virtual Packet *attachTaskParameters(Packet *pk);
    virtual void displayAttachParameters(Packet *pk);
    virtual void sendPacket() override;

    virtual void processSend() override {}
    virtual void processPacket(Packet *msg) override;
  protected:
    virtual void processStart() override;
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void finish() override;
    virtual void setMyID();
    virtual void DisplayListofDestIP();
    virtual void updateTskList();
    void updateTskComputingRates();
    void setComputingRate();
    //void nextEvent(double delay);
    void nextEvent(cloudTask * schdtk);
    void eUpdate();
    void setCurrentConsumption();
    double getMostUrgentTaskRate();
    void computeCurrentLoad();  /* updates current load of the server after task is received or its execution completed */
    double getCurrentLoad();
    void _sched(double delay, cMessage *msg);
    void _cancel(cMessage *cmg);
    void DisplayVectorTable();

    L3Address RoundRobin();
    L3Address FCFS_ComputeNode();

    bool UpdateVirtualTable(L3Address src, int taskId, L3Address dest);
    void ResetVirtualTable();

    void PacketConversion(Packet *);
    void ShortestJobFirst();
    void FirstComeFirstServe();
    void LastInFirstOut();
    void DecreasingTimeAlgo();

    Packet *CopyCloudTaskPacket(Packet *);

public:
    UDPSchedular();
    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;

};

}
}
#endif /* UDPAPPLICATION_H_ */
