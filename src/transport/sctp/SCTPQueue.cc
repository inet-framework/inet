//
// Copyright (C) 2008 Irene Ruengeler
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


#include "SCTPQueue.h"
#include "SCTPAssociation.h"

Register_Class(SCTPQueue);


SCTPQueue::SCTPQueue()
{
assoc=NULL;
}

SCTPQueue::~SCTPQueue()
{
	for (PayloadQueue::iterator i = payloadQueue.begin(); i!=payloadQueue.end(); i++)
		delete i->second->userData;
	
	if (!payloadQueue.empty())
		payloadQueue.clear();
}


/*uint32 SCTPQueue::insertMessage(uint32 key,SCTPDataVariables *datVar)
{

	payloadQueue[key]=datVar;
	printQueue();
	return payloadQueue.size();
}*/

bool SCTPQueue::checkAndInsertVar(uint32 key,SCTPDataVariables *datVar)
{
	PayloadQueue::iterator i = payloadQueue.find(key);
	if (i!=payloadQueue.end())
	{
		return false;
	}
	else
	{
		payloadQueue[key]=datVar;
	}
		
	return true;
}

uint32 SCTPQueue::getQueueSize()
{
	  return payloadQueue.size();
}

SCTPDataVariables *SCTPQueue::extractMessage()
{	
	if (!payloadQueue.empty())
	{
		PayloadQueue::iterator i = payloadQueue.begin();
		SCTPDataVariables *datVar = i->second;
		payloadQueue.erase(i);
		return datVar;
	}
	else
		 
		sctpEV3<<"Queue is empty\n";
		
	 return NULL;
}

SCTPDataVariables *SCTPQueue::getAndExtractMessage(uint32 tsn)
{
	if (!payloadQueue.empty())
	{
		PayloadQueue::iterator i = payloadQueue.find(tsn);
		SCTPDataVariables *datVar = i->second;
		payloadQueue.erase(i);
		return datVar;
	}
	else
		 
		sctpEV3<<"Queue is empty\n";
		
	 return NULL;
}

void SCTPQueue::printQueue()
{
	SCTPDataVariables* datVar;
	uint32	key;
	for (PayloadQueue::iterator i = payloadQueue.begin(); i!=payloadQueue.end(); ++i)
	{
		datVar = i->second;
		key = i->first;	
		sctpEV3<<key<<"\t";
	}
	 
	sctpEV3<<"\n";
	
}


 SCTPDataVariables* SCTPQueue::getFirstVar()
{
	PayloadQueue::iterator pl=payloadQueue.begin();
	SCTPDataVariables * datVar=pl->second;

	return datVar;
}

 cMessage* SCTPQueue::getMsg(uint32 tsn)
{
	PayloadQueue::iterator pl=payloadQueue.find(tsn);
	SCTPDataVariables * datVar=pl->second;
	cMessage* smsg=check_and_cast<cMessage*>(datVar->userData);
	return smsg;
}

SCTPDataVariables* SCTPQueue::getVar(uint32 tsn)
{
	PayloadQueue::iterator pl=payloadQueue.find(tsn);
	if (pl!=payloadQueue.end())
		return (pl->second);
	else
		return NULL;
}

SCTPDataVariables* SCTPQueue::getNextVar(uint32 tsn, uint32 toTsn)
{
	
	for (uint32 i=tsn+1; i<toTsn; i++)
	{
		PayloadQueue::iterator pl=payloadQueue.find(i);
		if (pl!=payloadQueue.end())
		{
			return (pl->second);
		}
	}
	return NULL;
}



void SCTPQueue::removeMsg(uint32 tsn)
{
	PayloadQueue::iterator pl=payloadQueue.find(tsn);
	//delete check_and_cast<SCTPSimpleMessage*>(pl->second->userData);
	payloadQueue.erase(pl);
}

bool SCTPQueue::deleteMsg(uint32 tsn)
{
	PayloadQueue::iterator pl=payloadQueue.find(tsn);
	if (pl!=payloadQueue.end())
	{
		delete check_and_cast<SCTPSimpleMessage*>(pl->second->userData);
		payloadQueue.erase(pl);
		return true;
	}
	return false;
}

int32 SCTPQueue::getNumBytes()
{
	int32 qb=0;

	for (PayloadQueue::iterator i=payloadQueue.begin(); i!=payloadQueue.end(); i++)
		qb+=i->second->len/8;

	return qb;
}

SCTPDataVariables* SCTPQueue::dequeueVarBySsn(uint16 ssn)
{
	SCTPDataVariables* datVar;
	for (PayloadQueue::iterator i=payloadQueue.begin();i!=payloadQueue.end(); i++)
	{
		if (i->second->ssn==ssn)
		{
			datVar = i->second;
			payloadQueue.erase(i);
			return datVar;
		}
	}
	sctpEV3<<"ssn "<<ssn<<" not found\n";
	return NULL;
}
