//
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2010-2012 Thomas Dreibholz
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/transportlayer/sctp/SCTPSendStream.h"

#include "inet/transportlayer/contract/sctp/SCTPCommand_m.h"

namespace inet {

namespace sctp {

SCTPSendStream::SCTPSendStream(const uint16 id)
{
    streamId = id;
    nextStreamSeqNum = 0;

    char queueName[64];
    snprintf(queueName, sizeof(queueName), "OrderedSendQueue ID %d", id);
    streamQ = new cPacketQueue(queueName);
    snprintf(queueName, sizeof(queueName), "UnorderedSendQueue ID %d", id);
    uStreamQ = new cPacketQueue(queueName);
}

SCTPSendStream::~SCTPSendStream()
{
    deleteQueue();
}

void SCTPSendStream::deleteQueue()
{
    SCTPDataMsg *datMsg;
    SCTPSimpleMessage *smsg;
    int32 count = streamQ->getLength();
    while (!streamQ->isEmpty()) {
        datMsg = check_and_cast<SCTPDataMsg *>(streamQ->pop());
        smsg = check_and_cast<SCTPSimpleMessage *>(datMsg->decapsulate());
        delete smsg;
        delete datMsg;
        count--;
    }
    while (!uStreamQ->isEmpty()) {
        datMsg = check_and_cast<SCTPDataMsg *>(uStreamQ->pop());
        smsg = check_and_cast<SCTPSimpleMessage *>(datMsg->decapsulate());
        delete smsg;
        delete datMsg;
    }
    delete streamQ;
    delete uStreamQ;
}

} // namespace sctp

} // namespace inet

