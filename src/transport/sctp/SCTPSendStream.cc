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

#include "SCTPSendStream.h"

SCTPSendStream::SCTPSendStream(uint16 id)
{

	streamId = id;
	nextStreamSeqNum = 0;
	char str[50];
	sprintf(str, "OrderedSendQueue ID %d",id);
	streamQ = new cQueue(str);
	sprintf(str, "UnorderedSendQueue ID %d",id);
	uStreamQ = new cQueue(str);
}

SCTPSendStream::~SCTPSendStream()
{
	deleteQueue();
}

void SCTPSendStream::deleteQueue()
{
SCTPDataMsg* datMsg;
SCTPSimpleMessage* smsg;
int32 count = streamQ->length();
	while (!streamQ->empty())
	{
		datMsg = check_and_cast<SCTPDataMsg*>(streamQ->pop());
		smsg = check_and_cast<SCTPSimpleMessage*>(datMsg->decapsulate());
		delete smsg;
		delete datMsg;	
		count--;
	}
	while (!uStreamQ->empty())
	{
		datMsg = check_and_cast<SCTPDataMsg*>(uStreamQ->pop());
		smsg = check_and_cast<SCTPSimpleMessage*>(datMsg->decapsulate());
		delete smsg;
		delete datMsg;		
	}
	delete streamQ;
	delete uStreamQ;
}
