//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_ROUTING_LEACH_LEACH_H_
#define INET_ROUTING_LEACH_LEACH_H_

#include <string.h>
#include <vector>

#include "inet/common/INETDefs.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/networklayer/ipv4/Ipv4RoutingTable.h"
#include "inet/routing/base/RoutingProtocolBase.h"
#include "inet/routing/leach/LeackPkts_m.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/mobility/base/StationaryMobilityBase.h"
#include "inet/power/contract/ICcEnergyStorage.h"
#include "inet/common/lifecycle/LifecycleController.h"

using namespace std;

namespace inet {
namespace leach {

class INET_API Leach: public RoutingProtocolBase {

private:
    NetworkInterface *wirelessInterface = nullptr;
    unsigned int sequencenumber = 0;
    cModule *host = nullptr;
    Ipv4Address idealCH;
    cMessage *event = nullptr;
    int round = 0;
    ModuleRefByPar<IInterfaceTable> interfaceTable;

protected:
    simtime_t dataPktSendDelay;
    simtime_t CHPktSendDelay;
    simtime_t roundDuration;

    int numNodes = 0;
    double clusterHeadPercentage = 0.0;
    int roundDurationVariance = 0;

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
    void configureInterfaces();

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

} /* namespace leach */
} /* namespace inet */

#endif /* INET_ROUTING_LEACH_LEACH_H_ */
