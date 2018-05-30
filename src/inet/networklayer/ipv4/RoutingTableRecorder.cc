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
#include "inet/common/INETUtils.h"

#include <cinttypes>

#if OMNETPP_VERSION >= 0x0500 && defined HAVE_CEVENTLOGLISTENER    /* cEventlogListener is only supported from 5.0 */

#include "inet/common/Simsignals.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/IRoutingTable.h"
#include "inet/networklayer/ipv4/Ipv4RoutingTable.h"
#include "inet/networklayer/ipv6/Ipv6RoutingTable.h"
#include "inet/networklayer/generic/NextHopRoutingTable.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"    // TODO: remove?
#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/ipv4/RoutingTableRecorder.h"

Define_Module(RoutingTableRecorder);

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
        if (par("enabled"))
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
    systemModule->subscribe(interfaceCreatedSignal, listener);
    systemModule->subscribe(interfaceDeletedSignal, listener);
    systemModule->subscribe(interfaceConfigChangedSignal, listener);
    systemModule->subscribe(interfaceIpv4ConfigChangedSignal, listener);
    //systemModule->subscribe(interfaceIpv6ConfigChangedSignal, listener);
    //systemModule->subscribe(interfaceStateChangedSignal, listener);
    systemModule->subscribe(routeAddedSignal, listener);
    systemModule->subscribe(routeDeletedSignal, listener);
    systemModule->subscribe(routeChangedSignal, listener);

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
    if (signalID == routeAddedSignal || signalID == routeDeletedSignal || signalID == routeChangedSignal)
        recordRoute(host, check_and_cast<const IRoute *>(obj), signalID);
    else if (signalID == interfaceCreatedSignal || signalID == interfaceDeletedSignal)
        recordInterface(host, check_and_cast<const InterfaceEntry *>(obj), signalID);
    else if (signalID == interfaceConfigChangedSignal || signalID == interfaceIpv4ConfigChangedSignal)
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
        Ipv4RoutingTable *rt = dynamic_cast<Ipv4RoutingTable *>(module);
        if (rt) {
            cModule *host = getContainingNode(module);
            for (int i = 0; i < rt->getNumRoutes(); i++)
                recordRoute(host, rt->getRoute(i), -1);
        }
        Ipv6RoutingTable *rt6 = dynamic_cast<Ipv6RoutingTable *>(module);
        if (rt6) {
            cModule *host = getContainingNode(module);
            for (int i = 0; i < rt6->getNumRoutes(); i++)
                recordRoute(host, rt6->getRoute(i), -1);
        }
        NextHopRoutingTable *nhrt = dynamic_cast<NextHopRoutingTable *>(module);
        if (nhrt) {
            cModule *host = getContainingNode(module);
            for (int i = 0; i < nhrt->getNumRoutes(); i++)
                recordRoute(host, nhrt->getRoute(i), -1);
        }
    }
}

void RoutingTableRecorder::recordInterface(cModule *host, const InterfaceEntry *interface, simsignal_t signalID)
{
    cEnvir *envir = getSimulation()->getEnvir();
    // moduleId, ifname, address
    std::stringstream content;
    content << host->getId() << " " << interface->getName() << " ";
    content << (interface->ipv4Data() != nullptr ? interface->ipv4Data()->getIPAddress().str() : Ipv4Address().str());

    if (signalID == interfaceCreatedSignal) {
        envir->customCreatedEntry("IT", interfaceKey, content.str().c_str());
        interfaceEntryToKey[interface] = interfaceKey;
        interfaceKey++;
    }
    else if (signalID == interfaceDeletedSignal) {
        envir->customDeletedEntry("IT", interfaceEntryToKey[interface]);
        interfaceEntryToKey.erase(interface);
    }
    else if (signalID == interfaceConfigChangedSignal || signalID == interfaceIpv4ConfigChangedSignal) {
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

    if (signalID == routeAddedSignal) {
        envir->customCreatedEntry("RT", routeKey, content.str().c_str());
        routeToKey[route] = routeKey;
        routeKey++;
    }
    else if (signalID == routeDeletedSignal) {
        envir->customDeletedEntry("RT", routeToKey[route]);
        routeToKey.erase(route);
    }
    else if (signalID == routeChangedSignal) {
        envir->customChangedEntry("RT", routeToKey[route], content.str().c_str());
    }
    else if (signalID == -1) {
        envir->customFoundEntry("RT", routeToKey[route], content.str().c_str());
    }
    else
        throw cRuntimeError("Unexpected signal: %s", getSignalName(signalID));
}

#else /*OMNETPP_VERSION*/

#include "inet/common/Simsignals.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"
#include "inet/networklayer/ipv4/Ipv4Route.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/ipv4/RoutingTableRecorder.h"

namespace inet {

Define_Module(RoutingTableRecorder);

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
        if (par("enabled"))
            hookListeners();
    }
}

void RoutingTableRecorder::handleMessage(cMessage *)
{
    throw cRuntimeError(this, "This module doesn't process messages");
}

void RoutingTableRecorder::finish()
{
    if (routingLogFile != nullptr) {
        fclose(routingLogFile);
        routingLogFile = nullptr;
    }
}

void RoutingTableRecorder::hookListeners()
{
    cModule *systemModule = getSimulation()->getSystemModule();
    systemModule->subscribe(interfaceCreatedSignal, this);
    systemModule->subscribe(interfaceDeletedSignal, this);
    systemModule->subscribe(interfaceConfigChangedSignal, this);
    systemModule->subscribe(interfaceIpv4ConfigChangedSignal, this);
    //systemModule->subscribe(interfaceIpv6ConfigChangedSignal, this);
    //systemModule->subscribe(interfaceStateChangedSignal, this);

    systemModule->subscribe(routeAddedSignal, this);
    systemModule->subscribe(routeDeletedSignal, this);
    systemModule->subscribe(routeChangedSignal, this);
}

void RoutingTableRecorder::ensureRoutingLogFileOpen()
{
    if (routingLogFile == nullptr) {
        std::string fname = getEnvir()->getConfig()->getAsFilename(CFGID_ROUTINGLOG_FILE);
        inet::utils::makePathForFile(fname.c_str());
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
    if (signalID == routeAddedSignal || signalID == routeDeletedSignal || signalID == routeChangedSignal)
        recordRouteChange(host, check_and_cast<const IRoute *>(obj), signalID);
    else if (signalID == interfaceCreatedSignal || signalID == interfaceDeletedSignal)
        recordInterfaceChange(host, check_and_cast<const InterfaceEntry *>(obj), signalID);
    else if (signalID == interfaceConfigChangedSignal || signalID == interfaceIpv4ConfigChangedSignal)
        recordInterfaceChange(host, check_and_cast<const InterfaceEntryChangeDetails *>(obj)->getInterfaceEntry(), signalID);
}

void RoutingTableRecorder::recordInterfaceChange(cModule *host, const InterfaceEntry *ie, simsignal_t signalID)
{
    // Note: ie->getInterfaceTable() may be nullptr (entry already removed from its table)

    const char *tag;

    if (signalID == interfaceCreatedSignal)
        tag = "+I";
    else if (signalID == interfaceDeletedSignal)
        tag = "-I";
    else if (signalID == interfaceConfigChangedSignal)
        tag = "*I";
    else if (signalID == interfaceIpv4ConfigChangedSignal)
        tag = "*I";
    else
        throw cRuntimeError("Unexpected signal: %s", getSignalName(signalID));

    // action, eventNo, simtime, moduleId, ifname, address
    ensureRoutingLogFileOpen();
    fprintf(routingLogFile, "%s  %" PRId64 "  %s  %d  %s %s\n",
            tag,
            getSimulation()->getEventNumber(),
            SIMTIME_STR(simTime()),
            host->getId(),
            ie->getInterfaceName(),
            (ie->ipv4Data() != nullptr ? ie->ipv4Data()->getIPAddress().str().c_str() : Ipv4Address().str().c_str())
            );
    fflush(routingLogFile);
}

void RoutingTableRecorder::recordRouteChange(cModule *host, const IRoute *route, simsignal_t signalID)
{
    IRoutingTable *rt = route->getRoutingTableAsGeneric();    // may be nullptr! (route already removed from its routing table)

    const char *tag;
    if (signalID == routeAddedSignal)
        tag = "+R";
    else if (signalID == routeChangedSignal)
        tag = "*R";
    else if (signalID == routeDeletedSignal)
        tag = "-R";
    else
        throw cRuntimeError("Unexpected signal: %s", getSignalName(signalID));

    // action, eventNo, simtime, moduleId, routerID, dest, dest netmask, nexthop
    ensureRoutingLogFileOpen();
    fprintf(routingLogFile, "%s %" PRId64 "  %s  %d  %s  %s  %d  %s\n",
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

