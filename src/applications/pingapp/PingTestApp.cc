//
// Copyright (C) 2012 OpenSim Ltd.
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

#include <iostream>

#include "PingTestApp.h"

#include "InterfaceEntry.h"
#include "IInterfaceTable.h"
#include "IPvXAddressResolver.h"
#include "PingPayload_m.h"

#ifdef WITH_IPv4
#include "IPv4ControlInfo.h"
#include "IPv4InterfaceData.h"
#endif

#ifdef WITH_IPv6
#include "IPv6ControlInfo.h"
#include "IPv6InterfaceData.h"
#endif

using std::cout;

Define_Module(PingTestApp);

simsignal_t PingTestApp::rttSignal = SIMSIGNAL_NULL;
simsignal_t PingTestApp::numLostSignal = SIMSIGNAL_NULL;
simsignal_t PingTestApp::outOfOrderArrivalsSignal = SIMSIGNAL_NULL;
simsignal_t PingTestApp::pingTxSeqSignal = SIMSIGNAL_NULL;
simsignal_t PingTestApp::pingRxSeqSignal = SIMSIGNAL_NULL;


void PingTestApp::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    // because of IPvXAddressResolver, we need to wait until interfaces are registered,
    // address auto-assignment takes place etc.
    if (stage != 3)
        return;

    // read params
    // (defer reading srcAddr/destAddr to when ping starts, maybe
    // addresses will be assigned later by some protocol)
    packetSize = par("packetSize");
    sendIntervalp = & par("sendInterval");
    hopLimit = par("hopLimit");
    count = par("count");
    startTime = par("startTime").doubleValue();
    stopTime = par("stopTime").doubleValue();
    if (stopTime != 0 && stopTime <= startTime)
        error("Invalid startTime/stopTime parameters");
    printPing = par("printPing").boolValue();
    continuous = par("continuous").boolValue();

    // state
    sendSeqNo = expectedReplySeqNo = 0;
    WATCH(sendSeqNo);
    WATCH(expectedReplySeqNo);

    // statistics
    rttStat.setName("pingRTT");
    rttSignal = registerSignal("rtt");
    numLostSignal = registerSignal("numLost");
    outOfOrderArrivalsSignal = registerSignal("outOfOrderArrivals");
    pingTxSeqSignal = registerSignal("pingTxSeq");
    pingRxSeqSignal = registerSignal("pingRxSeq");

    lossCount = outOfOrderArrivalCount = numPongs = 0;
    WATCH(lossCount);
    WATCH(outOfOrderArrivalCount);
    WATCH(numPongs);

    if (strcmp(par("destAddresses").stringValue(), ""))
    {
        srcAddr = IPvXAddressResolver().resolve(par("srcAddr"));
        // schedule first ping (use empty destAddr to disable)
        cMessage *msg = new cMessage("sendPing");
        scheduleAt(startTime, msg);
    }
}

void PingTestApp::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        if (sendSeqNo == 0)
        {
            const char *destAddrs = par("destAddresses");
            if (!strcmp(destAddrs, "*"))
            {
                destAddresses = getAllAddresses();
            }
            else
            {
                cStringTokenizer tokenizer(destAddrs);
                const char *token;

                while ((token = tokenizer.nextToken()) != NULL)
                {
                    IPvXAddress addr = IPvXAddressResolver().resolve(token);
                    destAddresses.push_back(addr);
                }
            }
        }

        if (sendSeqNo % count == 0)
        {
            // choose next dest address
            unsigned long i = sendSeqNo / count;
            if (i >= destAddresses.size() && !continuous)
            {
                delete msg;
                return;
            }

            i = i % destAddresses.size();
            destAddr = destAddresses[i];
            EV << "Starting up: dest=" << destAddr << "  src=" << srcAddr << "seqNo=" << sendSeqNo << endl;
            ASSERT(!destAddr.isUnspecified());
        }

        // send a ping
        sendPing();

        // then schedule next one if needed
        scheduleNextPing(msg);
    }
    else
    {
        // process ping response
        processPingResponse(check_and_cast<PingPayload *>(msg));
    }
}

std::vector<IPvXAddress> PingTestApp::getAllAddresses()
{
    std::vector<IPvXAddress> result;

    for (int i=0; i<=simulation.getLastModuleId(); i++)
    {
        IInterfaceTable *ift = dynamic_cast<IInterfaceTable *>(simulation.getModule(i));
        if (ift)
        {
            for (int j=0; j < ift->getNumInterfaces(); j++)
            {
                InterfaceEntry *ie = ift->getInterface(j);
                if (ie && !ie->isLoopback())
                {
#ifdef WITH_IPv4
                    if (ie->ipv4Data())
                    {
                        IPv4Address address = ie->ipv4Data()->getIPAddress();
                        if (!address.isUnspecified())
                            result.push_back(IPvXAddress(address));
                    }
#endif
#ifdef WITH_IPv6
                    if (ie->ipv6Data())
                    {
                        for (int k=0; k < ie->ipv6Data()->getNumAddresses(); k++)
                        {
                            IPv6Address address = ie->ipv6Data()->getAddress(k);
                            if (!address.isUnspecified() && address.isGlobal())
                                result.push_back(IPvXAddress(address));
                        }
                    }
#endif
                }
            }
        }
    }
    return result;
}

void PingTestApp::sendPing()
{
    EV << "Sending ping #" << sendSeqNo << "\n";

    char name[32];
    sprintf(name, "ping%ld", sendSeqNo);

    PingPayload *msg = new PingPayload(name);
    msg->setOriginatorId(getId());
    msg->setSeqNo(sendSeqNo);
    msg->setByteLength(packetSize);

    // store the sending time in a circular buffer so we can compute RTT when the packet returns
    sendTimeHistory[sendSeqNo % PINGTEST_HISTORY_SIZE] = simTime();

    sendToICMP(msg, destAddr, srcAddr, hopLimit);
    emit(pingTxSeqSignal, sendSeqNo);
    sendSeqNo++;
}

void PingTestApp::scheduleNextPing(cMessage *timer)
{
    simtime_t nextPing = simTime() + sendIntervalp->doubleValue();
    if (sendSeqNo % count == 0)
        nextPing += par("sleepDuration").doubleValue();

    if (stopTime == 0 || nextPing < stopTime)
        scheduleAt(nextPing, timer);
    else
        delete timer;
}

void PingTestApp::sendToICMP(cMessage *msg, const IPvXAddress& destAddr, const IPvXAddress& srcAddr, int hopLimit)
{
    if (!destAddr.isIPv6())
    {
#ifdef WITH_IPv4
        // send to IPv4
        IPv4ControlInfo *ctrl = new IPv4ControlInfo();
        ctrl->setSrcAddr(srcAddr.get4());
        ctrl->setDestAddr(destAddr.get4());
        ctrl->setTimeToLive(hopLimit);
        msg->setControlInfo(ctrl);
        send(msg, "pingOut");
#else
        throw cRuntimeError("INET compiled without IPv4 features!");
#endif
    }
    else
    {
#ifdef WITH_IPv6
        // send to IPv6
        IPv6ControlInfo *ctrl = new IPv6ControlInfo();
        ctrl->setSrcAddr(srcAddr.get6());
        ctrl->setDestAddr(destAddr.get6());
        ctrl->setHopLimit(hopLimit);
        msg->setControlInfo(ctrl);
        send(msg, "pingv6Out");
#else
        throw cRuntimeError("INET compiled without IPv6 features!");
#endif
    }
}

void PingTestApp::processPingResponse(PingPayload *msg)
{
    // get src, hopCount etc from packet, and print them
    IPvXAddress src, dest;
    int msgHopCount = -1;

    ASSERT(msg->getOriginatorId() == getId());  // ICMP module error

#ifdef WITH_IPv4
    if (dynamic_cast<IPv4ControlInfo *>(msg->getControlInfo()) != NULL)
    {
        IPv4ControlInfo *ctrl = (IPv4ControlInfo *)msg->getControlInfo();
        src = ctrl->getSrcAddr();
        dest = ctrl->getDestAddr();
        msgHopCount = ctrl->getTimeToLive();
    }
    else
#endif
#ifdef WITH_IPv6
    if (dynamic_cast<IPv6ControlInfo *>(msg->getControlInfo()) != NULL)
    {
        IPv6ControlInfo *ctrl = (IPv6ControlInfo *)msg->getControlInfo();
        src = ctrl->getSrcAddr();
        dest = ctrl->getDestAddr();
        msgHopCount = ctrl->getHopLimit();
    }
    else
#endif
    {}

    // calculate the RTT time by looking up the the send time of the packet
    // if the send time is no longer available (i.e. the packet is very old and the
    // sendTime was overwritten in the circular buffer) then we just return a 0
    // to signal that this value should not be used during the RTT statistics)
    simtime_t rtt = sendSeqNo - msg->getSeqNo() > PINGTEST_HISTORY_SIZE ?
                       0 : simTime() - sendTimeHistory[msg->getSeqNo() % PINGTEST_HISTORY_SIZE];

    if (printPing)
    {
        cout << getFullPath() << ": reply of " << std::dec << msg->getByteLength()
             << " bytes from " << src
             << " icmp_seq=" << msg->getSeqNo() << " ttl=" << msgHopCount
             << " time=" << (rtt * 1000) << " msec"
             << " (" << msg->getName() << ")" << endl;
    }

    // update statistics
    countPingResponse(msg->getByteLength(), msg->getSeqNo(), rtt);
    delete msg;
}

void PingTestApp::countPingResponse(int bytes, long seqNo, simtime_t rtt)
{
    EV << "Ping reply #" << seqNo << " arrived, rtt=" << rtt << "\n";
    emit(pingRxSeqSignal, seqNo);

    numPongs++;

    // count only non 0 RTT values as 0s are invalid
    if (rtt > 0)
    {
        rttStat.collect(rtt);
        emit(rttSignal, rtt);
    }

    if (seqNo == expectedReplySeqNo)
    {
        // expected ping reply arrived; expect next sequence number
        expectedReplySeqNo++;
    }
    else if (seqNo > expectedReplySeqNo)
    {
        EV << "Jump in seq numbers, assuming pings since #" << expectedReplySeqNo << " got lost\n";

        // jump in the sequence: count pings in gap as lost for now
        // (if they arrive later, we'll decrement back the loss counter)
        long jump = seqNo - expectedReplySeqNo;
        lossCount += jump;
        emit(numLostSignal, lossCount);

        // expect sequence numbers to continue from here
        expectedReplySeqNo = seqNo+1;
    }
    else // seqNo < expectedReplySeqNo
    {
        // ping reply arrived too late: count as out-of-order arrival (not loss after all)
        EV << "Arrived out of order (too late)\n";
        outOfOrderArrivalCount++;
        lossCount--;
        emit(outOfOrderArrivalsSignal, outOfOrderArrivalCount);
        emit(numLostSignal, lossCount);
    }
}

void PingTestApp::finish()
{
    if (sendSeqNo == 0)
    {
        if (printPing)
            EV << getFullPath() << ": No pings sent, skipping recording statistics and printing results.\n";
        return;
    }

    lossCount += sendSeqNo - expectedReplySeqNo;
    // record statistics
    recordScalar("Pings sent", sendSeqNo);
    recordScalar("ping loss rate (%)", 100 * lossCount / (double)sendSeqNo);
    recordScalar("ping out-of-order rate (%)", 100 * outOfOrderArrivalCount / (double)sendSeqNo);

    // print it to stdout as well
    if (printPing)
    {
        cout << "--------------------------------------------------------" << endl;
        cout << "\t" << getFullPath() << endl;
        cout << "--------------------------------------------------------" << endl;

        cout << "sent: " << sendSeqNo << "   received: " << numPongs << "   loss rate (%): " << (100 * lossCount / (double) sendSeqNo) << endl;
        cout << "round-trip min/avg/max (ms): " << (rttStat.getMin() * 1000.0) << "/"
             << (rttStat.getMean() * 1000.0) << "/" << (rttStat.getMax() * 1000.0) << endl;
        cout << "stddev (ms): " << (rttStat.getStddev() * 1000.0) << "   variance:" << rttStat.getVariance() << endl;
        cout << "--------------------------------------------------------" << endl;
    }
}

