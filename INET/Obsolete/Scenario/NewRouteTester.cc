/*******************************************************************
*
*       This library is free software, you can redistribute it
*       and/or modify
*       it under  the terms of the GNU Lesser General Public License
*       as published by the Free Software Foundation;
*       either version 2 of the License, or any later version.
*       The library is distributed in the hope that it will be useful,
*       but WITHOUT ANY WARRANTY; without even the implied warranty of
*       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*       See the GNU Lesser General Public License for more details.
*
*
*********************************************************************/

#include <list>
#include "MPLSModule.h"
#include "RSVPTesterCommands.h"
#include "TED.h"



class INET_API NewRouteTester : public cSimpleModule
{
private:
   simtime_t startTime;

public:
   Module_Class_Members(NewRouteTester, cSimpleModule, 16384);
   virtual void initialize();
   virtual void activity();
};


Define_Module(NewRouteTester);


void NewRouteTester::initialize()
{
    startTime = par("startTime");
}

void NewRouteTester::activity()
{
    cMessage *timeout_msg = new cMessage;
    scheduleAt( simTime()+startTime, timeout_msg );

    cMessage* msg = receive();

    //Find gate between LSR2 & LSR4
    cModule* lsr2 = simulation.moduleByPath("LSR2");
    if (!lsr2) error("LSR2 module not found");
    cGate* modifiedGate = lsr2->gate("out", 2);

    bubble("NOW! Changing bandwith of LSR2 --> LSR4 link\n"
           "and telling LSR1 to recalculate the route");

    modifiedGate->setDisplayString("o=yellow");
    cPar* par1 = new cMsgPar();
    cPar* par2 = new cMsgPar();
    par1->setDoubleValue(6000);
    par2->setDoubleValue(1);

    modifiedGate->setDataRate(par1);
    modifiedGate->setDelay(par2);

    simple_link_t aLink;
    aLink.advRouter = IPAddress("1.0.0.2").getInt();
    aLink.id = IPAddress("1.0.0.4").getInt();

    TED *ted = TED::getGlobalInstance();
    ted->updateLink(&aLink, 1, 6000);
    ted->buildDatabase();

    // Send command to IR to signal the path recalculation
    cModule *irNode = simulation.moduleByPath("LSR1");
    if (!irNode)
        error("LSR1 module not found");
    cModule *signalMod = irNode->submodule("signal_module");
    if (!signalMod)
        error("cannot locate signal module in IR node LSR1");

    cMessage *cmdMsg = new cMessage();
    cmdMsg->addPar("test_command")=NEW_ROUTE_DISCOVER;
    sendDirect(cmdMsg, 0.0, signalMod, "from_tester");

    delete msg;
}


