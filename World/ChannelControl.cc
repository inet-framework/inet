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
#include "stlwatch.h"
#include "FWMath.h"
#include <cassert>


#define coreEV (ev.disabled()||!coreDebug) ? std::cout : ev << "ChannelControl: "

Define_Module(ChannelControl);


std::ostream& operator<<(std::ostream& os, const ChannelControl::HostEntry& h)
{
    os << h.host->fullPath() << " (x=" << h.pos.x << ",y=" << h.pos.y << "), "
       << h.neighbors.size() << " neighbor(s)";
    return os;
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

    maxInterferenceDistance = calcInterfDist();

    WATCH(maxInterferenceDistance);
    WATCH_LIST(hosts);

    updateDisplayString(parentModule());
}

/**
 * Sets up background size by adding the following tags:
 * "p=0,0;b=$playgroundSizeX,$playgroundSizeY"
 */
void ChannelControl::updateDisplayString(cModule *playgroundMod)
{
    cDisplayString& d = playgroundMod->backgroundDisplayString();
    char xStr[32], yStr[32];
    sprintf(xStr, "%d", FWMath::round(playgroundSize.x));
    sprintf(yStr, "%d", FWMath::round(playgroundSize.y));
    d.setTagArg("p",0,"0");
    d.setTagArg("p",1,"0");
    d.setTagArg("b",0,xStr);
    d.setTagArg("b",1,yStr);
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

ChannelControl::HostRef ChannelControl::registerHost(cModule * host, const Coord& initialPos)
{
    if (lookupHost(host) != NULL)
        error("ChannelControl::registerHost(): host (%s)%s already registered",
              host->className(), host->fullPath().c_str());

    HostEntry he;
    he.host = host;
    he.pos = initialPos;
    he.isModuleListValid = false;
    hosts.push_back(he);
    return --(hosts.end()); // last element
}

ChannelControl::HostRef ChannelControl::lookupHost(cModule *host)
{
    for (HostList::iterator it = hosts.begin(); it != hosts.end(); it++)
        if (it->host == host)
            return it;
    return 0;
}

const ChannelControl::ModuleList& ChannelControl::getNeighbors(HostRef h)
{
    if (!h->isModuleListValid)
    {
        h->neighborModules.clear();
        for (std::set<HostRef>::const_iterator it = h->neighbors.begin(); it != h->neighbors.end(); it++)
            h->neighborModules.push_back((*it)->host);
        h->isModuleListValid = true;
    }
    return h->neighborModules;
}

void ChannelControl::updateConnections(HostRef h)
{
    Coord& hpos = h->pos;
    double maxDistSquared = maxInterferenceDistance * maxInterferenceDistance;
    for (HostList::iterator it = hosts.begin(); it != hosts.end(); ++it)
    {
        if (it == h)
            continue;

        // get the distance between the two hosts.
        // (omitting the square root (calling sqrdist() instead of distance()) saves about 5% CPU)
        bool inRange = hpos.sqrdist(it->pos) < maxDistSquared;

        if (inRange)
        {
            // nodes within communication range: connect
            if (h->neighbors.insert(it).second == true)
            {
                it->neighbors.insert(h);
                h->isModuleListValid = it->isModuleListValid = false;
            }
        }
        else
        {
            // out of range: disconnect
            if (h->neighbors.erase(it))
            {
                it->neighbors.erase(h);
                h->isModuleListValid = it->isModuleListValid = false;
            }
        }
    }
}
