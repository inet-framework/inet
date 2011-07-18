//
// Copyright (C) 2011 Zoltan Bojthe
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


#include "ResultFilters.h"

Register_ResultFilter("messageAge", MessageAgeFilter);

void MessageAgeFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object)
{
    if (dynamic_cast<cMessage *>(object))
    {
        cMessage *msg = (cMessage *)object;
        fire(this, t, t - msg->getCreationTime());
    }
}

//---

Register_ResultFilter("messageTSAge", MessageTSAgeFilter);

void MessageTSAgeFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object)
{
    if (dynamic_cast<cMessage *>(object))
    {
        cMessage *msg = (cMessage *)object;
        fire(this, t, t - msg->getTimestamp());
    }
}

