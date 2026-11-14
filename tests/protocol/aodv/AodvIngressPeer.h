// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __AODV_INGRESS_TEST_PEER_H
#define __AODV_INGRESS_TEST_PEER_H
#include "AodvPeer.h"

// Test-only adapter owns the completed UDP indication before handing it to AODV.
class AodvIngressFilter : public SimpleModule
{
  protected:
    void handleMessage(cMessage *message) override {
        if (auto packet = dynamic_cast<Packet *>(message)) {
            if (par("mode").stdstringValue() == "Missing")
                packet->removeTagIfPresent<InterfaceInd>();
            else
                packet->addTagIfAbsent<InterfaceInd>()->setInterfaceId(INT_MAX);
        }
        send(message, "out");
    }
};
Define_Module(AodvIngressFilter);

class AodvIngressPeer : public AodvPeer
{
  protected:
    void handleMessage(cMessage *message) override {
        if (message != timer) { AodvPeer::handleMessage(message); return; }
        if (step++ == 0) {
            request(0, 1, true);
            scheduleAt(0.15, timer);
            return;
        }
        ASSERT(!findRoute(peer) && replies == 0);
        pass("ControlIngress");
    }
};
Define_Module(AodvIngressPeer);
#endif
