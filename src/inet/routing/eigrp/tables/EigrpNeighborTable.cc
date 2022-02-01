//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
/**
 * @author Jan Zavrel (honza.zavrel96@gmail.com)
 * @author Jan Bloudicek (jbloudicek@gmail.com)
 * @author Vit Rek (rek@kn.vutbr.cz)
 * @author Vladimir Vesely (ivesely@fit.vutbr.cz)
 * @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
 */

// IPv6 ready
#include "inet/routing/eigrp/tables/EigrpNeighborTable.h"
namespace inet {
namespace eigrp {

Define_Module(EigrpIpv4NeighborTable); // TODO - PROB-01 - How to register template class? Define_Module(EigrpNeighborTable<IPv4Address>)

#ifndef DISABLE_EIGRP_IPV6
Define_Module(EigrpIpv6NeighborTable);
#endif /* DISABLE_EIGRP_IPV6 */

template<typename IPAddress>
std::ostream& operator<<(std::ostream& os, const EigrpNeighbor<IPAddress>& neigh)
{
//    const char *state = neigh.isStateUp() ? "up" : "pending";

    os << "ID:" << neigh.getNeighborId();
    os << "  Address:" << neigh.getIPAddress();
    os << "  IF:" << neigh.getIfaceName() << "(" << neigh.getIfaceId() << ")";
    os << "  HoldInt:" << neigh.getHoldInt();
//    os << "  state is " << state;
    os << "  SeqNum:" << neigh.getSeqNumber();
//    os << "  waitForAck:" << neigh.getAck();
    /*if (neigh.isStubEnabled())
    {
        os << "  stub:enabled (";
        EigrpStub stub = neigh.getStubConf();
        if (stub.connectedRt) os << "connected ";
        if (stub.leakMapRt) os << "leak-map ";
        if (stub.recvOnlyRt) os << "recv-only ";
        if (stub.redistributedRt) os << "redistrib ";
        if (stub.staticRt) os << "static ";
        if (stub.summaryRt) os << "summary ";
        os << ")";
    }
    else
        os << "  stub:disabled";*/
    return os;
}

// Must be there (cancelHoldTimer method)
template<typename IPAddress>
EigrpNeighborTable<IPAddress>::~EigrpNeighborTable()
{
/*
    int cnt = neighborVec.size();
    EigrpNeighbor<IPAddress> *neigh;

    for (int i = 0; i < cnt; i++)
    {
        neigh = neighborVec[i];
//        cancelHoldTimer(neigh);
        delete neigh;
    }
    neighborVec.clear();
*/
}

template<typename IPAddress>
void EigrpNeighborTable<IPAddress>::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        WATCH_PTRVECTOR(neighborVec);
    }
}

template<typename IPAddress>
void EigrpNeighborTable<IPAddress>::handleMessage(cMessage *msg)
{
    throw cRuntimeError("This module does not process messages");
}

template<typename IPAddress>
EigrpNeighbor<IPAddress> *EigrpNeighborTable<IPAddress>::findNeighbor(const IPAddress& ipAddress)
{
    typename NeighborVector::iterator it;
    EigrpNeighbor<IPAddress> *neigh;

    for (it = neighborVec.begin(); it != neighborVec.end(); it++) {
        neigh = *it;
        if (neigh->getIPAddress() == ipAddress) {
            return neigh;
        }
    }

    return nullptr;
}

template<typename IPAddress>
EigrpNeighbor<IPAddress> *EigrpNeighborTable<IPAddress>::findNeighborById(int id)
{
    typename NeighborVector::iterator it;

    for (it = neighborVec.begin(); it != neighborVec.end(); it++) {
        if ((*it)->getNeighborId() == id) {
            return *it;
        }
    }

    return nullptr;
}

template<typename IPAddress>
int EigrpNeighborTable<IPAddress>::addNeighbor(EigrpNeighbor<IPAddress> *neighbor)
{
    neighbor->setNeighborId(neighborCounter);
    this->neighborVec.push_back(neighbor);
    return neighborCounter++;
}

/**
 * Removes neighbor form the table, but the record still exists.
 */
template<typename IPAddress>
EigrpNeighbor<IPAddress> *EigrpNeighborTable<IPAddress>::removeNeighbor(EigrpNeighbor<IPAddress> *neighbor)
{
    typename NeighborVector::iterator it;

    for (it = neighborVec.begin(); it != neighborVec.end(); ++it) {
        if (*it == neighbor) {
            neighborVec.erase(it);
            return neighbor;
        }
    }

    return nullptr;
}

template<typename IPAddress>
EigrpNeighbor<IPAddress> *EigrpNeighborTable<IPAddress>::getFirstNeighborOnIf(int ifaceId)
{
    typename NeighborVector::iterator it;

    for (it = neighborVec.begin(); it != neighborVec.end(); ++it) {
        if ((*it)->getIfaceId() == ifaceId)
            return *it;
    }

    return nullptr;
}

template<typename IPAddress>
int EigrpNeighborTable<IPAddress>::setAckOnIface(int ifaceId, uint32_t ackNum)
{
    typename NeighborVector::iterator it;
    int neighCnt = 0;

    for (it = neighborVec.begin(); it != neighborVec.end(); ++it) {
        if ((*it)->getIfaceId() == ifaceId) {
            neighCnt++;
            (*it)->setAck(ackNum);
        }
    }

    return neighCnt;
}

template class EigrpNeighborTable<Ipv4Address>;

#ifndef DISABLE_EIGRP_IPV6
template class EigrpNeighborTable<Ipv6Address>;
#endif /* DISABLE_EIGRP_IPV6 */
} // eigrp
} // inet

