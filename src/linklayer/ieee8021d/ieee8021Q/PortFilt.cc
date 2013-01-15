 /**
******************************************************
* @file PortFilt.cc
* @brief 802.1Q Tagging and filtering skills
* @author Juan Luis Garrote Molinero
* @version 1.0
* @date Feb 2011
******************************************************/

#include "PortFilt.h"
#include "MVRPDU.h"
#include "BPDU.h"



Define_Module(PortFilt);
PortFilt::PortFilt()
{
}

PortFilt::~PortFilt(){}

void PortFilt::initialize()
{
	//Getting and validating parameters
    verbose=par("verbose");
    tagged=par("tagged");
    cost=par("cost");
    priority=par("priority");
    interFrameTime=(simtime_t) par("interFrameTime");
    int dV=par("defaultVID");
    if(tagged)
    	{

			if(dV>=0)
				error("defaultVID can not be set for a tagged port.");
    	}
    else
    {
    	if(dV==-1)
    		error("defaultVID was not set for an untagged port.");
    	defaultVID=(vid) par("defaultVID");
    	cMessage *msg = new cMessage();
		msg->setName("MVRPDUtime");   //Next hello message generation.
		scheduleAt(simTime()+0.00001, msg);
    }

    if (ev.isGUI())
        updateDisplayString();
}

void PortFilt::sendMVRPDUs()
{
	MVRPDU * frame=new MVRPDU();
	frame->setDest(MACAddress("01-80-C2-00-00-0D"));
	frame->setVIDSArraySize(1);
	frame->setVIDS(0,defaultVID);
	send(frame,"GatesOut",1);
}

void PortFilt::handleMessage(cMessage *msg)
{
	if(msg->isSelfMessage())
	{
		sendMVRPDUs();
    	//Scheduling next MVRPDU
    	scheduleAt(simTime()+interFrameTime,msg);
	}
	else
	{
		cGate * arrivalGate=msg->getArrivalGate();
		int arrival=arrivalGate->getIndex();
		// frame received. Possibly untagged just from port 0
		switch(arrival)
		{
		case 0:  // Arrival=0. Coming into the Bridge.
			if(tagged)
			{  //Tagged port.
				if((dynamic_cast<EthernetIIFrame *>(msg)!=NULL)
						||(dynamic_cast<MVRPDU *>(msg)!=NULL)
						||(dynamic_cast<BPDUieee8021D *>(msg)!=NULL))
				{
					processTaggedFrame(msg);
				}
				else
				{
					ev<<"Just Ethernet1QFrame, BPDU and MVRPDU frames allowed. Drop";
					delete msg;
				}
			}
			else
			{   //Untagged port. Tagging not tagged frames.
				if(dynamic_cast<EthernetIIFrame *>(msg)!=NULL)
				{
					TagFrame(check_and_cast<EthernetIIFrame *>(msg));
					processTaggedFrame(msg);
				}
				else
				{
					ev<<"Filtering. Not allowed frame type";
					delete msg;
				}
			}
			break;
		case 1:  //Arrival = 1 Going out of the bridge.
			if(tagged)
			{
				processTaggedFrame(msg);
			}
			else
			{
				if(dynamic_cast<EthernetIIFrame *> (msg)!=NULL)
				{
					UntagFrame(check_and_cast<EthernetIIFrame *>(msg));
					processTaggedFrame(msg);
				}
				else if((dynamic_cast<MVRPDU *>(msg)!=NULL)
						||(dynamic_cast<BPDUieee8021D *>(msg)!=NULL))
				{
					ev<<"Filtering MVRP and RSTP";
					delete msg;
				}
				else
				{
					error("Unknown frame. Modify PortFilt to make it allowed or filtered.");
					delete msg;
				}
			}
			break;
		default:
			error("Unknown arrival gate");
		}
	}
}


void PortFilt::updateDisplayString()
{  //Tkenv shows port type.
    char buf[80];
    if(tagged==true)
    	sprintf(buf, "Tagged");
    else
    	sprintf(buf, "Untagged");
    getDisplayString().setTagArg("t",0,buf);
}

void PortFilt::processTaggedFrame (cMessage *msg)
{ //Just relay
	int arrival=msg->getArrivalGate()->getIndex();
   switch(arrival)
   {
	   case 0:
		   send(msg,"GatesOut",1);
		   break;
	   case 1:
		   send(msg,"GatesOut",0);
		   break;
	   default:
		   error("Unknown arrival gate");
		   delete msg;
		   break;
   }
}

void PortFilt::TagFrame (EthernetIIFrame *frame)
{ //Tagging
	Ethernet1QTag * Tag=new Ethernet1QTag("8021Q");
	Tag->setVID(defaultVID);
	Tag->setByteLength(ETHER_1Q_TAG_LENGTH);
	cPacket * temp=frame->decapsulate();
	if(temp!=NULL)
		Tag->encapsulate(temp);
	frame->encapsulate(Tag);
    if (frame->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
        frame->setByteLength(MIN_ETHERNET_FRAME_BYTES);

	if (!cSimulation::getActiveEnvir()->isDisabled())
	    frame->setDisplayString(ETHER_1Q_DISPLAY_STRING);
}

void PortFilt::UntagFrame (EthernetIIFrame *frame)
{  //Untagging
	Ethernet1QTag * Tag=check_and_cast<Ethernet1QTag *>(((EthernetIIFrame *)frame)->decapsulate());
	cPacket * Data=check_and_cast<cPacket *>(Tag->decapsulate());
	((EthernetIIFrame *)frame)->encapsulate(Data);
    if (frame->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
        frame->setByteLength(MIN_ETHERNET_FRAME_BYTES);
	delete Tag;
	if (!cSimulation::getActiveEnvir()->isDisabled())
	    frame->setDisplayString(ETHER_II_DISPLAY_STRING);
}

int PortFilt::getCost()
{// Returns RSTP port cost.
	return cost;
}

int PortFilt::getPriority()
{// Returns RSTP port priority.
	return priority;
}
bool PortFilt::isEdge()
{// Returns RSTP port type.
	return !tagged;
}
