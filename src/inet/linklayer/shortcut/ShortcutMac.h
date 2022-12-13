//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SHORTCUTMAC_H
#define __INET_SHORTCUTMAC_H

#include "inet/common/INETMath.h"
#include "inet/linklayer/base/MacProtocolBase.h"

namespace inet {

class INET_API ShortcutMac : public MacProtocolBase
{
  protected:
    std::map<MacAddress, ShortcutMac *>& shortcutMacs = SIMULATION_SHARED_VARIABLE(shortcutMacs);

  protected:
    double bitrate = NaN;
    cPar *propagationDelay = nullptr;
    cPar *lengthOverhead = nullptr;
    cPar *durationOverhead = nullptr;
    cPar *packetLoss = nullptr;

  public:
    ~ShortcutMac();

  protected:
    virtual void initialize(int stage) override;
    virtual void configureNetworkInterface() override;

    virtual void handleMessageWhenUp(cMessage *message) override;
    virtual void handleUpperPacket(Packet *packet) override;
    virtual void handleLowerPacket(Packet *packet) override;

    virtual ShortcutMac *findPeer(MacAddress address);
    virtual void sendToPeer(Packet *packet, ShortcutMac *peer);
    virtual void receiveFromPeer(Packet *packet);
};

} // namespace inet

#endif

