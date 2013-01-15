 /**
******************************************************
* @file Admacrelay.cc
* @brief Part of the B-Component module. Switch
* Uses RSTP and MVRP modules information to determine the
* output gate.
* @author Juan Luis Garrote Molinero
* @version 1.0
* @date Feb 2011
******************************************************/
#include "Admacrelay.h"
#include "Ethernet.h"
#include "EtherMAC.h"
#include "Delivery_m.h"
#include "RSTPAccess.h"
#include "MVRPAccess.h"





Define_Module( Admacrelay );

Admacrelay::Admacrelay(){}
Admacrelay::~Admacrelay(){}

void Admacrelay::initialize(int stage)
{
	if(stage==2)  // "auto" MAC addresses assignment takes place in stage 0. rstpModule gets address in stage 1.
	{
		//Obtaining other modules pointers
		cache = Cache1AHAccess().get();
		rstpModule=RSTPAccess().get();
		mvrpModule=MVRPAccess().get();

		if((cache==NULL)||(rstpModule==NULL)||(mvrpModule==NULL))
			error("Initialization error");
		//Gets bridge MAC address from rstpModule
		address=rstpModule->getAddress();

		//Gets parameter values
		verbose=(bool) par("verbose");
		if(verbose==true)
			ev<<"Bridge MAC address "<<address.str()<<endl;
	}
}

bool Admacrelay::isISidBroadcast(MACAddress mac, int ISid)
{
	bool result=false;
	//Construct ISid broadcast address (00-1E-83- ISId) Table 26-1 pag.93
	char BrAdd []= "00-00-00-00-00-00";
	int oct1=ISid&0xFF0000;
	int oct2=ISid&0xFF00;
	int oct3=ISid&0xFF;
	sprintf(BrAdd,"00-1E-83-%.2X-%.2X-%.2X",oct1,oct2,oct3);
	MACAddress * bcst=new MACAddress(BrAdd);
	if(bcst->compareTo(mac)==0)
	{
		result=true;
	}
	return result;
}


void Admacrelay::handleEtherFrame(EthernetIIFrame *frame)
{//Specific 802.1ah handler.
	Ethernet1QTag * BTag=check_and_cast<Ethernet1QTag *>(frame->getEncapsulatedPacket());
	Ethernet1ahITag * ITag=check_and_cast<Ethernet1ahITag *>(BTag->getEncapsulatedPacket());
	std::vector<int> outputPorts;
	bool broadcast=false;
	int ArrivalState=rstpModule->getPortState(frame->getArrivalGate()->getIndex());
	int arrival=frame->getArrivalGate()->getIndex();

	if(verbose==true)
	{
		cache->printState();  //Shows cache info.
		mvrpModule->printState(); //MVRP info
	}

	//Learning in case of FORWARDING or LEARNING state.
	if((ArrivalState==FORWARDING)|| (ArrivalState==LEARNING))
	{
		if(frame->getSrc()!= MACAddress::UNSPECIFIED_ADDRESS)
		{
				cache->registerMAC(BTag->getVID(),ITag->getISid(),frame->getSrc(),arrival); //Registers source at arrival gate.
		}
	}
	//Processing in case of FORWARDING state
	if(ArrivalState==FORWARDING)
	{
		if ((frame->getDest().isBroadcast())||isISidBroadcast(frame->getDest(),ITag->getISid()))
		{
			ev << "Broadcast frame";
			broadcast=true;
		}
		else if(frame->getDest()==address)
		{
			for(int i=0;i<gateSize("GatesOut");i++)
			{
				if(rstpModule->isEdge(i))
				{
					outputPorts.push_back(i);
				}
			}
		}
		else if(!cache->resolveMAC(BTag->getVID(),ITag->getISid(),frame->getDest(),&outputPorts))
		{//There is not at least one known output port.
			broadcast=true;
		}
		if(broadcast==true)
		{ //If there was no entry for that MAC destination or destination was broadcast.
			outputPorts.clear();
			if(verbose==true)
				ev<< "Resolve VID " <<BTag->getVID() <<"and ISid"<<ITag->getISid()<<" arrived at "<<frame->getArrivalGate()<<" "<<endl;
			if(!mvrpModule->resolveVLAN(BTag->getVID(),&outputPorts))
			{ //Gets the associated gates to that VLAN including arrival.
				ev<<"VID not registered";
			}
		}
		//I-SID filtering and then relay.
		for(unsigned int i=0;i<outputPorts.size();i++)
		{
			if(!cache->isRegistered(ITag->getISid(),outputPorts[i]))
			{
				outputPorts.erase(outputPorts.begin()+i);
				i--;
			}
		}

		relayMsg(frame,outputPorts); //Relaying this message.
	}
	else
	{
		delete frame;
	}
}

void Admacrelay::relayMsg(cMessage * msg,std::vector <int> outputPorts)
{//Relaying task
	int arrival=msg->getArrivalGate()->getIndex();
	for(unsigned int i=0;i<outputPorts.size();i++)
	{
		int outputState=rstpModule->getPortState(outputPorts[i]);
		if((outputState==FORWARDING)
				&&((rstpModule->isEdge(arrival))||(arrival!=outputPorts[i])))
		{//If it is a Client same gate sending too.
			if(verbose==true)
				ev << "Sending frame to port " << outputPorts[i] << endl;
			cMessage * msg2=msg->dup();
			send(msg2,"GatesOut",outputPorts[i]);
		}
	}
	delete msg;
}



