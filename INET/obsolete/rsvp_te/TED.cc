/*******************************************************************
*
*    This library is free software, you can redistribute it
*    and/or modify
*    it under  the terms of the GNU Lesser General Public License
*    as published by the Free Software Foundation;
*    either version 2 of the License, or any later version.
*    The library is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*    See the GNU Lesser General Public License for more details.
*
*
*********************************************************************/

#include "TED.h"
#include "IPAddressResolver.h"
#include "InterfaceTableAccess.h"
#include "IPv4InterfaceData.h"
#include "RoutingTableAccess.h"


Define_Module(TED);


int TED::tedModuleId;


std::ostream & operator<<(std::ostream & os, const TELinkState & linkstate)
{
    os << "AdvRte:" << linkstate.advrouter;
    os << "  t:" << linkstate.type;
    os << "  id:" << linkstate.linkid;
    os << "  loc:" << linkstate.local;
    os << "  rem:" << linkstate.remote;
    os << "  M:" << linkstate.metric;
    os << "  BW:" << linkstate.MaxBandwith;
    os << "  rBW:" << linkstate.MaxResvBandwith;
    os << "  urBW[0]:" << linkstate.UnResvBandwith[0];
    os << "  AdmGrp:" << linkstate.AdminGrp;
    return os;
};

TED *TED::getGlobalInstance()
{
    cModule *m;
    if (tedModuleId==0 || dynamic_cast<TED *>(m = simulation.module(tedModuleId)) == NULL)
        opp_error("TED::getGlobalInstance(): TED module not yet initialized");
    return (TED *) m;
}

void TED::initialize(int stage)
{
    // we have to wait for stage 2 until interfaces get registered (stage 0)
    // and get their auto-assigned IP addresses (stage 2)
    if (stage!=3)
        return;

    if (dynamic_cast<TED *>(simulation.module(tedModuleId)) != NULL)
        opp_error("A TED module already exists in the network -- there should be only one");

    tedModuleId = id();

    buildDatabase();
    printDatabase();

    WATCH_VECTOR(ted);
}

TED::~TED()
{
    tedModuleId = 0;
}

const std::vector < TELinkState > &TED::getTED()
{
    Enter_Method("getTED()");
    return ted;
}


void TED::updateTED(const std::vector < TELinkState > &copy)
{
    Enter_Method("updateTED()");
    ted = copy;
}


void TED::buildDatabase()
{
    if (!(ted.empty()))
        ted.clear();

    cTopology topo;
    const char *moduleTypes = par("moduleTypes").stringValue();
    std::vector<std::string> types = cStringTokenizer(moduleTypes, " ").asVector();
    topo.extractByModuleType(types);
    ev << "Total number of RSVP LSR nodes = " << topo.nodes() << "\n";

    for (int i = 0; i < topo.nodes(); i++)
    {
        sTopoNode *node = topo.node(i);
        cModule *module = node->module();

        InterfaceTable *myIFT = IPAddressResolver().interfaceTableOf(module);
        RoutingTable *myRT = IPAddressResolver().routingTableOf(module);
        IPAddress modAddr = myRT->getRouterId();
        if (modAddr.isUnspecified())
            modAddr = IPAddressResolver().getAddressFrom(myIFT).get4();

        for (int j = 0; j < node->outLinks(); j++)
        {
            cModule *neighbour = node->out(j)->remoteNode()->module();

            InterfaceTable *neighbourIFT = IPAddressResolver().interfaceTableOf(neighbour);
            RoutingTable *neighbourRT = IPAddressResolver().routingTableOf(neighbour);
            IPAddress neighbourAddr = neighbourRT->getRouterId();
            if (neighbourAddr.isUnspecified())
                neighbourAddr = IPAddressResolver().getAddressFrom(neighbourIFT).get4();

            // For each link
            // Get linkId
            TELinkState entry;

            entry.advrouter = modAddr;
            entry.linkid = neighbourAddr;

            int local_gateIndex = node->out(j)->localGate()->index();
            int remote_gateIndex = node->out(j)->remoteGate()->index();

            // Get local address
            entry.local = myIFT->interfaceByPortNo(local_gateIndex)->ipv4()->inetAddress();
            // Get remote address
            entry.remote = neighbourIFT->interfaceByPortNo(remote_gateIndex)->ipv4()->inetAddress();

            double BW = node->out(j)->localGate()->channel()->par("datarate").doubleValue();
            double delay = node->out(j)->localGate()->channel()->par("delay").doubleValue();
            entry.MaxBandwith = BW;
            entry.MaxResvBandwith = BW;
            entry.metric = delay;
            for (int k = 0; k < 8; k++)
                entry.UnResvBandwith[k] = BW;
            entry.type = 1;

            entry.AdminGrp = 0;

            ted.push_back(entry);
        }
    }

    printDatabase();

    // update display string
    char buf[80];
    sprintf(buf, "%d nodes\n%d directed links", topo.nodes(), ted.size());
    displayString().setTagArg("t",0,buf);
}

void TED::handleMessage(cMessage *)
{
    error("Message arrived to TED -- it doesn't process messages, it is used via direct method calls");
}


void TED::printDatabase()
{
    ev << "*************TED DATABASE*******************\n";
    std::vector < TELinkState >::iterator i;
    for (i = ted.begin(); i != ted.end(); i++)
    {
        const TELinkState & linkstate = *i;
        ev << "Adv Router: " << linkstate.advrouter << "\n";
        ev << "Link Id (neighbour IP): " << linkstate.linkid << "\n";
        ev << "Max Bandwidth: " << linkstate.MaxBandwith << "\n";
        ev << "Metric: " << linkstate.metric << "\n\n";
    }
}


void TED::updateLink(simple_link_t * aLink, double metric, double bw)
{
    Enter_Method("updateLink()");

    for (unsigned int i = 0; i < ted.size(); i++)
    {
        if (ted[i].advrouter.getInt() == aLink->advRouter &&
            ted[i].linkid.getInt() == aLink->id)
        {
            ev << "TED update an entry\n";
            ev << "Advrouter=" << ted[i].advrouter << "\n";
            ev << "linkId=" << ted[i].linkid << "\n";
            double bwIncrease = bw - (ted[i].MaxBandwith);
            ted[i].MaxBandwith = ted[i].MaxBandwith + bwIncrease;
            ted[i].MaxResvBandwith = ted[i].MaxResvBandwith + bwIncrease;
            ev << "Old metric= " << ted[i].metric << "\n";

            ted[i].metric = metric;
            ev << "New metric= " << ted[i].metric << "\n";

            for (int j = 0; j < 8; j++)
                ted[i].UnResvBandwith[j] = ted[i].UnResvBandwith[j] + bwIncrease;
            break;
        }
    }
    printDatabase();
}
