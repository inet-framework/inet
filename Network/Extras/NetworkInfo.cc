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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#include "NetworkInfo.h"

#include "RoutingTable.h"
#include "IPAddressResolver.h"

#include <fstream>
#include <algorithm>

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
    cModule *target = simulation.moduleByPath(node.getAttribute("target"));

    if (!strcmp(node.getTagName(), "routing"))
    {
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
    s.open(filename, append?(std::ios::app):(std::ios::out));
    if (s.fail())
        error("cannot open `%s' for write", filename);
        
    if(compat) s << "Kernel IP routing table" << endl;
    s << "Destination     Gateway         Genmask         ";
    if(compat) s << "Flags ";
    s << "Metric ";
    if(compat) s << "Ref    Use ";
    s << "Iface" << endl;
    
    cModule *rtmod = target->submodule("routingTable");                                         
    if (rtmod)
    {
        std::vector<std::string> lines;
        
        RoutingTable *rt = check_and_cast<RoutingTable *>(rtmod);
        for (int i = 0; i < rt->numRoutingEntries(); i++)
        {
            IPAddress host = rt->routingEntry(i)->host;
            
            if(host.isMulticast())
                continue;
                
            if(rt->routingEntry(i)->interfacePtr->isLoopback())
                continue;
                
            IPAddress netmask = rt->routingEntry(i)->netmask;
            IPAddress gateway = rt->routingEntry(i)->gateway;
            int metric = rt->routingEntry(i)->metric;
                
            std::ostringstream line; 
    
            line << std::left;
            IPAddress dest = compat?host.doAnd(netmask):host;
            line.width(16);
            if(dest.isUnspecified()) line << "0.0.0.0"; else line << dest;
            
            line.width(16);
            if(gateway.isUnspecified()) line << "0.0.0.0"; else line << gateway;
            
            line.width(16);
            if(netmask.isUnspecified()) line << "0.0.0.0"; else line << netmask;
            
            if(compat) 
            {
                int pad = 3;
                line << "U"; // routes in INET are always up
                if(!gateway.isUnspecified()) line << "G"; else ++pad;
                if(netmask.equals(IPAddress::ALLONES_ADDRESS)) line << "H"; else ++pad;
                line.width(pad);
                line << " ";
            }
            
            line.width(7);
            if(compat && rt->routingEntry(i)->source == RoutingEntry::IFACENETMASK) metric = 0;
            line << metric;
    
            if(compat) line << "0        0 ";
            
            line << rt->routingEntry(i)->interfaceName << endl;

            if(compat) lines.push_back(line.str());
            else s << line.str();
        }
        
        if(compat)
        {
            // sort to avoid random order 
            // typically routing tables are sorted by netmask prefix length (descending)
            // sorting by reversed natural order looks weired, but allows easy comparison
            // with `route -n | sort -r` output by means of `diff` command...
            std::sort(lines.begin(), lines.end());
            for(std::vector<std::string>::reverse_iterator it = lines.rbegin(); it != lines.rend(); it++)
            {
                s << *it;
            }
        }
    }
    s << endl;   
    s.close();
}
