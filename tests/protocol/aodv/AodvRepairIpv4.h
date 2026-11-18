// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __AODV_REPAIR_IPV4_H
#define __AODV_REPAIR_IPV4_H
#include "AodvPeer.h"
#include "inet/networklayer/ipv4/Ipv4.h"
class AodvRepairTransitPacket : public Packet
{
  public:
    inline static int created = 0;
    inline static int destroyed = 0;
    explicit AodvRepairTransitPacket(const Packet& packet) : Packet(packet) { created++; }
    ~AodvRepairTransitPacket() { destroyed++; }
    Packet *dup() const override { return new Packet(*this); }
};
// This adapter owns raw input before IP invokes the production prerouting hook.
class AodvRepairIpv4 : public Ipv4
{
  protected:
    void handleIncomingDatagram(Packet *packet) override {
        if (!strcmp(packet->getName(), "repair-data")) {
            auto tracked = new AodvRepairTransitPacket(*packet);
            delete packet;
            packet = tracked;
        }
        Ipv4::handleIncomingDatagram(packet);
    }
  public:
    size_t queuedCount() const { return queuedDatagramsForHooks.size(); }
};


Define_Module(AodvRepairIpv4);
#endif
