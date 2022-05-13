//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
/**
 * @file EigrpNeighbor.h
 * @brief File contains interface for neighbor table.
 * @date 7.2.2014
 * @author Jan Zavrel (honza.zavrel96@gmail.com)
 * @author Jan Bloudicek (jbloudicek@gmail.com)
 * @author Vit Rek (rek@kn.vutbr.cz)
 * @author Vladimir Vesely (ivesely@fit.vutbr.cz)
 * @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
 */

#ifndef __INET_EIGRPNEIGHBOR_H
#define __INET_EIGRPNEIGHBOR_H

// IPv6 ready

#include <omnetpp.h>

#include "inet/routing/eigrp/EigrpTimer_m.h"
#include "inet/routing/eigrp/messages/EigrpMessage_m.h"
namespace inet {
namespace eigrp {
/**
 * Class represents one entry in EIGRP Neighbor Table.
 */
template<typename IPAddress>
class INET_API EigrpNeighbor : public cObject
{
  protected:
    int ifaceId;                /**< ID of interface that is connected to the neighbor. */
    const char *ifaceName;
    int neighborId;             /**< ID of neighbor */
    IPAddress ipAddress;        /**< IP address of neighbor. */

    bool isUp;                  /**< State of neighbor. Possible states are pending or up. */
    int holdInt;                /**< Neighbor's Hold interval used to set Hold timer */
    uint32_t seqNumber;         /**< Sequence number received from neighbor */
    bool routesForDeletion;     /**< After receiving Ack from the neighbor will be removed unreachable routes from TT */
    uint32_t waitForAck;        /**< Waiting for ack number from neighbor (for reliable transport) */
    EigrpTimer *holdt;          /**< Pointer to Hold timer */
    bool stubEnabled;           /**< Neighbor is a stub */
    EigrpStub stubConf;         /**< Neighbor's stub configuration */

  public:
    static const int UNSPEC_ID = 0;

    virtual ~EigrpNeighbor() { delete holdt; }
    EigrpNeighbor(int ifaceId, const char *ifaceNname, IPAddress ipAddress) :
        ifaceId(ifaceId), ifaceName(ifaceNname), neighborId(UNSPEC_ID), ipAddress(ipAddress), isUp(false), holdt(nullptr)
    { seqNumber = 0; holdInt = 0; routesForDeletion = false; waitForAck = 0; stubEnabled = false; }

    void setStateUp(bool stateUp) { this->isUp = stateUp; }
    void setHoldInt(int holdInt) { this->holdInt = holdInt; }
    /**
     * Sets timer for a neighbor
     */
    void setHoldTimer(EigrpTimer *holdt) { ASSERT(this->holdt == nullptr); this->holdt = holdt; }
    void setNeighborId(int neighborId) { this->neighborId = neighborId; }
    void setSeqNumber(int seqNumber) { this->seqNumber = seqNumber; }
    void setAck(uint32_t waitForAck) { this->waitForAck = waitForAck; }
    void setRoutesForDeletion(bool routesForDeletion) { this->routesForDeletion = routesForDeletion; }
    void setStubConf(const EigrpStub& stubConf) { this->stubConf = stubConf; }
    void setStubEnable(bool stubEnabled) { this->stubEnabled = stubEnabled; }

    IPAddress getIPAddress() const { return this->ipAddress; }
    int getIfaceId() const { return this->ifaceId; }
    bool isStateUp() const { return this->isUp; }
    int getHoldInt() const { return this->holdInt; }
    EigrpTimer *getHoldTimer() const { return this->holdt; }
    int getNeighborId() const { return neighborId; }
    int getSeqNumber() const { return seqNumber; }
    uint32_t getAck() const { return waitForAck; }
    bool getRoutesForDeletion() const { return this->routesForDeletion; }
    EigrpStub getStubConf() const { return this->stubConf; }
    bool isStubEnabled() const { return this->stubEnabled; }

    const char *getIfaceName() const { return ifaceName; }
};
} // eigrp
} // inet
#endif

