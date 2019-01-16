//
// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
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
// Authors: Veronika Rybova, Vladimir Vesely (ivesely@fit.vutbr.cz),
//          Tamas Borbely (tomi@omnetpp.org)

#ifndef __INET_PIMSPLITTER_H
#define __INET_PIMSPLITTER_H

#include "inet/common/INETDefs.h"
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
    IInterfaceTable *ift = nullptr;
    PimInterfaceTable *pimIft = nullptr;

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

}    // namespace inet

#endif // ifndef __INET_PIMSPLITTER_H

