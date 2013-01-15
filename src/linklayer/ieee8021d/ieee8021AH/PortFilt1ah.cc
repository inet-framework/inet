/**
******************************************************
* @file PortFilt1ah.cc
* @brief Port associated functions. Filtering, tagging, etc.
*
* @author Juan Luis Garrote Molinero
* @version 1.0
* @date Feb 2011
******************************************************/

#include "PortFilt1ah.h"
#include "MVRPDU.h"
#include "BPDU.h"
#include <XMLUtils.h>



Define_Module(PortFilt1ah);

PortFilt1ah::PortFilt1ah(){}
PortFilt1ah::~PortFilt1ah(){}

void PortFilt1ah::initialize()
{
    cost=par("cost");
    priority=par("priority");
}

void PortFilt1ah::handleMessage(cMessage *msg)
{//Filtering and relaying
	cGate * arrivalGate=msg->getArrivalGate();
	int arrival=arrivalGate->getIndex();
	// frame received. Port 0 is the outter gate.
	switch(arrival)
	{
	case 0:
		if((dynamic_cast<EthernetIIFrame *>(msg)!=NULL)
				||(dynamic_cast<MVRPDU *>(msg)!=NULL)
				||(dynamic_cast<BPDUieee8021D *>(msg)!=NULL))
		{
			//Filtering functions. To be defined.
			processTaggedFrame(msg);
		}
		else
		{
			ev<<"Just EthernetIIFrame, BPDU and MVRPDU frames allowed";
			delete msg;
		}
		break;
	case 1:
		if((dynamic_cast<EthernetIIFrame *> (msg)!=NULL)
				||(dynamic_cast<MVRPDU *>(msg)!=NULL)
				||(dynamic_cast<BPDUieee8021D *>(msg)!=NULL))
		{
			//Filtering functions. To be defined.
			processTaggedFrame(msg);
		}
		else
		{
			ev<<"Just Ethernet1AHFrame, BPDU and MVRPDU frames allowed";
			delete msg;
		}
		break;
	default:
		error("Unknown arrival gate");
	}
}

bool PortFilt1ah::isEdge()
{//Returns Edge parameter.
	bool result=false;
	cGate * gate=this->gate("GatesOut",0);
	if(gate!=NULL)
	{
		gate=gate->getNextGate();
		if(gate!=NULL)
		{
			gate=gate->getNextGate();
			if(gate!=NULL)
			{
				if(strcmp(gate->getBaseName(), "upperLayerIn")!=0)
					result=true;
				return result;
			}
		}
	}
	error("Wrong connection. PortFilt could not determine if this is an edge port.");
	return false;
}



