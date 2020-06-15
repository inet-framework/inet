//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2007 Universidad de Málaga
// Copyright (C) 2011 Zoltan Bojthe
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


#include "inet/applications/base/ApplicationPacket_m.h"
#include "inet/applications/udpapp/UdpBasicBurst2.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"
#include "inet/mobility/single/RandomWaypointMobility2.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/common/RouteTrace_m.h"
#include <algorithm>    // std::find

namespace inet {


Define_Module(UdpBasicBurst2);


int UdpBasicBurst2::numStatics = 0;
int UdpBasicBurst2::numMobiles = 0;
std::vector<L3Address> UdpBasicBurst2::staticNodes;
int UdpBasicBurst2::packetStatic = 0;
int UdpBasicBurst2::packetMob = 0;
int UdpBasicBurst2::packetStaticRec = 0;
int UdpBasicBurst2::packetMobRec = 0;
cHistogram * UdpBasicBurst2::delay = nullptr;
int UdpBasicBurst2::stablePaths = 0;

void UdpBasicBurst2::initialize(int stage)
{
    UdpBasicBurst::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {

        numStatics = 0;
        numMobiles = 0;
        staticNodes.clear();
        packetStatic = 0;
        packetMob = 0;
        packetStaticRec = 0;
        packetMobRec = 0;
        stablePaths = 0;
        delay = nullptr;
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        auto node = getContainingNode(this);
        auto mob = node->getSubmodule("mobility");
        auto rw = dynamic_cast<RandomWaypointMobility2 *>(mob);
        if (rw != nullptr && rw->isStationary())  {
            numStatics++;
            staticNodes.push_back(L3AddressResolver().addressOf(node));
            isStatic = true;
            if (delay == nullptr)
                delay = new cHistogram("Global delay");
        }
        else {
            isStatic = false;
            numMobiles++;
        }
    }
}

void UdpBasicBurst2::processPacket(Packet *pk)
{
    if (pk->getKind() == UDP_I_ERROR) {
        EV_WARN << "UDP error received\n";
        delete pk;
        return;
    }

    if (pk->hasPar("sourceId") && pk->hasPar("msgId")) {
        // duplicate control
        int moduleId = pk->par("sourceId");
        int msgId = pk->par("msgId");
        auto it = sourceSequence.find(moduleId);
        if (it != sourceSequence.end()) {
            if (it->second >= msgId) {
                EV_DEBUG << "Out of order packet: " << UdpSocket::getReceivedPacketInfo(pk) << endl;
                emit(outOfOrderPkSignal, pk);
                delete pk;
                numDuplicated++;
                return;
            }
            else
                it->second = msgId;
        }
        else
            sourceSequence[moduleId] = msgId;
    }
    else
        throw cRuntimeError("no id in the packet");

    if (delayLimit > 0) {
        if (simTime() - pk->getTimestamp() > delayLimit) {
            EV_DEBUG << "Old packet: " << UdpSocket::getReceivedPacketInfo(pk) << endl;
            PacketDropDetails details;
            details.setReason(CONGESTION);
            emit(packetDroppedSignal, pk, &details);
            delete pk;
            numDeleted++;
            return;
        }
    }
    if (excessiveDelay > 0) {
        if (simTime() - pk->getTimestamp() > excessiveDelay) {
            numExessiveDelay++;
            emit(excessiveDelayPksignal, pk);
        }
    }

    EV_INFO << "Received packet: " << UdpSocket::getReceivedPacketInfo(pk) << endl;
    emit(packetReceivedSignal, pk);

    auto l3Addresses = pk->getTag<L3AddressInd>();
    L3Address srcAddr = l3Addresses->getSrcAddress();
    L3Address destAddr = l3Addresses->getDestAddress();
    auto it1 = std::find(staticNodes.begin(), staticNodes.end(), srcAddr);
    auto it2 = std::find(staticNodes.begin(), staticNodes.end(), destAddr);
    if (it1 != staticNodes.end() && it2 != staticNodes.end())
        packetStaticRec++;
    else
        packetMobRec++;
    if (delay != nullptr)
        delay->collect(simTime() - pk->getTimestamp());


    auto tag = pk->findTag<RouteTraceTag>();
    if (tag != nullptr) {
        bool stable = true;
        for (auto elem : tag->getFlags()) {
            stable = stable && ((elem & FLAG_STABLE) != 0);
        }
        stable = stable && ((tag->getFlagDst() & FLAG_STABLE) != 0) && ((tag->getFlagSrc() & FLAG_STABLE) != 0);
        if (stable)
            stablePaths++;
    }

    numReceived++;
    delete pk;
}

void UdpBasicBurst2::generateBurst()
{
    simtime_t now = simTime();

    if (nextPkt < now)
        nextPkt = now;

    double sendInterval = *sendIntervalPar;
    if (sendInterval <= 0.0)
        throw cRuntimeError("The sendInterval parameter must be bigger than 0");
    nextPkt += sendInterval;

    if (activeBurst && nextBurst <= now) {    // new burst
        double burstDuration = *burstDurationPar;
        if (burstDuration < 0.0)
            throw cRuntimeError("The burstDuration parameter mustn't be smaller than 0");
        double sleepDuration = *sleepDurationPar;

        if (burstDuration == 0.0)
            activeBurst = false;
        else {
            if (sleepDuration < 0.0)
                throw cRuntimeError("The sleepDuration parameter mustn't be smaller than 0");
            nextSleep = now + burstDuration;
            nextBurst = nextSleep + sleepDuration;
        }

        if (chooseDestAddrMode == PER_BURST)
            destAddr = chooseDestAddr();
    }

    if (chooseDestAddrMode == PER_SEND)
        destAddr = chooseDestAddr();

    Packet *payload = createPacket();
    if(dontFragment)
        payload->addTag<FragmentationReq>()->setDontFragment(true);
    payload->setTimestamp();
    emit(packetSentSignal, payload);
    socket.sendTo(payload, destAddr, destPort);
    numSent++;

    auto it =  std::find(staticNodes.begin(), staticNodes.end(), destAddr);
    if (it != staticNodes.end() && isStatic)
        packetStatic++;
    else
        packetMob++;

    // Next timer
    if (activeBurst && nextPkt >= nextSleep)
        nextPkt = nextBurst;

    if (stopTime >= SIMTIME_ZERO && nextPkt >= stopTime) {
        timerNext->setKind(STOP);
        nextPkt = stopTime;
    }
    scheduleAt(nextPkt, timerNext);
}

void UdpBasicBurst2::finish()
{
    if (numStatics != 0) {
        recordScalar("Static sources", numStatics);
        numStatics = 0;
    }
    if (numMobiles != 0) {
        recordScalar("mobile sources", numMobiles);
        numMobiles = 0;
    }

    if (packetStaticRec != 0) {
        recordScalar("Global pkt send static-static", packetStaticRec);
    }
    if (packetMobRec != 0) {
        recordScalar("Global pkt no static", packetMobRec);
    }

    if (packetStatic != 0) {
        recordScalar("Global pkt rec static-static", packetStatic);
    }

    if (packetMob != 0) {
        recordScalar("Global pkt rec no static", packetMob);
    }


    if (packetStaticRec != 0) {
        recordScalar("Global Pdr static-static", (double)packetStaticRec/(double)packetStatic);
    }

    if (packetMobRec != 0) {
        recordScalar("Global Pdr no static", (double)packetMobRec/(double)packetMob);

    }

    if (stablePaths != 0) {
        recordScalar("Global stable paths", stablePaths);

    }
    if (packetStaticRec + packetMobRec > 0) {
          recordScalar("Global no stable paths", packetStaticRec + packetMobRec - stablePaths);
    }

    if (delay != nullptr) {
        delay->record();
        delete delay;
        delay = nullptr;
    }

    if ((packetStaticRec + packetMobRec)  > 0) {
        recordScalar("Global pkt total rec", (packetStaticRec + packetMobRec));
        recordScalar("Global pkt total send", (packetStatic + packetMob));
        recordScalar("Global Pdr", (packetStaticRec + packetMobRec)/(packetStatic + packetMob));
    }

    stablePaths = 0;
    packetMob = 0;
    packetStatic = 0;
    packetMobRec = 0;
    packetStaticRec = 0;

    UdpBasicBurst::finish();
}


} // namespace inet

