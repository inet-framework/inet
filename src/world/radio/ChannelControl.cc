/***************************************************************************
 * file:        ChannelControl.cc
 *
 * copyright:   (C) 2005 Andras Varga
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 **************************************************************************/


#include "ChannelControl.h"
#include "FWMath.h"
#include <cassert>

#include "AirFrame_m.h"

#define coreEV (ev.isDisabled()||!coreDebug) ? ev : ev << "ChannelControl: "

Define_Module(ChannelControl);


std::ostream& operator<<(std::ostream& os, const ChannelControl::RadioEntry& radio)
{
    os << radio.radioModule->getFullPath() << " (x=" << radio.pos.x << ",y=" << radio.pos.y << "), "
       << radio.neighbors.size() << " neighbor(s)";
    return os;
}

std::ostream& operator<<(std::ostream& os, const ChannelControl::TransmissionList& tl)
{
    for (ChannelControl::TransmissionList::const_iterator it = tl.begin(); it != tl.end(); ++it)
        os << endl << *it;
    return os;
}

ChannelControl::ChannelControl()
{
}

ChannelControl::~ChannelControl()
{
    for (unsigned int i = 0; i < transmissions.size(); i++)
        for (TransmissionList::iterator it = transmissions[i].begin(); it != transmissions[i].end(); it++)
            delete *it;
}

/**
 * Calculates maxInterferenceDistance.
 *
 * @ref calcInterfDist
 */
void ChannelControl::initialize()
{
    coreDebug = hasPar("coreDebug") ? (bool) par("coreDebug") : false;

    coreEV << "initializing ChannelControl\n";

    numChannels = par("numChannels");
    transmissions.resize(numChannels);

    lastOngoingTransmissionsUpdate = 0;

    maxInterferenceDistance = calcInterfDist();

    WATCH(maxInterferenceDistance);
    WATCH_LIST(radios);
    WATCH_VECTOR(transmissions);
}

/**
 * Calculation of the interference distance based on the transmitter
 * power, wavelength, pathloss coefficient and a threshold for the
 * minimal receive Power
 *
 * You may want to overwrite this function in order to do your own
 * interference calculation
 */
double ChannelControl::calcInterfDist()
{
    double interfDistance;

    //the carrier frequency used
    double carrierFrequency = par("carrierFrequency");
    //maximum transmission power possible
    double pMax = par("pMax");
    //signal attenuation threshold
    double sat = par("sat");
    //path loss coefficient
    double alpha = par("alpha");

    double waveLength = (SPEED_OF_LIGHT / carrierFrequency);
    //minimum power level to be able to physically receive a signal
    double minReceivePower = pow(10.0, sat / 10.0);

    interfDistance = pow(waveLength * waveLength * pMax /
                         (16.0 * M_PI * M_PI * minReceivePower), 1.0 / alpha);

    coreEV << "max interference distance:" << interfDistance << endl;

    return interfDistance;
}

ChannelControl::RadioRef ChannelControl::registerRadio(cModule *radio, cGate *radioInGate)
{
    Enter_Method_Silent();

    RadioRef radioRef = lookupRadio(radio);

    if (radioRef)
        throw cRuntimeError("Radio %s already registered", radio->getFullPath().c_str());

    if (!radioInGate)
        radioInGate = radio->gate("radioIn");

    RadioEntry re;
    re.radioModule = radio;
    re.radioInGate = radioInGate->getPathStartGate();
    re.isNeighborListValid = false;
    re.channel = 0;  // for now
    re.isActive = true;
    radios.push_back(re);
    return &radios.back(); // last element
}

void ChannelControl::unregisterRadio(RadioRef r)
{
    Enter_Method_Silent();
    for (RadioList::iterator it = radios.begin(); it != radios.end(); it++)
    {
        if (it->radioModule == r->radioModule)
        {
            RadioRef radioToRemove = &*it;
            // erase radio from all registered radios' neighbor list
            for (RadioList::iterator i2 = radios.begin(); i2 != radios.end(); ++i2)
            {
                RadioRef otherRadio = &*i2;
                otherRadio->neighbors.erase(radioToRemove);
                otherRadio->isNeighborListValid = false;
                radioToRemove->isNeighborListValid = false;
            }

            // erase radio from registered radios
            radios.erase(it);
            return;
        }
    }

    error("unregisterRadio failed: no such radio");
}

ChannelControl::RadioRef ChannelControl::lookupRadio(cModule *radio)
{
    Enter_Method_Silent();
    for (RadioList::iterator it = radios.begin(); it != radios.end(); it++)
        if (it->radioModule == radio)
            return &(*it);
    return 0;
}

const ChannelControl::RadioRefVector& ChannelControl::getNeighbors(RadioRef h)
{
    Enter_Method_Silent();
    if (!h->isNeighborListValid)
    {
        h->neighborList.clear();
        for (std::set<RadioRef,RadioEntry::Compare>::iterator it = h->neighbors.begin(); it != h->neighbors.end(); it++)
            h->neighborList.push_back(*it);
        h->isNeighborListValid = true;
    }
    return h->neighborList;
}

void ChannelControl::updateConnections(RadioRef h)
{
    Coord& hpos = h->pos;
    double maxDistSquared = maxInterferenceDistance * maxInterferenceDistance;
    for (RadioList::iterator it = radios.begin(); it != radios.end(); ++it)
    {
        RadioEntry *hi = &(*it);
        if (hi == h)
            continue;

        // get the distance between the two radios.
        // (omitting the square root (calling sqrdist() instead of distance()) saves about 5% CPU)
        bool inRange = hpos.sqrdist(hi->pos) < maxDistSquared;

        if (inRange)
        {
            // nodes within communication range: connect
            if (h->neighbors.insert(hi).second == true)
            {
                hi->neighbors.insert(h);
                h->isNeighborListValid = hi->isNeighborListValid = false;
            }
        }
        else
        {
            // out of range: disconnect
            if (h->neighbors.erase(hi))
            {
                hi->neighbors.erase(h);
                h->isNeighborListValid = hi->isNeighborListValid = false;
            }
        }
    }
}

void ChannelControl::checkChannel(int channel)
{
    if (channel >= numChannels || channel < 0)
        error("Invalid channel, must above 0 and below %d", numChannels);
}

void ChannelControl::setRadioPosition(RadioRef r, const Coord& pos)
{
    Enter_Method_Silent();
    r->pos = pos;
    updateConnections(r);
}

void ChannelControl::setRadioChannel(RadioRef r, int channel)
{
    Enter_Method_Silent();
    checkChannel(channel);

    r->channel = channel;
}

const ChannelControl::TransmissionList& ChannelControl::getOngoingTransmissions(int channel)
{
    Enter_Method_Silent();

    checkChannel(channel);
    purgeOngoingTransmissions();
    return transmissions[channel];
}

void ChannelControl::addOngoingTransmission(RadioRef h, AirFrame *frame)
{
    Enter_Method_Silent();

    // we only keep track of ongoing transmissions so that we can support
    // NICs switching channels -- so there's no point doing it if there's only
    // one channel
    if (numChannels == 1)
    {
        delete frame;
        return;
    }

    // purge old transmissions from time to time
    if (simTime() - lastOngoingTransmissionsUpdate > TRANSMISSION_PURGE_INTERVAL)
    {
        purgeOngoingTransmissions();
        lastOngoingTransmissionsUpdate = simTime();
    }

    // register ongoing transmission
    take(frame);
    frame->setTimestamp(); // store time of transmission start
    transmissions[frame->getChannelNumber()].push_back(frame);
}

void ChannelControl::purgeOngoingTransmissions()
{
    for (int i = 0; i < numChannels; i++)
    {
        for (TransmissionList::iterator it = transmissions[i].begin(); it != transmissions[i].end();)
        {
            TransmissionList::iterator curr = it;
            AirFrame *frame = *it;
            it++;

            if (frame->getTimestamp() + frame->getDuration() + TRANSMISSION_PURGE_INTERVAL < simTime())
            {
                delete frame;
                transmissions[i].erase(curr);
            }
        }
    }
}

void ChannelControl::sendToChannel(RadioRef srcRadio, AirFrame *airFrame)
{
    // NOTE: no Enter_Method()! We pretend this method is part of ChannelAccess

    // loop through all radios in range
    const RadioRefVector& neighbors = getNeighbors(srcRadio);
    int n = neighbors.size();
    int channel = airFrame->getChannelNumber();
    for (int i=0; i<n; i++)
    {
        RadioRef r = neighbors[i];
        if (!r->isActive)
        {
            coreEV << "skipping disabled radio interface \n";
            continue;
        }
        if (r->channel == channel)
        {
            coreEV << "sending message to radio listening on the same channel\n";
            // account for propagation delay, based on distance in meters
            // Over 300m, dt=1us=10 bit times @ 10Mbps
            simtime_t delay = srcRadio->pos.distance(r->pos) / LIGHT_SPEED;
            check_and_cast<cSimpleModule*>(srcRadio->radioModule)->sendDirect(airFrame->dup(), delay, airFrame->getDuration(), r->radioInGate);
        }
        else
            coreEV << "skipping radio listening on a different channel\n";
    }

    // register transmission
    addOngoingTransmission(srcRadio, airFrame);
}
