//
// Copyright (C) 2013 OpenSim Ltd
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
// author: Zoltan Bojthe
//

#include "IdealChannelModel.h"

#include "IdealRadio.h"


Define_Module(IdealChannelModel);


std::ostream& operator<<(std::ostream& os, const IdealChannelModel::RadioEntry& radio)
{
    os << radio.radioModule->getFullPath() << " (x=" << radio.pos.x << ",y=" << radio.pos.y << ")";
    return os;
}

IdealChannelModel::IdealChannelModel()
{
}

IdealChannelModel::~IdealChannelModel()
{
}

void IdealChannelModel::initialize()
{
    EV << "initializing IdealChannelModel" << endl;

    maxTransmissionRange = 0;

    WATCH_LIST(radios);
}

IdealChannelModel::RadioEntry *IdealChannelModel::registerRadio(cModule *radio, cGate *radioInGate)
{
    Enter_Method_Silent();

    RadioEntry *radioRef = lookupRadio(radio);

    if (radioRef)
        throw cRuntimeError("Radio %s already registered", radio->getFullPath().c_str());

    IdealRadio *idealRadio = check_and_cast<IdealRadio *>(radio);

    if (maxTransmissionRange < 0.0)    // invalid value
        recalculateMaxTransmissionRange();

    if (maxTransmissionRange < idealRadio->getTransmissionRange())
        maxTransmissionRange = idealRadio->getTransmissionRange();

    if (!radioInGate)
        radioInGate = radio->gate("radioIn");

    RadioEntry re;
    re.radioModule = radio;
    re.radioInGate = radioInGate->getPathStartGate();
    re.isActive = true;
    radios.push_back(re);
    return &radios.back(); // last element
}

void IdealChannelModel::recalculateMaxTransmissionRange()
{
    double newRange = 0.0;

    for (RadioList::iterator it = radios.begin(); it != radios.end(); ++it)
    {
        IdealRadio *idealRadio = check_and_cast<IdealRadio *>(it->radioModule);
        if (newRange < idealRadio->getTransmissionRange())
            newRange = idealRadio->getTransmissionRange();
    }
    maxTransmissionRange = newRange;
}

void IdealChannelModel::unregisterRadio(RadioEntry *r)
{
    Enter_Method_Silent();
    for (RadioList::iterator it = radios.begin(); it != radios.end(); ++it)
    {
        if (it->radioModule == r->radioModule)
        {
            // erase radio from registered radios
            radios.erase(it);
            maxTransmissionRange = -1.0;    // invalidate the value
            return;
        }
    }

    error("unregisterRadio failed: no such radio");
}

IdealChannelModel::RadioEntry *IdealChannelModel::lookupRadio(cModule *radio)
{
    for (RadioList::iterator it = radios.begin(); it != radios.end(); it++)
        if (it->radioModule == radio)
            return &(*it);
    return NULL;
}

void IdealChannelModel::setRadioPosition(RadioEntry *r, const Coord& pos)
{
    r->pos = pos;
}

void IdealChannelModel::sendToChannel(RadioEntry *srcRadio, IdealAirFrame *airFrame)
{
    // NOTE: no Enter_Method()! We pretend this method is part of ChannelAccess

    if (maxTransmissionRange < 0.0)    // invalid value
        recalculateMaxTransmissionRange();

    double sqrTransmissionRange = airFrame->getTransmissionRange()*airFrame->getTransmissionRange();

    // loop through all radios
    for (RadioList::iterator it=radios.begin(); it !=radios.end(); ++it)
    {
        RadioEntry *r = &*it;
        if (r == srcRadio)
            continue;   // skip sender radio

        if (!r->isActive)
            continue;   // skip disabled radio interfaces

        double sqrdist = srcRadio->pos.sqrdist(r->pos);
        if (sqrdist <= sqrTransmissionRange)
        {
            // account for propagation delay, based on distance in meters
            // Over 300m, dt=1us=10 bit times @ 10Mbps
            simtime_t delay = sqrt(sqrdist) / SPEED_OF_LIGHT;
            check_and_cast<cSimpleModule*>(srcRadio->radioModule)->sendDirect(airFrame->dup(), delay, airFrame->getDuration(), r->radioInGate);
        }
    }
    delete airFrame;
}

