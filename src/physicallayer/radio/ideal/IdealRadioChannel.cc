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

#include "IdealRadioChannel.h"
#include "IdealRadio.h"

Define_Module(IdealRadioChannel);

std::ostream& operator<<(std::ostream& os, const IdealRadioChannel::RadioEntry& radioEntry)
{
    Coord pos = radioEntry.radio->getMobility()->getCurrentPosition();
    os << radioEntry.radioModule->getFullPath() << " (x=" << pos.x << ",y=" << pos.y << ")";
    return os;
}

void IdealRadioChannel::initialize(int stage)
{
    RadioChannelBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
    {
        EV << "initializing IdealRadioChannel" << endl;
        maxTransmissionRange = 0;
        WATCH_LIST(radios);
        WATCH(maxTransmissionRange);
    }
}

IdealRadioChannel::RadioEntry *IdealRadioChannel::registerRadio(cModule *radioModule)
{
    Enter_Method_Silent();
    RadioEntry *radioRef = lookupRadio(radioModule);
    if (radioRef)
        throw cRuntimeError("Radio %s already registered", radioModule->getFullPath().c_str());
    IdealRadio *idealRadio = check_and_cast<IdealRadio *>(radioModule);
    if (maxTransmissionRange < 0.0)    // invalid value
        recalculateMaxTransmissionRange();
    if (maxTransmissionRange < idealRadio->getTransmissionRange())
        maxTransmissionRange = idealRadio->getTransmissionRange();
    RadioEntry radioEntry;
    IRadio *radio = check_and_cast<IRadio *>(radioModule);
    radioEntry.radioModule = radioModule;
    radioEntry.radio = radio;
    radios.push_back(radioEntry);
    return &radios.back(); // last element
}

void IdealRadioChannel::recalculateMaxTransmissionRange()
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

void IdealRadioChannel::unregisterRadio(RadioEntry *r)
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
    throw cRuntimeError("unregisterRadio failed: no such radio");
}

IdealRadioChannel::RadioEntry *IdealRadioChannel::lookupRadio(cModule *radio)
{
    for (RadioList::iterator it = radios.begin(); it != radios.end(); it++)
        if (it->radioModule == radio)
            return &(*it);
    return NULL;
}

void IdealRadioChannel::sendToChannel(RadioEntry *srcRadio, IdealRadioFrame *radioFrame)
{
    // NOTE: no Enter_Method()! We pretend this method is part of ChannelAccess
    if (maxTransmissionRange < 0.0)    // invalid value
        recalculateMaxTransmissionRange();
    double sqrTransmissionRange = radioFrame->getTransmissionRange() * radioFrame->getTransmissionRange();
    double sqrInterferenceRange = radioFrame->getInterferenceRange() * radioFrame->getInterferenceRange();

    // loop through all radios
    for (RadioList::iterator it=radios.begin(); it !=radios.end(); ++it)
    {
        RadioEntry *r = &*it;
        if (r == srcRadio)
            continue;   // skip sender radio
        double sqrdist = srcRadio->radio->getMobility()->getCurrentPosition().sqrdist(r->radio->getMobility()->getCurrentPosition());
        if (sqrdist <= sqrTransmissionRange || sqrdist <= sqrInterferenceRange)
        {
            // account for propagation delay, based on distance in meters
            // Over 300m, dt=1us=10 bit times @ 10Mbps
            simtime_t delay = sqrt(sqrdist) / SPEED_OF_LIGHT;
            cGate *gate = const_cast<cGate *>(r->radio->getRadioGate()->getPathStartGate());
            check_and_cast<cSimpleModule*>(srcRadio->radioModule)->sendDirect(radioFrame->dup(), delay, radioFrame->getDuration(), gate);
        }
    }
    delete radioFrame;
}
