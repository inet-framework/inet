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


#include "NotifierConsts.h"
#include "NotificationBoard.h"
#include "IRoutingTable.h"
#include "IPv4Route.h"
#include "IInterfaceTable.h"
#include "IPv4InterfaceData.h"
#include "RoutingTableRecorder.h"


Define_Module(RoutingTableRecorder);

#define LL INT64_PRINTF_FORMAT  // for eventnumber_t

Register_PerRunConfigOption(CFGID_ROUTINGLOG_FILE, "routinglog-file", CFG_FILENAME, "${resultdir}/${configname}-${runnumber}.rt", "Name of the routing log file to generate.");


// We need this because we want to know which NotificationBoard the notification comes from
// (INotifiable::receiveChangeNotification() doesn't have NotificationBoard* as arg).
class RoutingTableRecorderListener : public INotifiable
{
private:
    NotificationBoard *nb;
    RoutingTableRecorder *recorder;
public:
    RoutingTableRecorderListener(RoutingTableRecorder *recorder, NotificationBoard *nb) {this->recorder = recorder; this->nb = nb;}
    virtual void receiveChangeNotification(int category, const cObject *details) {recorder->receiveChangeNotification(nb, category, details);}
};

RoutingTableRecorder::RoutingTableRecorder()
{
    routingLogFile = NULL;
}

RoutingTableRecorder::~RoutingTableRecorder()
{
}

void RoutingTableRecorder::initialize(int stage)
{
    if (par("enabled").boolValue())
        hookListeners();
}

void RoutingTableRecorder::handleMessage(cMessage *)
{
    throw cRuntimeError(this, "This module doesn't process messages");
}

void RoutingTableRecorder::hookListeners()
{
    // hook existing notification boards (we won't cover dynamically created hosts/routers, but oh well)
    for (int id = 0; id < simulation.getLastModuleId(); id++)
    {
        NotificationBoard *nb = dynamic_cast<NotificationBoard *>(simulation.getModule(id));
        if (nb)
        {
            INotifiable *listener = new RoutingTableRecorderListener(this, nb);
            nb->subscribe(listener, NF_INTERFACE_CREATED);
            nb->subscribe(listener, NF_INTERFACE_DELETED);
            nb->subscribe(listener, NF_INTERFACE_CONFIG_CHANGED);
            nb->subscribe(listener, NF_INTERFACE_IPv4CONFIG_CHANGED);
            //nb->subscribe(listener, NF_INTERFACE_IPv6CONFIG_CHANGED);
            //nb->subscribe(listener, NF_INTERFACE_STATE_CHANGED);

            nb->subscribe(listener, NF_IPv4_ROUTE_ADDED);
            nb->subscribe(listener, NF_IPv4_ROUTE_DELETED);
            nb->subscribe(listener, NF_IPv4_ROUTE_CHANGED);
        }
    }
}

void RoutingTableRecorder::ensureRoutingLogFileOpen()
{
    if (routingLogFile == NULL)
    {
        // hack to ensure that results/ folder is created
        simulation.getSystemModule()->recordScalar("hackForCreateResultsFolder", 0);

        std::string fname = ev.getConfig()->getAsFilename(CFGID_ROUTINGLOG_FILE);
        routingLogFile = fopen(fname.c_str(), "w");
        if (!routingLogFile)
            throw cRuntimeError("Cannot open file %s", fname.c_str());
    }
}

void RoutingTableRecorder::receiveChangeNotification(NotificationBoard *nb, int category, const cObject *details)
{
    cModule *host = nb->getParentModule();
    if (category==NF_IPv4_ROUTE_ADDED || category==NF_IPv4_ROUTE_DELETED || category==NF_IPv4_ROUTE_CHANGED)
        recordRouteChange(host, check_and_cast<const IPv4Route *>(details), category);
    else
        recordInterfaceChange(host, check_and_cast<const InterfaceEntry *>(details), category);
}

void RoutingTableRecorder::recordInterfaceChange(cModule *host, const InterfaceEntry *ie, int category)
{
    // Note: ie->getInterfaceTable() may be NULL (entry already removed from its table)

    const char *tag;
    switch (category) {
    case NF_INTERFACE_CREATED: tag = "+I"; break;
    case NF_INTERFACE_DELETED: tag = "-I"; break;
    case NF_INTERFACE_CONFIG_CHANGED: tag = "*I"; break;
    case NF_INTERFACE_IPv4CONFIG_CHANGED: tag = "*I"; break;
    default: throw cRuntimeError("Unexpected notification category %d", category);
    }

    // time, moduleId, ifname, address
    ensureRoutingLogFileOpen();
    fprintf(routingLogFile, "%s  %"LL"d  %s  %d  %s %s\n",
            tag,
            simulation.getEventNumber(),
            SIMTIME_STR(simTime()),
            host->getId(),
            ie->getName(),
            (ie->ipv4Data()!=NULL ? ie->ipv4Data()->getIPAddress().str().c_str() : IPv4Address().str().c_str())
            );
    fflush(routingLogFile);
}

void RoutingTableRecorder::recordRouteChange(cModule *host, const IPv4Route *route, int category)
{
    IRoutingTable *rt = route->getRoutingTable(); // may be NULL! (route already removed from its routing table)

    const char *tag;
    switch (category) {
    case NF_IPv4_ROUTE_ADDED: tag = "+R"; break;
    case NF_IPv4_ROUTE_CHANGED: tag = "*R"; break;
    case NF_IPv4_ROUTE_DELETED: tag = "-R"; break;
    default: throw cRuntimeError("Unexpected notification category %d", category);
    }

    // time, moduleId, routerID, dest, dest netmask, nexthop
    ensureRoutingLogFileOpen();
    fprintf(routingLogFile, "%s %"LL"d  %s  %d  %s  %s  %s  %s\n",
            tag,
            simulation.getEventNumber(),
            SIMTIME_STR(simTime()),
            host->getId(),
            (rt ? rt->getRouterId().str().c_str() : "*"),
            route->getDestination().str().c_str(),
            route->getNetmask().str().c_str(),
            route->getGateway().str().c_str()
    );
    fflush(routingLogFile);
}


//TODO: routerID change
//    // time, moduleId, routerID
//    ensureRoutingLogFileOpen();
//    fprintf(routingLogFile, "ID  %s  %d  %s\n",
//            SIMTIME_STR(simTime()),
//            getParentModule()->getId(), //XXX we assume routing table is direct child of the node compound module
//            a.str().c_str()
//            );
//    fflush(routingLogFile);
//}
