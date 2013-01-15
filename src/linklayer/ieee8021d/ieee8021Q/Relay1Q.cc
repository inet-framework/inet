 /**
******************************************************
* @file Relay1Q.cc
* @brief Relay function. Part of the ieee802.Q bridge.
* Uses RSTP and MVRP modules information to deliver
* messages through the correct gate
* @author Juan Luis Garrote Molinero
* @version 1.0
* @date Feb 2011
******************************************************/
#include "Admacrelay.h"
#include "Ethernet.h"
#include "EtherMAC.h"
#include "Cache1QAccess.h"
#include "RSTPAccess.h"
#include "MVRPAccess.h"
#include "Relay1Q.h"





Define_Module( Relay1Q );

Relay1Q::Relay1Q()
{
}
Relay1Q::~Relay1Q()
{
}

void Relay1Q::initialize(int stage)
{
	if(stage==2)  // "auto" MAC addresses assignment takes place in stage 0. rstpModule gets address in stage 1.
	{
		//Obtaining cache, rstp and mvrp modules pointers
		cache = Cache1QAccess().get();
		rstpModule=RSTPAccess().get();
		mvrpModule=MVRPAccess().get();

		//Gets bridge MAC address from rstpModule
		address=rstpModule->getAddress();

		//Gets parameter values
		verbose=(bool) par("verbose");
		if(verbose==true)
			ev<<"Bridge MAC address "<<address.str()<<endl;
	}
}


void Relay1Q::handleMessage(cMessage *msg){
	ev << "New message"<<endl;

	//rstp module update.
	rstpModule=RSTPAccess().get();
	int ArrivalState=rstpModule->getPortState(msg->getArrivalGate()->getIndex());
	int ArrivalRole=rstpModule->getPortRole(msg->getArrivalGate()->getIndex());

	//Sends to correct handler after verification
	if (dynamic_cast<BPDUieee8021D *>(msg) != NULL)
	{
		ev<<"BPDU";
		if(ArrivalRole!=DISABLED)
			handleIncomingFrame(check_and_cast<BPDUieee8021D *> (msg));
		else
			delete msg;
	}
	else if (dynamic_cast<Delivery *>(msg)!=NULL)
	{ //Outgoing Frame. Delivery encapsulates the frame.
		ev<<"Outgoing Frame";
		handleIncomingFrame(check_and_cast<Delivery *>(msg));
	}
	else if(dynamic_cast<MVRPDU *>(msg)!= NULL)
	{
			ev<<"MVRPDU";
			if((ArrivalState==LEARNING)
					||(ArrivalState==FORWARDING))
			{//If they come from a valid port.
				handleIncomingFrame(check_and_cast<MVRPDU *>(msg));
			}
			else
				delete msg;
	}
	else
	{
		ev<<"Data Frame";
		handleIncomingFrame(msg);
	}
}

void Relay1Q::handleIncomingFrame(cMessage * msg)
{  //Check port state and frame type.
	int ArrivalState=rstpModule->getPortState(msg->getArrivalGate()->getIndex());

	if(dynamic_cast<EthernetIIFrame *>(msg) != NULL)
	{
		if((ArrivalState==FORWARDING)||(ArrivalState==LEARNING))
		{
			ev<<"Handling Ethernet Frame";
			handleEtherFrame(check_and_cast<EthernetIIFrame *> (msg));
		}
		else
			delete msg;
	}
	else
	{
		error("Not supported frame type");
	}
}

/*
 *  Outgoing frames handling
 */
void Relay1Q::handleIncomingFrame(Delivery *frame)
{
	int sendBy=frame->getSendByPort();
	if(sendBy>gateSize("GatesOut"))
	{
		ev<<"Error. Wrong sending port.";
	}
	else
	{
		cMessage * msg=frame->decapsulate();
		send(msg,"GatesOut",sendBy);
	}
	delete frame;
}


/*
 *  Specific BPDU handler
 */
void Relay1Q::handleIncomingFrame(BPDUieee8021D *frame)
{
	if((frame->getDest()==address)||(frame->getDest()==MACAddress("01-80-C2-00-00-00")))
	{
		Delivery * frame2= new Delivery();
		frame2->setArrivalPort(frame->getArrivalGate()->getIndex());
		frame2->encapsulate(frame);
		send(frame2,"RSTPGate$o"); //And sends the frame to the RSTP module.
	}
	else
	{
		ev<<"Wrong formated BPDU";
		delete frame;
	}
}

/*
 *  Specific 802.1Q handler.
 */
void Relay1Q::handleEtherFrame(EthernetIIFrame *frame)
{
	std::vector<int> outputPorts;
	bool broadcast=false;
	int ArrivalState=rstpModule->getPortState(frame->getArrivalGate()->getIndex());
	int arrival=frame->getArrivalGate()->getIndex();
	Ethernet1QTag * Tag=check_and_cast<Ethernet1QTag *>(frame->getEncapsulatedPacket());
	EthernetIIFrame * EthIITemp= frame;
	if(verbose==true)
	{
		cache->printState();  //Shows cache info.
		mvrpModule->printState(); //MVRP info
	}

	//Learning in case of FORWARDING or LEARNING state.
	if((ArrivalState==FORWARDING)|| (ArrivalState==LEARNING))
	{
		if(EthIITemp->getSrc()!= MACAddress::UNSPECIFIED_ADDRESS )
		{
				cache->registerMAC(Tag->getVID(),EthIITemp->getSrc(),arrival); //Registers source at arrival gate.
		}
	}
	//Processing in case of FORWARDING state
	if(ArrivalState==FORWARDING)
	{
		if (EthIITemp->getDest().isBroadcast())
		{
			ev << "Broadcast frame";
			broadcast=true;
		}
		else if(!cache->resolveMAC(Tag->getVID(),EthIITemp->getDest(),&outputPorts))
		{//There is at least one known output port.
			broadcast=true;
		}
		if(broadcast==true)
		{ //If there was no entry for that MAC destination or destination was broadcast.
			outputPorts.clear();
			if(verbose==true)
				ev<< "Resolve VID " <<Tag->getVID() <<" arrived at "<<frame->getArrivalGate()<<" "<<endl;
			if(!mvrpModule->resolveVLAN(Tag->getVID(),&outputPorts))
			{ //Gets the associated gates to that VLAN including arrival.
				ev<<"VID not registered";
			}
		}
		relayMsg(frame,outputPorts);
	}
	else
	{
		delete frame;
	}
}

void Relay1Q::relayMsg(cMessage * msg, std::vector<int> outputPorts)
{
	int arrival=msg->getArrivalGate()->getIndex();
	for(unsigned int i=0;i<outputPorts.size();i++)
	{
		int outputState=rstpModule->getPortState(outputPorts[i]);
		if((arrival!=outputPorts[i])&&(outputState==FORWARDING))
		{//Avoiding same gate sending.
			if(verbose==true)
				ev << "Sending frame to port " << outputPorts[i] << endl;
			cMessage * msg2=msg->dup();
			send(msg2,"GatesOut",outputPorts[i]);
		}
	}
	delete msg;
}


/*
 *  Specific MVRPDU handler
 */
void Relay1Q::handleIncomingFrame(MVRPDU *frame)
{
//If this is an incoming MVRPDU. Not forwarding or learning are ignored.
	if((rstpModule->getPortState(frame->getArrivalGate()->getIndex())==FORWARDING)|| (rstpModule->getPortState(frame->getArrivalGate()->getIndex())==LEARNING))
		{
			Delivery * frame2=new Delivery();
			frame2->setArrivalPort(frame->getArrivalGate()->getIndex());
			frame2->encapsulate(frame);
			send(frame2,"MVRPGate$o"); // It sends it to the MVRP module
		}
	else
		delete frame;

}

void Relay1Q::finish()
{

}



