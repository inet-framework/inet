// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __AODV_TIMING_TEST_PEER_H
#define __AODV_TIMING_TEST_PEER_H

#include "AodvPeer.h"

class AodvTimingPeer : public AodvPeer
{
    std::vector<int> ttls;
    std::vector<simtime_t> arrivals;
  protected:
    void handleMessage(cMessage *message) override {
        if (message != timer) {
            AodvPeer::handleMessage(message);
            return;
        }
        if (step++ == 0 && par("retainedHops").intValue() >= 0) {
            auto route = routes->createRoute();
            route->setDestination(destination);
            route->setPrefixLength(32);
            route->setNextHop(peer);
            auto interfaces = check_and_cast<IInterfaceTable *>(getModuleByPath("^.^.dut.interfaceTable"));
            route->setInterface(interfaces->findInterfaceByName("wlan0"));
            route->setMetric(par("retainedHops"));
            route->setSourceType(IRoute::AODV);
            route->setSource(getModuleByPath("^.^.dut.aodv"));
            auto data = new AodvRouteData();
            data->setIsActive(false);
            data->setHasValidDestNum(true);
            data->setDestSeqNum(7);
            data->setLifeTime(0.5);
            route->setProtocolData(data);
            routes->addRoute(route);
            scheduleAt(2, timer);
            return;
        }
        std::vector<int> expected;
        cStringTokenizer ttlTokens(par("expectedTtls"));
        while (ttlTokens.hasMoreTokens())
            expected.push_back(std::stoi(ttlTokens.nextToken()));
        ASSERT(ttls == expected);
        cStringTokenizer intervalTokens(par("expectedIntervals"));
        size_t index = 1;
        while (intervalTokens.hasMoreTokens()) {
            simtime_t interval(std::stod(intervalTokens.nextToken()));
            ASSERT(index < arrivals.size());
            ASSERT(std::abs((arrivals[index] - arrivals[index - 1] - interval).dbl()) < 1e-6);
            index++;
        }
        ASSERT(index == arrivals.size());
        ASSERT(AodvDataCounters::created == 1 && AodvDataCounters::destroyed == 1);
        ASSERT(AodvDataCounters::errors == 0);
        pass("DiscoveryTiming");
    }
    void socketDataArrived(UdpSocket *socket, Packet *packet) override {
        if (auto request = dynamicPtrCast<const Rreq>(packet->peekAtFront<AodvControlPacket>())) {
            ttls.push_back(packet->getTag<HopLimitInd>()->getHopLimit());
            arrivals.push_back(simTime());
            ASSERT(request->getRreqId() == ttls.size());
            ASSERT(request->getOriginatorSeqNum() == ttls.size());
            ASSERT(request->getUnknownSeqNumFlag() == (par("retainedHops").intValue() < 0));
            if (!request->getUnknownSeqNumFlag())
                ASSERT(request->getDestSeqNum() == 7);
        }
        AodvPeer::socketDataArrived(socket, packet);
    }
};
Define_Module(AodvTimingPeer);
#endif
