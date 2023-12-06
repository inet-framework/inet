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

#ifndef INET_ROUTING_LEACH_LEACH_H_
#define INET_ROUTING_LEACH_LEACH_H_

#include <list>
#include <map>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <algorithm>
#include <functional>

#include "inet/common/INETDefs.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/networklayer/ipv4/Ipv4RoutingTable.h"
#include "inet/routing/base/RoutingProtocolBase.h"
#include "LeackPkts_m.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/mobility/base/StationaryMobilityBase.h"
#include "inet/power/contract/ICcEnergyStorage.h"
#include "inet/common/lifecycle/LifecycleController.h"

using namespace std;

namespace inet {

class INET_API Leach: public RoutingProtocolBase {

private:

    NetworkInterface *interface80211ptr = nullptr;
    int interfaceId = -1;
    unsigned int sequencenumber = 0;
    cModule *host = nullptr;
    Ipv4Address idealCH;
    cMessage *event = nullptr;
    int round;

protected:
    IInterfaceTable *ift = nullptr;
    simtime_t dataPktSendDelay;
    simtime_t CHPktSendDelay;
    simtime_t roundDuration;

    int numNodes = 0;
    double clusterHeadPercentage = 0.0;

    // object for storing CH data per node in vector array
    struct nodeMemoryObject {
        Ipv4Address nodeAddr;
        Ipv4Address CHAddr;
        double energy;
    };

    struct TDMAScheduleEntry {
        Ipv4Address nodeAddress;
        double TDMAdelay;
    };

    vector<nodeMemoryObject> nodeMemory; // NCH nodes store data about CH broadcasts
    vector<TDMAScheduleEntry> nodeCHMemory; // CH store data about CH acknowledgements
    vector<TDMAScheduleEntry> extractedTDMASchedule;
    bool wasCH;

public:
    Leach();
    virtual ~Leach();

    enum LeachState {
        nch, ch
    }; // written in lower caps as it conflicts with enum in LeachPkts.msg file

    LeachState leachState;
    double TDMADelayCounter;

    virtual int numInitStages() const override {
        return NUM_INIT_STAGES;
    }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void handleMessageWhenDown(cMessage *msg) override;

    void handleSelfMessage(cMessage *msg);
    void processMessage(cMessage *msg);

    // lifecycle
    virtual void handleStartOperation(LifecycleOperation *operation) override {
        start();
    }
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;
    void start();
    void stop();
    virtual void refreshDisplay() const override;
    enum SelfMsgKinds {
        SELF = 1, DATA2CH, DATA2BS
    };

    double generateThresholdValue(int subInterval);

    void sendDataToCH(Ipv4Address nodeAddr, Ipv4Address CHAddr,
            double TDMAslot);
    void sendDataToBS(Ipv4Address CHAddr);
    void sendAckToCH(Ipv4Address nodeAddr, Ipv4Address CHAddr);
    void sendSchToNCH(Ipv4Address selfAddr);

    void addToNodeMemory(Ipv4Address nodeAddr, Ipv4Address CHAddr,
            double energy);
    void addToNodeCHMemory(Ipv4Address NCHAddr);
    bool isCHAddedInMemory(Ipv4Address CHAddr);
    bool isNCHAddedInCHMemory(Ipv4Address NCHAddr);
    void generateTDMASchedule();

    virtual void setLeachState(LeachState ls);
    Ipv4Address getIdealCH(Ipv4Address nodeAddr, Ipv4Address CHAddr);
};

} /* namespace inet */

#endif /* INET_ROUTING_LEACH_LEACH_H_ */
