/*******************************************************************
*
*	This library is free software, you can redistribute it 
*	and/or modify 
*	it under  the terms of the GNU Lesser General Public License 
*	as published by the Free Software Foundation; 
*	either version 2 of the License, or any later version.
*	The library is distributed in the hope that it will be useful, 
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
*	See the GNU Lesser General Public License for more details.
*
*
*********************************************************************/

#include "MPLSModule.h"
#include "RSVPTester.h"
#include "TED.h"
#include <list>



class NewRouteTester : public cSimpleModule
{
private:

   TED* myTED;
   int timeOut;

public:
	Module_Class_Members(NewRouteTester, cSimpleModule, 16384);
	virtual void initialize();
	virtual void activity();
	void findTED();
	cObject* findObject(char* modName);


};
Define_Module_Like(NewRouteTester, NetworkManager);




void NewRouteTester::initialize()
{
findTED();
timeOut = par("timeStart").longValue();
  

}



void NewRouteTester::activity()
{
cMessage *timeout_msg = new cMessage;
scheduleAt( simTime()+timeOut, timeout_msg );

cMessage* msg = receive();

//Find gate between LSR2 & LSR4
cModule* lsr2 = (cModule*)findObject("LSR2");
cGate* modifiedGate = lsr2->gate("out", 2);


modifiedGate->setDisplayString("o=yellow");
cPar* par1 = new cPar();
cPar* par2 = new cPar();
par1->setDoubleValue(6000);
par2->setDoubleValue(1);

modifiedGate->setDataRate(par1);
modifiedGate->setDelay(par2);

simple_link_t* aLink = new simple_link_t;
aLink->advRouter = IPAddress("1.0.0.2").getInt();
aLink->id = IPAddress("1.0.0.4").getInt();

//myTED->updateLink(aLink, 1, 6000);
myTED->buildDatabase();
//Send command to IR to signal the path recalculation
cModule* irNode = (cModule*)findObject("LSR1");
cModule* signalMod = (cModule*)(irNode->findObject("signal_module", false));
if(signalMod ==NULL)
{
	ev << "Error ! cannot locate signal module in IR node\n";
}
else
{
	cMessage* cmdMsg = new cMessage();
	cmdMsg->addPar("test_command")=NEW_ROUTE_DISCOVER;
	sendDirect(cmdMsg, 0.0, signalMod, "from_tester");
}

delete msg;



}

void NewRouteTester::findTED()
{

	// find TED
    cTopology topo;
    topo.extractByModuleType( "TED", NULL );
    
    sTopoNode *node = topo.node(0);
    myTED = (TED*)(node->module());
    if(myTED == NULL)
    {
    ev << "Tester Error: Fail to locate TED";
    return;
    }

 
}

cObject* NewRouteTester::findObject(char* modName)
{

	cObject *foundmod =NULL;
	cModule *curmod = this;

	for (curmod = parentModule(); curmod != NULL;
			curmod = curmod->parentModule())
	{
		if ((foundmod = curmod->findObject(modName, false)) != NULL)
		{
		
			ev << "TESTER: Module" << modName << " found\n";
			return foundmod;
		}
	}

	ev << "TESTER: Fail to find " << modName << "\n";


}

