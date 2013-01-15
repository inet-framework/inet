 /**
******************************************************
* @file Cache1Q.cc
* @brief Cached data base
* @author Juan Luis Garrote Molinero
* @version 1.0
* @date Feb 2011
******************************************************/
#include "Cache1Q.h"
#include "XMLUtils.h"



Define_Module( Cache1Q );



Cache1Q::Cache1Q(){
}
Cache1Q::~Cache1Q(){
}


void Cache1Q::initialize()
{
	agingTime=(simtime_t) par("agingTime");
	verbose=(bool) par ("verbose");

}

void Cache1Q::handleMessage(cMessage *)
{
        opp_error("This module doesn't process messages");
}




/*
 * RESOLVEMAC
 * For a known arriving port, V-TAG and destination MAC. It generates a vector with the ports where relay component
 * should deliver the message.
 * returns false if not found
 */
bool Cache1Q::resolveMAC(vid Vid, MACAddress MACDest,std::vector<int> * outputPorts)
{
	Enter_Method("Obtaining output ports");
	cleanAgedEntries();
	bool result=false;
	outputPorts->clear();
	for(unsigned int i=0; i<RelayTable.size(); i++)
	{
		if(Vid != RelayTable[i].Vid)
			continue;
		if(MACDest != RelayTable[i].MAC)
			continue;
		outputPorts->push_back(RelayTable[i].Gate);
		result=true;
	}
	return result;
}


/*
 * REGISTERMAC
 * Register a new MAC at RelayTable.
 * True if refreshed. False if it is new.
 */
bool Cache1Q::registerMAC (vid Vid, MACAddress MAC, int Gate)
{
	bool exists=false;
	if(!MAC.isBroadcast())
	{
		for(unsigned int i=0; i<RelayTable.size(); i++)
		{//Looking for the entry
			if((Vid == RelayTable[i].Vid)&&(MAC == RelayTable[i].MAC)&&(Gate==RelayTable[i].Gate))
			{
				RelayTable[i].inserted=simulation.getSimTime();
				exists=true;
			}
		}
		if(exists==false)
		{//It is a new entry.
			Enter_Method("Registering MAC");
			RelayEntry Entry;
			Entry.MAC=MAC;
			Entry.Vid=Vid;
			Entry.Gate=Gate;
			Entry.inserted = simulation.getSimTime();
			RelayTable.push_back(Entry);
		}
	}
	return exists;
}


/*
 * Clears gate MAC cache.
 */
void Cache1Q::flush(int Gate)
{
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

/*
 * Prints verbose information
 */

void Cache1Q::printState()
{
	ev<<endl<<"RelayTable"<<endl;
	ev<<"Vid    MAC    Gate    Inserted"<<endl;
	for(unsigned int i=0;i<RelayTable.size();i++)
	{
		ev<<RelayTable[i].Vid<<"   "<<RelayTable[i].MAC<<"    "<<RelayTable[i].Gate<<"    "<<RelayTable[i].inserted<<endl;
	}
}


void Cache1Q::cpCache(int a, int b)
{ //Copy from a to b
	for(unsigned int i=0;i<RelayTable.size();i++)
	{
		if(RelayTable[i].Gate==a)
		{
			RelayEntry Entry;
			Entry.MAC=RelayTable[i].MAC;
			Entry.Vid=RelayTable[i].Vid;
			Entry.Gate=b;
			Entry.inserted = RelayTable[i].inserted;
			RelayTable.push_back(Entry);
		}
	}
}

void Cache1Q::cleanAgedEntries()
{
	for(unsigned int i=0;i<RelayTable.size();i++)
	{
		if((simulation.getSimTime()-(SimTime)(RelayTable[i].inserted)) > (SimTime) agingTime)
		{//Delete entry
			if(verbose==true)
				ev<<"Deleting old entry."<<endl;
			RelayTable.erase(RelayTable.begin()+i);
			i--;
		}
	}

}

