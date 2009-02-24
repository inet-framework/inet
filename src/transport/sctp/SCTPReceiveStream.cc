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

#include "SCTPReceiveStream.h"
#include <iostream>

SCTPReceiveStream::SCTPReceiveStream()
{
	streamId = 0;
	expectedStreamSeqNum = 0;
	deliveryQ = new SCTPQueue();
	orderedQ = new SCTPQueue();
	unorderedQ = new SCTPQueue();
}

SCTPReceiveStream::~SCTPReceiveStream()
{
	delete deliveryQ;
	delete orderedQ;
	delete unorderedQ;
}


uint32 SCTPReceiveStream::enqueueNewDataChunk(SCTPDataVariables* dchunk)
{
uint32 delivery = 0;	//0:orderedQ=false && deliveryQ=false; 1:orderedQ=true && deliveryQ=false; 2:oderedQ=true && deliveryQ=true

	 SCTPDataVariables* chunk;
	/* append to the respective queue */
	if (!dchunk->ordered) 
	{
		/* put message into the streams ->unorderedQ */
		if (deliveryQ->checkAndInsertVar(dchunk->tsn, dchunk))
		{
			delivery = 2;
		}
		
	} 
	else if (dchunk->ordered) 
	{
		/* put message into streams ->reassembyQ */
		if (orderedQ->checkAndInsertVar(dchunk->tsn, dchunk))
			delivery++;
		if (orderedQ->getQueueSize()>0) 
		{
			/* dequeue first from orderedQ */
			
			chunk = orderedQ-> dequeueVarBySsn(expectedStreamSeqNum);
			if (chunk)
			{	 
				if (deliveryQ->checkAndInsertVar(chunk->tsn, chunk))
				{ 					
					++expectedStreamSeqNum;
					if (expectedStreamSeqNum > 65535) 
						expectedStreamSeqNum = 0;
					delivery++;
				}
			}
		}
		
	} 
	
	return delivery;
}
