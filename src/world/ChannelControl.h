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
#include "AirFrame_m.h"
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
    typedef std::vector<HostRef> HostRefVector;
    typedef std::list<AirFrame*> TransmissionList;

  protected:
    /**
     * Keeps track of hosts/NICs, their positions and channels;
     * also caches neighbor info (which other hosts are within
     * interference distance).
     */
    struct HostEntry {
        cModule *host;
        cGate *radioInGate;
        int channel;
        Coord pos; // cached
        std::set<HostRef> neighbors;  // cached neighbour list

        // we cache neighbors set in an std::vector, because std::set iteration is slow;
        // std::vector is created and updated on demand
        bool isNeighborListValid;
        HostRefVector neighborList;
    };
    HostList hosts;

    /** @brief keeps track of ongoing transmissions; this is needed when a host
     * switches to another channel (then it needs to know whether the target channel
     * is empty or busy)
     */
    typedef std::vector<TransmissionList> ChannelTransmissionLists;
    ChannelTransmissionLists transmissions; // indexed by channel number (size=numChannels)

    /** @brief used to clear the transmission list from time to time */
    simtime_t lastOngoingTransmissionsUpdate;

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
    virtual void updateConnections(HostRef h);

    /** @brief Calculate interference distance*/
    virtual double calcInterfDist();

    /** @brief Set up playground module's display string */
    virtual void updateDisplayString(cModule *playgroundMod);

    /** @brief Reads init parameters and calculates a maximal interference distance*/
    virtual void initialize();

    /** @brief Throws away expired transmissions. */
    virtual void purgeOngoingTransmissions();

    /** @brief Validate the channel identifier */
    virtual void checkChannel(const int channel);

  public:
    ChannelControl();
    virtual ~ChannelControl();

    /** @brief Finds the channelControl module in the network */
    static ChannelControl *get();

    /** @brief Registers the given host. If radioInGate==NULL, the "radioIn" gate is assumed */
    virtual HostRef registerHost(cModule *host, const Coord& initialPos, cGate *radioInGate=NULL);

    /** @brief Returns the module that was registered as HostRef h */
    cModule *getHost(HostRef h) const {return h->host;}

    /** @brief Returns the input gate of the host for receiving AirFrames */
    cGate *getRadioGate(HostRef h) const {return h->radioInGate;}

    /** @brief Returns the channel the given host listens on */
    int getHostChannel(HostRef h) const {return h->channel;}

    /** @brief Returns the "handle" of a previously registered host */
    virtual HostRef lookupHost(cModule *host);

    /** @brief To be called when the host moved; updates proximity info */
    virtual void updateHostPosition(HostRef h, const Coord& pos);

    /** @brief Called when host switches channel */
    virtual void updateHostChannel(HostRef h, const int channel);

    /** @brief Provides a list of transmissions currently on the air */
    const TransmissionList& getOngoingTransmissions(const int channel);

    /** @brief Notifies the channel control with an ongoing transmission */
    virtual void addOngoingTransmission(HostRef h, AirFrame *frame);

    /** @brief Returns the host's position */
    const Coord& getHostPosition(HostRef h)  {return h->pos;}

    /** @brief Get the list of modules in range of the given host */
    const HostRefVector& getNeighbors(HostRef h);

    /** @brief Called from ChannelAccess, to transmit a frame to the hosts in range, on the frame's channel */
    virtual void sendToChannel(cSimpleModule *srcRadioMod, HostRef srcHost, AirFrame *airFrame);

    /** @brief Reads init parameters and calculates a maximal interference distance*/
    virtual double getCommunicationRange(HostRef h) {
        return maxInterferenceDistance;
    }

    /** @brief Returns the playground size */
    const Coord *getPgs()  {return &playgroundSize;}

    /** @brief Returns the number of radio channels (frequencies) simulated */
    const int getNumChannels() {return numChannels;}
};

#endif

