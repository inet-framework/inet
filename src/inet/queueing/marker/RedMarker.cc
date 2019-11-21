//
// Copyright (C) 2019 Marcel Marek
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

#include "inet/common/INETUtils.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/queueing/marker/RedMarker.h"

#ifdef WITH_ETHERNET
#include "inet/linklayer/ethernet/EtherEncap.h"
#include "inet/linklayer/ethernet/EtherFrame_m.h"
#include "inet/linklayer/ethernet/Ethernet.h"
#endif // #ifdef WITH_ETHERNET

#include "inet/networklayer/common/EcnTag_m.h"
#include "inet/networklayer/common/L3Tools.h"
#ifdef WITH_IPv4
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#endif // ifdef WITH_IPv4

namespace inet {
namespace queueing {

Define_Module(RedMarker);

RedMarker::RedMarker()
{
    markNext = false;
}

RedMarker::~RedMarker()
{
}

void RedMarker::initialize(int stage)
{
    RedDropper::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        packetCapacity = par("packetCapacity");

        mark = par("mark");
        if (mark < 0.0)
            throw cRuntimeError("mark parameter must not be negative");
    }
}

bool RedMarker::matchesPacket(Packet *packet)
{
#ifdef WITH_ETHERNET
    auto ethHeader = packet->peekAtFront<EthernetMacHeader>();
    if (isEth2Header(*ethHeader)) {
        const Protocol *payloadProtocol = ProtocolGroup::ethertype.getProtocol(ethHeader->getTypeOrLength());
#ifdef WITH_IPv4
        if (*payloadProtocol == Protocol::ipv4) {
            auto ipv4Header = packet->peekDataAt<Ipv4Header>(ethHeader->getChunkLength());

            //congestion experienced CE
            int ect = ipv4Header->getEcn();

            // if packet supports marking (ECT(1) or ECT(0))
            if (ect != 0) {
                // if next packet should be marked and it is not
                if (markNext && !(ect == IP_ECN_CE)) {
                    markPacket(packet);
                    markNext = false;
                    return true;
                } else {
                    auto action = chooseAction(packet);
                    switch (action) {
                        case DROP:
                            return false;
                        case MARK:
                            if (ect == IP_ECN_CE)
                                markNext = true;
                            else
                                markPacket(packet);
                            return true;
                        case SEND:
                            return true;
                        default:
                            throw cRuntimeError("Model error");
                    }
                }
            }
        }
#endif // ifdef WITH_IPv4
    }
#endif // #ifdef WITH_ETHERNET

    //TODO maybe using next line instead of modified copy & paste:
    // return RedDropper::matchesPacket(packet);

    // For packets that does not support marking - shouldDrop is called instead
    const int queueLength = collection->getNumPackets();

    if (queueLength > 0) {
        // TD: This following calculation is only useful when the queue is not empty!
        avg = (1 - wq) * avg + wq * queueLength;
    }
    else {
        // TD: Added behaviour for empty queue.
        const double m = SIMTIME_DBL(simTime() - q_time) * pkrate;
        avg = pow(1 - wq, m) * avg;
    }

 //   if (minth <= avg && avg < maxth) {
 //       count++;
 //       const double pb = maxp * (avg - minth) / (maxth - minth);
 //       const double pa = pb / (1 - count * pb); // TD: Adapted to work as in [Floyd93].
 //       if (dblrand() < pa) {
 //           EV << "Random early packet drop (avg queue len=" << avg << ", pa=" << pb << ")\n";
 //           count = 0;
 //           return false;
 //       }
 //   }
 //   else if (avg >= maxth) {
 //       EV << "Avg queue len " << avg << " >= maxth, dropping packet.\n";
 //       count = 0;
 //       return false;
 //   }
 //   else
    if (queueLength >= packetCapacity) {    // UPDATE: packetCapacity is the new "hard" limit // maxth is also the "hard" limit
        EV << "Queue len " << queueLength << " >= maxth, dropping packet.\n";
        count = 0;
        return false;
    }
    else {
        count = -1;
    }

    return true;
}

/** It is almost copy&paste from RedDropper::matchesPacket.
 *  Marks packet if avg queue lenght > maxth or random  when  minth < avg < maxth.
 *  Drops when size is bigger then max allowed size.
 *  Returns MARK | DROP | SEND.
 * **/
RedMarker::Action RedMarker::chooseAction(Packet *packet)
{
    const int queueLength = collection->getNumPackets();

    if (maxth > packetCapacity) {
        EV << "Warning: packetCapacity < max_th. Setting capacity to max_th\n";
        packetCapacity = maxth;
    }

    if (queueLength > 0) {
        // TD: This following calculation is only useful when the queue is not empty!
        avg = (1 - wq) * avg + wq * queueLength;
    }
    else {
        // TD: Added behaviour for empty queue.
        const double m = SIMTIME_DBL(simTime() - q_time) * pkrate;
        avg = pow(1 - wq, m) * avg;
    }

//    Random dropping is disabled; returns true only for hard limit

    if (queueLength >= packetCapacity) {   // maxth is also the "hard" limit
        EV << "Queue len " << queueLength << " >= packetCapacity, dropping packet.\n";
        count = 0;
        return DROP;
    }
    else if (minth <= avg && avg < maxth) {
        count++;
        const double pb = maxp * (avg - minth) / (maxth - minth);
        const double pa = pb / (1 - count * pb); // TD: Adapted to work as in [Floyd93].
        if (dblrand() < pa) {
            EV << "Random early packet mark (avg queue len=" << avg << ", pa=" << pb << ")\n";
            count = 0;
            return MARK;
        }
    }
    else if (avg >= maxth) {
        EV << "Avg queue len " << avg << " >= maxth, marking packet.\n";
        count = 0;
        return MARK;
    }
    else {
        count = -1;
    }

    return SEND;
}

void RedMarker::markPacket(Packet *packet)
{
#if defined(WITH_ETHERNET) && defined(WITH_IPv4)
    EV_DETAIL << "Marking packet with ECN \n";

    packet->trim();
    auto protocol = packet->getTag<PacketProtocolTag>()->getProtocol();
    auto macHeader = packet->removeAtFront<EthernetMacHeader>();
    auto ethFcs = packet->removeAtBack<EthernetFcs>(ETHER_FCS_BYTES);

    auto ipv4Header = removeNetworkProtocolHeader<Ipv4Header>(packet);
    ipv4Header->setEcn(IP_ECN_CE);
    //TODO recalculate IP Header checksum
    insertNetworkProtocolHeader(packet, Protocol::ipv4, ipv4Header);
    packet->insertAtFront(macHeader);
    packet->insertAtBack(makeShared<EthernetFcs>(ethFcs->getFcsMode()));    //TODO CRC calculation?
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(protocol);
#else
    ASSERT(false);
#endif // #if defined(WITH_ETHERNET) && defined(WITH_IPv4)
}

} // namespace queueing
} //namespace inet

