//
// Copyright (C) 2008 Andras Varga
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

#ifndef __INET_IPROUTE_H
#define __INET_IPROUTE_H

#include <vector>
#include <omnetpp.h>
#include "INETDefs.h"
#include "IPAddress.h"

class InterfaceEntry;

/**
 * IPv4 route in IRoutingTable.
 *
 * @see IRoutingTable, IRoutingTable
 */
class INET_API IPRoute : public cPolymorphic
{
  public:
    /** Route type */
    enum RouteType
    {
        DIRECT,  ///< Directly attached to the router
        REMOTE   ///< Reached through another router
    };

    /** Specifies where the route comes from */
    enum RouteSource
    {
        MANUAL,       ///< manually added static route
        IFACENETMASK, ///< comes from an interface's netmask
        RIP,          ///< managed by the given routing protocol
        OSPF,         ///< managed by the given routing protocol
        BGP,          ///< managed by the given routing protocol
        ZEBRA,        ///< managed by the Quagga/Zebra based model
        MANET,        ///< managed by manet, search exact address
        MANET2,       ///< managed by manet, search approximate address
    };

  protected:
    IPAddress host;     ///< Destination
    IPAddress netmask;  ///< Route mask
    IPAddress gateway;  ///< Next hop
    InterfaceEntry *interfacePtr; ///< interface
    RouteType type;     ///< direct or remote
    RouteSource source; ///< manual, routing prot, etc.
    int metric;         ///< Metric ("cost" to reach the destination)
// DSDV protocol
    //Originated from destination.Ensures loop freeness.
    unsigned int sequencenumber;
    //Time of routing table entry creation
    simtime_t installtime;


  private:
    // copying not supported: following are private and also left undefined
    IPRoute(const IPRoute& obj);
    IPRoute& operator=(const IPRoute& obj);

  public:
    IPRoute();
    virtual ~IPRoute() {}
    virtual std::string info() const;
    virtual std::string detailedInfo() const;

    void setHost(IPAddress host)  {this->host = host;}
    void setNetmask(IPAddress netmask)  {this->netmask = netmask;}
    void setGateway(IPAddress gateway)  {this->gateway = gateway;}
    void setInterface(InterfaceEntry *interfacePtr)  {this->interfacePtr = interfacePtr;}
    void setType(RouteType type)  {this->type = type;}
    void setSource(RouteSource source)  {this->source = source;}
    void setMetric(int metric)  {this->metric = metric;}

    /** Destination address prefix to match */
    IPAddress getHost() const {return host;}

    /** Represents length of prefix to match */
    IPAddress getNetmask() const {return netmask;}

    /** Next hop address */
    IPAddress getGateway() const {return gateway;}

    /** Next hop interface */
    InterfaceEntry *getInterface() const {return interfacePtr;}

    /** Convenience method */
    const char *getInterfaceName() const;

    /** Route type: Direct or Remote */
    RouteType getType() const {return type;}

    /** Source of route. MANUAL (read from file), from routing protocol, etc */
    RouteSource getSource() const {return source;}

    /** "Cost" to reach the destination */
    int getMetric() const {return metric;}

    simtime_t getInstallTime() const {return installtime;}
    void setInstallTime(simtime_t time) {installtime = time;}
    void setSequencenumber(int i){sequencenumber =i;}
    unsigned int getSequencenumber() const {return sequencenumber;}

};

#endif

