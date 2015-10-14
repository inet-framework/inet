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

#include "inet/common/INETDefs.h"

#if OMNETPP_VERSION >= 0x0500 && defined HAVE_CEVENTLOGLISTENER    /* cEventlogListener is only supported from 5.0 */

#include "inet/common/NotifierConsts.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/IRoutingTable.h"
#include "inet/networklayer/ipv4/IPv4RoutingTable.h"
#include "inet/networklayer/ipv6/IPv6RoutingTable.h"
#include "inet/networklayer/generic/GenericRoutingTable.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"    // TODO: remove?
#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/ipv4/RoutingTableRecorder.h"

Define_Module(RoutingTableRecorder);

#define LL    INT64_PRINTF_FORMAT  // for eventnumber_t

class RoutingTableNotificationBoardListener : public cListener
{
  private:
    RoutingTableRecorder *recorder;

  public:
    RoutingTableNotificationBoardListener(RoutingTableRecorder *recorder) { this->recorder = recorder; }
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj) { recorder->receiveChangeNotification(source, signalID, obj); }
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

    if (stage == INITSTAGE_NETWORK_LAYER) {
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
    cModule *systemModule = getSimulation()->getSystemModule();
    cListener *listener = new RoutingTableNotificationBoardListener(this);
    systemModule->subscribe(NF_INTERFACE_CREATED, listener);
    systemModule->subscribe(NF_INTERFACE_DELETED, listener);
    systemModule->subscribe(NF_INTERFACE_CONFIG_CHANGED, listener);
    systemModule->subscribe(NF_INTERFACE_IPv4CONFIG_CHANGED, listener);
    //systemModule->subscribe(NF_INTERFACE_IPv6CONFIG_CHANGED, listener);
    //systemModule->subscribe(NF_INTERFACE_STATE_CHANGED, listener);
    systemModule->subscribe(NF_ROUTE_ADDED, listener);
    systemModule->subscribe(NF_ROUTE_DELETED, listener);
    systemModule->subscribe(NF_ROUTE_CHANGED, listener);

    // hook on eventlog manager
    cEnvir *envir = getSimulation()->getEnvir();
    cIndexedEventlogManager *eventlogManager = dynamic_cast<cIndexedEventlogManager *>(envir->getEventlogManager());
    if (eventlogManager)
        eventlogManager->addEventlogListener(this);
}

void RoutingTableRecorder::receiveChangeNotification(cComponent *source, simsignal_t signalID, cObject *obj)
{
    cModule *m = dynamic_cast<cModule *>(source);
    if (!m)
        m = source->getParentModule();
    cModule *host = findContainingNode(m, true);
    if (signalID == NF_ROUTE_ADDED || signalID == NF_ROUTE_DELETED || signalID == NF_ROUTE_CHANGED)
        recordRoute(host, check_and_cast<const IRoute *>(obj), signalID);
    else if (signalID == NF_INTERFACE_CREATED || signalID == NF_INTERFACE_DELETED)
        recordInterface(host, check_and_cast<const InterfaceEntry *>(obj), signalID);
    else if (signalID == NF_INTERFACE_CONFIG_CHANGED || signalID == NF_INTERFACE_IPv4CONFIG_CHANGED)
        recordInterface(host, check_and_cast<const InterfaceEntryChangeDetails *>(obj)->getInterfaceEntry(), signalID);
}

void RoutingTableRecorder::recordSnapshot()
{
    for (int id = 0; id <= getSimulation()->getLastComponentId(); id++) {
        cModule *module = getSimulation()->getModule(id);
        IInterfaceTable *ift = dynamic_cast<IInterfaceTable *>(module);
        if (ift) {
            cModule *host = getContainingNode(module);
            for (int i = 0; i < ift->getNumInterfaces(); i++)
                recordInterface(host, ift->getInterface(i), -1);
        }
    }
    for (int id = 0; id <= getSimulation()->getLastComponentId(); id++) {
        cModule *module = getSimulation()->getModule(id);
        IPv4RoutingTable *rt = dynamic_cast<IPv4RoutingTable *>(module);
        if (rt) {
            cModule *host = getContainingNode(module);
            for (int i = 0; i < rt->getNumRoutes(); i++)
                recordRoute(host, rt->getRoute(i), -1);
        }
        IPv6RoutingTable *rt6 = dynamic_cast<IPv6RoutingTable *>(module);
        if (rt6) {
            cModule *host = getContainingNode(module);
            for (int i = 0; i < rt6->getNumRoutes(); i++)
                recordRoute(host, rt6->getRoute(i), -1);
        }
        GenericRoutingTable *generic = dynamic_cast<GenericRoutingTable *>(module);
        if (generic) {
            cModule *host = getContainingNode(module);
            for (int i = 0; i < generic->getNumRoutes(); i++)
                recordRoute(host, generic->getRoute(i), -1);
        }
    }
}

void RoutingTableRecorder::recordInterface(cModule *host, const InterfaceEntry *interface, simsignal_t signalID)
{
    cEnvir *envir = getSimulation()->getEnvir();
    // moduleId, ifname, address
    std::stringstream content;
    content << host->getId() << " " << interface->getName() << " ";
    content << (interface->ipv4Data() != nullptr ? interface->ipv4Data()->getIPAddress().str() : IPv4Address().str());

    if (signalID == NF_INTERFACE_CREATED) {
        envir->customCreatedEntry("IT", interfaceKey, content.str().c_str());
        interfaceEntryToKey[interface] = interfaceKey;
        interfaceKey++;
    }
    else if (signalID == NF_INTERFACE_DELETED) {
        envir->customDeletedEntry("IT", interfaceEntryToKey[interface]);
        interfaceEntryToKey.erase(interface);
    }
    else if (signalID == NF_INTERFACE_CONFIG_CHANGED || signalID == NF_INTERFACE_IPv4CONFIG_CHANGED) {
        envir->customChangedEntry("IT", interfaceEntryToKey[interface], content.str().c_str());
    }
    else if (signalID == -1) {
        envir->customFoundEntry("IT", interfaceEntryToKey[interface], content.str().c_str());
    }
    else
        throw cRuntimeError("Unexpected signal: %s", getSignalName(signalID));
}

void RoutingTableRecorder::recordRoute(cModule *host, const IRoute *route, int signalID)
{
    cEnvir *envir = getSimulation()->getEnvir();
    // moduleId, dest, dest netmask, nexthop
    std::stringstream content;
    content << host->getId() << " " << route->getDestinationAsGeneric().str() << " " << route->getPrefixLength() << " " << route->getNextHopAsGeneric().str();

    if (signalID == NF_ROUTE_ADDED) {
        envir->customCreatedEntry("RT", routeKey, content.str().c_str());
        routeToKey[route] = routeKey;
        routeKey++;
    }
    else if (signalID == NF_ROUTE_DELETED) {
        envir->customDeletedEntry("RT", routeToKey[route]);
        routeToKey.erase(route);
    }
    else if (signalID == NF_ROUTE_CHANGED) {
        envir->customChangedEntry("RT", routeToKey[route], content.str().c_str());
    }
    else if (signalID == -1) {
        envir->customFoundEntry("RT", routeToKey[route], content.str().c_str());
    }
    else
        throw cRuntimeError("Unexpected signal: %s", getSignalName(signalID));
}

#else /*OMNETPP_VERSION*/

#include "inet/common/NotifierConsts.h"
#include "inet/networklayer/ipv4/IIPv4RoutingTable.h"
#include "inet/networklayer/ipv4/IPv4Route.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/ipv4/RoutingTableRecorder.h"

namespace inet {

Define_Module(RoutingTableRecorder);

#define LL    INT64_PRINTF_FORMAT  // for eventnumber_t

Register_PerRunConfigOption(CFGID_ROUTINGLOG_FILE, "routinglog-file", CFG_FILENAME, "${resultdir}/${configname}-${runnumber}.rt", "Name of the routing log file to generate.");

RoutingTableRecorder::RoutingTableRecorder()
{
    routingLogFile = nullptr;
}

RoutingTableRecorder::~RoutingTableRecorder()
{
}

void RoutingTableRecorder::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_NETWORK_LAYER) {
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
    cModule *systemModule = getSimulation()->getSystemModule();
    systemModule->subscribe(NF_INTERFACE_CREATED, this);
    systemModule->subscribe(NF_INTERFACE_DELETED, this);
    systemModule->subscribe(NF_INTERFACE_CONFIG_CHANGED, this);
    systemModule->subscribe(NF_INTERFACE_IPv4CONFIG_CHANGED, this);
    //systemModule->subscribe(NF_INTERFACE_IPv6CONFIG_CHANGED, this);
    //systemModule->subscribe(NF_INTERFACE_STATE_CHANGED, this);

    systemModule->subscribe(NF_ROUTE_ADDED, this);
    systemModule->subscribe(NF_ROUTE_DELETED, this);
    systemModule->subscribe(NF_ROUTE_CHANGED, this);
}

void RoutingTableRecorder::ensureRoutingLogFileOpen()
{
    if (routingLogFile == nullptr) {
        // hack to ensure that results/ folder is created
        getSimulation()->getSystemModule()->recordScalar("hackForCreateResultsFolder", 0);

        std::string fname = getEnvir()->getConfig()->getAsFilename(CFGID_ROUTINGLOG_FILE);
        routingLogFile = fopen(fname.c_str(), "w");
        if (!routingLogFile)
            throw cRuntimeError("Cannot open file %s", fname.c_str());
    }
}

void RoutingTableRecorder::receiveChangeNotification(cComponent *nsource, simsignal_t signalID, cObject *obj)
{
    cModule *m = dynamic_cast<cModule *>(nsource);
    if (!m)
        m = nsource->getParentModule();
    cModule *host = getContainingNode(m);
    if (signalID == NF_ROUTE_ADDED || signalID == NF_ROUTE_DELETED || signalID == NF_ROUTE_CHANGED)
        recordRouteChange(host, check_and_cast<const IRoute *>(obj), signalID);
    else if (signalID == NF_INTERFACE_CREATED || signalID == NF_INTERFACE_DELETED)
        recordInterfaceChange(host, check_and_cast<const InterfaceEntry *>(obj), signalID);
    else if (signalID == NF_INTERFACE_CONFIG_CHANGED || signalID == NF_INTERFACE_IPv4CONFIG_CHANGED)
        recordInterfaceChange(host, check_and_cast<const InterfaceEntryChangeDetails *>(obj)->getInterfaceEntry(), signalID);
}

void RoutingTableRecorder::recordInterfaceChange(cModule *host, const InterfaceEntry *ie, simsignal_t signalID)
{
    // Note: ie->getInterfaceTable() may be nullptr (entry already removed from its table)

    const char *tag;

    if (signalID == NF_INTERFACE_CREATED)
        tag = "+I";
    else if (signalID == NF_INTERFACE_DELETED)
        tag = "-I";
    else if (signalID == NF_INTERFACE_CONFIG_CHANGED)
        tag = "*I";
    else if (signalID == NF_INTERFACE_IPv4CONFIG_CHANGED)
        tag = "*I";
    else
        throw cRuntimeError("Unexpected signal: %s", getSignalName(signalID));

    // action, eventNo, simtime, moduleId, ifname, address
    ensureRoutingLogFileOpen();
    fprintf(routingLogFile, "%s  %" LL "d  %s  %d  %s %s\n",
            tag,
            getSimulation()->getEventNumber(),
            SIMTIME_STR(simTime()),
            host->getId(),
            ie->getName(),
            (ie->ipv4Data() != nullptr ? ie->ipv4Data()->getIPAddress().str().c_str() : IPv4Address().str().c_str())
            );
    fflush(routingLogFile);
}

void RoutingTableRecorder::recordRouteChange(cModule *host, const IRoute *route, simsignal_t signalID)
{
    IRoutingTable *rt = route->getRoutingTableAsGeneric();    // may be nullptr! (route already removed from its routing table)

    const char *tag;
    if (signalID == NF_ROUTE_ADDED)
        tag = "+R";
    else if (signalID == NF_ROUTE_CHANGED)
        tag = "*R";
    else if (signalID == NF_ROUTE_DELETED)
        tag = "-R";
    else
        throw cRuntimeError("Unexpected signal: %s", getSignalName(signalID));

    // action, eventNo, simtime, moduleId, routerID, dest, dest netmask, nexthop
    ensureRoutingLogFileOpen();
    fprintf(routingLogFile, "%s %" LL "d  %s  %d  %s  %s  %d  %s\n",
            tag,
            getSimulation()->getEventNumber(),
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

} // namespace inet

#endif /*OMNETPP_VERSION*/

