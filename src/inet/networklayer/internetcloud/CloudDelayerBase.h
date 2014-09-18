//
// Copyright (C) 2012 OpenSim Ltd
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
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
// @author Zoltan Bojthe
//

#ifndef __INET_CLOUDDELAYERBASE_H
#define __INET_CLOUDDELAYERBASE_H

#include "inet/common/INETDefs.h"

#include "inet/networklayer/common/INetfilter.h"

namespace inet {

//forward declarations:
class IPv4;

class INET_API CloudDelayerBase : public cSimpleModule, public INetfilter::IHook
{
  public:
    CloudDelayerBase();
    ~CloudDelayerBase();

  protected:
    virtual void initialize(int stage);
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void finish();
    virtual void handleMessage(cMessage *msg);

    /**
     * Returns true in outDrop if the msg is dropped in cloud,
     * otherwise returns calculated delay in outDelay.
     */
    virtual void calculateDropAndDelay(const cMessage *msg, int srcID, int destID, bool& outDrop, simtime_t& outDelay);

    virtual INetfilter::IHook::Result datagramPreRoutingHook(INetworkDatagram *datagram, const InterfaceEntry *inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, L3Address& nextHopAddress);
    virtual INetfilter::IHook::Result datagramForwardHook(INetworkDatagram *datagram, const InterfaceEntry *inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, L3Address& nextHopAddress);
    virtual INetfilter::IHook::Result datagramPostRoutingHook(INetworkDatagram *datagram, const InterfaceEntry *inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, L3Address& nextHopAddress);
    virtual INetfilter::IHook::Result datagramLocalInHook(INetworkDatagram *datagram, const InterfaceEntry *inputInterfaceEntry);
    virtual INetfilter::IHook::Result datagramLocalOutHook(INetworkDatagram *datagram, const InterfaceEntry *& outputInterfaceEntry, L3Address& nextHopAddress);

  protected:
    IPv4 *ipv4Layer;
};

} // namespace inet

#endif // ifndef __INET_CLOUDDELAYERBASE_H

