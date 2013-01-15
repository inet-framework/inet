 /**
******************************************************
* @file EthernetLink.cc
* @brief EthernetIIFrame encapsulation.
* Modified from EtherLLC model.
*  - Removed LLC capabilities.
*  - Just EthernetIIFrames.
*  - Pause frames have also been removed.
*
* @author Juan Luis Garrote Molinero
* @version 1.0
* @date Feb 2011
******************************************************/

#include "EthernetLink.h"
#include "Ieee802Ctrl_m.h"
#include "8021Q.h"
#include "EtherMACAccess.h"
#include "MVRPDU.h"


Define_Module(EthernetLink);
EthernetLink::EthernetLink()
{}
EthernetLink::~EthernetLink()
{}

void EthernetLink::initialize(int stage)
{
	if(stage==1)
	{
		seqNum = 0;
		outputFrame=par("outputFrame");
		verbose=par("verbose");
		defaultVID=par("defaultVID");
		defaultSVID=par("defaultSVID");
		interFrameTime=(simtime_t) par("interFrameTime");
		//Validating parameters
		if(outputFrame>0)
		{
			int chk=par("defaultVID");
			if(chk<0)
				error("defaultVID not set.");
			if(outputFrame>1)
			{
				chk=par("defaultSVID");
				if(chk<0)
					error("defaultSVID not set");
			}
			cMessage * mvrpTime=new cMessage("mvrpTime");
			scheduleAt(simTime()+0.000001,mvrpTime);
		}
		//Get module mac address.
		cModule * mac=check_and_cast<EtherMAC *>(EtherMACAccess().get());
		address.setAddress(mac->par("address"));

		//Init counters
		WATCH(seqNum);
		totalFromHigherLayer = totalFromMAC = totalPassedUp = 0;
		WATCH(totalFromHigherLayer);
		WATCH(totalFromMAC);
		WATCH(totalPassedUp);
	}
}

void EthernetLink::handleMessage(cMessage *msg)
{
    if (msg->arrivedOn("lowerLayerIn"))
    {
		if(dynamic_cast<EthernetIIFrame *>(msg)!=NULL)
		{//Data frame received from MAC layer.
			processFrameFromMAC(check_and_cast<EthernetIIFrame *>(msg));
		}
		else
		{
			ev<<"Unexpected message type. Drop.";
		}
    }
    else if(msg->isSelfMessage())
    { //Time to send a MVRPDU
        handleSelfMessage(msg);
    }
    else
    {
        processPacketFromHigherLayer(PK(msg));
    }

    if (ev.isGUI())
        updateDisplayString();

}

void EthernetLink::handleSelfMessage(cMessage * msg)
{
	if(strcmp(msg->getName(),"mvrpTime")==0)
	{ //Generating MVRPDU
		MVRPDU * frame=new MVRPDU();
		frame->setPortIndex(0); //TODO Comprobar si esto viaja de verdad en la trama.
		frame->setVIDSArraySize(1);
		frame->setDest(MACAddress("01-80-C2-00-00-0D"));
		frame->setSrc(address);
		switch(outputFrame)
		{
		case 1:
			frame->setVIDS(0,defaultVID);
			break;
		case 2:
			frame->setVIDS(0,defaultSVID);
			break;
		default:
			error("MVRP does not know what is the requested VID. You should set an outputFrame value 0=EthernetII 1=802.1q or 2=802.1ad.");
		}
		send(frame,"lowerLayerOut");

		//Programming next MVRPDU
		scheduleAt(simTime()+interFrameTime,msg);
	}
}

void EthernetLink::updateDisplayString()
{ //TKenv shows counters.
    char buf[80];
    sprintf(buf, "passed up: %ld\nsent: %ld", totalPassedUp, totalFromHigherLayer);
    getDisplayString().setTagArg("t",0,buf);
}

void EthernetLink::processPacketFromHigherLayer(cPacket *msg)
{
    if (msg->getByteLength() > (MAX_ETHERNET_DATA_BYTES))
        error("packet from higher layer (%d bytes) exceed maximum Ethernet payload length (%d)", (int)(msg->getByteLength()), MAX_ETHERNET_DATA_BYTES);

    totalFromHigherLayer++;

    // Creates MAC header information and encapsulates received higher layer data
    // with this information and transmits resultant frame to lower layer

    // create Ethernet frame, fill it in from Ieee802Ctrl and encapsulate msg in it
    EV << "Encapsulating higher layer packet `" << msg->getName() <<"' for MAC\n";
    EV << "Sent from " << simulation.getModule(msg->getSenderModuleId())->getFullPath() << " at " << msg->getSendingTime() << " and was created " << msg->getCreationTime() <<  "\n";

    Ieee802Ctrl *etherctrl = dynamic_cast<Ieee802Ctrl *>(msg->removeControlInfo());
    if (!etherctrl)
        error("packet `%s' from higher layer received without Ieee802Ctrl", msg->getName());

    //Generating frame.
	cPacket * frame=msg;
    Ethernet1QTag * CTag=NULL;
    Ethernet1QTag * STag=NULL;
    EthernetIIFrame * EthIIF = new EthernetIIFrame(msg->getName());
    EthIIF->setDisplayString(ETHER_II_DISPLAY_STRING);
    EthIIF->setSrc(address);
    EthIIF->setDest(etherctrl->getDest());
    EthIIF->setEtherType(etherctrl->getEtherType());
    EthIIF->setByteLength(ETHER_MAC_FRAME_BYTES);
	if(outputFrame>0) // 802.1Q
	{
		CTag=new Ethernet1QTag ("8021Q");
	    CTag->setVID(defaultVID);
	    CTag->setByteLength(ETHER_1Q_TAG_LENGTH);
	    CTag->encapsulate(frame);
	    frame=CTag;
	    EthIIF->setDisplayString(ETHER_1Q_DISPLAY_STRING);
	}
	if(outputFrame>1) // 802.1ad
	{
	  	STag=new Ethernet1QTag("8021ad");
	    STag->setVID(defaultSVID);
	    STag->setByteLength(ETHER_1Q_TAG_LENGTH);
	    STag->encapsulate(frame);
	    frame=STag;
	    EthIIF->setDisplayString(ETHER_1AD_DISPLAY_STRING);
    }

    EthIIF->encapsulate(frame);
    delete etherctrl;
    if (EthIIF->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
        EthIIF->setByteLength(MIN_ETHERNET_FRAME_BYTES);
    send(EthIIF, "lowerLayerOut");
}

void EthernetLink::processFrameFromMAC(EthernetIIFrame *frame)
{
    totalFromMAC++;
    // Decapsulate it and pass up to higher layers.
    Ethernet1QTag * tag2=NULL;
    Ethernet1QTag * tag=NULL;
    cPacket *higherlayermsg = NULL;

    switch(outputFrame)
    {
    case 2:  //802.1ad
    	tag2=check_and_cast<Ethernet1QTag *>(frame->decapsulate());
   		tag=check_and_cast<Ethernet1QTag *>(tag2->decapsulate());
   		higherlayermsg=tag->decapsulate();
   		delete tag;
   		delete tag2;
    	break;
    case 1:	//802.1Q
		tag=check_and_cast<Ethernet1QTag *>(frame->decapsulate());
   		higherlayermsg=tag->decapsulate();
   		delete tag;
		break;
    case 0: //Just EthernetIIFrame
    	higherlayermsg=frame->decapsulate();
    	break;
    default:
    	error("Not valid outputFrame value.");
    	break;
    }

    if(higherlayermsg==NULL)
    	error("Wrong frame format");

	Ieee802Ctrl *etherctrl = new Ieee802Ctrl();
    etherctrl->setSrc(frame->getSrc());
    etherctrl->setDest(frame->getDest());
    etherctrl->setEtherType(frame->getEtherType());
    higherlayermsg->setControlInfo(etherctrl);

    if(verbose==true)
    {
    	EV << "Decapsulating frame `" << frame->getName() <<"', "
			"passing up contained packet `" << higherlayermsg->getName() << "' "
			"to higher layer " << "\n";
    }
    send(higherlayermsg,"upperLayerOut");
    totalPassedUp++;
    delete frame;
}


void EthernetLink::finish()
{
    recordScalar("packets from higher layer", totalFromHigherLayer);
    recordScalar("frames from MAC", totalFromMAC);
    recordScalar("packets passed up", totalPassedUp);
}

