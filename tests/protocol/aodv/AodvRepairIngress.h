// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __AODV_REPAIR_INGRESS_H
#define __AODV_REPAIR_INGRESS_H
#include "AodvPeer.h"
// Own completed UDP input until its real AODV socket delivery event is scheduled.
class AodvRepairIngress : public SimpleModule
{
    Packet *held = nullptr;
  protected:
    void handleMessage(cMessage *message) override {
        auto packet = check_and_cast<Packet *>(message);
        auto reply = dynamicPtrCast<const Rrep>(packet->peekAtFront<AodvControlPacket>());
        if (reply && reply->getDestSeqNum() == 21) { ASSERT(!held); held = packet; }
        else send(packet, "out");
    }
  public:
    simtime_t delivery;
    int scheduled = 0;
    void releaseAt(simtime_t when) {
        Enter_Method("releaseAt"); ASSERT(held && when >= simTime() && scheduled == 0);
        delivery = when; scheduled++;
        auto packet = held; held = nullptr; sendDelayed(packet, when - simTime(), "out");
    }
    ~AodvRepairIngress() { delete held; }
};

Define_Module(AodvRepairIngress);
#endif
