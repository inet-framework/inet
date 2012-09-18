//
// Copyright (C) 2001, 2003, 2004 Johnny Lai, Monash University, Melbourne, Australia
// Copyright (C) 2005 Andras Varga
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

#include "PingApp.h"

#include "IPvXAddressResolver.h"
#include "PingPayload_m.h"
#include "IPv4ControlInfo.h"
#include "IPv6ControlInfo.h"


using std::cout;

Define_Module(PingApp);

simsignal_t PingApp::rttSignal = SIMSIGNAL_NULL;
simsignal_t PingApp::numLostSignal = SIMSIGNAL_NULL;
simsignal_t PingApp::outOfOrderArrivalsSignal = SIMSIGNAL_NULL;
simsignal_t PingApp::pingTxSeqSignal = SIMSIGNAL_NULL;
simsignal_t PingApp::pingRxSeqSignal = SIMSIGNAL_NULL;

void PingApp::initialize(int stage)
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
    startTime = par("startTime");
    stopTime = par("stopTime");
    if (stopTime != 0 && stopTime <= startTime)
        error("Invalid startTime/stopTime parameters");
    printPing = (bool)par("printPing");

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

    // schedule first ping (use empty destAddr to disable)
    if (par("destAddr").stringValue()[0])
    {
        cMessage *msg = new cMessage("sendPing");
        scheduleAt(startTime, msg);
    }
}

void PingApp::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        // on first call we need to initialize
        if (sendSeqNo == 0)
        {
            srcAddr = IPvXAddressResolver().resolve(par("srcAddr"));
            destAddr = IPvXAddressResolver().resolve(par("destAddr"));
            ASSERT(!destAddr.isUnspecified());

            EV << "Starting up: dest=" << destAddr << "  src=" << srcAddr << "\n";
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

void PingApp::sendPing()
{
    EV << "Sending ping #" << sendSeqNo << "\n";

    char name[32];
    sprintf(name, "ping%ld", sendSeqNo);

    PingPayload *msg = new PingPayload(name);
    msg->setOriginatorId(getId());
    msg->setSeqNo(sendSeqNo);
    msg->setByteLength(packetSize);

    // store the sending time in a circular buffer so we can compute RTT when the packet returns
    sendTimeHistory[sendSeqNo % PING_HISTORY_SIZE] = simTime();

    sendToICMP(msg, destAddr, srcAddr, hopLimit);
    emit(pingTxSeqSignal, sendSeqNo);
    sendSeqNo++;
}

void PingApp::scheduleNextPing(cMessage *timer)
{
    simtime_t nextPing = simTime() + sendIntervalp->doubleValue();

    if ((count == 0 || sendSeqNo < count) && (stopTime == 0 || nextPing < stopTime))
        scheduleAt(nextPing, timer);
    else
        delete timer;
}

void PingApp::sendToICMP(cMessage *msg, const IPvXAddress& destAddr, const IPvXAddress& srcAddr, int hopLimit)
{
    if (!destAddr.isIPv6())
    {
        // send to IPv4
        IPv4ControlInfo *ctrl = new IPv4ControlInfo();
        ctrl->setSrcAddr(srcAddr.get4());
        ctrl->setDestAddr(destAddr.get4());
        ctrl->setTimeToLive(hopLimit);
        msg->setControlInfo(ctrl);
        send(msg, "pingOut");
    }
    else
    {
        // send to IPv6
        IPv6ControlInfo *ctrl = new IPv6ControlInfo();
        ctrl->setSrcAddr(srcAddr.get6());
        ctrl->setDestAddr(destAddr.get6());
        ctrl->setHopLimit(hopLimit);
        msg->setControlInfo(ctrl);
        send(msg, "pingv6Out");
    }
}

void PingApp::processPingResponse(PingPayload *msg)
{
    // get src, hopCount etc from packet, and print them
    IPvXAddress src, dest;
    int msgHopCount = -1;

    ASSERT(msg->getOriginatorId() == getId());  // ICMP module error

    if (dynamic_cast<IPv4ControlInfo *>(msg->getControlInfo()) != NULL)
    {
        IPv4ControlInfo *ctrl = (IPv4ControlInfo *)msg->getControlInfo();
        src = ctrl->getSrcAddr();
        dest = ctrl->getDestAddr();
        msgHopCount = ctrl->getTimeToLive();
    }
    else if (dynamic_cast<IPv6ControlInfo *>(msg->getControlInfo()) != NULL)
    {
        IPv6ControlInfo *ctrl = (IPv6ControlInfo *)msg->getControlInfo();
        src = ctrl->getSrcAddr();
        dest = ctrl->getDestAddr();
        msgHopCount = ctrl->getHopLimit();
    }

    // calculate the RTT time by looking up the the send time of the packet
    // if the send time is no longer available (i.e. the packet is very old and the
    // sendTime was overwritten in the circular buffer) then we just return a 0
    // to signal that this value should not be used during the RTT statistics)
    simtime_t rtt = sendSeqNo - msg->getSeqNo() > PING_HISTORY_SIZE ?
                       0 : simTime() - sendTimeHistory[msg->getSeqNo() % PING_HISTORY_SIZE];

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

void PingApp::countPingResponse(int bytes, long seqNo, simtime_t rtt)
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

void PingApp::finish()
{
    if (sendSeqNo==0)
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
        cout << "ping " << par("destAddr").stringValue() << " (" << destAddr << "):" << endl;
        cout << "sent: " << sendSeqNo << "   received: " << numPongs << "   loss rate (%): " << (100 * lossCount / (double) sendSeqNo) << endl;
        cout << "round-trip min/avg/max (ms): " << (rttStat.getMin() * 1000.0) << "/"
             << (rttStat.getMean() * 1000.0) << "/" << (rttStat.getMax() * 1000.0) << endl;
        cout << "stddev (ms): " << (rttStat.getStddev() * 1000.0) << "   variance:" << rttStat.getVariance() << endl;
        cout << "--------------------------------------------------------" << endl;
    }
}

