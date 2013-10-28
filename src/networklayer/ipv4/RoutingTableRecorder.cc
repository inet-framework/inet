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

#include "INETDefs.h"

#if OMNETPP_VERSION >= 0x0500 && defined HAVE_CEVENTLOGLISTENER  /* cEventlogListener is only supported from 5.0 */

#include "NotifierConsts.h"
#include "NotificationBoard.h"
#include "IInterfaceTable.h"
#include "IRoutingTable.h"
#include "IPv4RoutingTable.h"
#include "IPv6RoutingTable.h"
#include "GenericRoutingTable.h"
#include "IPv4InterfaceData.h" // TODO: remove?
#include "RoutingTableRecorder.h"


Define_Module(RoutingTableRecorder);

#define LL INT64_PRINTF_FORMAT  // for eventnumber_t


// We need this because we want to know which NotificationBoard the notification comes from
// (INotifiable::receiveChangeNotification() doesn't have NotificationBoard* as arg).
class RoutingTableNotificationBoardListener : public INotifiable
{
  private:
    NotificationBoard *nb;
    RoutingTableRecorder *recorder;

  public:
    RoutingTableNotificationBoardListener(RoutingTableRecorder *recorder, NotificationBoard *nb) {this->recorder = recorder; this->nb = nb;}
    virtual void receiveChangeNotification(int category, const cObject *details) {
        recorder->receiveChangeNotification(nb, category, details);
    }
};

RoutingTableRecorder::RoutingTableRecorder()
{
    this->interfaceKey = 0;
    this->routeKey = 0;
}

RoutingTableRecorder::~RoutingTableRecorder()
{
}

void RoutingTableRecorder::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_NETWORK_LAYER)
    {
        if (par("enabled").boolValue())
            hookListeners();
    }
}

void RoutingTableRecorder::handleMessage(cMessage *)
{
    throw cRuntimeError(this, "This module doesn't process messages");
}

void RoutingTableRecorder::hookListeners()
{
    // hook existing notification boards (we won't cover dynamically created hosts/routers, but oh well)
    for (int id = 0; id < simulation.getLastModuleId(); id++) {
        NotificationBoard *nb = dynamic_cast<NotificationBoard *>(simulation.getModule(id));
        if (nb) {
            INotifiable *listener = new RoutingTableNotificationBoardListener(this, nb);
            nb->subscribe(listener, NF_INTERFACE_CREATED);
            nb->subscribe(listener, NF_INTERFACE_DELETED);
            nb->subscribe(listener, NF_INTERFACE_CONFIG_CHANGED);
            nb->subscribe(listener, NF_INTERFACE_IPv4CONFIG_CHANGED);
            //nb->subscribe(listener, NF_INTERFACE_IPv6CONFIG_CHANGED);
            //nb->subscribe(listener, NF_INTERFACE_STATE_CHANGED);
            nb->subscribe(listener, NF_ROUTE_ADDED);
            nb->subscribe(listener, NF_ROUTE_DELETED);
            nb->subscribe(listener, NF_ROUTE_CHANGED);
        }
    }
    // hook on eventlog manager
    cEnvir* envir = simulation.getEnvir();
    cIndexedEventlogManager *eventlogManager = dynamic_cast<cIndexedEventlogManager *>(envir->getEventlogManager());
    if (eventlogManager)
        eventlogManager->addEventlogListener(this);
}

void RoutingTableRecorder::receiveChangeNotification(NotificationBoard *nb, int category, const cObject *details)
{
    cModule *host = nb->getParentModule();
    if (category==NF_ROUTE_ADDED || category==NF_ROUTE_DELETED || category==NF_ROUTE_CHANGED)
        recordRoute(host, check_and_cast<const IRoute *>(details), category);
    else if (category==NF_INTERFACE_CREATED || category==NF_INTERFACE_DELETED)
        recordInterface(host, check_and_cast<const InterfaceEntry *>(details), category);
    else if (category==NF_INTERFACE_CONFIG_CHANGED || category==NF_INTERFACE_IPv4CONFIG_CHANGED)
        recordInterface(host, check_and_cast<const InterfaceEntryChangeDetails *>(details)->getInterfaceEntry(), category);
}

void RoutingTableRecorder::recordSnapshot()
{
    for (int id = 0; id < simulation.getLastModuleId(); id++) {
        cModule* module = simulation.getModule(id);
        IInterfaceTable *ift = dynamic_cast<IInterfaceTable *>(module);
        if (ift) {
            cModule *host = module->getParentModule();
            for (int i = 0; i < ift->getNumInterfaces(); i++)
                recordInterface(host, ift->getInterface(i), -1);
        }
    }
    for (int id = 0; id < simulation.getLastModuleId(); id++) {
        cModule* module = simulation.getModule(id);
        IPv4RoutingTable *rt = dynamic_cast<IPv4RoutingTable *>(module);
        if (rt) {
            // TODO: find out host correctly
            cModule *host = module->getParentModule();
            for (int i = 0; i < rt->getNumRoutes(); i++)
                recordRoute(host, rt->getRoute(i), -1);
        }
        IPv6RoutingTable *rt6 = dynamic_cast<IPv6RoutingTable *>(module);
        if (rt6) {
            // TODO: find out host correctly
            cModule *host = module->getParentModule();
            for (int i = 0; i < rt6->getNumRoutes(); i++)
                recordRoute(host, rt6->getRoute(i), -1);
        }
        GenericRoutingTable *generic = dynamic_cast<GenericRoutingTable *>(module);
        if (generic) {
            // TODO: find out host correctly
            cModule *host = module->getParentModule();
            for (int i = 0; i < generic->getNumRoutes(); i++)
                recordRoute(host, generic->getRoute(i), -1);
        }
    }
}

void RoutingTableRecorder::recordInterface(cModule *host, const InterfaceEntry *interface, int category)
{
    cEnvir* envir = simulation.getEnvir();
    // moduleId, ifname, address
    std::stringstream content;
    content << host->getId() << " " << interface->getName() << " ";
    content << (interface->ipv4Data()!=NULL ? interface->ipv4Data()->getIPAddress().str() : IPv4Address().str());
    switch (category) {
        case NF_INTERFACE_CREATED:
            envir->customCreatedEntry("IT", interfaceKey, content.str().c_str());
            interfaceEntryToKey[interface] = interfaceKey;
            interfaceKey++;
            break;
        case NF_INTERFACE_DELETED:
            envir->customDeletedEntry("IT", interfaceEntryToKey[interface]);
            interfaceEntryToKey.erase(interface);
            break;
        case NF_INTERFACE_CONFIG_CHANGED:
        case NF_INTERFACE_IPv4CONFIG_CHANGED:
            envir->customChangedEntry("IT", interfaceEntryToKey[interface], content.str().c_str());
            break;
        case -1:
            envir->customFoundEntry("IT", interfaceEntryToKey[interface], content.str().c_str());
            break;
        default:
            throw cRuntimeError("Unexpected notification category %d", category);
    }
}

void RoutingTableRecorder::recordRoute(cModule *host, const IRoute *route, int category)
{
    cEnvir* envir = simulation.getEnvir();
    // moduleId, dest, dest netmask, nexthop
    std::stringstream content;
    content << host->getId() << " " << route->getDestinationAsGeneric().str() << " " << route->getPrefixLength() << " " << route->getNextHopAsGeneric().str();
    switch (category) {
        case NF_ROUTE_ADDED:
            envir->customCreatedEntry("RT", routeKey, content.str().c_str());
            routeToKey[route] = routeKey;
            routeKey++;
            break;
        case NF_ROUTE_DELETED:
            envir->customDeletedEntry("RT", routeToKey[route]);
            routeToKey.erase(route);
            break;
        case NF_ROUTE_CHANGED:
            envir->customChangedEntry("RT", routeToKey[route], content.str().c_str());
            break;
        case -1:
            envir->customFoundEntry("RT", routeToKey[route], content.str().c_str());
            break;
        default:
            throw cRuntimeError("Unexpected notification category %d", category);
    }
}




#else /*OMNETPP_VERSION*/




#include "NotifierConsts.h"
#include "NotificationBoard.h"
#include "IIPv4RoutingTable.h"
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
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_NETWORK_LAYER)
    {
        if (par("enabled").boolValue())
            hookListeners();
    }
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

            nb->subscribe(listener, NF_ROUTE_ADDED);
            nb->subscribe(listener, NF_ROUTE_DELETED);
            nb->subscribe(listener, NF_ROUTE_CHANGED);
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
    if (category==NF_ROUTE_ADDED || category==NF_ROUTE_DELETED || category==NF_ROUTE_CHANGED)
        recordRouteChange(host, check_and_cast<const IRoute *>(details), category);
    else if (category==NF_INTERFACE_CREATED || category==NF_INTERFACE_DELETED)
        recordInterfaceChange(host, check_and_cast<const InterfaceEntry *>(details), category);
    else if (category==NF_INTERFACE_CONFIG_CHANGED || category==NF_INTERFACE_IPv4CONFIG_CHANGED)
        recordInterfaceChange(host, check_and_cast<const InterfaceEntryChangeDetails *>(details)->getInterfaceEntry(), category);
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

    // action, eventNo, simtime, moduleId, ifname, address
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

void RoutingTableRecorder::recordRouteChange(cModule *host, const IRoute *route, int category)
{
    IRoutingTable *rt = route->getRoutingTableAsGeneric(); // may be NULL! (route already removed from its routing table)

    const char *tag;
    switch (category) {
    case NF_ROUTE_ADDED: tag = "+R"; break;
    case NF_ROUTE_CHANGED: tag = "*R"; break;
    case NF_ROUTE_DELETED: tag = "-R"; break;
    default: throw cRuntimeError("Unexpected notification category %d", category);
    }

    // action, eventNo, simtime, moduleId, routerID, dest, dest netmask, nexthop
    ensureRoutingLogFileOpen();
    fprintf(routingLogFile, "%s %"LL"d  %s  %d  %s  %s  %d  %s\n",
            tag,
            simulation.getEventNumber(),
            SIMTIME_STR(simTime()),
            host->getId(),
            (rt ? rt->getRouterIdAsGeneric().str().c_str() : "*"),
            route->getDestinationAsGeneric().str().c_str(),
            route->getPrefixLength(),
            route->getNextHopAsGeneric().str().c_str()
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


#endif /*OMNETPP_VERSION*/
