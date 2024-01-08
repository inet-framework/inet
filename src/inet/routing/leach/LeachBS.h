//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_ROUTING_LEACH_LEACHBS_H_
#define INET_ROUTING_LEACH_LEACHBS_H_

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
#include "inet/routing/leach/Leach.h"

namespace inet {
namespace leach {

// Base Station of network which acts as data sink
class INET_API LeachBS: public RoutingProtocolBase {
private:
    int bsPktReceived; // number of packets received at BS

public:
    LeachBS();
    virtual ~LeachBS();

    int interfaceId = -1;
    NetworkInterface *interface80211ptr = nullptr;
    IInterfaceTable *ift = nullptr;
    unsigned int sequencenumber = 0;
    cModule *host = nullptr;

protected:
    virtual int numInitStages() const override {
        return NUM_INIT_STAGES;
    }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;

    virtual void handleStartOperation(LifecycleOperation *operation) override {
        start();
    }
    virtual void handleStopOperation(LifecycleOperation *operation) override {
        stop();
    }
    virtual void handleCrashOperation(LifecycleOperation *operation) override {
        stop();
    }
    void start();
    void stop();
};

} /* namespace leach */
} /* namespace inet */

#endif /* INET_ROUTING_LEACH_LEACHBS_H_ */
