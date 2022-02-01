//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include <sstream>
#include <fstream>
#include <iomanip>

#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"

using namespace std;

namespace inet {

class MeterTestApp : public cSimpleModule
{
    int numPackets;
    simtime_t stopTime;
    vector<string> colors;

    int counter;
    cMessage *timer;
    ofstream out;

  protected:
    virtual void initialize();
    virtual void finalize();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(MeterTestApp);

void MeterTestApp::initialize()
{
    numPackets = par("numPackets");
    stopTime = par("stopTime");
    colors = cStringTokenizer(par("colors")).asVector();

    if ((int)colors.size() != gateSize("in"))
        throw cRuntimeError("Too %s colors are specified in the colors parameter.",
                (int)colors.size() < gateSize("in") ? "few" : "many");

    counter = 0;
    timer = new cMessage("Timer");

    const char *filename = par("resultFile");
    out.open(filename);
    if (out.fail())
        throw cRuntimeError("Can not open file %s", filename);
    out << left << setw(12) << "Packet" << setw(12) << "Conformance" << "\n";

    double startTime = par("startTime");
    if (stopTime < SIMTIME_ZERO || stopTime >= startTime)
        scheduleAt(startTime, timer);
}

void MeterTestApp::finalize()
{
    cancelAndDelete(timer);
    out.close();
}

void MeterTestApp::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
      ostringstream packetName;
      packetName << "packet-" << (++counter);
      Packet *packet = new Packet(packetName.str().c_str());     //FIXME its was created an IPv4Datagram
      packet->insertAtBack(makeShared<ByteCountChunk>(B(par("packetSize"))));
      send(packet, "out");

      if ((numPackets == 0 || counter < numPackets) &&
          (stopTime < SIMTIME_ZERO || simTime() < stopTime))
          scheduleAfter(par("iaTime"), msg);
      else
          delete msg;
    }
    else
    {
      int gateIndex = msg->getArrivalGate()->getIndex();
      out << left << setw(12) << msg->getName() << setw(12) << colors[gateIndex] << "\n";
      delete msg;
    }
}

} // namespace inet

