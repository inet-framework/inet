//
// Copyright (C) 2012 Opensim Ltd.
// Author: Tamas Borbely
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

#include <sstream>
#include <fstream>
#include <iomanip>

#include "inet/common/INETDefs.h"
#include "inet/networklayer/ipv4/IPv4Datagram.h"

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
      cPacket *packet = new IPv4Datagram(packetName.str().c_str());
      packet->setByteLength(par("packetSize").longValue());
      send(packet, "out");

      if ((numPackets == 0 || counter < numPackets) &&
          (stopTime < SIMTIME_ZERO || simTime() < stopTime))
          scheduleAt(simTime() + par("iaTime"), msg);
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

