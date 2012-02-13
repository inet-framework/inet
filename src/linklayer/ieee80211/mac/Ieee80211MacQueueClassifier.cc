//
// Copyright (C) 2005 Andras Varga
// Copyright (C) 2010 Alfonso Ariza
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

#include "Ieee80211MacQueueClassifier.h"
#include "Ieee80211Frame_m.h"

Register_Class(Ieee80211MacQueueClassifier);


int Ieee80211MacQueueClassifier::getNumQueues()
{
    return 3;
}

int Ieee80211MacQueueClassifier::classifyPacket(cMessage *msg)
{
    Ieee80211DataOrMgmtFrame *frame = dynamic_cast<Ieee80211DataOrMgmtFrame *>(msg);

    ASSERT(frame!=NULL);

    Ieee80211DataFrame *dataFrame = dynamic_cast<Ieee80211DataFrame *>(msg);
    if (dataFrame == NULL)
        return 0;

    if (dataFrame->getReceiverAddress() == MACAddress::BROADCAST_ADDRESS)
        return 1;
    return 2;
}

Register_Class(Ieee80211MacQueueClassifier2);

int Ieee80211MacQueueClassifier2::getNumQueues()
{
    return 2;
}

int Ieee80211MacQueueClassifier2::classifyPacket(cMessage *msg)
{
    Ieee80211DataOrMgmtFrame *frame = dynamic_cast<Ieee80211DataOrMgmtFrame *>(msg);

    ASSERT(frame!=NULL);

    Ieee80211DataFrame *dataFrame = dynamic_cast<Ieee80211DataFrame *>(msg);
    if (dataFrame == NULL)
        return 0;
     return 1;
}
