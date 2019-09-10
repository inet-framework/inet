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

#include "inet/common/queue/RedMarker.h"
#include "inet/common/INETUtils.h"

//#include "inet/networklayer/diffserv/DscpMarker.h"

#include "inet/common/ProtocolTag_m.h"


#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/linklayer/ethernet/EtherFrame_m.h"
#include "inet/linklayer/ethernet/Ethernet.h"
#include "inet/linklayer/ethernet/EtherEncap.h"

#include "inet/networklayer/common/L3Tools.h"
#include "inet/networklayer/common/EcnTag_m.h"


//#ifdef WITH_IPv6
//#include "inet/networklayer/ipv6/Ipv6Header.h"
//#endif // ifdef WITH_IPv6

//#include "inet/networklayer/diffserv/Dscp_m.h"
//
//#include "inet/networklayer/diffserv/DiffservUtil.h"

namespace inet {

Define_Module(RedMarker);


RedMarker::RedMarker()
{
  markNext = false;

}

RedMarker::~RedMarker()
{
  delete[] marks;

}

void RedMarker::initialize()
{
  RedDropper::initialize();

  frameQueueCapacity = par("frameQueueCapacity");
  marks = new double[numGates];

  cStringTokenizer marksTokens(par("marks"));
  for (int i = 0; i < numGates; ++i) {
      marks[i] = marksTokens.hasMoreTokens() ? utils::atod(marksTokens.nextToken()) :
          (i > 0 ? marks[i - 1] : 0);


      if (marks[i] < 0.0)
          throw cRuntimeError("marks parameter must not be negative");

  }
}

void RedMarker::handleMessage(cMessage *msg)
{
  Packet *packet = check_and_cast<Packet *>(msg);

  const Protocol *payloadProtocol = nullptr;
  auto headerOffset = packet->getFrontOffset();

  auto ethHeader = packet->peekAtFront<EthernetMacHeader>();
  if (isEth2Header(*ethHeader))
  {
    payloadProtocol = ProtocolGroup::ethertype.getProtocol(ethHeader->getTypeOrLength());
    if (*payloadProtocol == Protocol::ipv4)
    {
//      auto headerOffset = packet->getFrontOffset();

      packet->setFrontOffset(headerOffset + ethHeader->getChunkLength());
      auto ipv4Header = packet->peekAtFront<Ipv4Header>();

      //congestion experiencd CE
      int ect = ipv4Header->getExplicitCongestionNotification();
  //      packet->insertAtFront(ipv4Header);

  // if packet supports marking (ECT(1) or ECT(0))
      if ((ect == IP_ECN_ECT_0) || (ect == IP_ECN_ECT_1))
      {

        // if next packet should be marked and it is not
        if (markNext && !(ect == IP_ECN_CE))
        {
          markPacket(packet);
          markNext = false;
        }
        else if (ect == IP_ECN_CE)
        {
          if (shouldMark(packet))
          {
            markNext = true;
            if(packetDropped)
            {
              packetDropped = false;
              return;
            }
          }
        }
        else
        {
          if(shouldMark(packet))
          {
            if(packetDropped)
            {
              packetDropped = false;
              return;
            }
          }

        }
        packet->setFrontOffset(headerOffset);
        sendOut(packet);
        return;
      }
      packet->setFrontOffset(headerOffset);

    }
  }


  if (shouldDrop(packet)){
    EV_DETAIL<< "RedMarker: Dropping packet\n";
    dropPacket(packet);
    return;
  }


   sendOut(packet);
}
/** It is almost copy&paste from RedDropper::shouldDrop.
 *  Marks packet if avg queue lenght > maxth or random  when  minth < avg < maxth.
 *  Drops when size is bigger then max allowed size.
 *  Returns true when drop or mark occurs, false otherwise.
 * **/
bool RedMarker::shouldMark(Packet *packet)
{
    const int i = packet->getArrivalGate()->getIndex();
    ASSERT(i >= 0 && i < numGates);
    const double minth = minths[i];
    const double maxth = maxths[i];
    const double maxp = maxps[i];
    const double pkrate = pkrates[i];
    const int queueLength = getLength();

    if(maxth > frameQueueCapacity)
    {
      EV <<"Warning: FrameQueueCapacity < max_th. Setting capacity to max_th\n";
      frameQueueCapacity = maxth;
    }

    if (queueLength > 0)
    {
        // TD: This following calculation is only useful when the queue is not empty!
        avg = (1 - wq) * avg + wq * queueLength;
    }
    else
    {
        // TD: Added behaviour for empty queue.
        const double m = SIMTIME_DBL(simTime() - q_time) * pkrate;
        avg = pow(1 - wq, m) * avg;
    }

//    Random dropping is disabled; returns true only for hard limit

    if (queueLength >= frameQueueCapacity) {    // maxth is also the "hard" limit
            EV << "Queue len " << queueLength << " >= frameQueueCapacity, dropping packet.\n";
            count[i] = 0;
            dropPacket(packet);
            packetDropped = true;
            return true;
    }
    else if (minth <= avg && avg < maxth)
    {
        count[i]++;
        const double pb = maxp * (avg - minth) / (maxth - minth);
        const double pa = pb / (1 - count[i] * pb); // TD: Adapted to work as in [Floyd93].
        if (dblrand() < pa)
        {
            EV << "Random early packet mark (avg queue len=" << avg << ", pa=" << pb << ")\n";
            count[i] = 0;
            markPacket(check_and_cast<Packet *>(packet));
            return true;
        }
    }
    else if (avg >= maxth) {
        EV << "Avg queue len " << avg << " >= maxth, marking packet.\n";
        count[i] = 0;
        markPacket(check_and_cast<Packet *>(packet));
        return true;
    }

    else
    {
        count[i] = -1;
    }

    return false;
}

bool RedMarker::shouldDrop(cPacket *packet)
{
  const int i = packet->getArrivalGate()->getIndex();
   ASSERT(i >= 0 && i < numGates);
//   const double minth = minths[i];
//   const double maxth = maxths[i];
//   const double maxp = maxps[i];
   const double pkrate = pkrates[i];
   const int queueLength = getLength();

   if (queueLength > 0)
   {
       // TD: This following calculation is only useful when the queue is not empty!
       avg = (1 - wq) * avg + wq * queueLength;
   }
   else
   {
       // TD: Added behaviour for empty queue.
       const double m = SIMTIME_DBL(simTime() - q_time) * pkrate;
       avg = pow(1 - wq, m) * avg;
   }

//   if (minth <= avg && avg < maxth)
//   {
//       count[i]++;
//       const double pb = maxp * (avg - minth) / (maxth - minth);
//       const double pa = pb / (1 - count[i] * pb); // TD: Adapted to work as in [Floyd93].
//       if (dblrand() < pa)
//       {
//           EV << "Random early packet drop (avg queue len=" << avg << ", pa=" << pb << ")\n";
//           count[i] = 0;
//           return true;
//       }
//   }
//   else if (avg >= maxth) {
//       EV << "Avg queue len " << avg << " >= maxth, dropping packet.\n";
//       count[i] = 0;
//       return true;
//   }
//   else
   if (queueLength >= frameQueueCapacity) {    // UPDATE: frameQueueCapacity is the new "hard" limit // maxth is also the "hard" limit
       EV << "Queue len " << queueLength << " >= maxth, dropping packet.\n";
       count[i] = 0;
       return true;
   }
   else
   {
       count[i] = -1;
   }

   return false;
}


bool RedMarker::markPacket(Packet *packet)
{
  EV_DETAIL << "Marking packet with ECN \n";
//  std::cout << "Marking packet with ECN \n";

  packet->setFrontOffset(b(0));
  auto macHeader = packet->popAtFront<EthernetMacHeader>();
  packet->popAtBack<EthernetFcs>(ETHER_FCS_BYTES);

  const auto& ipv4Header = removeNetworkProtocolHeader<Ipv4Header>(packet);
  ipv4Header->setExplicitCongestionNotification(3);
  insertNetworkProtocolHeader(packet, Protocol::ipv4, ipv4Header);

  packet->insertAtFront(macHeader);

  EtherEncap::addPaddingAndFcs(packet, FCS_DECLARED_CORRECT);


//
//  auto networkHeader = getNetworkProtocolHeader(packet);
//
//  Ipv4Header* ipv4Header = static_cast<Ipv4Header*>(networkHeader);
//
//  //  auto ipv4Header = packet->peekAtFront<Ipv4Header>();
////  auto ipv4Header = removeNetworkProtocolHeader<Ipv4Header>(packet);
//
//  //congestion experiencd CE
//  ipv4Header->setExplicitCongestionNotification(3);
//  insertNetworkProtocolHeader(packet, Protocol::ipv4, ipv4Header);

  return true;


//#ifdef WITH_IPv6
//  if (protocol == &Protocol::ipv6) {
//      packet->trimFront();
//      const auto& ipv6Header = packet->removeAtFront<Ipv6Header>();
//      ipv6Header->setDiffServCodePoint(dscp);
//      packet->insertAtFront(ipv6Header);
//      return true;
//  }
//#endif // ifdef WITH_IPv6

  return false;


}





} //namespace
