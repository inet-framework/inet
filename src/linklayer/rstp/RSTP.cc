//
// Copyright (C) 2011 Juan Luis Garrote Molinero
// Copyright (C) 2013 Zsolt Prontvai
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "RSTP.h"

#include "EtherFrame.h"
#include "EtherMAC.h"
#include "Ethernet.h"
#include "MACAddressTableAccess.h"
#include "InterfaceTableAccess.h"
#include "InterfaceEntry.h"
#include "ModuleAccess.h"
#include "NodeOperations.h"
#include "NodeStatus.h"



Define_Module (RSTP);

RSTP::RSTP()
{
    Parent = NULL;
    sw = NULL;
	helloM = new cMessage("itshellotime", SELF_HELLOTIME);
	forwardM = new cMessage("upgrade", SELF_UPGRADE);
	migrateM = new cMessage("timetodesignate", SELF_TIMETODESIGNATE);
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
    if (stage==1)
    {
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
    }
	if(stage==1) // "auto" MAC addresses assignment takes place in stage 0.
	{
		sw=MACAddressTableAccess().get(); //cache pointer
		//Gets the relay pointer.
		ifTable=InterfaceTableAccess().get();

		portCount = this->getParentModule()->gate("ethg$o", 0)->getVectorSize();

		//Gets the backbone mac address
		InterfaceEntry * ifEntry=ifTable->getInterface(0);
		if(ifEntry!=NULL)
		{
			address = ifEntry->getMacAddress();
		}
		else
		{
			ev<<"interface not found. Is not this module connected to another BEB?"<<endl;
			ev<<"Setting AAAAAA000001 as backbone mac address."<<endl;
			address.setAddress("AAAAAA000001");
		}

		autoEdge=par("autoEdge");
		maxAge=par("maxAge");
		verbose=(bool) par("verbose");
		priority=par("priority");
		tcWhileTime=(simtime_t) par("tcWhileTime");
		hellotime=(simtime_t) par("helloTime");
		fwdDelay=(simtime_t)par("fwdDelay");
		migrateTime=(simtime_t) par ("migrateTime");  //Time the bridge waits for a better root info. After migrateTime time it swiches all ports which are not bk or alternate to designated.

		initPorts();

		double waitTime=0.000001;  //Now
		//Programming next auto-messages.
		scheduleAt(simTime()+waitTime, helloM); //Next hello message generation.
		scheduleAt(simTime()+fwdDelay,forwardM);//Next port upgrade. Learning to forwarding and so on.
		scheduleAt(simTime()+migrateTime,migrateM); //Next switch to designate time. This will be a periodic message too.
	}
	if(verbose && stage == 1)
	{
		printState();
		colorRootPorts();
	}
}

void RSTP::finish(){}

void RSTP::handleMessage(cMessage *msg)
{//It can receive BPDU or self messages. Self messages are hello time, time to switch to designated, or status upgrade time.
    if (!isOperational)
    {
        EV << "Message '" << msg << "' arrived when module status is down, dropped it\n";
        delete msg;
        return;
    }

	if(msg->isSelfMessage())
	{
        switch (msg->getKind())
        {
            case SELF_HELLOTIME:
                handleHelloTime(msg);
                break;

            case SELF_UPGRADE:
                // Designated ports state upgrading. discarding-->learning. learning-->forwarding
                handleUpgrade(msg);
                break;

            case SELF_TIMETODESIGNATE:
                // Not asigned ports switch to designated.
                handleMigrate(msg);
                break;

            default:
                error("Unknown self message");
                break;
		}
	}
	else
	{
		ev<<"BPDU received at RSTP module."<<endl;
	handleIncomingFrame(check_and_cast<BPDU *> (msg));   //Handling BPDU (not self message)

	}
	if(verbose==true)
	{
		ev<<"Post message State"<<endl;
		printState();
	}
}

void RSTP::handleMigrate(cMessage * msg)
{
	for(unsigned int i=0;i<portCount;i++)
	{
	    IEEE8021DInterfaceData * port = getPortInterfaceData(i);
		if(port->getRole() == IEEE8021DInterfaceData::NOTASSIGNED)
		{
		    port->setRole(IEEE8021DInterfaceData::DESIGNATED);
		    port->setState(IEEE8021DInterfaceData::DISCARDING);  // Contest to become forwarding.
		}
	}
	scheduleAt(simTime()+migrateTime, msg);  //Programming next switch to designate
}

void RSTP::handleUpgrade(cMessage * msg)
{
	for(unsigned int i=0;i<portCount;i++)
	{
	    IEEE8021DInterfaceData * port = getPortInterfaceData(i);
		if(port->getRole() == IEEE8021DInterfaceData::DESIGNATED)
		{
		    if(port->getState() == IEEE8021DInterfaceData::DISCARDING)
		        port->setState(IEEE8021DInterfaceData::LEARNING);
		    if(port->getState() == IEEE8021DInterfaceData::LEARNING)
		    {
		        port->setState(IEEE8021DInterfaceData::FORWARDING);
                //Flushing other ports
                //TCN over all active ports
                for(unsigned int j=0;j<portCount;j++)
                {
                    IEEE8021DInterfaceData * port2 = getPortInterfaceData(j);
                    port2->setTCWhile(simulation.getSimTime()+tcWhileTime);
                    if(j!=i)
                    {
                        sw->flush(j);
                    }
                }
			}
		}
	}
	scheduleAt(simTime()+fwdDelay, msg); //Programming next upgrade
}

void RSTP::handleHelloTime(cMessage * msg)
{
	if(verbose==true)
	{
		ev<<"Hello time."<<endl;
		printState();
	}
	for(unsigned int i=0;i<portCount;i++)
	{
	    //Sends hello through all (active ports). ->learning, forwarding or not asigned ports.
		//Increments LostBPDU just from ROOT, ALTERNATE and BACKUP
	    IEEE8021DInterfaceData * port = getPortInterfaceData(i);
		if(!port->isEdge()
		        && (port->getRole() == IEEE8021DInterfaceData::ROOT
		                || port->getRole() == IEEE8021DInterfaceData::ALTERNATE
		                || port->getRole() == IEEE8021DInterfaceData::BACKUP))
		{
		    port->setLostBPDU(port->getLostBPDU()+1);
			if(port->getLostBPDU()>3)   // 3 HelloTime without the best BPDU.
			{
			    //Starts contest.
				if(port->getRole() == IEEE8021DInterfaceData::ROOT)
				{
				    //Looking for the best ALTERNATE port.
					int candidato=getBestAlternate();
					if(candidato!=-1)
					{
					    //If an alternate gate has been found, switch to alternate.
						ev<<"To Alternate"<<endl;
						//ALTERNATE->ROOT. Discarding->FW.(immediately)
						//Old root gate goes to Designated and discarding. A new contest should be done to determine the new root path from this LAN
						//Updating root vector.
						IEEE8021DInterfaceData * candidatoPort = getPortInterfaceData(candidato);
						port->setRole(IEEE8021DInterfaceData::DESIGNATED);
						port->setState(IEEE8021DInterfaceData::DISCARDING); //If there is not a better BPDU, that will become Forwarding.
						initInterfacedata(i);  //FIXME: bla //Reset. Then, a new BPDU will be allowed to upgrade the best received info for this port.
						candidatoPort->setRole(IEEE8021DInterfaceData::ROOT);
						candidatoPort->setState(IEEE8021DInterfaceData::FORWARDING);
						candidatoPort->setLostBPDU(0);
						//Flushing other ports
						//Sending TCN over all active ports
						for(unsigned int j=0;j<portCount;j++)
						{
						    IEEE8021DInterfaceData * jPort = getPortInterfaceData(j);
                            jPort->setTCWhile(simulation.getSimTime()+tcWhileTime);
                            if(j!=(unsigned int)candidato)
                            {
                                sw->flush(j);
                            }
						}
						sw->copyTable(i,candidato); //Copy cache from old to new root.
					}
					else
					{
					    //Alternate not found. Selects a new Root.
						ev<<"Alternate not found. Starts from beginning."<<endl;
						//Initializing Puertos. Starts from the beginning.
						initPorts();
					}
				}
				else if(port->getRole() == IEEE8021DInterfaceData::ALTERNATE
				        ||port->getRole() == IEEE8021DInterfaceData::BACKUP)
				{
				    //It should take care of this LAN, switching to designated.
					port->setRole(IEEE8021DInterfaceData::DESIGNATED);
					port->setState(IEEE8021DInterfaceData::DISCARDING); // A new content will start in case of another switch were in alternate.
					//If there is no problem, this will become forwarding in a few seconds.
					initInterfacedata(i); //Reset port rstp vector.
				}
				port->setLostBPDU(0); //Reseting lost bpdu counter after a change.
			}
		}
	}
	sendBPDUs(); //Generating and sending new BPDUs.
	sendTCNtoRoot();
	if(verbose==true)
	{
		//colorRootPorts();
	}
	scheduleAt(simTime()+hellotime, msg); //Programming next hello time.
}

void RSTP::checkTC(BPDU * frame, int arrival)
{
    IEEE8021DInterfaceData * port = getPortInterfaceData(arrival);
	if((frame->getTcFlag() == true) && (port->getState() == IEEE8021DInterfaceData::FORWARDING))
	{
		Parent->bubble("TCN received");
		for(unsigned int i=0;i<portCount;i++)
		{
			if((int)i!=arrival)
			{
			    IEEE8021DInterfaceData * port2 = getPortInterfaceData(i);
				//Flushing other ports
				//TCN over other ports.
				sw->flush(i);
				port2->setTCWhile(simulation.getSimTime()+tcWhileTime);
			}
		}
	}
}

void RSTP::handleBK(BPDU * frame, int arrival)
{
    IEEE8021DInterfaceData * port = getPortInterfaceData(arrival);
	if((frame->getPortPriority() < port->getPortPriority())
			|| ((frame->getPortPriority() == port->getPortPriority())
			        && (frame->getPortNum() < arrival)))
	{
		//Flushing this port
		sw->flush(arrival);
		port->setRole(IEEE8021DInterfaceData::BACKUP);
		port->setState(IEEE8021DInterfaceData::DISCARDING);
		port->setLostBPDU(0);
	}
	else if(frame->getPortPriority() > port->getPortPriority()
			|| (frame->getPortPriority() == port->getPortPriority()
			        && frame->getPortNum() > arrival))
	{
	    IEEE8021DInterfaceData * port2 = getPortInterfaceData(frame->getPortNum());
		//Flushing that port
		sw->flush(frame->getPortNum());  //PortNumber is sender port number. It is not arrival port.
		port2->setRole(IEEE8021DInterfaceData::BACKUP);
		port2->setState(IEEE8021DInterfaceData::DISCARDING);
        port2->setLostBPDU(0);
	}
	else
	{
	    IEEE8021DInterfaceData * port2 = getPortInterfaceData(frame->getPortNum());
	    //Unavoidable loop. Received its own message at the same port.Switch to disabled.
		ev<<"Unavoidable loop. Received its own message at the same port. To disabled."<<endl;
		//Flushing that port
		sw->flush(frame->getPortNum());  //PortNumber is sender port number. It is not arrival port.
        port2->setRole(IEEE8021DInterfaceData::DISABLED);
        port2->setState(IEEE8021DInterfaceData::DISCARDING);
	}
}

void RSTP::handleIncomingFrame(BPDU *frame)
{  //Incoming BPDU handling
	if(verbose==true)
	{
		printState();
	}

	//First. Checking message age
	Ieee802Ctrl * etherctrl=check_and_cast<Ieee802Ctrl *>(frame->removeControlInfo());
    int arrival=etherctrl->getInterfaceId();
    MACAddress src = etherctrl->getSrc();
    delete etherctrl;
	if(frame->getMessageAge()<maxAge)
	{
		//Checking TC.
		checkTC(frame, arrival); //Sets TCWhile if arrival port was Forwarding

		int r=getRootIndex();

		//Checking possible backup
		if(src.compareTo(address)==0)  //more than one port in the same LAN.
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
		    IEEE8021DInterfaceData * arrivalPort = getPortInterfaceData(arrival);
			int caso=0;
			bool Flood=false;
			caso=compareInterfacedata(arrival,frame,arrivalPort->getLinkCost());
			EV_DEBUG<<"caso: "<<caso<<endl;
			if((caso>0)&&(frame->getRootAddress().compareTo(address)!=0)) //Root will not participate in a loop with its own address
			{
				//Update that port rstp info.
				updateInterfacedata(frame,arrival);
				if(r==-1)
				{
				    //There was not root
				    arrivalPort->setRole(IEEE8021DInterfaceData::ROOT);
				    arrivalPort->setState(IEEE8021DInterfaceData::FORWARDING);
				    arrivalPort->setLostBPDU(0);
					//Flushing other ports
					//TCN over all ports.
					for(unsigned int j=0;j<portCount;j++)
					{
					    IEEE8021DInterfaceData * jPort = getPortInterfaceData(j);
						jPort->setTCWhile(simulation.getSimTime()+tcWhileTime);
						if(j!=(unsigned int)arrival)
						{
							sw->flush(j);
						}
					}
					Flood=true;
				}
				else
				{
				    IEEE8021DInterfaceData * rootPort = getPortInterfaceData(r);
				    //There was a Root. Challenge 2. Compare with the root.
					int caso2=compareInterfacedata(r,frame,arrivalPort->getLinkCost()); //Comparing with root port vector
					EV_DEBUG<<"caso2: "<<caso2<<endl;
					int caso3=0;

					switch(caso2)
					{
						case 0: //Double link to the same port of the root source.Tie breaking. Better local port first.
							if(rootPort->getPortPriority() < arrivalPort->getPortPriority()
									|| (rootPort->getPortPriority() == arrivalPort->getPortPriority()
									        && r<arrival))
							{
								//Flushing that port
								sw->flush(arrival);
			                    arrivalPort->setRole(IEEE8021DInterfaceData::ALTERNATE);
			                    arrivalPort->setState(IEEE8021DInterfaceData::DISCARDING);
			                    arrivalPort->setLostBPDU(0);
							}
							else
							{
								if(arrivalPort->getState() != IEEE8021DInterfaceData::FORWARDING)
								{
								    //Flushing other ports
								    //TCN over all ports.
									for(unsigned int j=0;j<portCount;j++)
									{
										IEEE8021DInterfaceData * jPort = getPortInterfaceData(j);
                                        jPort->setTCWhile(simulation.getSimTime()+tcWhileTime);
                                        if(j!=(unsigned int)arrival)
                                        {
                                            sw->flush(j);
                                        }
									}
								}
								else
								{
								    //Flushing r. Needed in case arrival were previously Fw
									sw->flush(r);
								}
								rootPort->setRole(IEEE8021DInterfaceData::ALTERNATE);
								rootPort->setState(IEEE8021DInterfaceData::DISCARDING); //Comes from root. Preserve lostBPDU
								arrivalPort->setRole(IEEE8021DInterfaceData::ROOT);
                                arrivalPort->setState(IEEE8021DInterfaceData::FORWARDING);
                                arrivalPort->setLostBPDU(0);
								sw->copyTable(r,arrival); //Copy cache from old to new root.

								//The change does not deserve flooding
							}
							break;

						case 1:   //New port rstp info is better than the root in another gate. Root change.
							for(unsigned int i=0;i<portCount;i++)
							{
							    IEEE8021DInterfaceData * iPort = getPortInterfaceData(i);
								if(!iPort->isEdge())   //Avoiding clients reseting
								{
									if(arrivalPort->getState()!=IEEE8021DInterfaceData::FORWARDING)
									{
										iPort->setTCWhile(simulation.getSimTime()+tcWhileTime);
									}
									//Flush i
									sw->flush(i);
									if(i!=(unsigned)arrival)
									{
										iPort->setRole(IEEE8021DInterfaceData::NOTASSIGNED);
										iPort->setState(IEEE8021DInterfaceData::DISCARDING);
										initInterfacedata(i);
									}
								}
							}
							arrivalPort->setRole(IEEE8021DInterfaceData::ROOT);
                            arrivalPort->setState(IEEE8021DInterfaceData::FORWARDING);
                            arrivalPort->setLostBPDU(0);

							Flood=true;
							break;

						case 2: //Same that Root but better RPC
						case 3: //Same that Root RPC but better source
						case 4: //Same that Root RPC Source but better port

							if(arrivalPort->getState()!=IEEE8021DInterfaceData::FORWARDING)
							{
							    //Flushing other ports
								//TCN over all ports
								for(unsigned int j=0;j<portCount;j++)
								{
								    IEEE8021DInterfaceData * jPort = getPortInterfaceData(j);
                                    jPort->setTCWhile(simulation.getSimTime()+tcWhileTime);
                                    if(j!=(unsigned int)arrival)
                                    {
                                        sw->flush(j);
                                    }
								}
							}
                            arrivalPort->setRole(IEEE8021DInterfaceData::ROOT);
                            arrivalPort->setState(IEEE8021DInterfaceData::FORWARDING);
                            arrivalPort->setLostBPDU(0);
                            rootPort->setRole(IEEE8021DInterfaceData::ALTERNATE); //Temporary. Just one port can be root at contest time.
							sw->copyTable(r,arrival); //Copy cache from old to new root.
							Flood=true;
							caso3=contestInterfacedata(r);
							EV_DEBUG<<"caso3: "<<caso3<<endl;
							if(caso3>=0)
							{
							    rootPort->setRole(IEEE8021DInterfaceData::ALTERNATE);
								//LostBPDU not reset.
								//Flushing r
								sw->flush(r);
							}
							else
							{
							    rootPort->setRole(IEEE8021DInterfaceData::DESIGNATED);
							}
							rootPort->setState(IEEE8021DInterfaceData::DISCARDING);
							break;

						case -1: //Worse Root
							sendBPDU(arrival);	//BPDU to show him a better Root as soon as possible
							break;

						case -2: //Same Root but worse RPC
						case -3: //Same Root RPC but worse source
						case -4: //Same Root RPC Source but worse port
							//Compares with frame that would be sent if it were the root for this LAN.
							caso3=contestInterfacedata(frame,arrival); //Case 0 not possible. < if own vector better than frame.
							EV_DEBUG<<"caso3: "<<caso3<<endl;
							if(caso3<0)
							{
								arrivalPort->setRole(IEEE8021DInterfaceData::DESIGNATED);
                                arrivalPort->setState(IEEE8021DInterfaceData::DISCARDING);
								sendBPDU(arrival); //BPDU to show him a better Root as soon as possible
							}
							else
							{
								//Flush arrival
								sw->flush(arrival);
								arrivalPort->setRole(IEEE8021DInterfaceData::ALTERNATE);
                                arrivalPort->setState(IEEE8021DInterfaceData::DISCARDING);
                                arrivalPort->setLostBPDU(0);
							}
							break;
					}
				}
			}
			else if((src.compareTo(arrivalPort->getBridgeAddress())==0) //Worse or similar, but the same source
					&&(frame->getRootAddress().compareTo(address)!=0))   // Root will not participate
			{
			    //Source has updated BPDU information.
				switch(caso)
				{
				case 0:
				    arrivalPort->setLostBPDU(0);   // Same BPDU. Not updated.
					break;

				case -1://Worse Root
					if(arrivalPort->getRole() == IEEE8021DInterfaceData::ROOT)
					{
						int alternative=getBestAlternate(); //Searching old alternate
						if(alternative>=0)
						{
						    IEEE8021DInterfaceData * alternativePort = getPortInterfaceData(alternative);
						    arrivalPort->setRole(IEEE8021DInterfaceData::DESIGNATED);
                            arrivalPort->setState(IEEE8021DInterfaceData::DISCARDING);
							sw->copyTable(arrival,alternative);  //Copy cache from old to new root.
							//Flushing other ports
							//TCN over all ports. Alternative was alternate.
							for(unsigned int j=0;j<portCount;j++)
                            {
                                IEEE8021DInterfaceData * jPort = getPortInterfaceData(j);
                                jPort->setTCWhile(simulation.getSimTime()+tcWhileTime);
                                if(j!=(unsigned int)alternative)
                                {
                                    sw->flush(j);
                                }
                            }
							alternativePort->setRole(IEEE8021DInterfaceData::ROOT);
							alternativePort->setState(IEEE8021DInterfaceData::FORWARDING); //Comes from alternate. Preserves lostBPDU.
							updateInterfacedata(frame,arrival);
							sendBPDU(arrival); //Show him a better Root as soon as possible
						}
						else
						{
						    //If there is not alternate, is it better than own initial vector?
							int caso2=0;
							initPorts(); //Allowing other ports to contest again.
							//Flushing all ports.
							for(unsigned int j=0;j<portCount;j++)
							{
								sw->flush(j);
							}
							caso2=compareInterfacedata(arrival,frame,arrivalPort->getLinkCost());
							EV_DEBUG<<"caso2: "<<caso2<<endl;
							if(caso2>0)
							{
							    updateInterfacedata(frame,arrival); //If this module is not better. Keep it as a ROOT.
								arrivalPort->setRole(IEEE8021DInterfaceData::ROOT);
                                arrivalPort->setState(IEEE8021DInterfaceData::FORWARDING);
							}
							//Propagating new information.
							Flood=true;
						}
					}
					else if(arrivalPort->getRole() == IEEE8021DInterfaceData::ALTERNATE)
					{
					    arrivalPort->setRole(IEEE8021DInterfaceData::DESIGNATED);
                        arrivalPort->setState(IEEE8021DInterfaceData::DISCARDING);
                        updateInterfacedata(frame,arrival);
						sendBPDU(arrival); //Show him a better Root as soon as possible
					}
					break;

				case -2:
				case -3:
				case -4: //Same Root.Worse source
					if(arrivalPort->getRole() == IEEE8021DInterfaceData::ROOT)
					{
					    arrivalPort->setLostBPDU(0);
						int alternative=getBestAlternate(); //Searching old alternate
						if(alternative>=0)
						{
						    IEEE8021DInterfaceData * alternativePort = getPortInterfaceData(alternative);
							int caso2=0;
							caso2=compareInterfacedata(alternative,frame,arrivalPort->getLinkCost());
							EV_DEBUG<<"caso2: "<<caso2<<endl;
							if(caso2<0)//If alternate is better, change
							{
								alternativePort->setRole(IEEE8021DInterfaceData::ROOT);
								alternativePort->setState(IEEE8021DInterfaceData::FORWARDING);
								arrivalPort->setRole(IEEE8021DInterfaceData::DESIGNATED);//Temporary. Just one port can be root at contest time.
								int caso3=0;
								caso3=contestInterfacedata(frame,arrival);
								EV_DEBUG<<"caso3: "<<caso3<<endl;
								if(caso3<0)
								{
									arrivalPort->setRole(IEEE8021DInterfaceData::DESIGNATED);
								}
								else
								{
									arrivalPort->setRole(IEEE8021DInterfaceData::ALTERNATE);
								}
								arrivalPort->setState(IEEE8021DInterfaceData::DISCARDING);
								//Flushing other ports
								//TC over all ports.
								for(unsigned int j=0;j<portCount;j++)
                                {
                                    IEEE8021DInterfaceData * jPort = getPortInterfaceData(j);
                                    jPort->setTCWhile(simulation.getSimTime()+tcWhileTime);
                                    if(j!=(unsigned int)alternative)
                                    {
                                        sw->flush(j);
                                    }
                                }
								sw->copyTable(arrival,alternative); //Copy cache from old to new root.
							}
						}
						updateInterfacedata(frame,arrival);
						//Propagating new information.
						Flood=true;
						//If alternate is worse than root, or there is not alternate, keep old root as root.
					}
					else if(arrivalPort->getRole() == IEEE8021DInterfaceData::ALTERNATE)
					{
						int caso2=0;
						caso2=contestInterfacedata(frame,arrival);
						EV_DEBUG<<"caso2: "<<caso2<<endl;
						if(caso2<0)
						{
							arrivalPort->setRole(IEEE8021DInterfaceData::DESIGNATED); //If the frame is worse than this module generated frame. Switch to Designated/Discarding
							arrivalPort->setState(IEEE8021DInterfaceData::DISCARDING);
							sendBPDU(arrival); //Show him a better BPDU as soon as possible
						}
						else
						{
							arrivalPort->setLostBPDU(0); //If it is better than this module generated frame, keep it as alternate
							//This does not deserve expedited BPDU
						}
					}
					updateInterfacedata(frame,arrival);
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
	int r=getRootIndex();
	if((r>=0)&&(r<portCount))
	{
	    IEEE8021DInterfaceData * rootPort = getPortInterfaceData(r);
		if(rootPort->getRole() != IEEE8021DInterfaceData::DISABLED)
		{
			if(simulation.getSimTime()<rootPort->getTCWhile())
			{
				BPDU * frame = new BPDU();
				Ieee802Ctrl * etherctrl= new Ieee802Ctrl();

				frame->setRootPriority(rootPort->getRootPriority());
				frame->setRootAddress(rootPort->getRootAddress());
				frame->setMessageAge(rootPort->getAge());
				frame->setRootPathCost(rootPort->getRootPathCost());
				frame->setBridgePriority(priority);
				frame->setTcaFlag(false);
				frame->setPortNum(r);  //Src port number.
				frame->setBridgeAddress(address);
				frame->setTcFlag(true);
		        if (frame->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
		            frame->setByteLength(MIN_ETHERNET_FRAME_BYTES);
		        etherctrl->setSrc(address);
		        etherctrl->setDest(MACAddress::STP_MULTICAST_ADDRESS);
				etherctrl->setInterfaceId(r);
				frame->setControlInfo(etherctrl);
				send(frame,"STPGate$o");
			}
		}
	}
}

void RSTP::sendBPDUs()
{//Send BPDUs through all ports if they are required.
	for(unsigned int i=0;i<portCount;i++)
	{
	    IEEE8021DInterfaceData * iPort = getPortInterfaceData(i);
		if((iPort->getRole() != IEEE8021DInterfaceData::ROOT)
		        && (iPort->getRole() != IEEE8021DInterfaceData::ALTERNATE)
		        && (iPort->getRole() != IEEE8021DInterfaceData::DISABLED)
		        && (!iPort->isEdge()))
		{
			sendBPDU(i);
		}
	}
}

void RSTP::sendBPDU(int port)
{
    //Send a BPDU throuth port.
    IEEE8021DInterfaceData * iport = getPortInterfaceData(port);
    int r=getRootIndex();
    IEEE8021DInterfaceData * rootPort;
    if (r!=-1)
        rootPort = getPortInterfaceData(r);
	if(iport->getRole() != IEEE8021DInterfaceData::DISABLED)
	{
		BPDU * frame = new BPDU();
		Ieee802Ctrl * etherctrl=new Ieee802Ctrl();
		if (r!=-1)
		{
		    frame->setRootPriority(rootPort->getRootPriority());
		    frame->setRootAddress(rootPort->getRootAddress());
		    frame->setMessageAge(rootPort->getAge());
		    frame->setRootPathCost(rootPort->getRootPathCost());
		}
		else{
		    frame->setRootPriority(priority);
            frame->setRootAddress(address);
            frame->setMessageAge(0);
            frame->setRootPathCost(0);
		}
		frame->setBridgePriority(priority);
		frame->setTcaFlag(false);
		frame->setPortNum(port);  //Src port number.
		frame->setBridgeAddress(address);
		if(simulation.getSimTime() < iport->getTCWhile())
		{
			frame->setTcFlag(true);
		}
		else
			frame->setTcFlag(false);
        if (frame->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
            frame->setByteLength(MIN_ETHERNET_FRAME_BYTES);
        etherctrl->setSrc(address);
        etherctrl->setDest(MACAddress::STP_MULTICAST_ADDRESS);
		etherctrl->setInterfaceId(port);
		frame->setControlInfo(etherctrl);
		send(frame,"STPGate$o");
	}
}

void RSTP::colorRootPorts()
{
    //Gives color to the root link, or module border if it is root.
    IEEE8021DInterfaceData * port;
    bool isRoot=true;
    for (unsigned int i = 0; i < portCount; i++)
    {
        port = getPortInterfaceData(i);
        cGate * outGate = getParentModule()->gate("ethg$o", i);
        cGate * inputGate = getParentModule()->gate("ethg$i", i);
        cGate * outGateNext = outGate->getNextGate();
        cGate * inputGatePrev = inputGate->getPreviousGate();

        if(port->getRole() == IEEE8021DInterfaceData::ROOT)
        {
            isRoot=false;
            if(outGate && inputGate && inputGatePrev && outGateNext){
                if (port->isForwarding())
                {
                    outGate->getDisplayString().setTagArg("ls", 0, "#a5ffff");
                    outGate->getDisplayString().setTagArg("ls", 1, 3);

                    inputGate->getDisplayString().setTagArg("ls", 0, "#a5ffff");
                    inputGate->getDisplayString().setTagArg("ls", 1, 3);

                    outGateNext->getDisplayString().setTagArg("ls", 0, "#a5ffff");
                    outGateNext->getDisplayString().setTagArg("ls", 1, 3);

                    inputGatePrev->getDisplayString().setTagArg("ls", 0, "#a5ffff");
                    inputGatePrev->getDisplayString().setTagArg("ls", 1, 3);
                }
                else
                {
                    outGate->getDisplayString().setTagArg("ls", 0, "#000000");
                    outGate->getDisplayString().setTagArg("ls", 1, 1);

                    inputGate->getDisplayString().setTagArg("ls", 0, "#000000");
                    inputGate->getDisplayString().setTagArg("ls", 1, 1);

                    outGateNext->getDisplayString().setTagArg("ls", 0, "#000000");
                    outGateNext->getDisplayString().setTagArg("ls", 1, 1);

                    inputGatePrev->getDisplayString().setTagArg("ls", 0, "#000000");
                    inputGatePrev->getDisplayString().setTagArg("ls", 1, 1);
                }
            }
        }

        cModule * puerta=Parent->getSubmodule("eth",i);
        if(puerta!=NULL)
        {
            char buf[20];
            int state=port->getState();
            switch(state)
            {
            case 0 :
                sprintf(buf,"DISCARDING\n");
                break;
            case 1:
                sprintf(buf,"LEARNING\n");
                break;
            case 2:
                sprintf(buf,"FORWARDING\n");
                break;
            default:
                sprintf(buf,"UNKNOWN\n");
                break;
            }
            puerta->getDisplayString().setTagArg("t",0,buf);
        }
    }

    if(isOperational) //only when the router is working
	{
		if(isRoot)
		{
		    //Root mark
			Parent->getDisplayString().setTagArg("i2",0,"status/check");
			Parent->getDisplayString().setTagArg("i",1,"#a5ffff");
			char buf[50];
			sprintf(buf,"Root: %s",address.str().c_str());
			getDisplayString().setTagArg("t",0,buf);
		}
		else
		{
		    //Remove possible root mark
			Parent->getDisplayString().removeTag("i2");
			Parent->getDisplayString().setTagArg("i",1,"");
		}
	}
}

void RSTP::printState()
{
    // Prints current database info
	if(Parent!=NULL)
		ev<<endl<<Parent->getName()<<endl;
	int r=getRootIndex();
	ev<<"RSTP state"<<endl;
	ev<<"Priority: "<<priority<<endl;
	ev<<"Local MAC: "<<address<<endl;
	if(r>=0)
	{
	    IEEE8021DInterfaceData * rootPort = getPortInterfaceData(r);
		ev<<"Root Priority: "<<rootPort->getRootPriority()<<endl;
		ev<<"Root address: "<<rootPort->getRootAddress().str()<<endl;
		ev<<"cost: "<<rootPort->getRootPathCost()<<endl;
		ev<<"age:  "<<rootPort->getAge()<<endl;
		ev<<"Bridge priority: "<<rootPort->getBridgePriority()<<endl;
		ev<<"Bridge address: "<<rootPort->getBridgeAddress().str()<<endl;
		ev<<"Src TxGate Priority: "<<rootPort->getPortPriority()<<endl;
		ev<<"Src TxGate: "<<rootPort->getPortNum()<<endl;
	}
	ev<<"Port State/Role: "<<endl;
	for(unsigned int i=0;i<portCount;i++)
	{
	    IEEE8021DInterfaceData * iPort = getPortInterfaceData(i);
		ev<<iPort->getState()<<" ";
		if(iPort->getState() == IEEE8021DInterfaceData::DISCARDING)
		{
			ev<<"Discarding";
		}
		else if (iPort->getState() == IEEE8021DInterfaceData::LEARNING)
		{
			ev<<"Learning";
		}
		else if (iPort->getState() == IEEE8021DInterfaceData::FORWARDING)
		{
			ev<<"Forwarding";
		}
		ev<<"  ";
		if(iPort->getRole() == IEEE8021DInterfaceData::ROOT)
			ev<<"Root";
		else if (iPort->getRole() == IEEE8021DInterfaceData::DESIGNATED)
			ev<<"Designated";
		else if (iPort->getRole() == IEEE8021DInterfaceData::BACKUP)
			ev<<"Backup";
		else if (iPort->getRole() == IEEE8021DInterfaceData::ALTERNATE)
			ev<<"Alternate";
		else if (iPort->getRole() == IEEE8021DInterfaceData::DISABLED)
			ev<<"Disabled";
		else if (iPort->getRole() == IEEE8021DInterfaceData::NOTASSIGNED)
			ev<<"Not assigned";
		if(iPort->isEdge())
			ev<<" (Client)";
		ev<<endl;
	}
	ev<<"Per port best source. Root/Src"<<endl;
	for(unsigned int i=0;i<portCount;i++)
	{
	    IEEE8021DInterfaceData * iPort = getPortInterfaceData(i);
		ev<<iPort->getRootAddress().str()<<"/"<<iPort->getBridgeAddress().str()<<endl;
	}
}

void RSTP::initInterfacedata(unsigned int portNum)
{
    IEEE8021DInterfaceData * ifd = getPortInterfaceData(portNum);
    ifd->setRootPriority(priority);
    ifd->setRootAddress(address);
    ifd->setRootPathCost(0);
    ifd->setAge(0);
    ifd->setBridgePriority(priority);
    ifd->setBridgeAddress(address);
    ifd->setPortPriority(-1);
    ifd->setPortNum(-1);
    ifd->setLostBPDU(0);
}

void RSTP::initPorts()
{
    //Initialize RSTP dynamic information.
	for(unsigned int j=0;j<portCount;j++)
	{
	    IEEE8021DInterfaceData * jPort = getPortInterfaceData(j);
		if(!jPort->isEdge())
		{
			jPort->setRole(IEEE8021DInterfaceData::NOTASSIGNED);
			jPort->setState(IEEE8021DInterfaceData::DISCARDING);
		}
		initInterfacedata(j);
		sw->flush(j);
	}
}

void RSTP::updateInterfacedata(BPDU *frame,unsigned int portNum)
{
    IEEE8021DInterfaceData * ifd = getPortInterfaceData(portNum);
    ifd->setRootPriority(frame->getRootPriority());
    ifd->setRootAddress(frame->getRootAddress());
    ifd->setRootPathCost(frame->getRootPathCost()+ifd->getLinkCost());
    ifd->setAge(frame->getMessageAge()+1);
    ifd->setBridgePriority(frame->getBridgePriority());
    ifd->setBridgeAddress(frame->getBridgeAddress());
    ifd->setPortPriority(frame->getPortPriority());
    ifd->setPortNum(frame->getPortNum());
    ifd->setLostBPDU(0);
}
int RSTP::contestInterfacedata(unsigned int portNum)
{
    //Compares vect2 with the vector that would be sent if this were the root node for the arrival LAN.
	//>0 if vect2 better than own vector. =0 if they are similar.
	// -1=Worse   0= Similar  1=Better Root. 2= Better RPC  3= Better Src   4= Better Port
	int r=getRootIndex();
	IEEE8021DInterfaceData * rootPort = getPortInterfaceData(r);
	IEEE8021DInterfaceData * ifd = getPortInterfaceData(portNum);

	int rootPriority1=rootPort->getRootPriority();
    MACAddress rootAddress1=rootPort->getRootAddress();
    int rootPathCost1=rootPort->getRootPathCost()+ifd->getLinkCost();
    int bridgePriority1=priority;
    MACAddress bridgeAddress1=address;
    int portPriority1=ifd->getPortPriority();
    int portNum1=portNum;

    int rootPriority2=ifd->getRootPriority();
    MACAddress rootAddress2=ifd->getRootAddress();
    int rootPathCost2=ifd->getRootPathCost();
    int bridgePriority2=ifd->getBridgePriority();
    MACAddress bridgeAddress2=ifd->getBridgeAddress();
    int portPriority2=ifd->getPortPriority();
    int portNum2=ifd->getPortNum();


	if(rootPriority1 != rootPriority2)
        return (rootPriority1 < rootPriority2) ? -1 : 1;

    int c = rootAddress1.compareTo(rootAddress2);
    if (c != 0)
        return (c < 0) ? -1 : 1;

    if (rootPathCost1 != rootPathCost2)
        return (rootPathCost1 < rootPathCost2) ? -2 : 2;

    if(bridgePriority1 != bridgePriority2)
        return (bridgePriority1 < bridgePriority2) ? -3 : 3;

    c = bridgeAddress1.compareTo(bridgeAddress2);
    if (c != 0)
        return (c < 0) ? -3 : 3;

    // srcAddress == vect2.srcAddress
    if (portPriority1 != portPriority2)
        return (portPriority1 < portPriority2) ? -4 : 4;
    // srcPortPriority == vect2.srcPortPriority
    if (portNum1 != portNum2)
        return (portNum1 < portNum2) ? -4 : 4;

    return 0;
}

int RSTP::contestInterfacedata(BPDU* msg,unsigned int portNum)
{
    //Compares msg with the vector that would be sent if this were the root node for the arrival LAN.
	// >0 if frame vector better than own vector. =0 if they are similar.
	// -1=Worse   0= Similar  1=Better Root. 2= Better RPC  3= Better Src   4= Better Port

    int r=getRootIndex();
    IEEE8021DInterfaceData * rootPort = getPortInterfaceData(r);
    IEEE8021DInterfaceData * ifd = getPortInterfaceData(portNum);

    int rootPriority1=rootPort->getRootPriority();
    MACAddress rootAddress1=rootPort->getRootAddress();
    int rootPathCost1=rootPort->getRootPathCost();
    int bridgePriority1=priority;
    MACAddress bridgeAddress1=address;
    int portPriority1=ifd->getPortPriority();
    int portNum1=portNum;

    int rootPriority2=msg->getRootPriority();
    MACAddress rootAddress2=msg->getRootAddress();
    int rootPathCost2=msg->getRootPathCost();
    int bridgePriority2=msg->getBridgePriority();
    MACAddress bridgeAddress2=msg->getBridgeAddress();
    int portPriority2=msg->getPortPriority();
    int portNum2=msg->getPortNum();


    if(rootPriority1 != rootPriority2)
        return (rootPriority1 < rootPriority2) ? -1 : 1;

    int c = rootAddress1.compareTo(rootAddress2);
    if (c != 0)
        return (c < 0) ? -1 : 1;

    if (rootPathCost1 != rootPathCost2)
        return (rootPathCost1 < rootPathCost2) ? -2 : 2;

    if(bridgePriority1 != bridgePriority2)
        return (bridgePriority1 < bridgePriority2) ? -3 : 3;

    c = bridgeAddress1.compareTo(bridgeAddress2);
    if (c != 0)
        return (c < 0) ? -3 : 3;

    // srcAddress == vect2.srcAddress
    if (portPriority1 != portPriority2)
        return (portPriority1 < portPriority2) ? -4 : 4;
    // srcPortPriority == vect2.srcPortPriority
    if (portNum1 != portNum2)
        return (portNum1 < portNum2) ? -4 : 4;

    return 0;


}
int RSTP::compareInterfacedata(unsigned int portNum, BPDU * msg,int linkCost)
{
    IEEE8021DInterfaceData * ifd = getPortInterfaceData(portNum);

    int rootPriority1=ifd->getRootPriority();
    MACAddress rootAddress1=ifd->getRootAddress();
    int rootPathCost1=ifd->getRootPathCost();
    int bridgePriority1=ifd->getBridgePriority();
    MACAddress bridgeAddress1=ifd->getBridgeAddress();
    int portPriority1=ifd->getPortPriority();
    int portNum1=ifd->getPortNum();

    int rootPriority2=msg->getRootPriority();
    MACAddress rootAddress2=msg->getRootAddress();
    int rootPathCost2=msg->getRootPathCost()+linkCost;
    int bridgePriority2=msg->getBridgePriority();
    MACAddress bridgeAddress2=msg->getBridgeAddress();
    int portPriority2=msg->getPortPriority();
    int portNum2=msg->getPortNum();


    if(rootPriority1 != rootPriority2)
        return (rootPriority1 < rootPriority2) ? -1 : 1;

    int c = rootAddress1.compareTo(rootAddress2);
    if (c != 0)
        return (c < 0) ? -1 : 1;

    if (rootPathCost1 != rootPathCost2)
        return (rootPathCost1 < rootPathCost2) ? -2 : 2;

    if(bridgePriority1 != bridgePriority2)
        return (bridgePriority1 < bridgePriority2) ? -3 : 3;

    c = bridgeAddress1.compareTo(bridgeAddress2);
    if (c != 0)
        return (c < 0) ? -3 : 3;

    // srcAddress == vect2.srcAddress
    if (portPriority1 != portPriority2)
        return (portPriority1 < portPriority2) ? -4 : 4;
    // srcPortPriority == vect2.srcPortPriority
    if (portNum1 != portNum2)
        return (portNum1 < portNum2) ? -4 : 4;

    return 0;
}

int RSTP::getRootIndex()
{
    //Returns the Root Gate index or -1 if there is not Root Gate.
	int result=-1;
	for(unsigned int i=0;i<portCount;i++)
	{
	    IEEE8021DInterfaceData * iPort = getPortInterfaceData(i);
		if(iPort->getRole() == IEEE8021DInterfaceData::ROOT)
		{
			result=i;
			break;
		}
	}
	return result;
}

int RSTP::getBestAlternate()
{
    // Gets best alternate port index.
	int candidato=-1;  //Index of the best alternate found.
	for(unsigned int j=0;j<portCount;j++)
	{
	    IEEE8021DInterfaceData * jPort = getPortInterfaceData(j);
		if(jPort->getRole() == IEEE8021DInterfaceData::ALTERNATE) //Just from Alternates. Others are not updated.
		{
			if(candidato<0)
			    candidato=j;
			else
			{
			    IEEE8021DInterfaceData * candidatoPort = getPortInterfaceData(candidato);
			    if((jPort->getRootPathCost() < candidatoPort->getRootPathCost())
							||(jPort->getRootPathCost() == candidatoPort->getRootPathCost() && jPort->getBridgePriority() < candidatoPort->getBridgePriority())
							||(jPort->getRootPathCost() == candidatoPort->getRootPathCost() && jPort->getBridgePriority() == candidatoPort->getBridgePriority() && jPort->getBridgeAddress().compareTo(candidatoPort->getBridgeAddress()) < 0)
							||(jPort->getRootPathCost() == candidatoPort->getRootPathCost() && jPort->getBridgePriority() == candidatoPort->getBridgePriority() && jPort->getBridgeAddress().compareTo(candidatoPort->getBridgeAddress()) == 0 && jPort->getPortPriority() < candidatoPort->getPortPriority())
							||(jPort->getRootPathCost() == candidatoPort->getRootPathCost() && jPort->getBridgePriority() == candidatoPort->getBridgePriority() && jPort->getBridgeAddress().compareTo(candidatoPort->getBridgeAddress()) == 0 && jPort->getPortPriority() == candidatoPort->getPortPriority() && jPort->getPortNum() < candidatoPort->getPortNum()))
                {
                    //It is the first alternate or better than the found one
                    candidato=j; //New candidate
                }
			}
		}
	}
	return candidato;
}

void RSTP::start()
{
    initPorts();
    isOperational = true;
}

void RSTP::stop()
{
    for(unsigned int i=0;i<portCount;i++)
    {
        IEEE8021DInterfaceData * iPort = getPortInterfaceData(i);
        iPort->setRole(IEEE8021DInterfaceData::DISABLED);
        iPort->setState(IEEE8021DInterfaceData::DISCARDING);
    }
    isOperational = false;
}

bool RSTP::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();

    if (dynamic_cast<NodeStartOperation *>(operation))
    {
        if (stage == NodeStartOperation::STAGE_LINK_LAYER) {
            start();
        }
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation))
    {
        if (stage == NodeShutdownOperation::STAGE_LINK_LAYER) {
            stop();
        }
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation))
    {
        if (stage == NodeCrashOperation::STAGE_CRASH) {
            stop();
        }
    }
    else
    {
        throw cRuntimeError("Unsupported operation '%s'", operation->getClassName());
    }

    return true;
}

IEEE8021DInterfaceData * RSTP::getPortInterfaceData(unsigned int portNum)
{
    cGate * gate = this->getParentModule()->gate("ethg$o", portNum);
    if(gate==NULL)
        error("gate is NULL");
    InterfaceEntry * gateIfEntry = ifTable->getInterfaceByNodeOutputGateId(gate->getId());
    if(gateIfEntry==NULL)
            error("gateIfEntry is NULL");
    IEEE8021DInterfaceData * portData = gateIfEntry->ieee8021DData();

    if (!portData)
        error("IEEE8021DInterfaceData not found!");

    return portData;
}
