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

#ifndef __INET_EIGRPROUTE_H
#define __INET_EIGRPROUTE_H

#include <omnetpp.h>

#include <algorithm>

#include "inet/common/stlutils.h"
#include "inet/routing/eigrp/messages/EigrpMessage_m.h"
#include "inet/routing/eigrp/pdms/EigrpMetricHelper.h"

namespace inet {

template<typename IPAddress> class EigrpRouteSource;

/**
 * Network destination, it contains state for DUAL.
 */
template<typename IPAddress>
class INET_API EigrpRoute : public cObject
{
  protected:
    int routeId;                        /** Unique ID of route */

    IPAddress routeAddress;             /**< IP address of destination */
    IPAddress routeMask;                /**< Mask of destination */

    int queryOrigin;                    /**< State of DUAL */
    std::vector<int> replyStatusTable;  /**< Reply status for each neighbor*/
    uint64_t fd;                        /**< Feasible distance */
    EigrpWideMetricPar rd;              /**< Parameters for computation of reported distance that belongs to this router */
    uint64_t Dij;                       /**< Shortest distance (Dij) */
    EigrpRouteSource<IPAddress> *successor; /**< Actual successor for route reported to neighbors of router */
    int numSuccessors;                  /**< Number of successors for the route */
    bool updateSent;                    /**< Sent update with the route */
    int numOfMsgsSent;                  /**< Number of sent messages with the route */

    int referenceCounter;               /**< Counts amount of references to this object. */

  public:
    EigrpRoute(const IPAddress& address, const IPAddress& mask);
    virtual ~EigrpRoute();

    bool operator==(const EigrpRoute<IPAddress>& route) const
    {
        return routeId == route.getRouteId();
    }

    int decrementRefCnt() { return --referenceCounter; }
    void incrementRefCnt() { ++referenceCounter; }
    int getRefCnt() { return referenceCounter; }

    int getRouteId() const { return routeId; }
    void setRouteId(int routeId) { this->routeId = routeId; }

    uint64_t getFd() const { return fd; }
    void setFd(uint64_t fd) { this->fd = fd; }

    EigrpWideMetricPar getRdPar() const { return rd; }
    void setRdPar(EigrpWideMetricPar rd) { this->rd = rd; }

    uint64_t getDij() const { return Dij; }
    void setDij(uint64_t Dij) { this->Dij = Dij; }

    int getQueryOrigin() const { return queryOrigin; }
    void setQueryOrigin(int queryOrigin) { this->queryOrigin = queryOrigin; }

    int getReplyStatusSum() const { return replyStatusTable.size(); }
    bool getReplyStatus(int neighborId);
    void setReplyStatus(int neighborId) { replyStatusTable.push_back(neighborId); }
    bool unsetReplyStatus(int neighborId);

    IPAddress getRouteAddress() const { return routeAddress; }
    void setRouteAddress(IPAddress routeAddress) { this->routeAddress = routeAddress; }

    IPAddress getRouteMask() const { return routeMask; }
    void setRouteMask(IPAddress routeMask) { this->routeMask = routeMask; }

    bool isActive() const { return replyStatusTable.size() > 0; }

    EigrpRouteSource<IPAddress> *getSuccessor() const { return this->successor; }
    void setSuccessor(EigrpRouteSource<IPAddress> *successor) { this->successor = successor; }

    void setNumSucc(int numSuccessors) { this->numSuccessors = numSuccessors; }
    int getNumSucc() const { return numSuccessors; }

    void setUnreachable() { Dij = eigrp::EigrpMetricHelper::METRIC_INF; rd.delay = eigrp::EigrpMetricHelper::DELAY_INF; rd.bandwidth = eigrp::EigrpMetricHelper::BANDWIDTH_INF; }

    bool isUpdateSent() const { return updateSent; }
    void setUpdateSent(bool updateSent) { this->updateSent = updateSent; }

    void setNumSentMsgs(int num) { this->numOfMsgsSent = num; }
    int getNumSentMsgs() const { return numOfMsgsSent; }
};

/**
 * Class represents EIGRP route to a destination network.
 */
template<typename IPAddress>
class INET_API EigrpRouteSource : public cObject
{
  protected:
    int sourceId;                       /** Unique ID of source */
    int routeId;                        /** Unique ID of route (same as in EigrpRoute) */

    Ipv4Address originator;               /**< IP of originating router */      //TODO - PROB-04 - is it address or RouterID? (I think it should be routerID)
    int nextHopId;                      /**< Id of next hop neighbor (usually same as sourceId, 0 -> connected) */
    IPAddress nextHop;                  /**< IP address of next hop router (0.0.0.0 -> connected), only informational. It does not correspond to the sourceId (next hop may not be source of the route). */
    int interfaceId;                    /** ID of outgoing interface for next hop */
    const char *interfaceName;
    bool internal;                      /**< Source of the route (internal or external) */
    uint64_t rd;                        /**< Reported distance from neighbor (RDkj) */
    uint64_t metric;                    /**< Actual metric value via that next Hop (not Dij - shortest distance) */
    EigrpWideMetricPar metricParams;    /**< Parameters for metric computation */
    EigrpWideMetricPar rdParams;        /**< Parameters from neighbor */
    bool successor;                     /**< Source is successor for route */
    bool summary;                       /**< Summarized route */
    bool redistributed;                 /**< Redistributed route */
    bool valid;                         /**< Invalid sources will be deleted */
    int delayedRemoveNID;                 /**< Source will be deleted after receiving Ack from neighbor with ID equal to NID */

    EigrpRoute<IPAddress> *routeInfo;   /**< Pointer to route */

  public:
    EigrpRouteSource(int interfaceId, const char *ifaceName, int nextHopId, int routeId, EigrpRoute<IPAddress> *routeInfo);
    virtual ~EigrpRouteSource();

    bool operator==(const EigrpRouteSource<IPAddress>& source) const
    {
        return sourceId == source.getSourceId();
    }

    uint64_t getRd() const { return rd; }
    void setRd(uint64_t rd) { this->rd = rd; }

    bool isInternal() const { return internal; }
    void setInternal(bool internal) { this->internal = internal; }

    uint64_t getMetric() const { return metric; }
    void setMetric(uint64_t metric) { this->metric = metric; }

    IPAddress getNextHop() const { return nextHop; }
    void setNextHop(IPAddress& nextHop) { this->nextHop = nextHop; }

    EigrpWideMetricPar getMetricParams() const { return metricParams; }
    void setMetricParams(EigrpWideMetricPar& par) { this->metricParams = par; }

    EigrpWideMetricPar getRdParams() const { return rdParams; }
    void setRdParams(const EigrpWideMetricPar& rdParams) { this->rdParams = rdParams; }

    bool isSuccessor() const { return successor; }
    void setSuccessor(bool successor) { this->successor = successor; }

    int getNexthopId() const { return nextHopId; }
    void setNexthopId(int nextHopId) { this->nextHopId = nextHopId; }

    int getIfaceId() const { return interfaceId; }
    void setIfaceId(int interfaceId) { this->interfaceId = interfaceId; }

    EigrpRoute<IPAddress> *getRouteInfo() const { return routeInfo; }
    void setRouteInfo(EigrpRoute<IPAddress> *routeInfo) { this->routeInfo = routeInfo; }

    int getRouteId() const { return routeId; }
    void setRouteId(int routeId) { this->routeId = routeId; }

    int getSourceId() const { return sourceId; }
    void setSourceId(int sourceId) { this->sourceId = sourceId; }

    Ipv4Address getOriginator() const { return originator; }
    void setOriginator(const Ipv4Address& originator) { this->originator = originator; }

    bool isValid() const { return valid; }
    void setValid(bool valid) { this->valid = valid; }

    bool isUnreachable() const { return metric == eigrp::EigrpMetricHelper::METRIC_INF; }
    /** Sets metric and RD to infinity */
    void setUnreachableMetric()
    {
        metric = eigrp::EigrpMetricHelper::METRIC_INF;
        metricParams.bandwidth = eigrp::EigrpMetricHelper::BANDWIDTH_INF;
        metricParams.delay = eigrp::EigrpMetricHelper::DELAY_INF;
        rd = eigrp::EigrpMetricHelper::METRIC_INF;
        rdParams.bandwidth = eigrp::EigrpMetricHelper::BANDWIDTH_INF;
        rdParams.delay = eigrp::EigrpMetricHelper::DELAY_INF;
    }

    int getDelayedRemove() const { return delayedRemoveNID; }
    void setDelayedRemove(int neighID) { this->delayedRemoveNID = neighID; }

    const char *getIfaceName() const { return interfaceName; }

    bool isRedistributed() const { return redistributed; }
    void setRedistributed(bool redistributed) { this->redistributed = redistributed; }

    bool isSummary() const { return summary; }
    void setSummary(bool summary) { this->summary = summary; }
};

template class EigrpRouteSource<Ipv4Address>;

template<typename IPAddress>
EigrpRouteSource<IPAddress>::EigrpRouteSource(int interfaceId, const char *ifaceName, int nextHopId, int routeId, EigrpRoute<IPAddress> *routeInfo) :
    routeId(routeId), nextHopId(nextHopId), interfaceId(interfaceId), interfaceName(ifaceName), routeInfo(routeInfo)
{
    metric = eigrp::EigrpMetricHelper::METRIC_INF;
    rd = 0;
    internal = true;
    successor = false;
    sourceId = 0;
    valid = true;
    delayedRemoveNID = 0;

    summary = false;
    redistributed = false;

    routeInfo->incrementRefCnt();
}

template<typename IPAddress>
EigrpRouteSource<IPAddress>::~EigrpRouteSource()
{
}

template<typename IPAddress>
EigrpRoute<IPAddress>::EigrpRoute(const IPAddress& address, const IPAddress& mask) :
    routeId(0), routeAddress(address), routeMask(mask)
{
    fd = Dij = eigrp::EigrpMetricHelper::METRIC_INF;
    rd.delay = eigrp::EigrpMetricHelper::METRIC_INF;

    successor = nullptr;
    queryOrigin = 1;

    numSuccessors = 0;
    referenceCounter = 0;

    updateSent = false;
    numOfMsgsSent = 0;
}

template<typename IPAddress>
EigrpRoute<IPAddress>::~EigrpRoute()
{
}

template<typename IPAddress>
bool EigrpRoute<IPAddress>::getReplyStatus(int neighborId)
{
    return contains(replyStatusTable, neighborId);
}

/**
 * Clear handle for specified neighbor in Reply Status table.
 */
template<typename IPAddress>
bool EigrpRoute<IPAddress>::unsetReplyStatus(int neighborId)
{
    auto it = find(replyStatusTable, neighborId);
    if (it != replyStatusTable.end()) {
        replyStatusTable.erase(it);
        return true;
    }
    return false;
} // eigrp

} // inet
#endif

