//
// Copyright (C) 2013 OpenSim Ltd.
// @author: Zoltan Bojthe
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


#include "inet/common/INETDefs.h"

#include "inet/linklayer/ethernet/EtherFrame_m.h"
#include "inet/linklayer/common/MACAddress.h"

namespace inet {

class INET_API EthTestApp : public cSimpleModule
{
  protected:
    MACAddress destAddr;

  public:
    EthTestApp() {}

  protected:
    void parseScript(const char *script);
    void processIncomingPacket(cMessage *msg);
    void createCommand(simtime_t t, int bytes);
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
};


Define_Module(EthTestApp);

void EthTestApp::initialize()
{
    const char *addr = par("destAddr");
    destAddr = MACAddress(addr);
    const char *script = par("script");
    parseScript(script);
}

void EthTestApp::createCommand(simtime_t t, int bytes)
{
    char name[100];
    sprintf(name, "PK at %s: %i Bytes", SIMTIME_STR(t), bytes);
    EtherFrame *packet = new EtherFrame(name);
    packet->setByteLength(bytes);
    packet->setDest(destAddr);
    //TODO set packet->destAddr
    scheduleAt(t, packet);
}

void EthTestApp::parseScript(const char *script)
{
    const char *s = script;

    while (isspace(*s)) s++;

    while (*s)
    {
        // simtime in seconds
        char *os;
        double sendingTime = strtod(s,&os);
        if ((s == os) | (*os != ':'))
            throw cRuntimeError("syntax error in script: missing ':' after time");
        s = os;
        s++;
        while (isspace(*s)) s++;

        // length in bytes
        if (!isdigit(*s))
            throw cRuntimeError("syntax error in script: wrong bytelength spec");
        int bytes = atoi(s);
        while (isdigit(*s)) s++;

        // add command
        createCommand(sendingTime, bytes);

        // skip delimiter
        while (isspace(*s)) s++;
    }
}

void EthTestApp::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
        send(msg, "out");
    else
        processIncomingPacket(msg);
}

void EthTestApp::processIncomingPacket(cMessage *msg)
{
    delete msg;
}

void EthTestApp::finish()
{
}

} // namespace inet

