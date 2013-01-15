 /**
******************************************************
* @file MVRP.cc
* @brief MVRP Protocol control
*
* @author Juan Luis Garrote Molinero
* @version 1.0
* @date Feb 2011
******************************************************/
#include "MVRP.h"
#include <algorithm>  // Needed for std::sort
#include "RSTPAccess.h"
#include "AdmacrelayAccess.h"
#include "Relay1QAccess.h"




Define_Module (MVRP);
void MVRP::initialize(int stage)
{
	if(stage==2) // rstp initialization takes place in stage 1
	{
		//Getting module parameters
		interFrameTime=(simtime_t) par("interFrameTime");
		agingTime=(simtime_t) par("agingTime");
		verbose=(bool) par("verbose");
		testing=(bool) par("testing");

		Relay1Q* relay=Relay1QAccess().getIfExists();
		if(relay==NULL)
			relay=AdmacrelayAccess().get();
		int GatesSize=0;

		GatesSize=relay->gateSize("GatesOut");
		rstpModule=RSTPAccess().get();
		address=rstpModule->getAddress();

		//Initializing MVRP module. Puertos will save MVRP info for every port.
		for(int i=0;i<GatesSize;i++)
		{
			Puertos.push_back( new PortMVRPStatus());
		}
		for(unsigned int i=0;i<Puertos.size();i++)
		{
			Puertos[i]->registered.clear();
		}
		// First MVRPDU message.
		cMessage *msg = new cMessage();
		msg->setName("itsMVRPDUtime");
		scheduleAt(simTime()+exponential(0.01), msg); //
	}
}

void MVRP::finish()
{//Saving statistics
	cleanAgedEntries();
	if(testing==true)
	{
		FILE * pFile;
		char buff[50];
		sprintf(buff,"results/MVRP-%s.info",getParentModule()->getName());
		pFile = fopen (buff,"w");
		  if (pFile!=NULL)
		  {
			  fprintf(pFile,"Local MAC: %s\n",address.str().c_str());
			  fprintf(pFile,"Registered VLANs\n");

			  for(unsigned int i=0 ; i<Puertos.size() ; i++)
			  {
				  fprintf(pFile,"************Port %d*********\n",i);
				  std::vector <int> list;
				  for(unsigned int j=0;j<Puertos[i]->registered.size();j++)
				  {
					  list.push_back(Puertos[i]->registered[j].VID);
				  }
				  std::sort(list.begin(),list.end());
				  for(unsigned int j=0;j<list.size();j++)
				  {
					  fprintf(pFile,"%d\n",list[j]);
				  }
			  }
			  fclose(pFile);
		  }
	}
}



void MVRP::handleMessage(cMessage *msg)
{	//MVRPDU and selfMessages can be received. SelfMessages indicate that it is time to send a new MVRPDU
//
	if(msg->isSelfMessage())
	{
		if(strcmp(msg->getName(),"itsMVRPDUtime")==0)
		{
			handleMVRPDUtime(msg);

		}
		else
		{
			ev<<"Unknown self message."<<endl;
		}
	}
	else
	{
		ev<<"New MVRPDU received at MVRP module"<<endl;
		handleIncomingFrame(check_and_cast<Delivery *> (msg));
	}
}

void MVRP::handleMVRPDUtime(cMessage * msg)
{
	if(verbose==true)
	{
		ev<<"Generating new MVRPDU."<<endl;
		printState();
	}
	//Updates the rstpModule pointer.
	rstpModule=RSTPAccess().get();
	cleanAgedEntries();
	for(unsigned int i=0;i<Puertos.size();i++)
	{//Generates a new MVRPDU for every forwarding gate. Just for backbone gates.
		if(!rstpModule->isEdge(i))
		{//If it is not a client gate cleans the old ones.

			if(rstpModule->getPortState(i)==FORWARDING)
			{
				std::vector <vid> VIDS;
				VIDS.clear();
				for(unsigned int j=0;j<Puertos.size();j++)
				{// All gates info except itself
					if(j!=i)
					{
						if(rstpModule->getPortState(j)==FORWARDING) // Just for forwarding gates.
						{
							for(unsigned int k=0;k<Puertos[j]->registered.size();k++)
							{ // Looking for the VIDS
								bool found=false;
								vid ToRegister=Puertos[j]->registered[k].VID;
								for(unsigned int l=0;l<VIDS.size();l++)
								{
									if(VIDS[l]==ToRegister)
									{
										found=true;
										break;
									}
								}

								if(!found) //If it is not registered yet. New VID registration.
								{
									VIDS.push_back(Puertos[j]->registered[k].VID);
								}
							}
						}
					}
				}
				//Generating the new MVRPDU.
				MVRPDU * frame=new MVRPDU();
				frame->setPortIndex(i);   //
				frame->setVIDSArraySize(VIDS.size());
				frame->setDest(MACAddress("01-80-C2-00-00-0D"));
				frame->setSrc(address);
				for(unsigned int k=0;k<VIDS.size();k++)
				{
					frame->setVIDS(k,VIDS[k]);
				}
                if (frame->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
                    frame->setByteLength(MIN_ETHERNET_FRAME_BYTES);
				Delivery* frame2=new Delivery();
				frame2->setSendByPort(i);
				frame2->encapsulate(frame);
				send(frame2,"MVRPPort$o"); //Sends to the admacrelay module
			}
		}
	}
	scheduleAt(simTime()+interFrameTime, msg); //Programming next MVRPDU time.
}

void MVRP::cleanAgedEntries()
{//Clean aged entries from MVRP registered cache. Client ports are not cleaned.
	for(unsigned int i=0;i<Puertos.size();i++)
	{// Just for backbone gates.
		if(!rstpModule->isEdge(i))
		{//If it is not a client gate cleans the old ones.

			for(unsigned int j=0;j<Puertos[i]->registered.size();j++)
			{
				if((simulation.getSimTime()-(SimTime)(Puertos[i]->registered[j].inserted)) > (SimTime) agingTime)
				{//Delete entry
					if(verbose==true)
						ev<<"Deleting old entry. Gate: "<<i<<" VID: "<<Puertos[i]->registered[j].VID<<" inserted: "<<Puertos[i]->registered[j].inserted<<endl;
					Puertos[i]->registered.erase(Puertos[i]->registered.begin()+j);
					j--;
				}
			}
		}
	}

}


void MVRP::handleIncomingFrame(Delivery *frame2)
{//It registers the new vid contained in the MVRPDU at the arrival gate.
	unsigned int portIndex=frame2->getArrivalPort();  //Arrival gate. That's the admacrelay arrival gate index.
	MVRPDU*frame=check_and_cast<MVRPDU *>(frame2->decapsulate());
	delete frame2;
	unsigned int numVIDs=frame->getVIDSArraySize(); //Number of new vids.
	if(verbose==true)
	{
		if (numVIDs <= 0)
		{
			ev << "Empty message" << endl;
		}
		else
		{
			ev << "Containing " << numVIDs << " vids. These are the vid carried:"<< endl;
			for (unsigned int i = 0; i < numVIDs; i++)
			{
				ev << "vid: " << frame->getVIDS(i) << endl;
			}
		}
	}
	for (unsigned int i = 0; i < numVIDs; i++)
	{//Registra or refreshes every VLAN
		registerVLAN(portIndex, frame->getVIDS(i));
	}
	delete frame;
	if(verbose==true)
	{
		ev<<endl<<"Afte update state."<<endl;
		printState();
	}
}


bool MVRP::registerVLAN(int port,vid vlan)
{//Registers vlan into the port info.
	bool found=false;
		for(unsigned int k=0;k<Puertos[port]->registered.size();k++)
		{//Looking for the entry
			if(Puertos[port]->registered[k].VID==vlan)
				{
					found=true;
					Puertos[port]->registered[k].inserted=simTime(); //Refreshing
					break;
				}
		}
		if(found!=true)
		{ //If it was not found, it registers it.
			vid_time * a=new vid_time();
			a->VID=vlan;
			a->inserted=simTime();
			Puertos[port]->registered.push_back(*a);
		}
	return found;
}



void MVRP::printState()
{// Prints basic module information and MVRP registered info.
	cleanAgedEntries();
	ev<<"MVRP State"<<endl;
	ev<<"Module mac: "<<address<<endl;
	ev<<"Per port registered VLANS"<<endl;
	for(unsigned int i=0 ; i<Puertos.size() ; i++)
	{
		ev<<"Port: ";
		ev<<i<<endl;
		for(unsigned int j=0;j<Puertos[i]->registered.size();j++)
		{
				ev<<Puertos[i]->registered[j].VID<<"   "<<Puertos[i]->registered[j].inserted<<endl;
		}
	}
}


bool MVRP::resolveVLAN(vid VID, std::vector<int> * gates)
{//Gets VID associated ports. Outter modules should avoid duplication.
	Enter_Method("Getting VID associated ports");
	bool found=false;
	gates->clear();
	for(unsigned int i=0;i<Puertos.size();i++)
	{//Looking for the info.
			for(unsigned int j=0;j<Puertos[i]->registered.size();j++)
			{
				if(Puertos[i]->registered[j].VID==VID)
				{
					found=true;
					gates->push_back(i);
					break;
				}
			}
	}
	return found;
}

