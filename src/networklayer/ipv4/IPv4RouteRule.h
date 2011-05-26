//
// Copyright (C) 2008 Andras Varga
// Copyright (C) 2011 Alfonso Ariza
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

#ifndef __INET_IPv4ROUTERULE_H
#define __INET_IPv4ROUTERULE_H

#include <map>

#include "INETDefs.h"

#include "IPProtocolId_m.h"
#include "IPv4Address.h"

class InterfaceEntry;

/**
 * IPv4 route in IRoutingTable.
 *
 * @see IRoutingTable, IRoutingTable
 */
class NatElement : public cPolymorphic
{
  public:
    IPv4Address addr;
    int port;

  private:
    // copying not supported: following are private and also left undefined
    NatElement(const NatElement& obj);
    NatElement& operator=(const NatElement& obj);
};

class INET_API IPv4RouteRule : public cPolymorphic
{
  public:
    /** Specifies where the route comes from */
    enum Rule
    {
        DROP,
        ACCEPT,
        NAT,
        NONE
    };

  protected:
    class Nat
    {
      private:
        std::map<int,NatElement*> natAddress;

      public:
        void addNatAddres(){}
        void delNatAddress(){}
        const NatElement* getNat() const;
        ~Nat();
    };

    IPv4Address srcAddress;     ///< Destination
    IPv4Address srcNetmask;  ///< Route mask
    IPv4Address destAddress;     ///< Destination
    IPv4Address destNetmask;  ///< Route mask
    int sPort;  ///
    int dPort;  ///

    IPProtocolId protocol;
    Rule     rule;
    InterfaceEntry *interfacePtr; ///< interface
    std::map<int, Nat> natRule;

  private:
    // copying not supported: following are private and also left undefined
    IPv4RouteRule(const IPv4RouteRule& obj);
    IPv4RouteRule& operator=(const IPv4RouteRule& obj);

  public:
    IPv4RouteRule();
    ~IPv4RouteRule();
    virtual std::string info() const;
    virtual std::string detailedInfo() const;

    void setSrcAddress(IPv4Address host)  {this->srcAddress = host;}
    void setSrcNetmask(IPv4Address netmask)  {this->srcNetmask = netmask;}
    void setSrcPort(int port)  {this->sPort = sPort;}
    void setDestAddress(IPv4Address host)  {this->destAddress = host;}
    void setDestNetmask(IPv4Address netmask)  {this->destNetmask = netmask;}
    void setDestPort(int port)  {this->dPort = dPort;}


    void setInterface(InterfaceEntry *interfacePtr)  {this->interfacePtr = interfacePtr;}
    void setRoule(Rule rule);
    void setProtocol(IPProtocolId protocol){this->protocol = protocol;}

    IPv4Address getSrcAddress() const {return srcAddress;}
    IPv4Address getSrcNetmask() const {return srcNetmask;}
    const int getSrcPort() const {return sPort;}
    IPv4Address getDestAddress() const {return destAddress;}
    IPv4Address getDestNetmask() const {return destNetmask;}
    const int getDestPort() const {return dPort;}

    const IPProtocolId getProtocol() const {return protocol;}

    /** Next hop interface */
    InterfaceEntry *getInterface() const {return interfacePtr;}

    /** Convenience method */
    const char *getInterfaceName() const;

    /** Route type: Direct or Remote */
    Rule getRule() const {return rule;}
};

#endif // __INET_IPv4ROUTERULE_H

