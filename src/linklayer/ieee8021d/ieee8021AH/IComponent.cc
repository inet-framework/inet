/**
******************************************************
* @file IComponent.cc
* @brief Part of the I-Component module. 802.1ah-802.1ad conversion
* Basic conversion from IEEE 802.1ad frames to 802.1ah frames.
* @author Juan Luis Garrote Molinero
* @version 1.0
* @date Feb 2010
******************************************************/

#include "IComponent.h"
#include "EtherFrame_m.h"
#include "Ethernet.h"
#include "EtherMAC.h"
#include "MVRP.h"
#include "XMLUtils.h"
#include "RSTPAccess.h"


Define_Module( IComponent );


IComponent::IComponent()
{}
IComponent::~IComponent(){}

void IComponent::initialize(int stage)
{
	if(stage==2)  // "auto" MAC addresses assignment takes place in stage 0.
	{
		// Getting other modules access
		it=ITagTableAccess().get();
		RSTP * rstp=RSTPAccess().get();

		// Gets parameters
		address=rstp->getAddress(); //Gets BEB backbone mac address
		VID=(vid) par("defaultVID");
		int i=par("defaultVID"); //VID is an unsigned value. vid default value is "-1", and that indicates that the vid should not be registered.
		outputFrame=par("outputFrame");
		interFrameTime=(simtime_t)par("interFrameTime");
		verbose=par("verbose");

		requestVID=par("requestVID");
		defaultSVID=par("defaultSVID");
		defaultCVID=par("defaultCVID");

		//Reading itagtable configuration.
		readconfigfromXML(par("configIS").xmlValue());
		if(verbose==true)
		{
			ev<<"************"<<endl;
			ev<<address.str();
			it->printState();
		}

		// Validating parameters
		if(outputFrame==0)
		{
			error("IComponent will not accept or generate EthernetII frames. At least 802.1Q frames are required");
		}
		if((i<0)||(i>4096))
		{
			error("defaultVID not set, or invalid value");
		}
		int chk=par("requestVID");
		if((outputFrame==1)&&(chk<0))
			error("requestVID not set. No traffic would get to the BEB.");
		chk=par("defaultSVID");
		if((outputFrame<2)&&(chk<0))
			error("defaultSVID was not set while outputFrame was 802.1Q or EthII.");
		chk=par("defaultCVID");
		if((outputFrame<1)&&(chk<0))
			error("defaultCVID was not set while outputFrame was EthII.");

		//Init registration.
		//Registers the vid in the MVRP module. A MVRPDU is sent.
		MVRPDU * frame=new MVRPDU();
		frame->setVIDSArraySize(1);
		frame->setDest(MACAddress("01-80-C2-00-00-0D"));
		frame->setVIDS(0,VID);
		send(frame,"IGatesOut");
		//It could be easy to register more than one vid. Just another or a bigger message.
		if(requestVID>=0)
		{
			cMessage * msg=new cMessage("RequestVID");
			scheduleAt(simTime()+0.00001,msg);
		}
	}

}

void IComponent::sendMVRPDUs(cMessage *msg)
{//Sends corresponding MVRPDUs through the client gates.
	for(int i=0;i<gateSize("CGatesOut");i++)
	{
		MVRPDU * frame=new MVRPDU();
		frame->setDest(MACAddress("01-80-C2-00-00-0D"));
		frame->setSrc(address);
		if(outputFrame==1) // Output frame IEEE 802.1Q
		{
			frame->setVIDSArraySize(1);
			frame->setVIDS(0,requestVID);
		}
		else if (outputFrame==2) // Output frame IEEE 802.1ah
		{//Requesting configured SVids for that port.
			std::vector<vid> SVids=it->getSVids(activeISid,i);
			frame->setVIDSArraySize(SVids.size());
			for(unsigned int i=0;i<SVids.size();i++)
			{
				frame->setVIDS(i,SVids[i]);
			}
		}
		send(frame,"CGatesOut",i);
	}
	scheduleAt(simTime()+interFrameTime,msg);
}


void IComponent::readconfigfromXML(const cXMLElement * isidtab)
{// Reads itagtable info from configIS xml.
    ASSERT(isidtab);
	ASSERT(!strcmp(isidtab->getTagName(), "isidtab"));
	checkTags(isidtab, "IComp");
	cXMLElementList list = isidtab->getChildrenByTagName("IComp");
	//Looking for the IComponent info using the index number.
	for (cXMLElementList::iterator iter=list.begin(); iter != list.end(); iter++)
	{
		const cXMLElement& IComponent = **iter;
		int ind=getParameterIntValue(&IComponent, "index");
		if(ind==this->getIndex())
		{
			ev<<endl<<endl<<"Reading IComponent info"<<this->getIndex()<<endl;
			cXMLElementList ISids = IComponent.getChildrenByTagName("ISid");
			//Getting ISid info
			for(cXMLElementList::iterator iter2=ISids.begin(); iter2!=ISids.end(); iter2++)
			{
				const cXMLElement& ISidElement=**iter2;
				vid ISid=(vid) getParameterIntValue(&ISidElement,"vid");
				activeISid.push_back(ISid);
				it->createISid(ISid,gateSize("CGatesOut"));
				cXMLElementList Gates=ISidElement.getChildrenByTagName("Gate");
				//Getting gate entries.
				for(cXMLElementList::iterator iter3=Gates.begin();iter3!=Gates.end();iter3++)
				{
					const cXMLElement& GateElement=**iter3;
					it->asociateSVid(ISid, getParameterIntValue(&GateElement,"index"),getParameterIntValue(&GateElement,"SVid"));
				}
			}
		}
	}
}

void IComponent::handleMessage(cMessage *msg)
{// Handling messages

	if(msg->arrivedOn("CGatesIn"))
	{//Coming from client network
		ev<<"From Client network"<<endl;
		//Selecting handler
		if((outputFrame==1)&&(dynamic_cast<EthernetIIFrame*>(msg) != NULL))
		{
			handle1QFrame(check_and_cast <EthernetIIFrame *> (msg));
		}
		else if((outputFrame==2)&&(dynamic_cast<EthernetIIFrame*>(msg) != NULL))
		{
			handle1adFrame(check_and_cast <EthernetIIFrame *> (msg));
		}
		else if((dynamic_cast<EthernetIIFrame*>(msg) != NULL))
		{
			handleEtherIIFrame(check_and_cast <EthernetIIFrame *> (msg));
		}
		else
		{
			ev<<"I-Component not supported message frame"<<endl;
			delete msg;
		}
	}
	else if(msg->arrivedOn("IGatesIn"))
	{//Coming from backbone network. (from B-Component)
		ev<<"From BackBone network"<<endl;
		handle1AHFrame(check_and_cast <EthernetIIFrame *> (msg));
	}
	else if(msg->isSelfMessage())
	{
		if(strcmp(msg->getName(),"RequestVID")==0)
		{
			sendMVRPDUs(msg);
		}
		else
		{
			error("Unknown self message");
		}
	}
	else
	{
		error("Not recognized gate");
		delete msg;
	}
}


void IComponent::handle1QFrame(EthernetIIFrame *frame)
{//Handling 802.1Q Frame.
	Ethernet1QTag * STag=NULL;
	Ethernet1QTag * CTag=NULL;

	if(outputFrame<=1)
	{
		CTag=check_and_cast<Ethernet1QTag *>(frame->decapsulate());
		//Handling 1Q frame from client network
		if((unsigned)CTag->getVID()==requestVID)
			{
			if(verbose==true)
				ev << "802.1Q at Aditagger. Generating 802.1AD frame"<<endl;
			STag=new Ethernet1QTag("8021ad");
			STag->setVID(defaultSVID);
			STag->setByteLength(ETHER_1Q_TAG_LENGTH);
			STag->encapsulate(CTag);
			frame->encapsulate(STag);
			frame->setDisplayString(ETHER_1AD_DISPLAY_STRING);
			handle1adFrame(frame);
			}
			else
			{
				if(verbose==true)
					ev<<"C-Vlan not requested."<<endl;
				delete frame;
			}
	}
	else
	{
		error("Unexpected EthernetII or 802.1Q  frame. outputFrame was Ethernet 802.1ah");
		delete frame;
	}
}
void IComponent::handle1adFrame(EthernetIIFrame *frame)
{
	EthernetIIFrame * frameAH=NULL;
	Ethernet1QTag * BTag=NULL;
	Ethernet1ahITag * ITag=NULL;
	Ethernet1QTag * STag=NULL;
	Ethernet1QTag * CTag=NULL;
	EthernetIIFrame * EthIITemp=NULL;

	int Gate=-1;
	vid SVid=0;
	bool found=false;

	//Handling 1ad frame from client network
	frameAH=new EthernetIIFrame(frame->getName());
	frameAH->setDisplayString(ETHER_1AH_DISPLAY_STRING);
	frameAH->setByteLength(ETHER_MAC_2ND_FRAME_BYTES);
	STag=check_and_cast<Ethernet1QTag *>(frame->getEncapsulatedPacket());
	CTag=check_and_cast<Ethernet1QTag *>(STag->getEncapsulatedPacket());
	EthIITemp=frame;

	frameAH->setSrc(address);
	frame->setContextPointer(frame->getArrivalGate()); 	//This mark will give the IComponent the chance of avoiding resending or
															//duplication over the arrival port when admacrelay returns frames to the icomponent
	Gate=frame->getArrivalGate()->getIndex();
	SVid=STag->getVID();

	if(verbose==true)
	{
		ev << "802.1AD at Aditagger. Generating 802.1ah frame. Src:"<<address.str()<<endl;
		ev<<"Gate: "<<Gate<<" and SVid: "<<SVid<<endl;
		it->printState();
	}

	found=false;
	for(unsigned int i=0;i<activeISid.size();i++)
	{// For every active isid
	  	if(it->checkISid(activeISid[i],Gate,SVid))
	  	{ //Registers or refreshes source client mac.
	  		if(verbose==true)
	   		{
	  			ev<<"activeISid[i]: "<<activeISid[i]<<endl;
	  			ev<<"Correct check."<<endl;
				ev<<"Registering source ClientMAC:"<<endl;
				ev<<"Gate   SVID   CMAC"<<endl;
				ev<<frame->getArrivalGate()->getIndex()<<"         "<<STag->getVID()<<"     "<<EthIITemp->getSrc()<<endl;
	   		}
			if(it->registerCMAC(activeISid[i],Gate,SVid,EthIITemp->getSrc()))
			{
				ev<<"New CMAC registered"<<endl;
			}
			ITag=new Ethernet1ahITag();
			ITag->setISid(activeISid[i]);
			found=true;
			break;
		 }
	}
	if(found==false)
	{//There is not an entry for this SVid/Gate
	  	if(verbose==true)
		ev<<"There is not an entry for this SVid/Gate tuple.";
	  	delete frame;
	}
	else
	{//Lookup at itagtable for the ISid+client MAC entry. If it is not found, the diffusion mac is set.
		MACAddress BMACd;
		// Checks if the client MAC destination was registered.
		if(!it->resolveBMAC(ITag->getISid(),EthIITemp->getDest(),&BMACd))
		{	//If that client MAC was not known, the diffusion mac is set.
			ev<<"Destination CMAC not known. Setting group diffusion BMAC."<<endl;
		}
		frameAH->setDest(BMACd);
		if(verbose==true)
		{
			ev<<"Destination BMAC set:"<<frameAH->getDest().str()<<endl;
			it->printState();
		}

		//Set frame parameters
		BTag=new Ethernet1QTag();
		BTag->setVID(VID);
		BTag->setByteLength(ETHER_1Q_TAG_LENGTH);
		ITag->setByteLength(ETHER_1AH_ITAG_LENGTH);
		frameAH->setByteLength(ETHER_MAC_FRAME_BYTES);
		ITag->encapsulate(frame);
		BTag->encapsulate(ITag);
		frameAH->encapsulate(BTag);
		if (frameAH->getByteLength() < MIN_ETHERNET_FRAME_BYTES)   //This will never happen
			frameAH->setByteLength(MIN_ETHERNET_FRAME_BYTES);
		send(frameAH,"IGatesOut");
		ev<<"Frame sent."<<endl;
	}

}


void IComponent::handleEtherIIFrame(EthernetIIFrame *frame)
{//Handling EthernetII frame from client network
	if(outputFrame==0)
	{
		if(verbose==true)
			ev << "Ethernet II at Aditagger. Generating 802.1Q frame."<<endl;
		Ethernet1QTag * CTag=new Ethernet1QTag("8021Q");
		CTag->setVID(defaultCVID);
		CTag->setByteLength(ETHER_1Q_TAG_LENGTH);
		cPacket * Data=check_and_cast<cPacket *>(frame->decapsulate());
		CTag->encapsulate(Data);
		frame->encapsulate(CTag);
		frame->setDisplayString(ETHER_1Q_DISPLAY_STRING);
		handle1QFrame(frame);
	}
	else
	{
		error("Unexpected frame type. outputFrame was EthernetII");
		delete frame;
	}

}
bool IComponent::isISidBroadcast(MACAddress mac, int ISid)
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

void IComponent::handle1AHFrame(EthernetIIFrame *frame)
{//Handling 1ah frame from provider network
	Ethernet1QTag * BTag=check_and_cast<Ethernet1QTag *>(frame->getEncapsulatedPacket());
	Ethernet1ahITag * ITag=check_and_cast<Ethernet1ahITag *>(BTag->getEncapsulatedPacket());
	EthernetIIFrame * EthIITemp=check_and_cast<EthernetIIFrame *>(ITag->getEncapsulatedPacket());

	if((frame->getDest().compareTo(address)==0)||(frame->getDest().isBroadcast())||isISidBroadcast(frame->getDest(),ITag->getISid()))
	{
		if(verbose==true)
			ev << "802.1AH at aditagger.Decapsulating 802.1AD frame.";
		//If the arrived frame belongs to this icomponent
		vid ISid=ITag->getISid();
		for(unsigned int i=0;i<activeISid.size();i++)
		{//If the arrived message ISid belongs to this IComponent
			if(activeISid[i]==ISid)
			{
				//Register the backbone mac
				if(frame->getSrc()!=address)
				{
					if(verbose==true)
					{
					ev<<"Registering BMAC"<<endl;
					ev<<"CMAC "<<EthIITemp->getSrc()<<"BMAC "<< frame->getSrc()<<"ISid"<< ISid<<endl;
					}
					if(it->registerBMAC(ISid,EthIITemp->getSrc(),frame->getSrc()))
					{
						ev<<"New BMAC registered."<<endl;
					}
					else
					{
						ev<<"BMAC time refreshed."<<endl;
					}
				}
				std::vector <int> SVid;
				std::vector <int> Gate;
				//Gate is a list of gates and SVid is a list of Svids. Message will be sent through all gates with the associated SVid.
				//Prints basic info.
				if(verbose==true)
				{
					//Shows itagtable info.
					ev<<"*************"<<endl;
					ev<<"Aditagger"<<endl;
					ev<<address<<endl;
					it->printState();
				}
				if(it->resolveGate(ISid,EthIITemp->getDest(),&Gate,&SVid)==true)
				{
					ev<<"Found gate for ISid/CMAC."<<endl;

				}
				else
				{
					if(verbose==true)
					{
						ev<<"Gate not found for the ISid/CMAC combination. The frame is sent through all the ISid Gates."<<endl;
						ev<<"Message will be sent through: "<<Gate.size()<<endl;
						for(unsigned int j=0;j<Gate.size();j++)
						{
							ev<<"Gate: "<<Gate[j]<<" SVid: "<<SVid[j]<<endl;
						}
					}
				}
				// If the MAC is registered in more than one gate, it will be sent through all of them.
				//	Decapsulation

				Ethernet1QTag * BTag = check_and_cast<Ethernet1QTag *>(frame->decapsulate());
				//Frame is deleted at the end.
				Ethernet1ahITag * ITag = check_and_cast<Ethernet1ahITag *>(BTag->decapsulate());
				delete BTag;
				EthernetIIFrame* OriginalMessage=check_and_cast<EthernetIIFrame*>(ITag->decapsulate());
				delete ITag;

				for(unsigned int i = 0;i<SVid.size();i++)  //SVid.size() =  Gate.size()
				{
					EthernetIIFrame* message=OriginalMessage->dup();
					// SVid substitution
					Ethernet1QTag * STag=check_and_cast<Ethernet1QTag *>(message->decapsulate());
					STag->setVID(SVid[i]);
					// Reassemble
					if(outputFrame<2) // 802.1q output
					{
						Ethernet1QTag * CTag=check_and_cast<Ethernet1QTag *>(STag->decapsulate());
						delete STag;
						message->encapsulate(CTag);
						message->setDisplayString(ETHER_1Q_DISPLAY_STRING);
					}
					else
					{
						message->encapsulate(STag);
					}

					if(gateSize("CGatesIn")<=Gate[i])
					{ //Avoiding configuration errors.
						error("Configuration error. Check configIS file. An I-Sid has been configured in more than one I-Component for the same BEB. Or an unexisting gate has been configured.");
					}
					if(gate("CGatesIn",Gate[i])!=OriginalMessage->getContextPointer())  // Filtering input gate.
					{//It was marked in the 802.1ad context pointer
						send(message,"CGatesOut",Gate[i]);
					}
					else
						delete message;
				}
				delete OriginalMessage;
			}
		}
	}
	delete frame;
}


void IComponent::finish()
{
}



