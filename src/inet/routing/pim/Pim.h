//
// Copyright (C) 2019 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//
// Authors: Zoltan Bojthe
//

#ifndef __INET_PIM_H
#define __INET_PIM_H

#include "inet/common/INETDefs.h"
#include "inet/networklayer/contract/INetfilter.h"
#include "inet/routing/base/RoutingProtocolBase.h"
#include "inet/routing/pim/PimPacket_m.h"
#include "inet/transportlayer/common/CrcMode_m.h"
#include "inet/transportlayer/common/TransportPseudoHeader_m.h"

namespace inet {

/**
 * Compound module for PIM protocol (RFC 4601).
 */
class INET_API Pim : public RoutingProtocolBase, protected cListener
{
  public:
    class CrcInsertion : public NetfilterBase::HookBase {
      public:
        virtual Result datagramPreRoutingHook(Packet *packet) override { return ACCEPT; }
        virtual Result datagramForwardHook(Packet *packet) override { return ACCEPT; }
        virtual Result datagramPostRoutingHook(Packet *packet) override;
        virtual Result datagramLocalInHook(Packet *packet) override { return ACCEPT; }
        virtual Result datagramLocalOutHook(Packet *packet) override { return ACCEPT; }
    };


  protected:
    // parameters
    CrcMode crcMode = CRC_MODE_UNDEFINED;
    CrcInsertion crcInsertion;

  public:
    Pim() {}
    virtual ~Pim();
    CrcMode getCrcMode() { return crcMode; }
    static void insertCrc(const Ptr<PimPacket>& pimPacket);
    static void insertCrc(const Protocol *networkProtocol, const L3Address& srcAddress, const L3Address& destAddress, const Ptr<PimPacket>& pimPacket);
    static bool verifyCrc(const Protocol *networkProtocol, const Ptr<const PimPacket>& pimPacket, Packet *packet);
    static uint16_t computeCrc(const Protocol *networkProtocol, const L3Address& srcAddress, const L3Address& destAddress, const Ptr<const PimPacket>& pimPacket);

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;
};

}    // namespace inet

#endif // ifndef __INET_PIM_H

