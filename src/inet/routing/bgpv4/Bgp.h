//
// Copyright (C) 2010 Helene Lageber
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

#ifndef __INET_BGPROUTING_H
#define __INET_BGPROUTING_H

#include "inet/common/INETDefs.h"
#include "inet/common/lifecycle/LifecycleUnsupported.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/routing/bgpv4/BgpCommon.h"
#include "inet/routing/bgpv4/BgpRouter.h"
#include "inet/routing/bgpv4/bgpmessage/BgpHeader_m.h"

namespace inet {

namespace bgp {

class INET_API Bgp : public cSimpleModule, protected cListener, public LifecycleUnsupported
{
private:
    IIpv4RoutingTable *rt = nullptr;
    IInterfaceTable *ift = nullptr;
    bool isUp = false;
    BgpRouter *bgpRouter = nullptr;    // data structure to fill in
    cMessage *startupTimer = nullptr;    // timer for delayed startup

  public:
    Bgp();
    virtual ~Bgp();

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    void createBgpRouter();
    void handleTimer(cMessage *timer);
    virtual void finish() override;
};

} // namespace bgp

} // namespace inet

#endif // ifndef __INET_BGPROUTING_H

