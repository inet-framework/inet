//
// Copyright (C) 2010 Philipp Berndt
// based on ../../networklayer/autorouting/FlatNetworkConfigurator.cc
// Copyright (C) 2004 Andras Varga
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


#include <algorithm>

#include <gnplib/impl/network/gnp/GnpLatencyModel.h>
#include <gnplib/impl/network/gnp/GnpNetLayerFactory.h>
#include <gnplib/api/common/random/Rng.h>
#include <gnplib/impl/network/AbstractNetLayer.h>

#include "InternetCloudNetworkConfigurator.h"

#include "IInterfaceTable.h"
#include "InterfaceEntry.h"
#include "IPv4InterfaceData.h"
#include "IPvXAddressResolver.h"
#include "IRoutingTable.h"


Define_Module(InternetCloudNetworkConfigurator);

namespace {
const char* INTERNET_CLOUD = "InternetCloud";

bool isInternetNode(const cModule* module)
{
    static const cModule* cached;
    if (module == cached)
        return true;
    if (!strcmp(module->getModuleType()->getName(), INTERNET_CLOUD))
    {
        cached = module;
        return true;
    }
    return false;
}

bool isInternetNode(const cTopology::Node* node)
{
    static const cTopology::Node* cached;
    if (node == cached)
        return true;
    if (!strcmp(node->getModule()->getModuleType()->getName(), INTERNET_CLOUD))
    {
        cached = node;
        return true;
    }
    return false;
}
}

using gnplib::impl::network::gnp::GnpNetLayerFactory;
using gnplib::impl::network::gnp::GnpLatencyModel;
using gnplib::impl::network::gnp::GnpNetLayer;


InternetCloudNetworkConfigurator::InternetCloudNetworkConfigurator()
: netLayerFactoryGnp(new GnpNetLayerFactory) { }

/**
 * Produces random integer in range [0,r) using generator 0.
 */
int intrand2(int r)
{
	return intrand(r);
}

void InternetCloudNetworkConfigurator::initialize(int stage)
{
    if (stage==0)
    {
	// Configure gnplib to use omnet's random number generator 
        gnplib::api::common::random::Rng::intrand = &intrand2;
        gnplib::api::common::random::Rng::dblrand = &dblrand;

        GnpLatencyModel*latencyModelGnp(new GnpLatencyModel);
        netLayerFactoryGnp->setGnpFile(par("gnpFile"));
        netLayerFactoryGnp->setLatencyModel(latencyModelGnp);
    }
    else if (stage==2)
    {
        cTopology topo("topo");
        NodeInfoVector nodeInfo; // will be of size topo.nodes[]

        // extract topology into the cTopology object, then fill in
        // isIPNode, rt and ift members of nodeInfo[]
        extractTopology(topo, nodeInfo);

        // assign addresses to IPv4 nodes, and also store result in nodeInfo[].address
        assignAddresses(topo, nodeInfo);

        // add default routes to hosts (nodes with a single attachment);
        // also remember result in nodeInfo[].usesDefaultRoute
        addDefaultRoutes(topo, nodeInfo);

        // calculate shortest paths, and add corresponding static routes
        fillRoutingTables(topo, nodeInfo);

        // update display string
        setDisplayString(topo, nodeInfo);
    }
}

InternetCloudNetworkConfigurator::~InternetCloudNetworkConfigurator()
{
    delete netLayerFactoryGnp;
}

void InternetCloudNetworkConfigurator::extractTopology(cTopology& topo, NodeInfoVector& nodeInfo)
{
    // extract topology
    topo.extractByProperty("node");
    EV << "cTopology found " << topo.getNumNodes() << " nodes\n";

    // fill in isIPNode, ift and rt members in nodeInfo[]
    nodeInfo.resize(topo.getNumNodes());
    for (int i=0; i<topo.getNumNodes(); i++)
    {
        cModule *mod = topo.getNode(i)->getModule();
        nodeInfo[i].isIPNode = IPvXAddressResolver().findInterfaceTableOf(mod)!=NULL;
        if (nodeInfo[i].isIPNode)
        {
            nodeInfo[i].ift = IPvXAddressResolver().interfaceTableOf(mod);
            nodeInfo[i].rt = IPvXAddressResolver().routingTableOf(mod);
        }
        if (mod->hasPar("group"))
            nodeInfo[i].group = mod->par("group").stdstringValue();
        else
        {
            const cProperty* prop = mod->getProperties()->get("group");
            if (prop)
            {
                nodeInfo[i].group = prop->getValue("");
                EV << "mod uses group " << prop->getValue("") << endl;
            } else
            {
                if (!isInternetNode(mod))
                {
                    EV << "Module " << mod->getFullName() << " has no GNP group par/property" << endl;
                }
            }
        }
    }
}

void InternetCloudNetworkConfigurator::assignAddresses(cTopology& topo, NodeInfoVector& nodeInfo)
{
    // assign IPv4 addresses
    for (int i=0; i<topo.getNumNodes(); i++)
    {
        // skip bus types
        if (!nodeInfo[i].isIPNode)
            continue;
        if (nodeInfo[i].group.empty())
            continue;
        cTopology::Node *node = topo.getNode(i);
        GnpNetLayer *netLayer = netLayerFactoryGnp->newNetLayer(nodeInfo[i].group); // TODO: This is a memory leak!!!
        uint32 addr = netLayer->getNetID().getID();

        EV << "Assigning " << node->getModule()->getFullName() << " of group \""
           << nodeInfo[i].group << "\" IP-Id " << addr << " (pseudo-IP address "
           << IPv4Address(addr) << ")\n";

        nodeInfo[i].address.set(addr);

        // find interface table and assign address to all (non-loopback) interfaces
        IInterfaceTable *ift = nodeInfo[i].ift;
        for (int k=0; k < ift->getNumInterfaces(); k++)
        {
            InterfaceEntry *ie = ift->getInterface(k);
            if (!ie->isLoopback())
            {
                ie->ipv4Data()->setIPAddress(IPv4Address(addr));
                ie->ipv4Data()->setNetmask(IPv4Address::ALLONES_ADDRESS); // full address must match for local delivery
            }
        }

        // Parametrize DatarateChannel connected to the Internet with node-specific data
        InterfaceEntry *ie(0);
        for (int k=0; k < node->getNumOutLinks(); ++k)
        {
            cTopology::LinkOut* linkOut = node->getLinkOut(k);
            EV << node->getModule()->getFullName() << " is connected to " << linkOut->getRemoteNode()->getModule()->getModuleType()->getName() << "\n";
            if (isInternetNode(linkOut->getRemoteNode()))
            {
                cDatarateChannel* internet_uplink = dynamic_cast<cDatarateChannel*>(linkOut->getLocalGate()->getChannel());
                if (internet_uplink)
                {
                    if (ie)
                       throw cRuntimeError("Can't connect %s upstream to the Internet with more than one interface", node->getModule()->getFullName());
                    ie = ift->getInterfaceByNodeOutputGateId(linkOut->getLocalGate()->getId());

                    const double upstream_bw_bps(netLayer->getMaxUploadBandwidth() * 8.0); // bytes/s -> bit/s
                    const double upstream_delay_s(netLayer->getAccessLatency() / (2.0*1000.0)); // distribute delay evenly between up- and downstream, ms -> s
                    EV << "Configuring upstream channel with " << upstream_bw_bps << "bps and " << upstream_delay_s << "s wire delay" << endl;
                    internet_uplink->setDatarate(upstream_bw_bps);
                    internet_uplink->setDelay(upstream_delay_s);
                }
                else
                {
                    throw cRuntimeError("Please use a DatarateChannel to connect %s upstream to the Internet", node->getModule()->getFullName());
                }
            }

        }

        // Parametrize DatarateChannel connected from the Internet with node-specific data
        for (int k=0; k < node->getNumInLinks(); ++k)
        {
            cTopology::LinkIn* linkIn = node->getLinkIn(k);
            if (isInternetNode(linkIn->getRemoteNode()))
            {
                cDatarateChannel* internet_downlink = dynamic_cast<cDatarateChannel*>(linkIn->getRemoteGate()->getChannel());
                if (internet_downlink)
                {
                    if (linkIn->getLocalGateId() != ie->getNodeInputGateId())
                        throw cRuntimeError("%s uses different interfaces to connect upstream/downstream to the Internet", node->getModule()->getFullName());

                    const double downstream_bw_bps(netLayer->getMaxDownloadBandwidth() * 8.0); // bytes/s -> bit/s
                    const double downstream_delay_s(netLayer->getAccessLatency() / (2.0*1000.0)); // distribute delay evenly between up- and downstream, ms -> s
                    EV << "Configuring downstream channel with " << downstream_bw_bps << "bps and " << downstream_delay_s << "s wire delay" << endl;
                    internet_downlink->setDatarate(downstream_bw_bps);
                    internet_downlink->setDelay(downstream_delay_s);
                }
                else
                {
                    throw cRuntimeError("Please use a DatarateChannel to connect %s downstream from the Internet", node->getModule()->getFullName());
                }
            }
        }
        // Do not delete netLayer here!

        EV << "Configured internet access from " << node->getModule()->getFullName() << " through " << ie->getName() << "\n";
    }
}

void InternetCloudNetworkConfigurator::addDefaultRoutes(cTopology& topo, NodeInfoVector& nodeInfo)
{
    // add default route to nodes with exactly one (non-loopback) interface
    for (int i=0; i<topo.getNumNodes(); i++)
    {
        cTopology::Node *node = topo.getNode(i);

        // skip bus types
        if (!nodeInfo[i].isIPNode)
            continue;

        IInterfaceTable *ift = nodeInfo[i].ift;
        IRoutingTable *rt = nodeInfo[i].rt;

        // count non-loopback interfaces
        int numIntf = 0;
        InterfaceEntry *ie = NULL;
        for (int k=0; k<ift->getNumInterfaces(); k++)
            if (!ift->getInterface(k)->isLoopback())
                {ie = ift->getInterface(k); numIntf++;}

        nodeInfo[i].usesDefaultRoute = (numIntf==1);
        if (numIntf!=1)
            continue; // only deal with nodes with one interface plus loopback

        EV << "  " << node->getModule()->getFullName() << "=" << nodeInfo[i].address
           << " has only one (non-loopback) interface, adding default route\n";

        // add default route
        IPv4Route *e = new IPv4Route();
        e->setDestination(IPv4Address());
        e->setNetmask(IPv4Address());
        e->setInterface(ie);
        e->setSource(IPv4Route::MANUAL);
        //e->getMetric() = 1;
        rt->addRoute(e);
    }
}

void InternetCloudNetworkConfigurator::fillRoutingTables(cTopology& topo, NodeInfoVector& nodeInfo)
{
    // fill in routing tables with static routes
    for (int i=0; i<topo.getNumNodes(); i++)
    {
        cTopology::Node *destNode = topo.getNode(i);

        // skip bus types
        if (!nodeInfo[i].isIPNode)
            continue;

        IPv4Address destAddr = nodeInfo[i].address;
        std::string destModName = destNode->getModule()->getFullName();

        // calculate shortest paths from everywhere towards destNode
        topo.calculateUnweightedSingleShortestPathsTo(destNode);

        // add route (with host=destNode) to every routing table in the network
        // (excepting nodes with only one interface -- there we'll set up a default route)
        for (int j=0; j<topo.getNumNodes(); j++)
        {
            if (i==j)
                continue;
            if (!nodeInfo[j].isIPNode)
                continue;

            cTopology::Node *atNode = topo.getNode(j);
            if (atNode->getNumPaths() == 0)
                continue; // not connected
            if (nodeInfo[j].usesDefaultRoute)
                continue; // already added default route here

            IPv4Address atAddr = nodeInfo[j].address;

            IInterfaceTable *ift = nodeInfo[j].ift;

            int outputGateId = atNode->getPath(0)->getLocalGate()->getId();
            InterfaceEntry *ie = ift->getInterfaceByNodeOutputGateId(outputGateId);
            if (!ie)
                error("%s has no interface for output gate id %d", ift->getFullPath().c_str(), outputGateId);

            EV << "  from " << atNode->getModule()->getFullName() << "=" << IPv4Address(atAddr);
            EV << " towards " << destModName << "=" << IPv4Address(destAddr) << " interface " << ie->getName() << endl;

            // add route
            IRoutingTable *rt = nodeInfo[j].rt;
            IPv4Route *e = new IPv4Route();
            e->setDestination(destAddr);
            e->setNetmask(IPv4Address(255, 255, 255, 255)); // full match needed
            e->setInterface(ie);
            e->setSource(IPv4Route::MANUAL);
            //e->getMetric() = 1;
            rt->addRoute(e);
        }
    }
}

void InternetCloudNetworkConfigurator::handleMessage(cMessage *msg)
{
    error("this module doesn't handle messages, it runs only in initialize()");
}

void InternetCloudNetworkConfigurator::setDisplayString(cTopology& topo, NodeInfoVector& nodeInfo)
{
    int numIPNodes = 0;
    for (int i=0; i<topo.getNumNodes(); i++)
        if (nodeInfo[i].isIPNode)
            numIPNodes++;

    // update display string
    char buf[80];
    sprintf(buf, "%d IPv4 nodes\n%d non-IPv4 nodes", numIPNodes, topo.getNumNodes()-numIPNodes);
    getDisplayString().setTagArg("t", 0, buf);
}

