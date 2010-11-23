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


#define coreEV (ev.isDisabled()||!coreDebug) ? ev : ev << "ChannelControl: "

Define_Module(ChannelControl);


std::ostream& operator<<(std::ostream& os, const ChannelControl::HostEntry& h)
{
    os << h.host->getFullPath() << " (x=" << h.pos.x << ",y=" << h.pos.y << "), "
       << h.neighbors.size() << " neighbor(s)";
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

ChannelControl *ChannelControl::get()
{
    ChannelControl *cc = dynamic_cast<ChannelControl *>(simulation.getModuleByPath("channelcontrol"));
    if (!cc)
        cc = dynamic_cast<ChannelControl *>(simulation.getModuleByPath("channelControl"));
    if (!cc)
        throw cRuntimeError("Could not find ChannelControl module");
    return cc;
}

/**
 * Sets up the playgroundSize and calculates the
 * maxInterferenceDistance
 *
 * @ref calcInterfDist
 */
void ChannelControl::initialize()
{
    coreDebug = hasPar("coreDebug") ? (bool) par("coreDebug") : false;

    coreEV << "initializing ChannelControl\n";

    playgroundSize.x = par("playgroundSizeX");
    playgroundSize.y = par("playgroundSizeY");

    numChannels = par("numChannels");
    transmissions.resize(numChannels);

    lastOngoingTransmissionsUpdate = 0;

    maxInterferenceDistance = calcInterfDist();

    WATCH(maxInterferenceDistance);
    WATCH_LIST(hosts);
    WATCH_VECTOR(transmissions);

    updateDisplayString(getParentModule());
}

/**
 * Sets up background size by adding the following tags:
 * "p=0,0;b=$playgroundSizeX,$playgroundSizeY"
 */
void ChannelControl::updateDisplayString(cModule *playgroundMod)
{
    cDisplayString& d = playgroundMod->getDisplayString();
    d.setTagArg("bgp", 0, 0L);
    d.setTagArg("bgp", 1, 0L);
    d.setTagArg("bgb", 0, (long) playgroundSize.x);
    d.setTagArg("bgb", 1, (long) playgroundSize.y);
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
    double SPEED_OF_LIGHT = 300000000.0;
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

ChannelControl::HostRef ChannelControl::registerHost(cModule *host, const Coord& initialPos, cGate *radioInGate)
{
    Enter_Method_Silent();
    if (lookupHost(host) != NULL)
        error("ChannelControl::registerHost(): host (%s)%s already registered",
              host->getClassName(), host->getFullPath().c_str());
    if (!radioInGate)
        radioInGate = host->gate("radioIn"); // throws error if gate does not exist

    HostEntry he;
    he.host = host;
    he.radioInGate = radioInGate;
    he.pos = initialPos;
    he.isNeighborListValid = false;
    he.channel = 0;  // for now
    hosts.push_back(he);
    return &hosts.back(); // last element
}

void ChannelControl::unregisterHost(cModule *host)
{
    Enter_Method_Silent();
    for (HostList::iterator it = hosts.begin(); it != hosts.end(); it++) {
        if (it->host == host) {
            HostRef h = &*it;

            // erase host from all registered hosts' neighbor list
            for (HostList::iterator i2 = hosts.begin(); i2 != hosts.end(); ++i2) {
                HostRef h2 = &*i2;
                h2->neighbors.erase(h);
                h2->isNeighborListValid = false;
                h->isNeighborListValid = false;
            }

            // erase host from registered hosts
            hosts.erase(it);
            return;
        }
    }
    error("unregisterHost failed: no such host");
}

ChannelControl::HostRef ChannelControl::lookupHost(cModule *host)
{
    Enter_Method_Silent();
    for (HostList::iterator it = hosts.begin(); it != hosts.end(); it++)
        if (it->host == host)
            return &(*it);
    return 0;
}

const ChannelControl::HostRefVector& ChannelControl::getNeighbors(HostRef h)
{
    Enter_Method_Silent();
    if (!h->isNeighborListValid)
    {
        h->neighborList.clear();
        for (std::set<HostRef>::const_iterator it = h->neighbors.begin(); it != h->neighbors.end(); it++)
            h->neighborList.push_back(*it);
        h->isNeighborListValid = true;
    }
    return h->neighborList;
}

void ChannelControl::updateConnections(HostRef h)
{
    Coord& hpos = h->pos;
    double maxDistSquared = maxInterferenceDistance * maxInterferenceDistance;
    for (HostList::iterator it = hosts.begin(); it != hosts.end(); ++it)
    {
        HostEntry *hi = &(*it);
        if (hi == h)
            continue;

        // get the distance between the two hosts.
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

void ChannelControl::checkChannel(const int channel)
{
    if (channel >= numChannels || channel < 0)
        error("Invalid channel, must be between 0 and %d", numChannels);
}

void ChannelControl::updateHostPosition(HostRef h, const Coord& pos)
{
    Enter_Method_Silent();
    h->pos = pos;
    updateConnections(h);
}

void ChannelControl::updateHostChannel(HostRef h, const int channel)
{
    Enter_Method_Silent();
    checkChannel(channel);

    h->channel = channel;
}

const ChannelControl::TransmissionList& ChannelControl::getOngoingTransmissions(const int channel)
{
    Enter_Method_Silent();

    checkChannel(channel);
    purgeOngoingTransmissions();
    return transmissions[channel];
}

void ChannelControl::addOngoingTransmission(HostRef h, AirFrame *frame)
{
    Enter_Method_Silent();

    // we only keep track of ongoing transmissions so that we can support
    // NICs switching channels -- so there's no point doing it if there's only
    // one channel
    if (numChannels==1)
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

void ChannelControl::sendToChannel(cSimpleModule *srcRadioMod, HostRef srcHost, AirFrame *airFrame)
{
    // NOTE: no Enter_Method()! We pretend this method is part of ChannelAccess

    // loop through all hosts in range
    const HostRefVector& neighbors = getNeighbors(srcHost);
    int n = neighbors.size();
    int channel = airFrame->getChannelNumber();
    for (int i=0; i<n; i++)
    {
        HostRef h = neighbors[i];
        if (h->channel == channel)
        {
            coreEV << "sending message to host listening on the same channel\n";
            // account for propagation delay, based on distance in meters
            // Over 300m, dt=1us=10 bit times @ 10Mbps
            simtime_t delay = srcHost->pos.distance(h->pos) / LIGHT_SPEED;
            srcRadioMod->sendDirect(airFrame->dup(), delay, airFrame->getDuration(), h->radioInGate);
        }
        else
            coreEV << "skipping host listening on a different channel\n";
    }

    // register transmission
    addOngoingTransmission(srcHost, airFrame);
}


const ChannelControl::HostRef ChannelControl::lookupHostByName(const char *name)
{
    Enter_Method_Silent();
    for (HostList::iterator it = hosts.begin(); it != hosts.end(); it++)
        if (strstr(it->host->getFullName(),name)!=NULL)
            return &(*it);
    return NULL;
}
