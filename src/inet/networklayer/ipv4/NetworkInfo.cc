//
// Copyright (C) 2007 Vojtech Janota
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include <fstream>
#include <algorithm>

#include "inet/networklayer/ipv4/NetworkInfo.h"

#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/ipv4/IIPv4RoutingTable.h"

namespace inet {

Define_Module(NetworkInfo);

void NetworkInfo::initialize()
{
    // so far no initialization
}

void NetworkInfo::handleMessage(cMessage *msg)
{
    ASSERT(false);
}

void NetworkInfo::processCommand(const cXMLElement& node)
{
    cModule *target = simulation.getModuleByPath(node.getAttribute("target"));

    if (!strcmp(node.getTagName(), "routing")) {
        const char *filename = node.getAttribute("file");
        ASSERT(filename);
        const char *mode = node.getAttribute("mode");
        const char *compat = node.getAttribute("compat");

        dumpRoutingInfo(target, filename, (mode && !strcmp(mode, "a")), (compat && !strcmp(compat, "linux")));
    }
    else
        ASSERT(false);
}

void NetworkInfo::dumpRoutingInfo(cModule *target, const char *filename, bool append, bool compat)
{
    std::ofstream s;
    s.open(filename, append ? (std::ios::app) : (std::ios::out));
    if (s.fail())
        throw cRuntimeError("cannot open `%s' for write", filename);

    if (compat)
        s << "Kernel IPv4 routing table" << endl;
    s << "Destination     Gateway         Genmask         ";
    if (compat)
        s << "Flags ";
    s << "Metric ";
    if (compat)
        s << "Ref    Use ";
    s << "Iface" << endl;

    cModule *rtmod = target->getSubmodule("routingTable");
    if (rtmod) {
        std::vector<std::string> lines;

        IIPv4RoutingTable *rt = check_and_cast<IIPv4RoutingTable *>(rtmod);
        for (int i = 0; i < rt->getNumRoutes(); i++) {
            IPv4Address dest = rt->getRoute(i)->getDestination();

            if (dest.isMulticast())
                continue;

            if (rt->getRoute(i)->getInterface()->isLoopback())
                continue;

            IPv4Address netmask = rt->getRoute(i)->getNetmask();
            IPv4Address gateway = rt->getRoute(i)->getGateway();
            int metric = rt->getRoute(i)->getMetric();

            std::ostringstream line;

            line << std::left;
            IPv4Address prefix = compat ? dest.doAnd(netmask) : dest;    // typically dest in routes is already masked, so this is a no-op
            line.width(16);
            if (prefix.isUnspecified())
                line << "0.0.0.0";
            else
                line << prefix;

            line.width(16);
            if (gateway.isUnspecified())
                line << "0.0.0.0";
            else
                line << gateway;

            line.width(16);
            if (netmask.isUnspecified())
                line << "0.0.0.0";
            else
                line << netmask;

            if (compat) {
                int pad = 3;
                line << "U";    // routes in INET are always up
                if (!gateway.isUnspecified())
                    line << "G";
                else
                    ++pad;
                if (netmask.equals(IPv4Address::ALLONES_ADDRESS))
                    line << "H";
                else
                    ++pad;
                line.width(pad);
                line << " ";
            }

            line.width(7);
            if (compat && rt->getRoute(i)->getSourceType() == IRoute::IFACENETMASK)
                metric = 0;
            line << metric;

            if (compat)
                line << "0        0 ";

            line << rt->getRoute(i)->getInterfaceName() << endl;

            if (compat)
                lines.push_back(line.str());
            else
                s << line.str();
        }

        if (compat) {
            // sort to avoid random order
            // typically routing tables are sorted by netmask prefix length (descending)
            // sorting by reversed natural order looks weired, but allows easy comparison
            // with `route -n | sort -r` output by means of `diff` command...
            std::sort(lines.begin(), lines.end());
            for (std::vector<std::string>::reverse_iterator it = lines.rbegin(); it != lines.rend(); it++)
                s << *it;
        }
    }
    s << endl;
    s.close();
}

} // namespace inet

