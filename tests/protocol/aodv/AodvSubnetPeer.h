// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __AODV_SUBNET_TEST_PEER_H
#define __AODV_SUBNET_TEST_PEER_H
#include "AodvPeer.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"
#include "inet/networklayer/ipv4/IcmpHeader.h"
#include "inet/common/lifecycle/LifecycleController.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"
class AodvSubnetSource : public SimpleModule
{
    UdpSocket output;
  protected:
    int numInitStages() const override { return NUM_INIT_STAGES; }
    void initialize(int stage) override {
        if (stage == INITSTAGE_APPLICATION_LAYER) { output.setOutputGate(gate("socketOut")); output.bind(12345); }
    }
    void handleMessage(cMessage *message) override { delete message; }
  public:
    void emitData(Packet *packet, const L3Address& destination) {
        Enter_Method("emitData"); take(packet); output.sendTo(packet, destination, 9999);
    }
};
Define_Module(AodvSubnetSource);
#endif
