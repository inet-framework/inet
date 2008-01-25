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



class INET_API NewBWRequestTester : public cSimpleModule
{
  private:
    simtime_t startTime;

  public:
    Module_Class_Members(NewBWRequestTester, cSimpleModule, 16384);
    virtual void initialize();
    virtual void activity();
};

Define_Module(NewBWRequestTester);


void NewBWRequestTester::initialize()
{
    startTime = par("startTime");
}

void NewBWRequestTester::activity()
{
    cMessage *timeout_msg = new cMessage;
    scheduleAt( simTime()+startTime, timeout_msg );

    cMessage* msg = receive();

    bubble("NOW! Sending new bandwidth request to LSR1");

    //Send command to IR to signal the path recalculation
    cModule* irNode = simulation.moduleByPath("LSR1");
    if (!irNode)
        error("LSR1 module not found");
    cModule* signalMod = irNode->submodule("signal_module");
    if (!signalMod)
        error("Cannot locate signal module in IR node");

    cMessage* cmdMsg = new cMessage();
    cmdMsg->addPar("test_command")=NEW_BW_REQUEST;
    cmdMsg->addPar("src") = "10.0.0.1";
    cmdMsg->addPar("dest") = "10.0.1.2";
    cmdMsg->addPar("bandwidth") = 130;
    sendDirect(cmdMsg, 0.0, signalMod, "from_tester");

    delete msg;
}


