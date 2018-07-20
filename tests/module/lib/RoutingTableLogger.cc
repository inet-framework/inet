//
// Copyright (C) 2013 Opensim Ltd.
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

#include <fstream>

#include "inet/common/INETDefs.h"
#include "inet/common/scenario/IScriptable.h"
#include "inet/networklayer/contract/IRoutingTable.h"
#include "inet/networklayer/contract/IL3AddressType.h"

using namespace std;

namespace inet {

struct DestFilter
{
    vector<L3Address> destAddresses;
    vector<int> prefixLengths;

    DestFilter(const char *str);
    bool matches(IRoute *route);
};

DestFilter::DestFilter(const char *str)
{
    if (str)
    {
        cStringTokenizer tokenizer(str);
        while (tokenizer.hasMoreTokens())
        {
            const char *dest = tokenizer.nextToken();
            const char *end = strchr(dest, '/');
            if (end)
            {
                destAddresses.push_back(L3Address(string(dest,end).c_str()));
                prefixLengths.push_back(atoi(end+1));
            }
            else
            {
                L3Address destAddr = L3Address(dest);
                destAddresses.push_back(destAddr);
                prefixLengths.push_back(destAddr.getAddressType()->getMaxPrefixLength());
            }
        }
    }
}

bool DestFilter::matches(IRoute *route)
{
    if (destAddresses.empty())
        return true;

    for (int i = 0; i < (int)destAddresses.size(); ++i)
    {
        L3Address &dest = destAddresses[i];
        int prefixLength = prefixLengths[i];
        if (route->getDestinationAsGeneric().matches(dest, prefixLength))
            return true;
    }
    return false;
}

class INET_API RoutingTableLogger : public cSimpleModule, public IScriptable
{
    ofstream out;
  protected:
    virtual void initialize();
    virtual void finish();
    virtual void processCommand(const cXMLElement &node);
  private:
    void dumpRoutes(cModule *node, IRoutingTable *rt, DestFilter &filter);
};

Define_Module(RoutingTableLogger);

void RoutingTableLogger::initialize()
{
    const char *filename = par("outputFile");
    if (filename && (*filename))
    {
        out.open(filename);
        if (out.fail())
            throw cRuntimeError("Failed to open output file: %s", filename);
    }
}

void RoutingTableLogger::finish()
{
    out << "END" << endl;
}

void RoutingTableLogger::processCommand(const cXMLElement &command) {
    Enter_Method_Silent();

    const char *tag = command.getTagName();

    if (!strcmp(tag, "dump-routes"))
    {
        const char *nodes = command.getAttribute("nodes");
        if (!nodes)
        throw cRuntimeError("missing @nodes attribute");

        DestFilter filter(command.getAttribute("dest"));

        cStringTokenizer tokenizer(nodes);
        while(tokenizer.hasMoreTokens())
        {
            const char *nodeName = tokenizer.nextToken();
            cModule *node = getModuleByPath(nodeName);
            if (!node)
            throw cRuntimeError("module '%s' not found at %s", nodeName, command.getSourceLocation());
            bool foundRt = false;

            for (cModule::SubmoduleIterator nl(node); !nl.end(); nl++)
            {
                for (cModule::SubmoduleIterator i(*nl); !i.end(); i++)
                {
                    if (IRoutingTable *rt = dynamic_cast<IRoutingTable*>(*i)) {
                        foundRt = true;
                        dumpRoutes(node, rt, filter);
                    }
                }
            }
            if (!foundRt)
                throw cRuntimeError("routing table not found in node '%s'", node->getFullPath().c_str());
        }
    }
}

void RoutingTableLogger::dumpRoutes(cModule *node, IRoutingTable *rt, DestFilter &filter)
{
    out << node->getFullName() << " " << simTime() << endl;

    for (int i = 0; i < rt->getNumRoutes(); ++i)
    {
        IRoute *route = rt->getRoute(i);
        if (filter.matches(route))
        {
            out << route->getDestinationAsGeneric() << "/" << route->getPrefixLength()
                << " " << route->getNextHopAsGeneric()
                << " " << (route->getInterface() ? route->getInterface()->getInterfaceName() : "*")
                << " " << IRoute::sourceTypeName(route->getSourceType()) << " " << route->getMetric()
                << endl;
        }
    }
}

} // namespace inet

