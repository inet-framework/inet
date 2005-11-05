/* -*- mode:c++ -*- ********************************************************
 * file:        ChannelControl.h
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

#ifndef CHANNELCONTROL_H
#define CHANNELCONTROL_H

#include <vector>
#include <list>
#include <set>
#include <omnetpp.h>
#include "Coord.h"


/**
 * @brief Monitors which hosts are "in range".
 *
 * @ingroup channelControl
 * @author Andras Varga
 * @sa ChannelAccess
 */
class INET_API ChannelControl : public cSimpleModule
{
  protected:
    struct HostEntry;
    typedef std::list<HostEntry> HostList;

  public:
    typedef HostList::iterator HostRef;
    typedef std::vector<cModule*> ModuleList;

  protected:
    struct HostEntry {
        cModule *host;
        Coord pos;
        std::set<HostRef> neighbors;

        bool isModuleListValid;  // "neighborModules" is produced from "neighbors" on demand
        ModuleList neighborModules; // derived from "neighbors"
    };
    HostList hosts;

    friend bool operator<(HostRef a, HostRef b) {return a->host < b->host;}
    friend std::ostream& operator<<(std::ostream&, const HostEntry&);

    /** @brief Set debugging for the basic module*/
    bool coreDebug;

    /** @brief x and y size of the area the nodes are in (in meters)*/
    Coord playgroundSize;

    /** @brief the biggest interference distance in the network.*/
    double maxInterferenceDistance;

  protected:
    void updateConnections(HostRef h);

    /** @brief Calculate interference distance*/
    virtual double calcInterfDist();

    /** @brief Set up playground module's display string */
    void updateDisplayString(cModule *playgroundMod);

    /** @brief Reads init parameters and calculates a maximal interference distance*/
    virtual void initialize();

  public:
    /** @brief Registers the given host */
    HostRef registerHost(cModule *host, const Coord& initialPos);

    /** @brief Returns the "handle" of a previously registered host */
    HostRef lookupHost(cModule *host);

    /** @brief To be called when the host moved; updates proximity info */
    void updateHostPosition(HostRef h, const Coord& pos) {
        h->pos = pos;
        updateConnections(h);
    }

    /** @brief Returns the host's position */
    const Coord& getHostPosition(HostRef h)  {return h->pos;}

    /** @brief Get the list of modules in range of the given host */
    const ModuleList& getNeighbors(HostRef h);

    /** @brief Reads init parameters and calculates a maximal interference distance*/
    double getCommunicationRange(HostRef h) {
        return maxInterferenceDistance; // FIXME this is rather the max...
    }

    /** @brief Returns the playground size */
    const Coord *getPgs()  {return &playgroundSize;}
};

#endif
