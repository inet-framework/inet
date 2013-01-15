 /**
******************************************************
* @file Cache1AH.h
* @brief Cached data base
* @author Juan Luis Garrote Molinero
* @version 1.0
* @date Feb 2010
******************************************************/

#include "Cache1AH.h"
#include "XMLUtils.h"
#include "AdmacrelayAccess.h"

Define_Module( Cache1AH );
Cache1AH::Cache1AH(){}
Cache1AH::~Cache1AH(){}
void Cache1AH::initialize()
{
	Cache1Q::initialize();
		admac=AdmacrelayAccess().getIfExists();
		if(admac!=NULL)
		{
			for(int i=0;i<(admac->gateSize("GatesOut"));i++)
			{
				ISIDregisterTable.push_back(* new ISIDregister());
			}
		}
		else
		{
			error("Admacrelay module not found.");
		}

		//Reading ISID configuration.
		readconfigfromXML(par("ISIDConfig").xmlValue());
		if(verbose==true)
			printState();
}

void Cache1AH::readconfigfromXML(const cXMLElement* isidConfig)
{// Reads ISID info from ISIDConfig xml.
	    ASSERT(isidConfig);
		ASSERT(!strcmp(isidConfig->getTagName(), "isidConfig"));
		checkTags(isidConfig, "Port");
		cXMLElementList list = isidConfig->getChildrenByTagName("Port");
		//Looking for the IComponent info using the index number.
		for (cXMLElementList::iterator iter=list.begin(); iter != list.end(); iter++)
		{
			const cXMLElement& Port = **iter;
			int ind=getParameterIntValue(&Port, "index");
			if((unsigned) ind<ISIDregisterTable.size())
			{
				ev<<endl<<endl<<"Reading Port "<<ind<<" info"<<endl;
				cXMLElement * ISids = Port.getFirstChildWithTag("ISids");
				cXMLElementList ISidList=ISids->getChildrenByTagName("ISid");
				//Getting ISid info
				for(unsigned int k=0;k<ISidList.size();k++)
				{
					int ISid=atoi(ISidList[k]->getNodeValue());
					ev<<"Register."<<ISid<<endl;
					ISIDregisterTable[ind].push_back(ISid);
				}
			}
			else
			{
				ev<<"Index out of bounds"<<endl;
			}
		}
}

void Cache1AH::flush(int Gate)
{//Clears gate MAC cache
	char buf[50];
	sprintf(buf,"Clearing gate %d cache.",Gate);
	Enter_Method(buf);
	for(unsigned int i=0;i<RelayTable.size();i++)
	{
		if(RelayTable[i].Gate==Gate)
		{
			RelayTable.erase(RelayTable.begin()+i);
			i--; //Keep position.
		}
	}
}

bool Cache1AH::resolveMAC(vid BVid, vid ISid, MACAddress MACDest,std::vector<int> * outputPorts)
{//It generates a vector with the ports where relay component should deliver the message. Returns false if not found
	Enter_Method("Obtaining output ports");
	cleanAgedEntries();
	bool result=false;
	outputPorts->clear();
	for(unsigned int i=0; i<RelayTable.size(); i++)
	{
		if(BVid != RelayTable[i].BVid)
			continue;
		if(ISid != RelayTable[i].ISid)
			continue;
		if(MACDest != RelayTable[i].MAC)
			continue;
		outputPorts->push_back(RelayTable[i].Gate);
		result=true;
	}
	return result;
}

bool Cache1AH::registerMAC(vid BVid, vid ISid, MACAddress MAC, int Gate)
{ //Register a new MAC at RelayTable. Returns true if refreshed or false if it is new.
	bool exists=false;
	for(unsigned int i=0; i<RelayTable.size(); i++)
	{//Looking for the entry
		if((BVid == RelayTable[i].BVid)&&(ISid ==RelayTable[i].ISid)&&(MAC == RelayTable[i].MAC)&&(Gate==RelayTable[i].Gate))
		{
			RelayTable[i].inserted=simulation.getSimTime();
			exists=true;
		}
	}
	if(exists==false)
	{//It is a new entry.
		Enter_Method("Registering MAC");
		AhRelayEntry Entry;
		Entry.MAC=MAC;
		Entry.BVid=BVid;
		Entry.ISid=ISid;
		Entry.Gate=Gate;
		Entry.inserted = simulation.getSimTime();
		RelayTable.push_back(Entry);
	}
	return exists;
}

void Cache1AH::printState()
{// Prints verbose information
	ev<<endl<<"RelayTable"<<endl;
	ev<<"BVid	ISid    MAC    Gate    Inserted"<<endl;
	for(unsigned int i=0;i<RelayTable.size();i++)
	{
		ev<<RelayTable[i].BVid<<"   "<<RelayTable[i].ISid<<"    "<<RelayTable[i].MAC<<"    "<<RelayTable[i].Gate<<"    "<<RelayTable[i].inserted<<endl;
	}

	for(unsigned int i=0;i<ISIDregisterTable.size();i++)
	{
		ev<<endl<<"Port: "<<i<<"  Registered ISIDs:";
		for(unsigned int j=0;j<ISIDregisterTable[i].size();j++)
		{
			ev<<"  "<<ISIDregisterTable[i][j];
		}
		ev<<endl;
	}
}

bool Cache1AH::isRegistered(vid ISid, int port)
{ //Check ISid registration at port.
	bool found=false;
	if((unsigned)port<ISIDregisterTable.size())
	{
		for(unsigned int i=0;i<ISIDregisterTable[port].size();i++)
		{
			if(ISIDregisterTable[port][i]==ISid)
			{
				found=true;
				break;
			}
		}
	}
	else
	{
		error("Wrong Cache1AH configuration. Asking for a not initialized port.");
	}
	return found;
}

void Cache1AH::cpCache(int a, int b)
{//Copy cache from port a to port b.
	for(unsigned int i=0;i<RelayTable.size();i++)
	{
		if(RelayTable[i].Gate==a)
		{
			AhRelayEntry Entry;
			Entry.MAC=RelayTable[i].MAC;
			Entry.BVid=RelayTable[i].BVid;
			Entry.ISid=RelayTable[i].ISid;
			Entry.Gate=b;
			Entry.inserted = RelayTable[i].inserted;
			RelayTable.push_back(Entry);
		}
	}
}


