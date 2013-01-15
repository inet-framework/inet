 /**
******************************************************
* @file TesterObject.cc
* @brief Test messages generator
* @author Juan Luis Garrote Molinero
* @version 1.0
* @date Feb 2011
******************************************************/
#include "MACAddress.h"
#include "Delivery_m.h"
#include "MVRPDU.h"
#include "XMLUtils.h"
#include "TesterObject.h"
#include "8021Q.h"
#include <algorithm>  //std::sort
#include "EtherApp_m.h"


Define_Module(TesterObject);

void TesterObject::initialize(int stage)
{
	//Get parameters
	verbose= par("verbose");
	testing= par("testing");
	detailedRSTP=par("detailedRSTP");
	detailedMVRP=par("detailedMVRP");
	detailedEthII=par("detailedEthII");
	detailedEth1Q=par("detailedEth1Q");
	detailedEth1ad=par("detailedEth1ad");
	detailedEth1ah=par("detailedEth1ah");

	//init counters
	SentMessages=0;
	ReceivedMessages=0;
	ReceivedBPDUs=0;
	ReceivedMVRPDUs=0;
	ReceivedEthernetII=0;
	ReceivedEthernet1Q=0;
	ReceivedEthernet1AD=0;
	ReceivedEthernet1AH=0;

	ReceivedTCNs=0;

	LastMVRPDU=NULL;
	LastEthII=NULL;
	LastEth1Q=NULL;
	LastEth1ad=NULL;
	LastEth1ah=NULL;

	WATCH(SentMessages);
	WATCH(ReceivedMessages);
	WATCH(ReceivedBPDUs);
	WATCH(ReceivedMVRPDUs);
	WATCH(ReceivedEthernetII);
	WATCH(ReceivedEthernet1Q);
	WATCH(ReceivedEthernet1AD);
	WATCH(ReceivedEthernet1AH);
	WATCH(ReceivedTCNs);

	//Reading messages to send from file
	const cXMLElement * tree= par("Messages").xmlValue();
    ASSERT(tree);
	ASSERT(!strcmp(tree->getTagName(), "Tests"));
	cXMLElementList list=tree->getChildrenByTagName("Message");
    for (cXMLElementList::iterator iter=list.begin(); iter != list.end(); iter++)
    {
    	cXMLElement * mens = *iter;
    	scheduleMessage(mens);
    }
	if(verbose==true)
	{
		printState();
	}

}

void TesterObject::scheduleMessage(cXMLElement * mens)
{
	//Getting parameters
	// simtime_t temp=(simtime_t) getParameterDoubleValue(mens,"Time");
	int message=getParameterIntValue(mens,"Type");
	cMessage * msg;
	Ethernet1QTag * tag;
	Ethernet1QTag * tag2;
	Ethernet1QTag * tag3;
	Ethernet1ahITag * itag;
	EtherFrame * msg2;
	cXMLElementList list;
	cXMLElement * aux;
	EtherAppReq * datapacket;

	switch(message)
	{
	case 0: //BPDU
		ev<<"BPDU"<<endl;
		msg=new BPDUieee8021D();
		((BPDUieee8021D *)msg)->setRootPriority(getParameterIntValue(mens,"RootP"));
		((BPDUieee8021D *)msg)->setRootMAC(MACAddress(getParameterStrValue(mens,"RootMAC")));
		((BPDUieee8021D *)msg)->setCost(getParameterIntValue(mens,"RPC"));
		((BPDUieee8021D *)msg)->setSrcPriority(getParameterIntValue(mens,"SrcP"));
		((BPDUieee8021D *)msg)->setSrc(MACAddress(getParameterStrValue(mens,"SrcMAC")));
		((BPDUieee8021D *)msg)->setPortPriority(getParameterIntValue(mens,"PortP"));
		((BPDUieee8021D *)msg)->setPortNumber(getParameterIntValue(mens,"Port"));
		((BPDUieee8021D *)msg)->setDest(MACAddress(getParameterStrValue(mens,"DestMAC")));
		((BPDUieee8021D *)msg)->setTC(getParameterBoolValue(mens,"TC"));
		if(((BPDUieee8021D *)msg)->getTC()==true)
		{
			((BPDUieee8021D *)msg)->setDisplayString("b=,,,#3e3ef3");
		}
        if (((BPDUieee8021D *)msg)->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
            ((BPDUieee8021D *)msg)->setByteLength(MIN_ETHERNET_FRAME_BYTES);
		sendDelayed(msg,(simtime_t) getParameterDoubleValue(mens,"Time"),"Out");
		SentMessages++;
		break;
	case 1: //MVRPDU
		ev<<"MVRPDU"<<endl;
		msg=new MVRPDU();
		((MVRPDU *)msg)->setSrc(MACAddress(getParameterStrValue(mens,"SrcMAC")));
		((MVRPDU *)msg)->setDest(MACAddress(getParameterStrValue(mens,"DestMAC")));
		aux=mens->getFirstChildWithTag("VIDs");
		list=aux->getChildrenByTagName("VID");
		((MVRPDU *)msg)->setVIDSArraySize(list.size());
		for(unsigned int k=0;k<list.size();k++)
		{
			((MVRPDU *)msg)->setVIDS(k,atoi(list[k]->getNodeValue()));
		}
		if (((MVRPDU *)msg)->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
		    ((MVRPDU *)msg)->setByteLength(MIN_ETHERNET_FRAME_BYTES);
		sendDelayed(msg,(simtime_t) getParameterDoubleValue(mens,"Time"),"Out");
		SentMessages++;
		break;
	case 2: //ETHII
		ev<<"ETHII"<<endl;

		datapacket = new EtherAppReq("req-", IEEE802CTRL_DATA);
		datapacket->setRequestId(0);
		datapacket->setResponseBytes(getParameterIntValue(mens,"ByteLength"));
		datapacket->setByteLength(getParameterIntValue(mens,"ByteLength"));
		msg=new EthernetIIFrame(datapacket->getName());
		((EthernetIIFrame *) msg)->setByteLength(ETHER_MAC_FRAME_BYTES);
		((EthernetIIFrame *) msg)->encapsulate(datapacket);   // [msg [datapacket] ]
		((EthernetIIFrame *)msg)->setSrc(MACAddress(getParameterStrValue(mens,"SrcMAC")));
		((EthernetIIFrame *)msg)->setDest(MACAddress(getParameterStrValue(mens,"DestMAC")));
        if (((EthernetIIFrame *)msg)->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
            ((EthernetIIFrame *)msg)->setByteLength(MIN_ETHERNET_FRAME_BYTES);
		((EthernetIIFrame *)msg)->setDisplayString(ETHER_II_DISPLAY_STRING);
		sendDelayed(msg,(simtime_t) getParameterDoubleValue(mens,"Time"),"Out");
		SentMessages++;
		break;
	case 3: //ETH1Q
		ev<<"ETH1Q"<<endl;
		datapacket = new EtherAppReq("req-", IEEE802CTRL_DATA);
		datapacket->setRequestId(0);
		datapacket->setResponseBytes(getParameterIntValue(mens,"ByteLength"));
		datapacket->setByteLength(getParameterIntValue(mens,"ByteLength"));
		msg=new EthernetIIFrame(datapacket->getName());
		((EthernetIIFrame *)msg)->setByteLength(ETHER_MAC_FRAME_BYTES);
		((EthernetIIFrame *)msg)->setSrc(MACAddress(getParameterStrValue(mens,"SrcMAC")));
		((EthernetIIFrame *)msg)->setDest(MACAddress(getParameterStrValue(mens,"DestMAC")));
		tag=new Ethernet1QTag("8021Q");
		((Ethernet1QTag *)tag)->setByteLength(ETHER_1Q_TAG_LENGTH);
		((Ethernet1QTag *)tag)->setVID(getParameterIntValue(mens,"VID"));
		((Ethernet1QTag *) tag)->encapsulate(datapacket);
		((EthernetIIFrame *)msg)->encapsulate((Ethernet1QTag *) tag);    //  [msg [tag [datapacket] ] ]
		((EthernetIIFrame *)msg)->setDisplayString(ETHER_1Q_DISPLAY_STRING);
		if (((EthernetIIFrame *)msg)->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
		    ((EthernetIIFrame *)msg)->setByteLength(MIN_ETHERNET_FRAME_BYTES);
		sendDelayed(msg,(simtime_t) getParameterDoubleValue(mens,"Time"),"Out");
		SentMessages++;
		break;
	case 4: //ETH1AD
		ev<<"ETH1AD"<<endl;
		datapacket = new EtherAppReq("req-", IEEE802CTRL_DATA);
		datapacket->setRequestId(0);
		datapacket->setResponseBytes(getParameterIntValue(mens,"ByteLength"));
		datapacket->setByteLength(getParameterIntValue(mens,"ByteLength"));
		msg=new EthernetIIFrame(datapacket->getName());
		tag2=new Ethernet1QTag("8021ad");
		tag=new Ethernet1QTag("8021Q");

		((Ethernet1QTag *)tag)->setByteLength(ETHER_1Q_TAG_LENGTH);
		((Ethernet1QTag *)tag)->setVID(getParameterIntValue(mens,"CVID"));
		((Ethernet1QTag *)tag2)->setByteLength(ETHER_1Q_TAG_LENGTH);
		((Ethernet1QTag *)tag2)->setVID(getParameterIntValue(mens,"SVID"));
		((EthernetIIFrame *)msg)->setByteLength(ETHER_MAC_FRAME_BYTES);
		((Ethernet1QTag *) tag)->encapsulate(datapacket);
		((Ethernet1QTag *)tag2)->encapsulate((Ethernet1QTag *)tag);
		((EthernetIIFrame *)msg)->encapsulate((Ethernet1QTag *)tag2);   // [msg [tag2 [tag [datapacket] ] ] ]
		((EthernetIIFrame *)msg)->setSrc(MACAddress(getParameterStrValue(mens,"SrcMAC")));
		((EthernetIIFrame *)msg)->setDest(MACAddress(getParameterStrValue(mens,"DestMAC")));
		((EthernetIIFrame *)msg)->setDisplayString(ETHER_1AD_DISPLAY_STRING);
		if (((EthernetIIFrame *)msg)->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
		    ((EthernetIIFrame *)msg)->setByteLength(MIN_ETHERNET_FRAME_BYTES);
		sendDelayed(msg,(simtime_t) getParameterDoubleValue(mens,"Time"),"Out");
		SentMessages++;
		break;
	case 5:	//ETH1AH
		ev<<"ETH1AH"<<endl;
		datapacket = new EtherAppReq("req-", IEEE802CTRL_DATA);
		datapacket->setRequestId(0);
		datapacket->setResponseBytes(getParameterIntValue(mens,"ByteLength"));
		datapacket->setByteLength(getParameterIntValue(mens,"ByteLength"));
		tag=new Ethernet1QTag("8021Q");
		((Ethernet1QTag *)tag)->setByteLength(ETHER_1Q_TAG_LENGTH);
		((Ethernet1QTag *)tag)->setVID(getParameterIntValue(mens,"CVID"));
		tag2=new Ethernet1QTag("8021ad");
		((Ethernet1QTag *)tag2)->setByteLength(ETHER_1Q_TAG_LENGTH);
		((Ethernet1QTag *)tag2)->setVID(getParameterIntValue(mens,"SVID"));
		((Ethernet1QTag *)tag)->encapsulate(datapacket);
		((Ethernet1QTag *)tag2)->encapsulate((Ethernet1QTag *)tag);
		msg=new EthernetIIFrame(datapacket->getName());
		((EthernetIIFrame *)msg)->setByteLength(ETHER_MAC_FRAME_BYTES);
		((EthernetIIFrame *)msg)->setSrc(MACAddress(getParameterStrValue(mens,"SrcMAC")));
		((EthernetIIFrame *)msg)->setDest(MACAddress(getParameterStrValue(mens,"DestMAC")));
		((EthernetIIFrame *)msg)->encapsulate((Ethernet1QTag *)tag2);   // [msg [tag2 [tag [datapacket] ] ] ]
		((EthernetIIFrame *)msg)->setDisplayString(ETHER_1AD_DISPLAY_STRING);
		itag=new Ethernet1ahITag();
		itag->setByteLength(ETHER_1AH_ITAG_LENGTH);
		itag->setISid(getParameterIntValue(mens,"ISID"));
		itag->encapsulate((EthernetIIFrame *)msg);
		tag3=new Ethernet1QTag("8021ah");
		tag3->setVID(getParameterIntValue(mens,"BVID"));
		tag3->setByteLength(ETHER_1Q_TAG_LENGTH);
		tag3->encapsulate((Ethernet1ahITag *)itag);
		msg2=new EthernetIIFrame("8021ah");
		msg2->setByteLength(ETHER_MAC_2ND_FRAME_BYTES);
		msg2->setSrc(MACAddress(getParameterStrValue(mens,"SrcBMAC")));
		msg2->setDest(MACAddress(getParameterStrValue(mens,"DestBMAC")));
		msg2->encapsulate(tag3);   								//  [msg2 [tag3 [itag [msg [tag2 [tag [datapacket] ] ] ] ] ] ]
		if (((EthernetIIFrame *)msg2)->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
		     ((EthernetIIFrame *)msg2)->setByteLength(MIN_ETHERNET_FRAME_BYTES);
		((EthernetIIFrame *)msg2)->setDisplayString(ETHER_1AH_DISPLAY_STRING);
		sendDelayed(msg2,(simtime_t) getParameterDoubleValue(mens,"Time"),"Out");
		SentMessages++;
		break;
	}
}

void TesterObject::finish()
{
	if(testing==true)
	{
		FILE * pFile;
		char buff[50];
		sprintf(buff,"results/TESTER-%s.info",getParentModule()->getName());
		pFile = fopen (buff,"w");
		  if (pFile!=NULL)
		  {
			 //Basic trace
				fprintf(pFile,"%s\n",getParentModule()->getName());
				fprintf(pFile,"Sent messages %d\n",SentMessages);
				fprintf(pFile,"Received messages %d\n",ReceivedMessages);
				fprintf(pFile,"BPDU %d\n",ReceivedBPDUs);

			// RSTP detailed info
				if(detailedRSTP==true)
				{
					fprintf(pFile,"   TCNs %d\n",ReceivedTCNs);
				}
			//MVRP
				fprintf(pFile,"MVRP %d\n",ReceivedMVRPDUs);
				//detailed info
				if((detailedMVRP==true)&&(LastMVRPDU!=NULL))
				{
					fprintf(pFile,"   Last MVRPDU:  Src:%s",LastMVRPDU->getSrc().str().c_str());
					fprintf(pFile,"   Dest:%s", LastMVRPDU->getDest().str().c_str());
					fprintf(pFile,"   VIDs:");
					int Size=LastMVRPDU->getVIDSArraySize();
					std::vector<vid> VIDS;
					for(int i=0;i<Size;i++)
					{
						VIDS.push_back(LastMVRPDU->getVIDS(i));
					}
					std::sort(VIDS.begin(),VIDS.end());
					for(int i=0;i<Size;i++)
					{
						fprintf(pFile,"   %d",VIDS[i]);
					}
					fprintf(pFile,"\n");
				}
			//ETHII
				fprintf(pFile,"Eth2 %d\n",ReceivedEthernetII);
				//detailed info
				if((detailedEthII==true)&&(LastEthII!=NULL))
				{
					fprintf(pFile,"   Last message: Src:%s",LastEthII->getSrc().str().c_str());
					fprintf(pFile,"  Dest:%s",LastEthII->getDest().str().c_str());
					fprintf(pFile," Size:%ld\n",(long int)LastEthII->getByteLength());
				}
			//802.1Q
				fprintf(pFile,"Eth1Q %d\n",ReceivedEthernet1Q);
				//detailed info
				if((detailedEth1Q==true)&&(LastEth1Q!=NULL))
				{
					Ethernet1QTag * CTag=check_and_cast<Ethernet1QTag *>(LastEth1Q->getEncapsulatedPacket());
					fprintf(pFile,"   Last message: Vlan:%d",CTag->getVID());
					fprintf(pFile," Src:%s",LastEth1Q->getSrc().str().c_str());
					fprintf(pFile,"  Dest:%s",LastEth1Q->getDest().str().c_str());
					fprintf(pFile," Size:%ld\n",(long int)LastEth1Q->getByteLength());
				}
			//802.1ad
				fprintf(pFile,"Eth1ad %d\n",ReceivedEthernet1AD);
				//detailed info
				if((detailedEth1ad==true)&&(LastEth1ad!=NULL))
				{
					Ethernet1QTag * STag=check_and_cast<Ethernet1QTag *>(LastEth1ad->getEncapsulatedPacket());
					Ethernet1QTag * CTag=check_and_cast<Ethernet1QTag *>(STag->getEncapsulatedPacket());
					fprintf(pFile,"   Last message: S-Vid:%d",STag->getVID());
					fprintf(pFile," Vlan:%d",CTag->getVID());
					fprintf(pFile," Src:%s",LastEth1ad->getSrc().str().c_str());
					fprintf(pFile,"  Dest:%s",LastEth1ad->getDest().str().c_str());
					fprintf(pFile," Size:%ld\n",(long int)LastEth1ad->getByteLength());
				}
			//802.1ah
				fprintf(pFile,"Eth1ah %d\n",ReceivedEthernet1AH);
				//detailed info
				if((detailedEth1ah==true)&&(LastEth1ah!=NULL))
				{
					Ethernet1QTag * BTag=check_and_cast<Ethernet1QTag *>(LastEth1ah->getEncapsulatedPacket());
					Ethernet1ahITag * ITag=check_and_cast<Ethernet1ahITag *>(BTag->getEncapsulatedPacket());
					EthernetIIFrame * EthIITemp=check_and_cast<EthernetIIFrame *>(ITag->getEncapsulatedPacket());
					Ethernet1QTag * STag=check_and_cast<Ethernet1QTag *>(EthIITemp->getEncapsulatedPacket());
					Ethernet1QTag * CTag=check_and_cast<Ethernet1QTag *>(STag->getEncapsulatedPacket());
					fprintf(pFile,"   Last message: B-Vid:%d",BTag->getVID());
					fprintf(pFile,"  B-Src:%s",LastEth1ah->getSrc().str().c_str());
					fprintf(pFile,"  B-Dest:%s",LastEth1ah->getDest().str().c_str());
					fprintf(pFile,"  S-Vid:%d",STag->getVID());
					fprintf(pFile," Vlan:%d",CTag->getVID());
					fprintf(pFile," Src:%s",EthIITemp->getSrc().str().c_str());
					fprintf(pFile,"  Dest:%s",EthIITemp->getDest().str().c_str());
					fprintf(pFile," Size:%ld\n",(long int)LastEth1ah->getByteLength());
				}
		  }
		  else
			  error("Could not open %s file.",buff);
	}
}




TesterObject::~TesterObject()
{
	if(LastMVRPDU!=NULL)
		delete LastMVRPDU;
	if(LastEthII!=NULL)
		delete LastEthII;
	if(LastEth1Q!=NULL)
		delete LastEth1Q;
	if(LastEth1ad!=NULL)
		delete LastEth1ad;
	if(LastEth1ah!=NULL)
		delete LastEth1ah;

}
void TesterObject::printState()
{//Printing basic trace
	ev<<getParentModule()->getName()<<endl;
	ev<<"Sent messages"<<SentMessages<<endl;
	ev<<"Received messages "<<ReceivedMessages<<endl;
	ev<<"BPDU "<<ReceivedBPDUs<<endl;
	if(testing==true)
	{
		ev<<"Printing file. results/TESTER-"<<getParentModule()->getName()<<".info"<<endl;
	}
}

void TesterObject::handleMessage(cMessage *msg)
{
	ReceivedMessages++;
	//Calling specific handler
	if(dynamic_cast<BPDUieee8021D *>(msg)!=NULL)
	{
		handleIncomingFrame(check_and_cast<BPDUieee8021D *>(msg));
	}
	else if(dynamic_cast<MVRPDU *>(msg)!=NULL)
	{
		handleIncomingFrame(check_and_cast<MVRPDU *>(msg));
	}
	else if(dynamic_cast<EthernetIIFrame *>(msg)!=NULL)
	{// Every Ethernet II Message. That includes 802.1Q, 802.1ad and 802.1ah
		handleIncomingFrame(check_and_cast<EthernetIIFrame *>(msg));
	}
}


void TesterObject::handleIncomingFrame(BPDUieee8021D * msg)
{
	//Counting
	ReceivedBPDUs++;
	if(msg->getTC()==true)
	{//Counts TC flags.
		ReceivedTCNs++;
	}
	if(verbose==true)
	{
		//Print message
		ev<<endl<<getParentModule()->getName()<<"-Received message"<<endl;
		ev<<"Dest:"<<msg->getDest().str()<<endl;
		ev<<"RootP:"<<msg->getRootPriority()<<endl;
		ev<<"Root:"<<msg->getRootMAC().str()<<endl;
		ev<<"RPC:"<<msg->getCost()<<endl;
		ev<<"SrcP:"<<msg->getSrcPriority()<<endl;
		ev<<"Src:"<<msg->getSrc().str()<<endl;
		ev<<"PortP:"<<msg->getPortPriority()<<endl;
		ev<<"Port:"<<msg->getPortNumber()<<endl;
	}
	delete msg;
}
void TesterObject::handleIncomingFrame(MVRPDU *msg)
{
	ReceivedMVRPDUs++; //Counting
	if(verbose==true)
	{
		//Print message
		ev<<endl<<getParentModule()->getName()<<"-Received message"<<endl;
		ev<<"Dest:"<<msg->getDest().str()<<endl;
		ev<<"Src:"<<msg->getSrc().str()<<endl;
		ev<<"Num vids:"<<msg->getVIDSArraySize();
		ev<<"VIDS: ";
		for(unsigned int i=0;i<msg->getVIDSArraySize();i++)
		{
			ev<<"  "<<msg->getVIDS(i);
		}
		ev<<endl;
	}
	delete LastMVRPDU;  //It will remember the last received MVRPDU.
	LastMVRPDU=msg;
}
void TesterObject::handleIncomingFrame(EthernetIIFrame *msg)
{
	//Detecting message kind. 0=EthernetII 1=802.1Q 2=802.1ad 3=802.1ah
	int kind=0;
	cPacket * Temp1=msg->getEncapsulatedPacket();
	if(dynamic_cast<Ethernet1QTag*>(Temp1)!=NULL)
	{
		cPacket * Temp2=Temp1->getEncapsulatedPacket();
		if(dynamic_cast<Ethernet1QTag*>(Temp2)!=NULL)
		{
			kind=2;
		}
		else if(dynamic_cast<Ethernet1ahITag*>(Temp2)!=NULL)
		{
			kind=3;
		}
		else
		{
			kind=1;
		}
	}
	else
	{
		kind=0;
	}

	switch(kind)
	{
	case 3:
			ReceivedEthernet1AH++; //Counting.
			if(verbose==true)
			{//Print message.
				ev<<endl<<getParentModule()->getName()<<"-Received message"<<endl;
				Ethernet1QTag * BTag=check_and_cast<Ethernet1QTag *>(msg->getEncapsulatedPacket());
				Ethernet1ahITag * ITag=check_and_cast<Ethernet1ahITag *>(BTag->getEncapsulatedPacket());
				EthernetIIFrame * EthIITemp=check_and_cast<EthernetIIFrame *>(ITag->getEncapsulatedPacket());
				Ethernet1QTag * STag=check_and_cast<Ethernet1QTag *>(EthIITemp->getEncapsulatedPacket());
				Ethernet1QTag * CTag=check_and_cast<Ethernet1QTag *>(STag->getEncapsulatedPacket());

				ev<<"Src:"<<EthIITemp->getSrc().str()<<endl;
				ev<<"Dest:"<<EthIITemp->getDest().str()<<endl;
				ev<<"ByteLength:"<<msg->getByteLength()<<endl;
				ev<<"CVID:"<<CTag->getVID()<<endl;
				ev<<"SVID:"<<STag->getVID()<<endl;
				ev<<"BVID:"<<BTag->getVID()<<endl;
				ev<<"ISID:"<<ITag->getISid()<<endl;
				ev<<"DestBMAC:"<<msg->getDest().str()<<endl;
				ev<<"SrcBMAC:"<<msg->getSrc().str()<<endl;
			}
			delete LastEth1ah;
			LastEth1ah=msg;  //Remember last received 802.1ah message.
			break;
	case 2:
			ReceivedEthernet1AD++; //Counting.
			if(verbose==true)
			{ //Print message.
			ev<<endl<<getParentModule()->getName()<<"-Received message"<<endl;
			Ethernet1QTag * STag=check_and_cast<Ethernet1QTag *>(msg->getEncapsulatedPacket());
			Ethernet1QTag * CTag=check_and_cast<Ethernet1QTag *>(STag->getEncapsulatedPacket());
			EthernetIIFrame * EthIITemp=msg;
			ev<<"Src:"<<EthIITemp->getSrc().str()<<endl;
			ev<<"Dest:"<<EthIITemp->getDest().str()<<endl;
			ev<<"ByteLength:"<<msg->getByteLength()<<endl;
			ev<<"CVID:"<<CTag->getVID()<<endl;
			ev<<"SVID:"<<STag->getVID()<<endl;
			}
			delete LastEth1ad;
			LastEth1ad=msg;  //Remember last received 802.1ad message.
			break;

	case 1:
			ReceivedEthernet1Q++; //Counting.
			if(verbose==true)
			{ //Print message.
			ev<<endl<<getParentModule()->getName()<<"-Received message"<<endl;
			Ethernet1QTag * CTag=check_and_cast<Ethernet1QTag *>(msg->getEncapsulatedPacket());
			EthernetIIFrame * EthIITemp=msg;
			ev<<"Src:"<<EthIITemp->getSrc().str()<<endl;
			ev<<"Dest:"<<EthIITemp->getDest().str()<<endl;
			ev<<"ByteLength:"<<msg->getByteLength()<<endl;
			ev<<"VID:"<<CTag->getVID()<<endl;
			}
			delete LastEth1Q;
			LastEth1Q=msg;  //Remember last received 802.1Q message.
			break;

	default:
		ReceivedEthernetII++; //Counting.
		if(verbose==true)
		{ //Print message.
			ev<<endl<<getParentModule()->getName()<<"-Received message"<<endl;
			ev<<"Src:"<<msg->getSrc().str()<<endl;
			ev<<"Dest:"<<msg->getDest().str()<<endl;
			ev<<"ByteLength:"<<msg->getByteLength()<<endl;
		}
		delete LastEthII;
		LastEthII=msg;  //Remember last received Ethernet II frame.
		break;
	}

}
