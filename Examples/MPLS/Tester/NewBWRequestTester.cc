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
#include <list>



class NewBWRequestTester : public cSimpleModule
{
private:

   int timeOut;

public:
	Module_Class_Members(NewBWRequestTester, cSimpleModule, 16384);
	virtual void initialize();
	virtual void activity();
  	cObject* findObject(char* modName);
   

};
Define_Module_Like(NewBWRequestTester, NetworkManager);




void NewBWRequestTester::initialize()
{

timeOut = par("timeStart").longValue();
  

}



void NewBWRequestTester::activity()
{
cMessage *timeout_msg = new cMessage;
scheduleAt( simTime()+timeOut, timeout_msg );

cMessage* msg = receive();

   //Send command to IR to signal the path recalculation
cModule* irNode = (cModule*)findObject("LSR1");
cModule* signalMod = (cModule*)(irNode->findObject("signal_module", false));
if(signalMod ==NULL)
{
	ev << "Error ! cannot locate signal module in IR node\n";
  return;
}
cMessage* cmdMsg = new cMessage();
cmdMsg->addPar("test_command")=NEW_BW_REQUEST;
cmdMsg->addPar("src") = "10.0.0.1";
cmdMsg->addPar("dest") = "10.0.1.2";
cmdMsg->addPar("bandwidth") = 130;
sendDirect(cmdMsg, 0.0, signalMod, "from_tester");


delete msg;


}

cObject* NewBWRequestTester::findObject(char* modName)
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

