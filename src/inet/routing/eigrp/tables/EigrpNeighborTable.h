//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//
/**
 * @author Jan Zavrel (honza.zavrel96@gmail.com)
 * @author Jan Bloudicek (jbloudicek@gmail.com)
 * @author Vit Rek (rek@kn.vutbr.cz)
 * @author Vladimir Vesely (ivesely@fit.vutbr.cz)
 * @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
 */

#ifndef __INET_EIGRPNEIGHBORTABLE_H_
#define __INET_EIGRPNEIGHBORTABLE_H_

//IPv6 ready

#include <omnetpp.h>

#include "inet/routing/eigrp/EigrpDualStack.h"
#include "inet/routing/eigrp/tables/EigrpNeighbor.h"
namespace inet {
namespace eigrp {
/**
 * Class represents EIGRP Neighbor Table.
 */
template<typename IPAddress>
class EigrpNeighborTable : public cSimpleModule
{
  protected:
    typedef typename std::vector<EigrpNeighbor<IPAddress> *> NeighborVector;

    NeighborVector neighborVec;    /**< Table with neighbors. */
    int neighborCounter;            /**< For unique ID of neighbor */
    int stubCount;                  /**< Number of stub neighbors for optimization */

    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }



  public:
    EigrpNeighborTable() { neighborCounter = 1; stubCount = 0; }
    virtual ~EigrpNeighborTable();
    /**
     * Adds neighbor to the table.
     */
    int addNeighbor(EigrpNeighbor<IPAddress> *neighbor);
    /**
     * Finds neighbor by IP address. Returns null if neighbor is not found.
     */
    EigrpNeighbor<IPAddress> *findNeighbor(const IPAddress& ipAddress);
    /**
     * Finds neighbor by ID.
     */
    EigrpNeighbor<IPAddress> *findNeighborById(int id);
    EigrpNeighbor<IPAddress> * removeNeighbor(EigrpNeighbor<IPAddress> *neighbor);
    /**
     * Returns first neighbor that resides on specified interface.
     */
    EigrpNeighbor<IPAddress> *getFirstNeighborOnIf(int ifaceId);
    int getNumNeighbors() const { return neighborVec.size(); }
    EigrpNeighbor<IPAddress> *getNeighbor(int k) const { return neighborVec[k]; }
    int setAckOnIface(int ifaceId, uint32_t ackNum);
    int getStubCount() const { return stubCount; }
    void incStubCount() { stubCount++; }
    void decStubCount() { stubCount--; }
};

//TODO - mozna predelat kontejnery do samostatnych souboru
//typedef EigrpNeighborTable<IPv4Address> EigrpIpv4NeighborTable;       //IPv6 - ADDED for backward compatibility with old IPv4-only version

class EigrpIpv4NeighborTable : public EigrpNeighborTable<Ipv4Address>
{
//container class for IPv4NT, must exist because of Define_Module()
public:
    virtual ~EigrpIpv4NeighborTable() {};
};

/*
class INET_API Eigrpv4NeighTableAccess : public ModuleAccess<EigrpIpv4NeighborTable>
{
    public:
    Eigrpv4NeighTableAccess() : ModuleAccess<EigrpIpv4NeighborTable>("eigrpIpv4NeighborTable") {}
};
*/


#ifndef DISABLE_EIGRP_IPV6
class EigrpIpv6NeighborTable : public EigrpNeighborTable<Ipv6Address>
{
//container class for IPv6NT, must exist because of Define_Module()
public:
    virtual ~EigrpIpv6NeighborTable() {};
};

/*
class INET_API Eigrpv6NeighTableAccess : public ModuleAccess<EigrpIpv6NeighborTable>
{
    public:
    Eigrpv6NeighTableAccess() : ModuleAccess<EigrpIpv6NeighborTable>("eigrpIpv6NeighborTable") {}
};
*/
#endif // DISABLE_EIGRP_IPV6
} //eigrp
} //inet
#endif
