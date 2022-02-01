//
// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

// Authors: Veronika Rybova, Vladimir Vesely (ivesely@fit.vutbr.cz),
//          Tamas Borbely (tomi@omnetpp.org)

#ifndef __INET_PIMSPLITTER_H
#define __INET_PIMSPLITTER_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/routing/pim/PimPacket_m.h"
#include "inet/routing/pim/tables/PimInterfaceTable.h"

namespace inet {

/**
 * PimSplitter register itself for PIM protocol (103) in the network layer,
 * and dispatches the received packets either to PimDm or PimSm according
 * to the PIM mode of the incoming interface.
 * Packets received from the PIM modules are simply forwarded to the
 * network layer.
 */
class INET_API PimSplitter : public cSimpleModule
{
  private:
    ModuleRefByPar<IInterfaceTable> ift;
    ModuleRefByPar<PimInterfaceTable> pimIft;

    cGate *ipIn = nullptr;
    cGate *ipOut = nullptr;
    cGate *pimDMIn = nullptr;
    cGate *pimDMOut = nullptr;
    cGate *pimSMIn = nullptr;
    cGate *pimSMOut = nullptr;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void processPIMPacket(Packet *pkt);
};

} // namespace inet

#endif

