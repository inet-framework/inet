 /**
******************************************************
* @file ITagTable.h
* @brief IComponent module cached data base
* @author Juan Luis Garrote Molinero
* @version 1.0
* @date Feb 2011
******************************************************/

#include "ITagTable.h"
#include "XMLUtils.h"
#include <simtime.h>



Define_Module(ITagTable);


ITagTable::ITagTable(){}
ITagTable::~ITagTable(){}

void ITagTable::initialize(int stage)
{
	agingTime=par("agingTime");
}

void ITagTable::handleMessage(cMessage *)
{
	opp_error("This module doesn't process messages");
	return;
}


std::vector<vid> ITagTable::getSVids(std::vector<vid> ISids,int Gate)
{//Gets SVids associated to a Gate.
	std::vector<vid> SVids;
	if(isidtable.size()>0) //Just checking.
	{
		for(unsigned int i=0;i<ISids.size();i++)  //For requested ISids
		{

			for(unsigned int j=0;j<isidtable.size();j++) //Look for them in isidtable
			{
				if(isidtable[j].ISid==ISids[i])
				{
					bool found=false;
					for(unsigned int k=0;k<SVids.size();k++) //Check if they are in SVids. (Avoiding duplication)
					{
						if(SVids[k]==isidtable[j].local[Gate].SVid)
						{
							found=true;
							break;
						}
					}
					if(found==false)
						SVids.push_back(isidtable[j].local[Gate].SVid);
				}
			}
		}
	}
	return SVids;
}




bool ITagTable::registerCMAC(vid ISid, int Gate, vid SVid, MACAddress CMACs)
{////TOWARDS BB. Saves a new CMAC to an ISid local info or refreshes the insertion time.
 //It returns True if the Gate,SVid, CMAC were not in the table. If they weren't, they are registered.
	Enter_Method("Registering Client MAC");

	bool found=false;
	for(unsigned int i=0;i<isidtable.size();i++)
	{ //Looks for the entry
		if(isidtable[i].ISid==ISid)
		{

			for(unsigned int j=0;j<isidtable[i].local[Gate].cache.size();j++)
			{
				if(isidtable[i].local[Gate].cache[j].CMAC==CMACs)
				{
					//Refresh
					isidtable[i].local[Gate].cache[j].inserted=simulation.getSimTime();
					found=true;
					break;
				}
			}
			if(found==false)
			{//New entry. This client MAC was not known.
				cmac_time a;
				a.CMAC=CMACs;
				a.inserted=simulation.getSimTime();
				isidtable[i].local[Gate].cache.push_back(a);
			}
		}
	}
	return !found;
}

bool ITagTable::checkISid(vid ISid, int Gate, vid SVid)
{//TOWARDS BB
 //checkISid. Is ISID associated to a known SVid/Gate?
	char buf[50];
	sprintf(buf,"Checking ISid %d.",ISid);
	Enter_Method(buf);
	bool result=false;

	for(unsigned int i=0;i<isidtable.size();i++)
	{//Looks for that entry.
		if(isidtable[i].ISid==ISid)
		{
			if(isidtable[i].local[Gate].SVid==SVid)
			{
				result=true;
			}
			break;
		}
	}
	return result;
}

bool ITagTable::resolveBMAC(vid ISid, MACAddress CMACd, MACAddress * BMACd)
{//TOWARDS BB
 //resolveBMAC  It gets the backbone MAC where a CMAC/ISid could be reached. (remote info)
	Enter_Method("resolving BMAC");
	bool result=false;

	for(unsigned int i=0;i<isidtable.size();i++)
	{ //Looks for the backbone MAC entry
		if(isidtable[i].ISid==ISid)
		{
			for(unsigned int j=0; j<isidtable[i].remote.size(); j++)
			{
				if(isidtable[i].remote[j].CMAC==CMACd)
				{
					*BMACd=isidtable[i].remote[j].BMAC;
					result=true;
					break;
				}
			}
		}
	}
	if(result==false) //If it is not found, sets broadcast.
	{
		//Construct ISid broadcast address (00-1E-83- ISId) Table 26-1 pag.93
		char BrAdd []= "00-00-00-00-00-00";
		int oct1=ISid&0xFF0000;
		int oct2=ISid&0xFF00;
		int oct3=ISid&0xFF;
		sprintf(BrAdd,"00-1E-83-%.2X-%.2X-%.2X",oct1,oct2,oct3);
		BMACd->setAddress(BrAdd);
	}

	return result;
}

bool ITagTable::registerBMAC (vid ISid, MACAddress CMACs, MACAddress BMACs)
{//TOWARDS CN. It Registers a new entry with data from the other side of te BB
 //True for a new entry. False if refreshing.
	Enter_Method("Registering BackBone MAC");
	bool found=false;

	for(unsigned int i=0;i<isidtable.size();i++)
	{//Look for that entry
		if(isidtable[i].ISid==ISid)
		{
			for(unsigned int j=0;j<isidtable[i].remote.size();j++)
			{
				if((isidtable[i].remote[j].BMAC==BMACs)&&(isidtable[i].remote[j].CMAC==CMACs))
				{//It refreshes the registered ones.
					found=true;
					isidtable[i].remote[j].inserted=simulation.getSimTime(); //Refreshing
					break;
				}
			}
			if(found==false)
			{//In case this entry is not found, register.
				cmac_bmac_time a;
				a.CMAC=CMACs;
				a.BMAC=BMACs;
				a.inserted=simulation.getSimTime();
				isidtable[i].remote.push_back(a);
			}

		}
	}
	return !found;
}

bool ITagTable::resolveGate (vid ISid, MACAddress CMAC, std::vector <int> * Gate, std::vector <int> * SVid)
{//TOWARDS CN. Look up for the Gate assigned to the ISid and CMAC
 //True for a perfect match. False if not found. When no entry is found Gate contains ISid associated ports.
	Enter_Method("Obtaining SVid and Gate");
	bool found = false;  // While the perfect match is not found, all gates registered for the ISid are selected.
	for(unsigned int i=0;i<isidtable.size();i++)
	{
		if(isidtable[i].ISid==ISid)
		{
			for(unsigned int j=0;j<isidtable[i].local.size();j++)
			{
				if(found==false)  //Not a perfect match yet. Adds the gate.
				{
					Gate->push_back(j);
					SVid->push_back(isidtable[i].local[j].SVid);
				}
				for(unsigned int k=0;k<isidtable[i].local[j].cache.size();k++)
				{//Take advantage to clean the table.
					if((simulation.getSimTime()-isidtable[i].local[j].cache[k].inserted)> agingTime)
					{
						isidtable[i].local[j].cache.erase(isidtable[i].local[j].cache.begin()+k); //K element deletion.
						continue;  // continues with next element.
					}
					if(isidtable[i].local[j].cache[k].CMAC==CMAC)
					{ //Perfect match
						if(found==false)    //If this is the first one, clean gate vector
						{
							found=true;
							Gate->clear();
							SVid->clear();
						}
						Gate->push_back(j);
						SVid->push_back(isidtable[i].local[j].SVid);
					}
				}
			}
			break;
		}
	}
	return found;
}


void ITagTable::printState()
{//Prints known data.

	ev<<endl<<"***[ITAGTABLE INFO]*** "<<endl;
	for(unsigned int i=0;i<isidtable.size();i++)
	{
		ev<<"ISid: "<<isidtable[i].ISid<<endl;
		ev<<"   Local:"<<endl;
		for(unsigned int j=0;j<isidtable[i].local.size();j++)
		{
			ev<<"      Gate: "<<j<<" SVid "<<isidtable[i].local[j].SVid<<endl;
			for(unsigned int k=0;k<isidtable[i].local[j].cache.size();k++)
			{
				ev<<"         CMAC: "<<isidtable[i].local[j].cache[k].CMAC.str()<<" time: "<<isidtable[i].local[j].cache[k].inserted<<endl;
			}
		}
		ev<<"   Remote:"<<endl;
		for(unsigned int j=0;j<isidtable[i].remote.size();j++)
		{
			ev<<"      BMAC: "<<isidtable[i].remote[j].BMAC.str()<<" CMAC: "<<isidtable[i].remote[j].CMAC.str()<<"  time: "<<isidtable[i].remote[j].inserted<<endl;
		}
	}
	ev<<"************************"<<endl;
}

/**

*/
bool ITagTable::createISid(vid ISid,unsigned int GateSize)
{// Creates a new ISid at isidtable
 //Used during initialization
	isidinfo a;
	a.ISid=ISid;
	a.local.clear();
	for(unsigned int i=0;i<GateSize;i++)
	{
		Local b;
		b.SVid=0;
		b.cache.clear();
		a.local.push_back(b);
	}
	isidtable.push_back(a);
	return true;
}


bool ITagTable::asociateSVid(vid ISid, unsigned int Gate, vid SVid)
{//Adds the asociation ISid/SVid
 //Used during initialization
	bool result=false;
	for(unsigned int i=0;i<isidtable.size();i++)
	{//Look for that entry
		if(isidtable[i].ISid==ISid)
		{
			if(Gate<isidtable[i].local.size())
			{//Sets the SVid that will be used with that gate.
			isidtable[i].local[Gate].SVid=SVid;
			result=true;
			}
			else
			{
				ev<<"Ignoring ISid/SVid association. Gate "<<Gate<<" does not exists for the ISid "<<ISid<<endl;
			}
		}
	}
	return result;
}


