// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __INET_DSDV_H
#define __INET_DSDV_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/IArp.h"
#include "inet/networklayer/contract/IRoutingTable.h"
#include "inet/routing/base/RoutingProtocolBase.h"
#include "inet/routing/dsdv/DsdvTable.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

namespace inet {
namespace dsdv {

class INET_API Dsdv : public RoutingProtocolBase, public UdpSocket::ICallback, public cListener
{
  protected:
    ModuleRefByPar<IInterfaceTable> interfaceTable;
    ModuleRefByPar<IRoutingTable> routingTable;
    ModuleRefByPar<IArp> arp;
    NetworkInterface *routingInterface = nullptr;
    UdpSocket socket;
    DsdvTable table;
    cMessage *periodicTimer = nullptr;
    cMessage *advertisementTimer = nullptr;
    cMessage *expiryTimer = nullptr;
    bool active = false;
    bool startupDump = false;
    bool useMacFailureFeedback = false;
    int controlPort = -1;
    size_t capacity = 0;
    simtime_t period;
    simtime_t jitter;

    void start();
    void suspend(bool crash);
    bool interfaceUsable() const;
    void scheduleTimers();
    void sendAdvertisement(bool full);
    void synchronizeRoutes(const char *withdrawalReason = nullptr);
    void removeOwnedRoutes();
    void scheduleTimer(cMessage *timer, simtime_t when);

    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *message) override;
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;
    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    virtual void socketClosed(UdpSocket *socket) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *value, cObject *details) override;

  public:
    virtual ~Dsdv();
};

} // namespace dsdv
} // namespace inet
#endif
