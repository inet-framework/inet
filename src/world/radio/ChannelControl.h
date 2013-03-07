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
#include <set>

#include "INETDefs.h"
#include "Coord.h"
#include "IChannelControl.h"

// Forward declarations
class AirFrame;

#define TRANSMISSION_PURGE_INTERVAL 1.0

/**
 * Keeps track of radios/NICs, their positions and channels;
 * also caches neighbor info (which other Radios are within
 * interference distance).
 */
struct IChannelControl::RadioEntry {
    cModule *radioModule;  // the module that registered this radio interface
    cGate *radioInGate;  // gate on host module used to receive airframes
    int channel;
    Coord pos; // cached radio position

    struct Compare {
        bool operator() (const RadioRef &lhs, const RadioRef &rhs) const {
            ASSERT(lhs && rhs);
            return lhs->radioModule->getId() < rhs->radioModule->getId();
        }
    };
    // we cache neighbors set in an std::vector, because std::set iteration is slow;
    // std::vector is created and updated on demand
    std::set<RadioRef, Compare> neighbors; // cached neighbor list
    std::vector<RadioRef> neighborList;
    bool isNeighborListValid;
    bool isActive;
};

/**
 * Monitors which radios are "in range". Supports multiple channels.
 *
 * @ingroup channelControl
 * @see ChannelAccess
 */
class INET_API ChannelControl : public cSimpleModule, public IChannelControl
{
  protected:
    typedef std::list<RadioEntry> RadioList;
    typedef std::vector<RadioRef> RadioRefVector;

    RadioList radios;

    /** keeps track of ongoing transmissions; this is needed when a radio
     * switches to another channel (then it needs to know whether the target channel
     * is empty or busy)
     */
    typedef std::vector<TransmissionList> ChannelTransmissionLists;
    ChannelTransmissionLists transmissions; // indexed by channel number (size=numChannels)

    /** used to clear the transmission list from time to time */
    simtime_t lastOngoingTransmissionsUpdate;

    friend std::ostream& operator<<(std::ostream&, const RadioEntry&);
    friend std::ostream& operator<<(std::ostream&, const TransmissionList&);

    /** Set debugging for the basic module*/
    bool coreDebug;

    /** the biggest interference distance in the network.*/
    double maxInterferenceDistance;

    /** the number of controlled channels */
    int numChannels;

  protected:
    virtual void updateConnections(RadioRef h);

    /** Calculate interference distance*/
    virtual double calcInterfDist();

    /** Reads init parameters and calculates a maximal interference distance*/
    virtual void initialize();

    /** Throws away expired transmissions. */
    virtual void purgeOngoingTransmissions();

    /** Validate the channel identifier */
    virtual void checkChannel(int channel);

    /** Get the list of modules in range of the given host */
    virtual const RadioRefVector& getNeighbors(RadioRef h);

    /** Notifies the channel control with an ongoing transmission */
    virtual void addOngoingTransmission(RadioRef h, AirFrame *frame);

    /** Returns the "handle" of a previously registered radio. The pointer to the registering (radio) module must be provided */
    virtual RadioRef lookupRadio(cModule *radioModule);

  public:
    ChannelControl();
    virtual ~ChannelControl();

    /** Registers the given radio. If radioInGate==NULL, the "radioIn" gate is assumed */
    virtual RadioRef registerRadio(cModule *radioModule, cGate *radioInGate = NULL);

    /** Unregisters the given radio */
    virtual void unregisterRadio(RadioRef r);

    /** Returns the host module that contains the given radio */
    virtual cModule *getRadioModule(RadioRef r) const { return r->radioModule; }

    /** Returns the input gate of the host for receiving AirFrames */
    virtual cGate *getRadioGate(RadioRef r) const { return r->radioInGate; }

    /** Returns the channel the given radio listens on */
    virtual int getRadioChannel(RadioRef r) const { return r->channel; }

    /** To be called when the host moved; updates proximity info */
    virtual void setRadioPosition(RadioRef r, const Coord& pos);

    /** Called when host switches channel */
    virtual void setRadioChannel(RadioRef r, int channel);

    /** Returns the number of radio channels (frequencies) simulated */
    virtual int getNumChannels() { return numChannels; }

    /** Provides a list of transmissions currently on the air */
    virtual const TransmissionList& getOngoingTransmissions(int channel);

    /** Called from ChannelAccess, to transmit a frame to the radios in range, on the frame's channel */
    virtual void sendToChannel(RadioRef srcRadio, AirFrame *airFrame);

    /** Returns the maximal interference distance*/
    virtual double getInterferenceRange(RadioRef r) { return maxInterferenceDistance; }

    /** Disable the reception in the reference module */
    virtual void disableReception(RadioRef r) { r->isActive = false; };

    /** Enable the reception in the reference module */
    virtual void enableReception(RadioRef r) { r->isActive = true; };

    /** Returns propagation speed of the signal in meter/sec */
    virtual double getPropagationSpeed() { return SPEED_OF_LIGHT; }
};

#endif
