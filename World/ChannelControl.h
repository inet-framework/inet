/* -*- mode:c++ -*- ********************************************************
 * file:        ChannelControl.h
 *
 * copyright:   (C) 2006 Levente Meszaros, 2005 Andras Varga
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
#include <deque>
#include <set>
#include <omnetpp.h>
// FIXME: revise directory structure
#include "../NetworkInterfaces/MFCore/AirFrame_m.h"
#include "Coord.h"

#define LIGHT_SPEED 3.0E+8
#define TRANSMISSION_PURGE_INTERVAL 1.0

/**
 * @brief Monitors which hosts are "in range". Supports multiple channels.
 *
 * @ingroup channelControl
 * @sa ChannelAccess
 */
//TODO complete support multiple NICs per host
class INET_API ChannelControl : public cSimpleModule
{
  protected:
    struct HostEntry;
    typedef std::list<HostEntry> HostList;

  public:
    typedef HostEntry *HostRef; // handle for ChannelControl's clients
    typedef std::vector<cModule*> ModuleList;
    typedef std::list<AirFrame*> TransmissionList;

  protected:
    /**
     * Keeps track of hosts/NICs, their positions and channels;
     * also caches neighbor info (which other hosts are within
     * interference distance).
     */
    struct HostEntry {
        cModule *host;
        Coord pos; // cached
        std::set<HostRef> neighbors;  // cached neighbour list
        // TODO: use ChannelAccess vector instead
        int channel;

        bool isModuleListValid;  // "neighborModules" is produced from "neighbors" on demand
        ModuleList neighborModules; // derived from "neighbors"
    };
    HostList hosts;

    /** @brief keeps track of ongoing transmissions; this is needed when a host
     * switches to another channel (then it needs to know whether the target channel
     * is empty or busy)
     */
    typedef std::vector<TransmissionList> ChannelTransmissionLists;
    ChannelTransmissionLists transmissions; // indexed by channel number (size=numChannels)

    /** @brief used to clear the transmission list from time to time */
    double lastOngoingTransmissionsUpdate;

    friend std::ostream& operator<<(std::ostream&, const HostEntry&);
    friend std::ostream& operator<<(std::ostream&, const TransmissionList&);

    /** @brief Set debugging for the basic module*/
    bool coreDebug;

    /** @brief x and y size of the area the nodes are in (in meters)*/
    Coord playgroundSize;

    /** @brief the biggest interference distance in the network.*/
    double maxInterferenceDistance;

    /** @brief the number of controlled channels */
    int numChannels;

  protected:
    void updateConnections(HostRef h);

    /** @brief Calculate interference distance*/
    virtual double calcInterfDist();

    /** @brief Set up playground module's display string */
    void updateDisplayString(cModule *playgroundMod);

    /** @brief Reads init parameters and calculates a maximal interference distance*/
    virtual void initialize();

    /** @brief Throws away expired transmissions. */
    void purgeOngoingTransmissions();

    /** @brief Validate the channel identifier */
    void checkChannel(const int channel);

  public:
    ChannelControl();
    ~ChannelControl();

    /** @brief Registers the given host */
    HostRef registerHost(cModule *host, const Coord& initialPos);

    /** @brief Returns the "handle" of a previously registered host */
    HostRef lookupHost(cModule *host);

    /** @brief To be called when the host moved; updates proximity info */
    void updateHostPosition(HostRef h, const Coord& pos);

    /** @brief Called when host switches channel */
    void updateHostChannel(HostRef h, const int channel);

    /** @brief Provides a list of transmissions currently on the air */
    const TransmissionList& getOngoingTransmissions(const int channel);

    /** @brief Notifies the channel control with an ongoing transmission */
    void addOngoingTransmission(HostRef h, AirFrame *frame);

    /** @brief Returns the host's position */
    const Coord& getHostPosition(HostRef h)  {return h->pos;}

    /** @brief Get the list of modules in range of the given host */
    const ModuleList& getNeighbors(HostRef h);

    /** @brief Reads init parameters and calculates a maximal interference distance*/
    double getCommunicationRange(HostRef h) {
        return maxInterferenceDistance;
    }

    /** @brief Returns the playground size */
    const Coord *getPgs()  {return &playgroundSize;}

    /** @brief Returns the number of radio channels (frequencies) simulated */
    const int getNumChannels() {return numChannels;}
};

#endif
