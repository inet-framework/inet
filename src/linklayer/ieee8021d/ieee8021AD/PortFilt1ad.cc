 /**
******************************************************
* @file PortFilt1ad.cc
* @brief Tagging and filtering skills.
*
* @author Juan Luis Garrote Molinero
* @version 1.0
* @date Feb 2011
******************************************************/
#include "PortFilt1ad.h"
#include "PortFilt.h"
#include "MVRPDU.h"
#include "BPDU.h"
#include <XMLUtils.h>



Define_Module(PortFilt1ad);

PortFilt1ad::PortFilt1ad()
{}
PortFilt1ad::~PortFilt1ad()
{}

//Reading itagtable configuration.
void PortFilt1ad::initialize()
{
	PortFilt::initialize();
	readCVIDsfromXML(par("confCVIDs").xmlValue());
	if(verbose==true)
	{
		ev<<"Registered CVIDs. "<<endl;
		for(unsigned int i=0;i<registeredCVids.size();i++)
		{
			ev<<registeredCVids[i]<<endl;
		}
	}
}
/*
 * Reading from xml.
 */
void PortFilt1ad::readCVIDsfromXML(const cXMLElement * CVIDstab)
{
    ASSERT(CVIDstab);
	ASSERT(!strcmp(CVIDstab->getTagName(), "CVIDstab"));
	    checkTags(CVIDstab, "PortFilt");
	    cXMLElementList list = CVIDstab->getChildrenByTagName("PortFilt");
	    //Looking for the IComponent info using the index number.
	    for (cXMLElementList::iterator iter=list.begin(); iter != list.end(); iter++)
	    {
	    	const cXMLElement& IComponent = **iter;
	    	int ind=getParameterIntValue(&IComponent, "index");
	    	// Each component copies its own info.
	    	if(ind==this->getIndex())
	    	{
	    		registeredCVids.clear();
	    		ev<<endl<<endl<<"Reading PortFilt info "<<this->getIndex()<<endl;

	    		cXMLElementList CVids = IComponent.getChildrenByTagName("CVids");
	    		//Getting CVids info
	    		for(cXMLElementList::iterator iter2=CVids.begin(); iter2!=CVids.end(); iter2++)
	    		{
	    			const cXMLElement& CVidsElement=**iter2;
	    			cXMLElementList CVidList = CVidsElement.getChildrenByTagName("CVid");
	    			for(cXMLElementList::iterator iter3=CVidList.begin(); iter3!=CVidList.end();iter3++)
	    			{
	    				cXMLElement * CVIDElement=*iter3;
	    				if(CVIDElement!=NULL)
	    				{
	    					vid CVid=(vid) atoi(CVIDElement->getNodeValue());
							registeredCVids.push_back(CVid);
	    				}
	    				else
	    					error("XML error");
	    			}
	    		}
	    	}
	   }
	   if((registeredCVids.size()==0)&&(tagged==false))  //Just checking.
	   {
			char buff[50];
			sprintf(buff,"confCVIDs does not contain at least one valid C-Vid for PortFilt %d",this->getIndex());
			error(buff);
	   }
}
void PortFilt1ad::handleMessage(cMessage *msg)
{

	if(msg->isSelfMessage())
	{
		sendMVRPDUs();
		//Schedule next MVRPDU
    	scheduleAt(simTime()+interFrameTime,msg);
	}
	else
	{
		cGate * arrivalGate=msg->getArrivalGate();
		int arrival=arrivalGate->getIndex();
		// frame received. Possibly untagged if it comes from port 0
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
					ev<<"Just Ethernet1ADFrame, BPDU and MVRPDU frames allowed";
					delete msg;
				}
			}
			else
			{   //Untagged port. Tagging not tagged frames.
				if(dynamic_cast<EthernetIIFrame *>(msg)!=NULL)
				{
					processUntaggedFrame(check_and_cast<EthernetIIFrame *>(msg));
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
			{   //Untag.
				if(dynamic_cast<EthernetIIFrame *> (msg)!=NULL)
				{
					Ethernet1QTag * STag=check_and_cast<Ethernet1QTag *>(((EthernetIIFrame *)msg)->decapsulate());
					if(STag==NULL)
						error("Wrong frame format");
					EthernetIIFrame *frame = dynamic_cast<EthernetIIFrame *> (msg);
					Ethernet1QTag * CTag=check_and_cast<Ethernet1QTag *>(STag->decapsulate());
					frame->encapsulate(CTag);
					if (frame->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
					    frame->setByteLength(MIN_ETHERNET_FRAME_BYTES);  // "padding"

					((EthernetIIFrame *)msg)->setDisplayString(ETHER_1Q_DISPLAY_STRING);
					bool registered=false;
					vid CVid=CTag->getVID();

					for(unsigned int i=0;i<registeredCVids.size();i++) //Check registration.
					{
						if(registeredCVids[i]==CVid)
							registered=true;
					}
					if(registered==true)
					{
						processTaggedFrame(msg);
					}
					else
					{ //Not registered
						delete msg;
					}
					delete STag; //Deleting spare tags
				}
				else if((dynamic_cast<MVRPDU *>(msg)!=NULL)
						||(dynamic_cast<BPDUieee8021D *>(msg)!=NULL))
				{
					ev<<"Filtering MVRP and RSTP";
					delete msg;
				}
				else
				{
					ev<<"Unknown frame type. Modify PortFilt1ad to make it allowed or filtered.";
					delete msg;
				}
			}
			break;
		default:
			error("Unknown arrival gate");
		}
	}
}

void PortFilt1ad::sendMVRPDUs()
{
	//Generating MVRPDUs
	MVRPDU * frame=new MVRPDU();
	frame->setDest(MACAddress("01-80-C2-00-00-0D"));
	frame->setVIDSArraySize(1);
	frame->setVIDS(0,defaultVID);
	send(frame,"GatesOut",1);

	frame=new MVRPDU();
	frame->setDest(MACAddress("01-80-C2-00-00-0D"));
	frame->setVIDSArraySize(registeredCVids.size());
	for(unsigned int i=0;i<registeredCVids.size();i++)
	{
		frame->setVIDS(i,registeredCVids[i]);
	}
    if (frame->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
        frame->setByteLength(MIN_ETHERNET_FRAME_BYTES);
	send(frame,"GatesOut",0);
}

void PortFilt1ad::processUntaggedFrame (EthernetIIFrame *frame)
{
	//Tagging
	bool registered=false;
	Ethernet1QTag * CTag=check_and_cast<Ethernet1QTag *>(frame->decapsulate());
	vid CVid=CTag->getVID();
	for(unsigned int i=0;i<registeredCVids.size();i++)  //Check registration
	{
		if(registeredCVids[i]==CVid)
			registered=true;
	}
	if(registered==true)
	{
		//Generating tags.
		Ethernet1QTag * STag=new Ethernet1QTag("8021ad");
		STag->setVID(defaultVID);
		STag->setByteLength(ETHER_1Q_TAG_LENGTH);
		STag->encapsulate(CTag);
		frame->encapsulate(STag);
	    if (frame->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
	        frame->setByteLength(MIN_ETHERNET_FRAME_BYTES);
		frame->setDisplayString(ETHER_1AD_DISPLAY_STRING);
		processTaggedFrame(frame);
	}
	else
	{ //It was not registered.
		delete frame;
	}
}



