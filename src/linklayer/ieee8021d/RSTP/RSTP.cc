 /**
******************************************************
* @file RSTP.cc
* @brief RSTP Protocol control
*
* @author Juan Luis Garrote Molinero
* @version 1.0
* @date Feb 2011
******************************************************/
#include "RSTP.h"
#include "Ethernet.h"
#include "EtherFrame_m.h"
#include "MACRelayUnitBase.h"
#include "Admacrelay.h"
#include "EtherMAC.h"
#include "Cache1QAccess.h"
#include "AdmacrelayAccess.h"
#include "XMLUtils.h"
#include "Relay1QAccess.h"



Define_Module (RSTP);
RSTP::RSTP()
{
	helloM = new cMessage();
	helloM->setName("itshellotime");
	forwardM=new cMessage();
	forwardM->setName("upgrade");
	migrateM=new cMessage();
	migrateM->setName("timetodesignate");

}

RSTP::~RSTP()
{
	cancelAndDelete(helloM);
	cancelAndDelete(forwardM);
	cancelAndDelete(migrateM);

}

void RSTP::initialize(int stage)
{
	Parent=this->getParentModule();
	if(Parent==NULL)
	{
		error("Parent not found");
	}
	if(stage==1) // "auto" MAC addresses assignment takes place in stage 0.
	{
		sw=Cache1QAccess().get(); //cache pointer
		//Gets the relay pointer.
		admac=AdmacrelayAccess().getIfExists();
		if(admac==NULL)
		{
			admac=Relay1QAccess().get();
		}
		if(admac==NULL)
			error("Relay module not found");


		//Gets the backbone mac address
		cModule * macUnit=check_and_cast<cModule *>(Parent)->getSubmodule("macB",0);
		if(macUnit==NULL)
		{
			macUnit=check_and_cast<cModule *>(Parent)->getSubmodule("mac",0);
		}
		if(macUnit!=NULL)
		{
			address.setAddress(check_and_cast<EtherMAC *>(macUnit)->par("address"));
		}
		else
		{
			ev<<"macB[0] not found. Is not this module connected to another BEB?"<<endl;
			ev<<"Setting AAAAAA000001 as backbone mac address."<<endl;
			address.setAddress("AAAAAA000001");
		}
		autoEdge=par("autoEdge");
		MaxAge=par("MaxAge");
		verbose=(bool) par("verbose");
		testing=(bool) par("testing");
		up=true;

		//Reading events from file
		const cXMLElement * tree= par("UpTimeEvents").xmlValue();
		ASSERT(tree);
		ASSERT(!strcmp(tree->getTagName(), "Events"));
		cXMLElementList list=tree->getChildrenByTagName("Event");
		for (cXMLElementList::iterator iter=list.begin(); iter != list.end(); iter++)
		{
			cXMLElement * event = *iter;
			scheduleUpTimeEvent(event);
		}

		priority=par("BridgePriority");
		TCWhileTime=(simtime_t) par("TCWhileTime");
		hellotime=(simtime_t) par("helloTime");
		forwardDelay=(simtime_t)par("forwardDelay");
		migrateTime=(simtime_t) par ("migrateTime");  //Time the bridge waits for a better root info. After migrateTime time it swiches all ports which are not bk or alternate to designated.



		//RSTP module initialization. Puertos will save per port RSTP info.
		for(int i=0;i<(admac->gateSize("GatesOut"));i++)
		{
			Puertos.push_back(* new PortStatus());
		}

		//Detecting Edge ports and initializing
		initPorts();
		for(unsigned int i=0;i<Puertos.size();i++)
		{
			if(Puertos[i].portfilt!=NULL)
			{
				Puertos[i].Edge=false;
				Puertos[i].PortCost=Puertos[i].portfilt->getCost();
				Puertos[i].PortPriority=Puertos[i].portfilt->getPriority();
				if((Puertos[i].portfilt->isEdge()==true)&&(autoEdge==true))
				{
					Puertos[i].Edge=true;
					Puertos[i].PortRole=DESIGNATED;
					Puertos[i].PortState=FORWARDING;
				}
			}
			else
			{
				error("PortFilt not initialized");
			}

		}
		double waitTime=0.000001;  //Now
		//Programming next auto-messages.
		scheduleAt(simTime()+waitTime, helloM); //Next hello message generation.
		scheduleAt(simTime()+forwardDelay,forwardM);//Next port upgrade. Learning to forwarding and so on.
		scheduleAt(simTime()+migrateTime,migrateM); //Next switch to designate time. This will be a periodic message too.
	}
	if(verbose==true)
	{
		printState();
		colorRootPorts();
	}
}

void RSTP::finish()
{//Saving statistics
	if(testing==true)
	{
		FILE * pFile;
		char buff[50];
		sprintf(buff,"results/RSTP-%s.info",getParentModule()->getName());
		pFile = fopen (buff,"w");
		  if (pFile!=NULL)
		  {
			int r=getRootIndex();
			fprintf(pFile,"%s\n",getParentModule()->getName());
			fprintf(pFile,"Priority %d\n",priority);
			fprintf(pFile,"Local Address %s\n",address.str().c_str());
			fprintf(pFile,"Root Gate %d\n",r);
			if(r>=0)
			{
				fprintf(pFile,"*******ROOT BPDU INFO***********\n");
				fprintf(pFile,"Root Priority %d\n",Puertos[r].PortRstpVector.RootPriority);
				fprintf(pFile,"Root MAC %s\n",Puertos[r].PortRstpVector.RootMAC.str().c_str());
				fprintf(pFile,"Cost %d\n",Puertos[r].PortRstpVector.RootPathCost);
				fprintf(pFile,"Src Priority %d\n",Puertos[r].PortRstpVector.srcPriority);
				fprintf(pFile,"Src MAC %s\n",Puertos[r].PortRstpVector.srcAddress.str().c_str());
				fprintf(pFile,"Src Tx Gate Priority %d\n",Puertos[r].PortRstpVector.srcPortPriority);
				fprintf(pFile,"Src Tx Gate %d\n",Puertos[r].PortRstpVector.srcPort);
				fprintf(pFile,"********************************\n");
			}
			fprintf(pFile,"***********PORTSTATE************\n"); //Print  ROL
			for(unsigned int i=0;i<Puertos.size();i++)
			{
				if(Puertos[i].PortRole==ROOT)
					fprintf(pFile,"Root");
				else if (Puertos[i].PortRole==DESIGNATED)
					fprintf(pFile,"Designated");
				else if (Puertos[i].PortRole==BACKUP)
					fprintf(pFile,"Backup");
				else if (Puertos[i].PortRole==ALTERNATE_PORT)
					fprintf(pFile,"Alternate");
				else if (Puertos[i].PortRole==DISABLED)
					fprintf(pFile,"Disabled");
				else if (Puertos[i].PortRole==NOTASIGNED)
					fprintf(pFile,"Not asigned");
				fprintf(pFile," / ");						// Print STATE
				if(Puertos[i].PortState==DISCARDING)
				{
					fprintf(pFile,"Discarding");
				}
				else if (Puertos[i].PortState==LEARNING)
				{
					fprintf(pFile,"Learning");
				}
				else if (Puertos[i].PortState==FORWARDING)
				{
					fprintf(pFile,"Forwarding");
				}

				if(Puertos[i].Edge==true)					// Print (Client)
					fprintf(pFile," (Client)");
				fprintf(pFile,"\n");

				if((Puertos[i].PortRole!=DISABLED)&&(Puertos[i].PortRole!=NOTASIGNED)&&(Puertos[i].PortRole!=DESIGNATED))
				{// Best received BPDUs are random for time dependent DISABLED, NOTASIGNED and DESIGNATED
				// Not useful for validation purposes
					fprintf(pFile,"Best BPDU: Root %s  / Src %s\n\n",Puertos[i].PortRstpVector.RootMAC.str().c_str(),Puertos[i].PortRstpVector.srcAddress.str().c_str());
				}
			}
			fclose (pFile);
		  }
	}
}


void RSTP::handleMessage(cMessage *msg)
{//It can receive BPDU or self messages. Self messages are hello time, time to switch to designated, or status upgrade time.
	if(msg->isSelfMessage())
	{
		if(strcmp(msg->getName(),"itshellotime")==0)
		{
			handleHelloTime(msg);
		}
		else if(strcmp(msg->getName(),"upgrade")==0)
		{// Designated ports state upgrading. discarding-->learning. learning-->forwarding
			handleUpgrade(msg);
		}
		else if(strcmp(msg->getName(),"timetodesignate")==0)
		{// Not asigned ports switch to designated.
			handleMigrate(msg);
		}
		else if(strcmp(msg->getName(),"UpTimeEvent")==0)
		{
			handleUpTimeEvent(msg);  //Handling UP or DOWN event.
		}
		else
		{
			error("Unknown self message");
		}
	}
	else
	{
		ev<<"BPDU received at RSTP module."<<endl;
	handleIncomingFrame(check_and_cast<Delivery *> (msg));   //Handling BPDU (not self message)

	}
	if(verbose==true)
	{
		ev<<"Post message State"<<endl;
		printState();
	}
}

void RSTP::handleMigrate(cMessage * msg)
{
	for(unsigned int i=0;i<Puertos.size();i++)
	{
		if(Puertos[i].PortRole==NOTASIGNED)
		{
			Puertos[i].PortRole=DESIGNATED;
			Puertos[i].PortState=DISCARDING;  // Contest to become forwarding.
		}
	}
	scheduleAt(simTime()+migrateTime, msg);  //Programming next switch to designate
}


void RSTP::handleUpgrade(cMessage * msg)
{
	for(unsigned int i=0;i<Puertos.size();i++)
	{
		if(Puertos[i].PortRole==DESIGNATED)
		{
			switch(Puertos[i].PortState)
			{
				case DISCARDING:
					Puertos[i].PortState=LEARNING;
					break;
				case LEARNING:
					Puertos[i].PortState=FORWARDING;
					//Flushing other ports
					//TCN over all active ports
					for(unsigned int j=0;j<Puertos.size();j++)
					{
						Puertos[j].TCWhile=simulation.getSimTime()+TCWhileTime;
						if(j!=i)
						{
							Puertos[j].Flushed++;
							sw->flush(j);
						}
					}
					break;
				case FORWARDING:
					break;
			}
		}
	}
	scheduleAt(simTime()+forwardDelay, msg); //Programming next upgrade
}

void RSTP::handleHelloTime(cMessage * msg)
{
	if(verbose==true)
	{
		ev<<"Hello time."<<endl;
		printState();
	}
	for(unsigned int i=0;i<Puertos.size();i++)
	{ //Sends hello through all (active ports). ->learning, forwarding or not asigned ports.
		//Increments LostBPDU just from ROOT, ALTERNATE and BACKUP
		if((Puertos[i].Edge!=true)&&((Puertos[i].PortRole==ROOT)||(Puertos[i].PortRole==ALTERNATE_PORT)||(Puertos[i].PortRole==BACKUP)))
		{
			Puertos[i].LostBPDU++;
			if(Puertos[i].LostBPDU>3)   // 3 HelloTime without the best BPDU.
			{//Starts contest.
				if(Puertos[i].PortRole==ROOT)
				{ //Looking for the best ALTERNATE port.
					int candidato=getBestAlternate();
					if(candidato!=-1)
					{//If an alternate gate has been found, switch to alternate.
						ev<<"To Alternate"<<endl;
						//ALTERNATE->ROOT. Discarding->FW.(immediately)
						//Old root gate goes to Designated and discarding. A new contest should be done to determine the new root path from this LAN
						//Updating root vector.
						Puertos[i].PortRole=DESIGNATED;
						Puertos[i].PortState=DISCARDING; //If there is not a better BPDU, that will become Forwarding.
						Puertos[i].PortRstpVector.init(priority,address);  //Reset. Then, a new BPDU will be allowed to upgrade the best received info for this port.
						Puertos[candidato].PortRole=ROOT;
						Puertos[candidato].PortState=FORWARDING;
						Puertos[candidato].LostBPDU=0;
						//Flushing other ports
						//Sending TCN over all active ports
						for(unsigned int j=0;j<Puertos.size();j++)
						{
							Puertos[j].TCWhile=simulation.getSimTime()+TCWhileTime;
							if(j!=(unsigned int)candidato)
							{
								Puertos[j].Flushed++;
								sw->flush(j);
							}
						}
						sw->cpCache(i,candidato); //Copy cache from old to new root.
					}
					else
					{//Alternate not found. Selects a new Root.
						ev<<"Alternate not found. Starts from beginning."<<endl;
						//Initializing Puertos. Starts from the beginning.
						initPorts();
					}
				}
				else if((Puertos[i].PortRole==ALTERNATE_PORT)||(Puertos[i].PortRole==BACKUP))
				{//It should take care of this LAN, switching to designated.
					Puertos[i].PortRole=DESIGNATED;
					Puertos[i].PortState=DISCARDING; // A new content will start in case of another switch were in alternate.
					//If there is no problem, this will become forwarding in a few seconds.
					Puertos[i].PortRstpVector.init(priority,address); //Reset port rstp vector.
				}
				Puertos[i].LostBPDU=0; //Reseting lost bpdu counter after a change.
			}
		}
	}
	sendBPDUs(); //Generating and sending new BPDUs.
	sendTCNtoRoot();
	if(verbose==true)
	{
		colorRootPorts();
	}
	scheduleAt(simTime()+hellotime, msg); //Programming next hello time.
}
void RSTP::checkTC(BPDUieee8021D * frame, int arrival)
{
	if((frame->getTC()==true)&&(Puertos[arrival].PortState==FORWARDING))
	{
		Parent->bubble("TCN received");
		for(unsigned int i=0;i<Puertos.size();i++)
		{
			if((int)i!=arrival)
			{
				//Flushing other ports
				//TCN over other ports.
				Puertos[i].Flushed++;
				sw->flush(i);
				Puertos[i].TCWhile=simulation.getSimTime()+TCWhileTime;
			}
		}
	}
}

void RSTP::handleBK(BPDUieee8021D * frame, int arrival)
{
	if((frame->getPortPriority()<Puertos[arrival].PortPriority)
						||((frame->getPortPriority()==Puertos[arrival].PortPriority)&&(frame->getPortNumber()<arrival)))
				{
					//Flushing this port
					Puertos[arrival].Flushed++;
					sw->flush(arrival);
					Puertos[arrival].PortRole=BACKUP;
					Puertos[arrival].PortState=DISCARDING;
					Puertos[arrival].LostBPDU=0;
				}
				else if((frame->getPortPriority()>Puertos[arrival].PortPriority)
						||((frame->getPortPriority()==Puertos[arrival].PortPriority)&&(frame->getPortNumber()>arrival)))
				{
					//Flushing that port
					Puertos[frame->getPortNumber()].Flushed++;
					sw->flush(frame->getPortNumber());  //PortNumber is sender port number. It is not arrival port.
					Puertos[frame->getPortNumber()].PortRole=BACKUP;
					Puertos[frame->getPortNumber()].PortState=DISCARDING;
					Puertos[frame->getPortNumber()].LostBPDU=0;

				}
				else
				{//Unavoidable loop. Received its own message at the same port.Switch to disabled.
					ev<<"Unavoidable loop. Received its own message at the same port. To disabled."<<endl;
					//Flushing that port
					Puertos[frame->getPortNumber()].Flushed++;
					sw->flush(frame->getPortNumber());  //PortNumber is sender port number. It is not arrival port.
					Puertos[frame->getPortNumber()].PortRole=DISABLED;
					Puertos[frame->getPortNumber()].PortState=DISCARDING;
				}
}

void RSTP::handleIncomingFrame(Delivery *frame2)
{  //Incoming BPDU handling

	if(verbose==true)
	{
		printState();
	}

	//First. Checking message age
	int arrival=frame2->getArrivalPort(); //arrival port.
	BPDUieee8021D * frame=check_and_cast<BPDUieee8021D *>(frame2->decapsulate());
	delete frame2;
	if(frame->getAge()<MaxAge)
	{
		//Checking TC.
		checkTC(frame, arrival); //Sets TCWhile if arrival port was Forwarding

		int r=getRootIndex();

		//Checking possible backup
		if(frame->getSrc().compareTo(address)==0)  //more than one port in the same LAN.
		{
			handleBK(frame,arrival);
		}
		else
		{
			//Three challenges.
			//
			//First vs Best received BPDU for that port --------->caso
			//Second vs Root BPDU-------------------------------->caso1
			//Third vs BPDU that would be sent from this Bridge.->caso2

			int caso=0;
			bool Flood=false;
			caso=Puertos[arrival].PortRstpVector.compareRstpVector(frame,Puertos[arrival].PortCost);
			if((caso>0)&&(frame->getRootMAC().compareTo(address)!=0)) //Root will not participate in a loop with its own address
			{
				//Update that port rstp info.
				Puertos[arrival].updatePortVector(frame,arrival);
				if(r==-1)
				{//There was not root
					Puertos[arrival].PortRole=ROOT;
					Puertos[arrival].PortState=FORWARDING;
					Puertos[arrival].LostBPDU=0;
					//Flushing other ports
					//TCN over all ports.
					for(unsigned int j=0;j<Puertos.size();j++)
					{
						Puertos[j].TCWhile=simulation.getSimTime()+TCWhileTime;
						if(j!=(unsigned int)arrival)
						{
							Puertos[j].Flushed++;
							sw->flush(j);
						}
					}
					Flood=true;
				}
				else
				{//There was a Root. Challenge 2. Compare with the root.
					int caso2=Puertos[r].PortRstpVector.compareRstpVector(frame,Puertos[arrival].PortCost); //Comparing with root port vector
					int caso3=0;
					switch(caso2)
					{
						case 0: //Double link to the same port of the root source.Tie breaking. Better local port first.

							if((Puertos[r].PortPriority<Puertos[arrival].PortPriority)
									||((Puertos[r].PortPriority==Puertos[arrival].PortPriority)&&(r<arrival)))
							{
								//Flushing that port
								Puertos[arrival].Flushed++;
								sw->flush(arrival);
								Puertos[arrival].PortRole=ALTERNATE_PORT;
								Puertos[arrival].PortState=DISCARDING;
								Puertos[arrival].LostBPDU=0;
							}
							else
							{
								if(Puertos[arrival].PortState!=FORWARDING)
								{//Flushing other ports
								 //TCN over all ports.
									for(unsigned int j=0;j<Puertos.size();j++)
									{
										Puertos[j].TCWhile=simulation.getSimTime()+TCWhileTime;
										if(j!=(unsigned int)arrival)
										{
											Puertos[j].Flushed++;
											sw->flush(j);
										}
									}
								}
								else
								{	//Flushing r. Needed in case arrival were previously Fw
									Puertos[r].Flushed++;
									sw->flush(r);
								}
								Puertos[r].PortRole=ALTERNATE_PORT;
								Puertos[r].PortState=DISCARDING; //Comes from root. Preserve lostBPDU
								Puertos[arrival].PortRole=ROOT;
								Puertos[arrival].PortState=FORWARDING;
								Puertos[arrival].LostBPDU=0;
								sw->cpCache(r,arrival); //Copy cache from old to new root.

								//The change does not deserve flooding

							}
							break;

						case 1:   //New port rstp info is better than the root in another gate. Root change.

							for(unsigned int i=0;i<Puertos.size();i++)
							{
								if(Puertos[i].Edge==false)   //Avoiding clients reseting
								{
									if(Puertos[arrival].PortState!=FORWARDING)
									{
										Puertos[i].TCWhile=simulation.getSimTime()+TCWhileTime;
									}
									//Flush i
									Puertos[i].Flushed++;
									sw->flush(i);
									if(i!=(unsigned)arrival)
									{
										Puertos[i].PortRole=NOTASIGNED;
										Puertos[i].PortState=DISCARDING;
										Puertos[i].PortRstpVector.init(priority,address);
									}
								}
							}
							Puertos[arrival].PortRole=ROOT;
							Puertos[arrival].PortState=FORWARDING;
							Puertos[arrival].LostBPDU=0;

							Flood=true;
							break;

						case 2: //Same that Root but better RPC
						case 3: //Same that Root RPC but better source
						case 4: //Same that Root RPC Source but better port

							if(Puertos[arrival].PortState!=FORWARDING)
							{//Flushing other ports
								//TCN over all ports
								for(unsigned int j=0;j<Puertos.size();j++)
								{
									Puertos[j].TCWhile=simulation.getSimTime()+TCWhileTime;
									if(j!=(unsigned int)arrival)
									{
										Puertos[j].Flushed++;
										sw->flush(j);
									}
								}
							}
							Puertos[arrival].PortRole=ROOT;
							Puertos[arrival].PortState=FORWARDING;
							Puertos[r].PortRole=ALTERNATE_PORT; //Temporary. Just one port can be root at contest time.
							Puertos[arrival].LostBPDU=0;
							sw->cpCache(r,arrival); //Copy cache from old to new root.
							Flood=true;
							caso3=contestRstpVector(Puertos[r].PortRstpVector, Puertos[r].PortCost);
							if(caso3>=0)
							{
								Puertos[r].PortRole=ALTERNATE_PORT;
								//LostBPDU not reset.
								//Flushing r
								Puertos[r].Flushed++;
								sw->flush(r);
							}
							else
							{
								Puertos[r].PortRole=DESIGNATED;
							}
							Puertos[r].PortState=DISCARDING;

							break;

						case -1: //Worse Root
							sendBPDU(arrival);	//BPDU to show him a better Root as soon as possible
							break;
						case -2: //Same Root but worse RPC
						case -3: //Same Root RPC but worse source
						case -4: //Same Root RPC Source but worse port
							//Compares with frame that would be sent if it were the root for this LAN.
							caso3=contestRstpVector(frame,arrival); //Case 0 not possible. < if own vector better than frame.
							if(caso3<0)
							{
								Puertos[arrival].PortRole=DESIGNATED;
								Puertos[arrival].PortState=DISCARDING;
								sendBPDU(arrival); //BPDU to show him a better Root as soon as possible
							}
							else
							{
								//Flush arrival
								Puertos[arrival].Flushed++;
								sw->flush(arrival);
								Puertos[arrival].PortRole=ALTERNATE_PORT;
								Puertos[arrival].PortState=DISCARDING;
								Puertos[arrival].LostBPDU=0;
							}
							break;
					}
				}
			}
			else if((frame->getSrc().compareTo(Puertos[arrival].PortRstpVector.srcAddress)==0) //Worse or similar, but the same source
					&&(frame->getRootMAC().compareTo(address)!=0))   // Root will not participate
			{//Source has updated BPDU information.

				switch(caso)
				{
				case 0:
					Puertos[arrival].LostBPDU=0;   // Same BPDU. Not updated.
					break;
				case -1://Worse Root
					if(Puertos[arrival].PortRole==ROOT)
					{

						int alternative=getBestAlternate(); //Searching old alternate
						if(alternative>=0)
						{
							Puertos[arrival].PortRole=DESIGNATED;
							Puertos[arrival].PortState=DISCARDING;
							sw->cpCache(arrival,alternative);  //Copy cache from old to new root.
							//Flushing other ports
							//TCN over all ports. Alternative was alternate.
							for(unsigned int j=0;j<Puertos.size();j++)
							{
								Puertos[j].TCWhile=simulation.getSimTime()+TCWhileTime;
								if(j!=(unsigned int)alternative)
								{
									Puertos[j].Flushed++;
									sw->flush(j);
								}
							}
							Puertos[alternative].PortRole=ROOT;
							Puertos[alternative].PortState=FORWARDING; //Comes from alternate. Preserves lostBPDU.
							Puertos[arrival].updatePortVector(frame,arrival);
							sendBPDU(arrival); //Show him a better Root as soon as possible
						}
						else
						{	//If there is not alternate, is it better than own initial vector?
							int caso2=0;
							initPorts(); //Allowing other ports to contest again.
							//Flushing all ports.
							for(unsigned int j=0;j<Puertos.size();j++)
							{
								Puertos[j].Flushed++;
								sw->flush(j);
							}
							caso2=Puertos[arrival].PortRstpVector.compareRstpVector(frame,Puertos[arrival].PortCost);
							if(caso2>0)
							{
								Puertos[arrival].updatePortVector(frame,arrival); //If this module is not better. Keep it as a ROOT.
								Puertos[arrival].PortRole=ROOT;
								Puertos[arrival].PortState=FORWARDING;
							}
							//Propagating new information.
							Flood=true;
						}
					}
					else if(Puertos[arrival].PortRole==ALTERNATE_PORT)
					{
						Puertos[arrival].PortState=DISCARDING;
						Puertos[arrival].PortRole=DESIGNATED;
						Puertos[arrival].updatePortVector(frame,arrival);
						sendBPDU(arrival); //Show him a better Root as soon as possible
					}
					break;
				case -2:
				case -3:
				case -4: //Same Root.Worse source
					if(Puertos[arrival].PortRole==ROOT)
					{
						Puertos[arrival].LostBPDU=0;
						int alternative=getBestAlternate(); //Searching old alternate
						if(alternative>=0)
						{
							int caso2=0;
							caso2=Puertos[alternative].PortRstpVector.compareRstpVector(frame,Puertos[arrival].PortCost);
							if(caso2<0)//If alternate is better, change
							{
								Puertos[alternative].PortRole=ROOT;
								Puertos[alternative].PortState=FORWARDING;
								Puertos[arrival].PortRole=DESIGNATED;//Temporary. Just one port can be root at contest time.
								int caso3=0;
								caso3=contestRstpVector(frame,arrival);
								if(caso3<0)
								{
									Puertos[arrival].PortRole=DESIGNATED;
								}
								else
								{
									Puertos[arrival].PortRole=ALTERNATE_PORT;
								}
								Puertos[arrival].PortState=DISCARDING;
								//Flushing other ports
								//TC over all ports.
								for(unsigned int j=0;j<Puertos.size();j++)
								{
									Puertos[j].TCWhile=simulation.getSimTime()+TCWhileTime;
									if(j!=(unsigned int)alternative)
									{
										Puertos[j].Flushed++;
										sw->flush(j);
									}
								}
								sw->cpCache(arrival,alternative); //Copy cache from old to new root.
							}
						}
						Puertos[arrival].updatePortVector(frame,arrival);
						//Propagating new information.
						Flood=true;
						//If alternate is worse than root, or there is not alternate, keep old root as root.
					}
					else if(Puertos[arrival].PortRole==ALTERNATE_PORT)
					{
						int caso2=0;
						caso2=contestRstpVector(frame,arrival);
						if(caso2<0)
						{
							Puertos[arrival].PortRole=DESIGNATED; //If the frame is worse than this module generated frame. Switch to Designated/Discarding
							Puertos[arrival].PortState=DISCARDING;
							sendBPDU(arrival); //Show him a better BPDU as soon as possible
						}
						else
						{
							Puertos[arrival].LostBPDU=0; //If it is better than this module generated frame, keep it as alternate
							//This does not deserve expedited BPDU
						}
					}
					Puertos[arrival].updatePortVector(frame,arrival);
					break;
				}
			}
			if(Flood==true)
			{
				sendBPDUs(); //Expedited BPDU
				sendTCNtoRoot();
			}
		}
	}
	else
	{
		ev<<"Expired BPDU"<<endl;
	}
	delete frame;
	if(verbose==true)
	{
		colorRootPorts();
	}
}

void RSTP::sendTCNtoRoot()
{//If TCWhile is not expired, sends BPDU with TC flag to the root.
	this->bubble("SendTCNtoRoot");
	unsigned int r=getRootIndex();
	if((r>=0)&&(r<Puertos.size()))
	{
		if(Puertos[r].PortRole!=DISABLED)
		{
			if(simulation.getSimTime()<Puertos[r].TCWhile)
			{
				BPDUieee8021D * frame = new BPDUieee8021D();
				Delivery * frame2= new Delivery();
				RSTPVector a = getRootRstpVector();

				frame->setRootPriority(a.RootPriority);
				frame->setRootMAC(a.RootMAC);
				frame->setAge(a.Age);
				frame->setCost(a.RootPathCost);
				frame->setSrcPriority(priority);
				frame->setSrc(address);
				frame->setDest(MACAddress("01-80-C2-00-00-00"));
				frame->setAck(false);
				frame->setPortNumber(r);  //Src port number.
				frame->setTC(true);
				frame->setDisplayString("b=,,,#3e3ef3");
		        if (frame->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
		            frame->setByteLength(MIN_ETHERNET_FRAME_BYTES);
				frame2->setSendByPort(r);
				frame2->encapsulate(frame);
				send(frame2,"RSTPPort$o");
			}
		}
	}
}



void RSTP::sendBPDUs()
{//Send BPDUs through all ports if they are required.

	for(unsigned int i=0;i<Puertos.size();i++)
	{
		if((Puertos[i].PortRole!=ROOT)&&(Puertos[i].PortRole!=ALTERNATE_PORT)&&(Puertos[i].PortRole!=DISABLED)&&(Puertos[i].Edge!=true))
		{
			sendBPDU(i);
		}
	}
}
void RSTP::sendBPDU(int port)
{//Send a BPDU throuth port.
	if(Puertos[port].PortRole!=DISABLED)
	{
		BPDUieee8021D * frame = new BPDUieee8021D();
		Delivery * frame2=new Delivery();
		RSTPVector a = getRootRstpVector();
		frame->setRootPriority(a.RootPriority);
		frame->setRootMAC(a.RootMAC);
		frame->setCost(a.RootPathCost);
		frame->setAge(a.Age);
		frame->setSrcPriority(priority);
		frame->setSrc(address);
		frame->setDest(MACAddress("01-80-C2-00-00-00"));
		frame->setAck(false);
		frame->setPortNumber(port);  //Src port number.
		if(simulation.getSimTime()<Puertos[port].TCWhile)
		{
			frame->setTC(true);
			frame->setDisplayString("b=,,,#3e3ef3");
		}
		else
			frame->setTC(false);
        if (frame->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
            frame->setByteLength(MIN_ETHERNET_FRAME_BYTES);
		frame2->setSendByPort(port);
		frame2->encapsulate(frame);
		send(frame2,"RSTPPort$o");
	}

}

void RSTP::colorRootPorts()
{//Gives color to the root link, or module border if it is root.

	bool found=false;
	for(unsigned int l=0;l<Puertos.size();l++)
	{//Marking port state
		if(Puertos[l].Edge!=true)
		{
			cGate * Gate1=NULL;
			cGate * Gate2=NULL;
			cGate * Gate3=NULL;
			cGate * Gate4=NULL;
			if(Parent->hasGate("ethgB$o",l))
			{
				Gate1=Parent->gate("ethgB$o",l); //That will work just for 1AH.
				Gate2=Parent->gate("ethgB$i",l);
				Gate3=Gate1->getNextGate();
				Gate4=Gate2->getPreviousGate();
			}
			else if(Parent->hasGate("ethg$o",l))
			{
				Gate1=Parent->gate("ethg$o",l); //That will work just for 1Q.
				Gate2=Parent->gate("ethg$i",l);
				Gate3=Gate1->getNextGate();
				Gate4=Gate2->getPreviousGate();
			}
			if(Puertos[l].PortRole==ROOT)
			{ //Marking root port.
				if((Gate1!=NULL)&&(Gate2!=NULL)&&(Gate3!=NULL)&&(Gate4!=NULL))
				{
					Gate1->getDisplayString().setTagArg("ls",0,"#a5ffff");  //Sets link color. Green.
					Gate1->getDisplayString().setTagArg("ls",1,3); //Making line wider
					Gate2->getDisplayString().setTagArg("ls",0,"#a5ffff");  //Sets link color. Green.
					Gate2->getDisplayString().setTagArg("ls",1,3); //Making line wider
					Gate3->getDisplayString().setTagArg("ls",0,"#a5ffff");  //Sets link color. Green.
					Gate3->getDisplayString().setTagArg("ls",1,3); //Making line wider
					Gate4->getDisplayString().setTagArg("ls",0,"#a5ffff");  //Sets link color. Green.
					Gate4->getDisplayString().setTagArg("ls",1,3); //Making line wider
				}
				char buf[50];
				std::string * mac=new std::string();
				* mac=address.str();
				sprintf(buf,"MAC: %s",mac->c_str());
				Parent->getDisplayString().setTagArg("tt",0,buf);
				std::string * rootM=new std::string();
				* rootM=Puertos[l].PortRstpVector.RootMAC.str();
				sprintf(buf,"Root: %s",rootM->c_str());
				getDisplayString().setTagArg("t",0,buf);
				found=true;
			}
			else if((Puertos[l].PortRole!=ROOT)&&(Puertos[l].PortRole!=DESIGNATED)&&(Puertos[l].PortRole!=NOTASIGNED))
			{ //Remove possible root port mark
				if((Gate1!=NULL)&&(Gate2!=NULL)&&(Gate3!=NULL)&&(Gate4!=NULL))
				{
					Gate1->getDisplayString().setTagArg("ls",0,"black");
					Gate1->getDisplayString().setTagArg("ls",1,1);
					Gate2->getDisplayString().setTagArg("ls",0,"black");
					Gate2->getDisplayString().setTagArg("ls",1,1);
					Gate3->getDisplayString().setTagArg("ls",0,"black");
					Gate3->getDisplayString().setTagArg("ls",1,1);
					Gate4->getDisplayString().setTagArg("ls",0,"black");
					Gate4->getDisplayString().setTagArg("ls",1,1);
				}
				std::string * b=new std::string();
				char buf[50];
				* b=address.str();
				sprintf(buf,"MAC: %s",b->c_str());
				Parent->getDisplayString().setTagArg("tt",0,buf);
			}
			cModule * puerta=Parent->getSubmodule("macB",l);
			if(puerta==NULL)
				puerta=Parent->getSubmodule("mac",l);
			if(puerta!=NULL)  //Marking port state
			{
				char buf[10];
				int estado=Puertos[l].PortState;
				if(estado==0)
				{
					sprintf(buf,"DISCARDING\n");
				}
				else if(estado==1)
				{
					sprintf(buf,"LEARNING\n");
				}
				else if(estado==2)
				{
					sprintf(buf,"FORWARDING\n");
				}
				puerta->getDisplayString().setTagArg("t",0,buf);
			}
		}
	}
	if(up==true)
	{
		if(found==false)
		{ //Root mark
			Parent->getDisplayString().setTagArg("i2",0,"status/check");
			Parent->getDisplayString().setTagArg("i",1,"#a5ffff");
			char buf[50];
			sprintf(buf,"Root: %s",address.str().c_str());
			getDisplayString().setTagArg("t",0,buf);

		}
		else
		{ //Remove possible root mark
			Parent->getDisplayString().removeTag("i2");
			Parent->getDisplayString().setTagArg("i",1,"");
		}
	}
}



void RSTP::printState()
{// Prints current database info
	if(Parent!=NULL)
		ev<<endl<<Parent->getName()<<endl;
	int r=getRootIndex();
	ev<<"RSTP state"<<endl;
	ev<<"Priority: "<<priority<<endl;
	ev<<"Local MAC: "<<address<<endl;
	if(r>=0)
	{
		ev<<"Root Priority: "<<Puertos[r].PortRstpVector.RootPriority<<endl;
		ev<<"Root MAC: "<<Puertos[r].PortRstpVector.RootMAC.str()<<endl;
		ev<<"cost: "<<Puertos[r].PortRstpVector.RootPathCost<<endl;
		ev<<"age:  "<<Puertos[r].PortRstpVector.Age<<endl;
		ev<<"Src priority: "<<Puertos[r].PortRstpVector.srcPriority<<endl;
		ev<<"Src address: "<<Puertos[r].PortRstpVector.srcAddress.str()<<endl;
		ev<<"Src TxGate Priority: "<<Puertos[r].PortRstpVector.srcPortPriority<<endl;
		ev<<"Src TxGate: "<<Puertos[r].PortRstpVector.srcPort<<endl;
	}
	ev<<"Port State/Role: "<<endl;
	for(unsigned int i=0;i<Puertos.size();i++)
		{
			ev<<Puertos[i].PortState<<" ";
			if(Puertos[i].PortState==DISCARDING)
			{
				ev<<"Discarding";
			}
			else if (Puertos[i].PortState==LEARNING)
			{
				ev<<"Learning";
			}
			else if (Puertos[i].PortState==FORWARDING)
			{
				ev<<"Forwarding";
			}
			ev<<"  ";
			if(Puertos[i].PortRole==ROOT)
				ev<<"Root";
			else if (Puertos[i].PortRole==DESIGNATED)
				ev<<"Designated";
			else if (Puertos[i].PortRole==BACKUP)
				ev<<"Backup";
			else if (Puertos[i].PortRole==ALTERNATE_PORT)
				ev<<"Alternate";
			else if (Puertos[i].PortRole==DISABLED)
				ev<<"Disabled";
			else if (Puertos[i].PortRole==NOTASIGNED)
				ev<<"Not assigned";
			if(Puertos[i].Edge==true)
				ev<<" (Client)";
			ev<<endl;
		}
	ev<<"Per port best source. Root/Src"<<endl;
	for(unsigned int i=0;i<Puertos.size();i++)
	{
		ev<<Puertos[i].PortRstpVector.RootMAC.str()<<"/"<<Puertos[i].PortRstpVector.srcAddress.str()<<endl;
	}
}
void RSTP::initPorts()
{ //Initialize RSTP dynamic information.
	for(unsigned int j=0;j<Puertos.size();j++)
	{
		Puertos[j].LostBPDU=0;
		if(!Puertos[j].Edge)
		{
			Puertos[j].PortRole=NOTASIGNED;
			Puertos[j].PortState=DISCARDING;
		}
		Puertos[j].PortRstpVector.init(priority,address);
		Puertos[j].Flushed++;
		sw->flush(j);

		if(dynamic_cast<Admacrelay *>(admac)!=NULL)
		{
			cGate * gate=admac->gate("GatesOut",j);
			if(gate!=NULL)
			{
				gate=gate->getNextGate();
				if(gate!=NULL)
						Puertos[j].portfilt=check_and_cast<PortFilt *>(gate->getOwner());
			}
		}
		else if(dynamic_cast<Relay1Q *>(admac)!=NULL)
		{
			cGate * gate=admac->gate("GatesOut",j);
			if(gate!=NULL)
			{
				gate=gate->getNextGate();
				if(gate!=NULL)
					Puertos[j].portfilt=check_and_cast<PortFilt *>(gate->getOwner());
			}

		}

	}
}

PortRoleT RSTP::getPortRole(int index)
{
	Enter_Method("Get rol");
	return Puertos[index].PortRole;
}
PortStateT RSTP::getPortState(int index)
{
	Enter_Method("Get State");
	return Puertos[index].PortState;
}

void RSTPVector::init(int priority, MACAddress address)
{//Initializing PortRstpVector.
	this->RootPriority=priority;
	this->RootMAC = address;
	this->RootPathCost=0;
	this->Age=0;
	this->srcPriority=priority;
	this->srcAddress = address;
	this->srcPortPriority=-1;
	this->srcPort=-1;
	this->arrivalPort=-1;
}

RSTPVector RSTP::getRootRstpVector()
{//Gets root gate rstp vector. (Cost not incremented)
	int RootIndex=getRootIndex();
	if(RootIndex>=0)
	{ //Selects the best known RstpVector.
		return Puertos[RootIndex].PortRstpVector;
	}
	else
	{
	    RSTPVector RootVect;
		RootVect.init(priority,address);
	    return RootVect;
	}
}


int RSTP::contestRstpVector(RSTPVector vect2,int v2Port)
{//Compares vect2 with the vector that would be sent if this were the root node for the arrival LAN.
	//>0 if vect2 better than own vector. =0 if they are similar.
	// -1=Worse   0= Similar  1=Better Root. 2= Better RPC  3= Better Src   4= Better Port
	RSTPVector vect;
	int r=getRootIndex();
	vect.RootPriority=Puertos[r].PortRstpVector.RootPriority;
	vect.RootMAC=Puertos[r].PortRstpVector.RootMAC;
	vect.RootPathCost=Puertos[r].PortRstpVector.RootPathCost+Puertos[v2Port].PortCost; //Compensating incoming added cost
	vect.srcPriority=priority;
	vect.srcAddress = address;
	vect.srcPortPriority=Puertos[vect2.arrivalPort].PortPriority;
	vect.srcPort=vect2.arrivalPort;
	int result=0;
	result=vect.compareRstpVector(vect2);
	return result;
}

int RSTP::contestRstpVector(BPDUieee8021D* msg,int arrival)
{//Compares msg with the vector that would be sent if this were the root node for the arrival LAN.
	// >0 if frame vector better than own vector. =0 if they are similar.
	// -1=Worse   0= Similar  1=Better Root. 2= Better RPC  3= Better Src   4= Better Port
	RSTPVector vect;
	int r=getRootIndex();
	vect.RootPriority=Puertos[r].PortRstpVector.RootPriority;
	vect.RootMAC=Puertos[r].PortRstpVector.RootMAC;
	vect.RootPathCost=Puertos[r].PortRstpVector.RootPathCost;
	vect.srcPriority=priority;
	vect.srcAddress = address;
	vect.srcPortPriority=Puertos[arrival].PortPriority;
	vect.srcPort=msg->getPortNumber();
	vect.arrivalPort=arrival;
	int result=0;
	result=vect.compareRstpVector(msg,0);
	return result;
}





int RSTPVector::compareRstpVector(BPDUieee8021D * msg,int PortCost)
{//Compares msg with vect. >0 if msg better than vect. =0 if they are similar.
	// -4=Worse Port  -3=Worse Src -2=Worse RPC -1=Worse Root  0= Similar  1=Better Root. 2= Better RPC  3= Better Src   4= Better Port
	//Adds Port cost to msg.
	int result=0;
	RSTPVector vect2;
	vect2.RootPriority=msg->getRootPriority();
	vect2.RootMAC = msg->getRootMAC();
	vect2.RootPathCost=msg->getCost()+PortCost;
	vect2.srcPriority=msg->getSrcPriority();
	vect2.srcAddress = msg->getSrc();
	vect2.srcPortPriority=msg->getPortPriority();
	vect2.srcPort=msg->getPortNumber();
	result=this->compareRstpVector(vect2);
	return result;
}

int RSTPVector::compareRstpVector(RSTPVector vect2)
{//Compares msg with vect. >0 if msg better than vect. =0 if they are similar.
	// -4=Worse Port  -3=Worse Src -2=Worse RPC -1=Worse Root  0= Similar  1=Better Root. 2= Better RPC  3= Better Src   4= Better Port
	int result=0;
	if((vect2.RootPriority<this->RootPriority)
			||((vect2.RootPriority==this->RootPriority)
					&&(vect2.RootMAC.compareTo(this->RootMAC)<0)))
	{
		result=1;   //Better Root
	}
	else if ((vect2.RootPriority==this->RootPriority)
			&&(vect2.RootMAC.compareTo(this->RootMAC)==0)
			&&(vect2.RootPathCost<this->RootPathCost))
	{
		result=2;  //Better RPC
	}
	else if ((vect2.RootPriority==this->RootPriority)
			&&(vect2.RootMAC.compareTo(this->RootMAC)==0)
			&&(vect2.RootPathCost==this->RootPathCost)
			&&((vect2.srcPriority<this->srcPriority)
					||((vect2.srcPriority==this->srcPriority)
							&&(vect2.srcAddress.compareTo(this->srcAddress)<0))))
	{
		result=3;	//Better Src
	}
	else if ((vect2.RootPriority==this->RootPriority)
			&&(vect2.RootMAC.compareTo(this->RootMAC)==0)
			&&(vect2.RootPathCost==this->RootPathCost)
			&&(vect2.srcPriority==this->srcPriority)
			&&(vect2.srcAddress.compareTo(this->srcAddress)==0)
			&&((vect2.srcPortPriority<this->srcPortPriority)
					||((vect2.srcPortPriority==this->srcPortPriority)
							&&(vect2.srcPort<this->srcPort))))
	{
		result=4;  //Better Src port
	}
	else if ((vect2.RootPriority==this->RootPriority)
			&&(vect2.RootMAC.compareTo(this->RootMAC)==0)
			&&(vect2.RootPathCost==this->RootPathCost)
			&&(vect2.srcPriority==this->srcPriority)
			&&(vect2.srcAddress.compareTo(this->srcAddress)==0)
			&&(vect2.srcPortPriority==this->srcPortPriority)
			&&(vect2.srcPort==this->srcPort))
	{
		result=0;  // Same BPDU
	}
	else if((vect2.RootPriority>this->RootPriority)
			||((vect2.RootPriority == this->RootPriority)
					&&(vect2.RootMAC.compareTo(this->RootMAC)>0)))
	{
		result=-1;  // Worse Root
	}
	else if ((vect2.RootPriority==this->RootPriority)
				&&(vect2.RootMAC.compareTo(this->RootMAC)==0)
				&&(vect2.RootPathCost>this->RootPathCost)>0)
		{
			result=-2;  //Worse RPC
		}
	else if ((vect2.RootPriority==this->RootPriority)
			&&(vect2.RootMAC.compareTo(this->RootMAC)==0)
			&&(vect2.RootPathCost==this->RootPathCost)
			&&((vect2.srcPriority>this->srcPriority)
					||((vect2.srcPriority==this->srcPriority)
							&&(vect2.srcAddress.compareTo(this->srcAddress)>0))))
		{
			result=-3;	//Worse Src
		}
		else if ((vect2.RootPriority==this->RootPriority)
				&&(vect2.RootMAC.compareTo(this->RootMAC)==0)
				&&(vect2.RootPathCost==this->RootPathCost)
				&&(vect2.srcPriority==this->srcPriority)
				&&(vect2.srcAddress.compareTo(this->srcAddress)==0)
				&&((vect2.srcPortPriority>this->srcPortPriority)
						||((vect2.srcPortPriority==this->srcPortPriority)
								&&(vect2.srcPort>this->srcPort))))
		{
			result=-4;  //Worse Src port
		}
	return result;
}

int RSTP::getRootIndex()
{//Returns the Root Gate index or -1 if there is not Root Gate.
	int result=-1;
	for(unsigned int i=0;i<Puertos.size();i++)
	{
		if(Puertos[i].PortRole==ROOT)
		{
			result=i;
			break;
		}
	}
	return result;
}

int RSTP::getBestAlternate()
{// Gets best alternate port index.
	int candidato=-1;  //Index of the best alternate found.
	for(unsigned int j=0;j<Puertos.size();j++)
	{
		if(Puertos[j].PortRole==ALTERNATE_PORT) //Just from Alternates. Others are not updated.
		{
			if(((candidato<0)||(Puertos[j].PortRstpVector.RootPathCost<Puertos[candidato].PortRstpVector.RootPathCost)
							||((Puertos[j].PortRstpVector.RootPathCost==Puertos[candidato].PortRstpVector.RootPathCost)&&(Puertos[j].PortRstpVector.srcPriority<Puertos[candidato].PortRstpVector.srcPriority))
							||((Puertos[j].PortRstpVector.RootPathCost==Puertos[candidato].PortRstpVector.RootPathCost)&&(Puertos[j].PortRstpVector.srcPriority==Puertos[candidato].PortRstpVector.srcPriority)&&(Puertos[j].PortRstpVector.srcAddress.compareTo(Puertos[candidato].PortRstpVector.srcAddress)<0))
							||((Puertos[j].PortRstpVector.RootPathCost==Puertos[candidato].PortRstpVector.RootPathCost)&&(Puertos[j].PortRstpVector.srcPriority==Puertos[candidato].PortRstpVector.srcPriority)&&(Puertos[j].PortRstpVector.srcAddress.compareTo(Puertos[candidato].PortRstpVector.srcAddress)==0)&&(Puertos[j].PortRstpVector.srcPortPriority<Puertos[candidato].PortRstpVector.srcPortPriority))
							||((Puertos[j].PortRstpVector.RootPathCost==Puertos[candidato].PortRstpVector.RootPathCost)&&(Puertos[j].PortRstpVector.srcPriority==Puertos[candidato].PortRstpVector.srcPriority)&&(Puertos[j].PortRstpVector.srcAddress.compareTo(Puertos[candidato].PortRstpVector.srcAddress)==0)&&(Puertos[j].PortRstpVector.srcPortPriority==Puertos[candidato].PortRstpVector.srcPortPriority)&&(Puertos[j].PortRstpVector.srcPort<Puertos[candidato].PortRstpVector.srcPort))))
			{//It is the first alternate or better than the found one
				candidato=j; //New candidate
			}
		}

	}
	return candidato;
}


bool RSTP::isEdge(int index)
{
	return Puertos[index].Edge;
}

PortStatus::PortStatus()
{//PortStatus constructor
	Edge=false;		 // It indicates that this is a client port.
	Flushed=0;

	PortRole=NOTASIGNED;
	PortState=DISCARDING;
	LostBPDU=0;			//Lost BPDU counter. Used to detect topology changes.
	PortCost=0;			//Cost for that port.
	PortPriority=0;		//Priority for that port.
	TCWhile=0;		//This port will send TC=true until this time has been overtaken.

	portfilt=NULL;

}

void PortStatus::updatePortVector(BPDUieee8021D *frame,int arrival)
{//Updates this port status with frame information, received at arrival.
	this->PortRstpVector.RootPriority=frame->getRootPriority();
	this->PortRstpVector.RootMAC = frame->getRootMAC();
	this->PortRstpVector.RootPathCost=frame->getCost()+this->PortCost;
	this->PortRstpVector.Age=frame->getAge()+1;
	this->PortRstpVector.srcPriority=frame->getSrcPriority();
	this->PortRstpVector.srcAddress = frame->getSrc();
	this->PortRstpVector.srcPortPriority=frame->getPortPriority();
	this->PortRstpVector.srcPort=frame->getPortNumber();
	this->PortRstpVector.arrivalPort=arrival;
	this->LostBPDU=0;
}




void RSTP::scheduleUpTimeEvent(cXMLElement * event)
{//Schedule "UPTimeEvents". Falling nodes or links.
	simtime_t temp=(simtime_t) getParameterDoubleValue(event,"Time");
	int EventType=getParameterBoolValue(event,"Type");
	cMessage * msg=new cMessage("UpTimeEvent",EventType);
	//cXMLElementList list;
	scheduleAt(temp,msg);
}

void RSTP::handleUpTimeEvent(cMessage * msg)
{//Handles scheduled "UPTimeEvents"

	if(msg->getKind()==UP)
	{
		initPorts();
		up=true;
	}
	else if(msg->getKind()==DOWN)
	{
		for(unsigned int i=0;i<Puertos.size();i++)
		{
			Puertos[i].PortRole=DISABLED;
			Puertos[i].PortState=DISCARDING;
		}
		Parent->getDisplayString().setTagArg("i2",0,"status/stop");
		Parent->getDisplayString().setTagArg("i",1,"#ff6565");
		up=false;
	}
	delete msg;
}

MACAddress RSTP::getAddress()
{
	return address;
}

