//
// Copyright (C) 2012 Andras Varga
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

#ifndef __INET_ROUTINGTABLERECORDER_H
#define __INET_ROUTINGTABLERECORDER_H

#include "inet/common/INETDefs.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"

namespace inet {

/**
 * Records routing table changes into a file.
 *
 * @see Ipv4RoutingTable, Ipv4Route
 */
class INET_API RoutingTableRecorder : public cSimpleModule, public cListener
{
  private:
    FILE *routingLogFile;

  public:
    RoutingTableRecorder();
    virtual ~RoutingTableRecorder();

    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override { receiveChangeNotification(source, signalID, obj); }

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *) override;
    virtual void finish() override;
    virtual void hookListeners();
    virtual void ensureRoutingLogFileOpen();
    virtual void receiveChangeNotification(cComponent *source, simsignal_t signalID, cObject *details);
    virtual void recordInterfaceChange(cModule *host, const InterfaceEntry *ie, simsignal_t signalID);
    virtual void recordRouteChange(cModule *host, const IRoute *route, simsignal_t signalID);
};

} // namespace inet

#endif // ifndef __INET_ROUTINGTABLERECORDER_H

