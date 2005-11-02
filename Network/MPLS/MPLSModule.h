/*******************************************************************
*
*    This library is free software, you can redistribute it
*    and/or modify
*    it under  the terms of the GNU Lesser General Public License
*    as published by the Free Software Foundation;
*    either version 2 of the License, or any later version.
*    The library is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*    See the GNU Lesser General Public License for more details.
*
*
*********************************************************************/

#ifndef __MPLSMODULE_H__
#define __MPLSMODULE_H__

#include <vector>
#include <omnetpp.h>

#include "MPLSPacket.h"
#include "IPDatagram.h"
#include "ConstType.h"

#include "LIBtable.h"
#include "InterfaceTable.h"

#include "Classifier.h"

//

class INET_API MPLSModule : public cSimpleModule
{
	private:

		simtime_t delay1;
		
		//no longer used, see comment in intialize
		//std::vector<bool> labelIf;
		
		LIBTable *lt;
		InterfaceTable *ift;
		IClassifier *pct;

	public:

		Module_Class_Members(MPLSModule, cSimpleModule, 0);
      
		virtual void initialize(int stage);
		virtual int numInitStages() const  {return 5;}
		virtual void handleMessage(cMessage *msg);

	private:
	
		void processPacketFromL3(cMessage *msg);
		void processPacketFromL2(cMessage *msg);
		void processMPLSPacketFromL2(MPLSPacket *mplsPacket);
		
		bool tryLabelAndForwardIPDatagram(IPDatagram *ipdatagram);
		void labelAndForwardIPDatagram(IPDatagram *ipdatagram);

		void sendToL2(cMessage *msg, int gateIndex);  
		void doStackOps(MPLSPacket *mplsPacket, const LabelOpVector& outLabel);
};

#endif

