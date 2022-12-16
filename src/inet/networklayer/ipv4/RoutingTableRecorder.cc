//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/ipv4/RoutingTableRecorder.h"

#include <cinttypes>

#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/networklayer/ipv4/Ipv4Route.h"

namespace inet {

Define_Module(RoutingTableRecorder);

Register_PerRunConfigOption(CFGID_ROUTINGLOG_FILE, "routinglog-file", CFG_FILENAME, "${resultdir}/${configname}-${runnumber}.rt", "Name of the routing log file to generate.");

RoutingTableRecorder::RoutingTableRecorder()
{
    routingLogFile = nullptr;
}

RoutingTableRecorder::~RoutingTableRecorder()
{
    if (routingLogFile != nullptr) {
        fclose(routingLogFile);
        routingLogFile = nullptr;
    }
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
}

void RoutingTableRecorder::hookListeners()
{
    cModule *systemModule = getSimulation()->getSystemModule();
    systemModule->subscribe(interfaceCreatedSignal, this);
    systemModule->subscribe(interfaceDeletedSignal, this);
    systemModule->subscribe(interfaceConfigChangedSignal, this);
    systemModule->subscribe(interfaceIpv4ConfigChangedSignal, this);
//    systemModule->subscribe(interfaceIpv6ConfigChangedSignal, this);
//    systemModule->subscribe(interfaceStateChangedSignal, this);

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
        recordInterfaceChange(host, check_and_cast<const NetworkInterface *>(obj), signalID);
    else if (signalID == interfaceConfigChangedSignal || signalID == interfaceIpv4ConfigChangedSignal)
        recordInterfaceChange(host, check_and_cast<const NetworkInterfaceChangeDetails *>(obj)->getNetworkInterface(), signalID);
}

void RoutingTableRecorder::recordInterfaceChange(cModule *host, const NetworkInterface *ie, simsignal_t signalID)
{
#if OMNETPP_BUILDNUM < 2001
    if (getSimulation()->getSimulationStage() == STAGE(CLEANUP))
#else
    if (getSimulation()->getStage() == STAGE(CLEANUP))
#endif
        return; // ignore notifications during cleanup

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
    auto ipv4Data = ie->findProtocolData<Ipv4InterfaceData>();
    fprintf(routingLogFile, "%s  #%" PRId64 "  %ss  %s  %s %s\n",
            tag,
            getSimulation()->getEventNumber(),
            SIMTIME_STR(simTime()),
            host->getFullPath().c_str(),
            ie->getInterfaceName(),
            (ipv4Data != nullptr ? ipv4Data->getIPAddress().str().c_str() : Ipv4Address().str().c_str()));
    fflush(routingLogFile);
}

void RoutingTableRecorder::recordRouteChange(cModule *host, const IRoute *route, simsignal_t signalID)
{
    IRoutingTable *rt = route->getRoutingTableAsGeneric(); // may be nullptr! (route already removed from its routing table)

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
    auto ie = route->getInterface();
    fprintf(routingLogFile, "%s #%" PRId64 "  %ss  %s  %s  %s/%d  %s  %s\n",
            tag,
            getSimulation()->getEventNumber(),
            SIMTIME_STR(simTime()),
            host->getFullPath().c_str(),
            (rt ? rt->getRouterIdAsGeneric().str().c_str() : "*"),
            route->getDestinationAsGeneric().str().c_str(),
            route->getPrefixLength(),
            route->getNextHopAsGeneric().str().c_str(),
            (ie ? ie->getInterfaceName() : "*"));
    fflush(routingLogFile);
}

//TODO routerID change
//    // time, moduleId, routerID
//    ensureRoutingLogFileOpen();
//    fprintf(routingLogFile, "ID  %s  %d  %s\n",
//            SIMTIME_STR(simTime()),
//            getParentModule()->getId(), //TODO we assume routing table is direct child of the node compound module
//            a.str().c_str());
//    fflush(routingLogFile);
//}

} // namespace inet

