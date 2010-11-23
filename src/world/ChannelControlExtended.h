/* -*- mode:c++ -*- ********************************************************
 * file:        ChannelControl.h
 *
 * copyright:   (C) 2006 Levente Meszaros, 2005 Andras Varga
 * copyright:   (C) 2009 Juan-Carlos Maureira
 * copyright:   (C) 2009 Alfonso Ariza
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

#ifndef CHANNELCONTROLEXTENDED_H
#define CHANNELCONTROLEXTENDED_H

#include <vector>
#include <list>
#include <deque>
#include <set>
#include <omnetpp.h>
#include "ChannelControl.h"


#define LIGHT_SPEED 3.0E+8
#define TRANSMISSION_PURGE_INTERVAL 1.0

/**
 * @brief Monitors which hosts are "in range". Supports multiple channels.
 *
 * @ingroup channelControl
 * @sa ChannelAccess
 */

class ChannelAccess;
class AbstractRadio;


class INET_API ChannelControlExtended : public ChannelControl
{
  protected:
    class HostEntryExtended;
    typedef std::list<HostEntryExtended> HostList;


    // JcM add: Radio entry structure
    struct RadioEntry
    {
        cModule* radioModule;
        int channel;
        int hostGateId; // gate id on the host compound radioIn gate array
        double radioCarrier;
        cGate *radioInGate;
    };
    typedef std::list<RadioEntry> RadioList;

  public:
    //typedef std::list<AirFrame*> TransmissionListExt;
    typedef HostEntryExtended *HostRefExtended; // handle for ChannelControl's clients
    typedef std::list<AirFrameExtended*> TransmissionList;


    // JcM add: we handle radio list instead of host lists
    typedef std::list<cGate*> radioGatesList;

  protected:
    /**
     * Keeps track of hosts/NICs, their positions and channels;
     * also caches neighbor info (which other hosts are within
     * interference distance).
     */

    typedef std::vector<TransmissionList> ChannelTransmissionLists;
    ChannelTransmissionLists transmissions; // indexed by channel number (size=numChannels)


    // JcM Fix: Change the HostEntry in order to support multiple radios
    //struct HostEntry {
    //   cModule *host;
    //   Coord pos; // cached
    //   std::set<HostRef> neighbors;  // cached neighbour list
    //   // TODO: use ChannelAccess vector instead
    //   int channel;
    //   bool isModuleListValid;  // "neighborModules" is produced from "neighbors" on demand
    //   ModuleList neighborModules; // derived from "neighbors"
    //};

    struct HostEntryExtended : public ChannelControl::HostEntry
    {
  public:
        std::set<HostRefExtended> neighbors;  // cached neighbor list
        double maxInterferenceDist;

        RadioList radioList;

        double carrierFrequency;
        double percentage;
        HostEntryExtended () {percentage=carrierFrequency=-1;}

        //radioGatesList getHostGatesOnChannel(int);
        radioGatesList getHostGatesOnChannel(int,double);

        bool isReceivingInChannel(int,double);
        void registerRadio(cModule*);
        void unregisterRadio(cModule*);
        // bool updateRadioChannel(cModule*,int);
        bool updateRadioChannel(cModule*,int,double);
        bool isRadioRegistered(cModule*);

    };

    HostList hosts;

    double carrierFrequency;
    double percentage;

  protected:
    virtual void updateConnections(HostRef h);

    /** @brief Calculate interference distance*/
    virtual double calcInterfDistExtended(double);

    virtual void initialize();

    /** @brief Throws away expired transmissions. */
    virtual void purgeOngoingTransmissions();

    friend std::ostream& operator<<(std::ostream&, const HostEntryExtended&);
    friend std::ostream& operator<<(std::ostream&, const ChannelControl::TransmissionList&);

  public:
    ChannelControlExtended();
    virtual ~ChannelControlExtended();

    static ChannelControlExtended * get();

    /** @brief Registers the given host. If radioInGate==NULL, the "radioIn" gate is assumed */
    virtual HostRef registerHost(cModule *host, const Coord& initialPos, cGate *radioInGate=NULL);

    /** @brief Unregisters the given host */
    virtual void unregisterHost(cModule *host);

    /** @brief Returns the "handle" of a previously registered host */
    virtual HostRef lookupHost(cModule *host);

    /** @brief Get the list of modules in range of the given host */
    const HostRefVector& getNeighbors(HostRef h);

    /** @brief Called from ChannelAccess, to transmit a frame to the hosts in range, on the frame's channel */
    virtual void sendToChannel(cSimpleModule *srcRadioMod, HostRef srcHost, AirFrame *airFrame);

    /** @brief Called when host switches channel (cModule* ca is the channel access (radio)) */
    //virtual void updateHostChannel(HostRef h, const int channel,cModule* ca);

    /** @brief Called when host switches channel (cModule* ca is the channel access (radio)) */
    virtual void updateHostChannel(HostRef h, const int channel,cModule* ca,double);
    virtual void updateHostChannel(HostRef h, const int channel);

    /** JcM Add: Get a host reference by its radio **/
    cModule* getHostByRadio(AbstractRadio* r);

    /** @brief Provides a list of transmissions currently on the air */
    const TransmissionList& getOngoingTransmissions(const int channel);

    /** @brief Notifies the channel control with an ongoing transmission */
    virtual void addOngoingTransmission(HostRef h, AirFrame *frame);

    /** @brief Returns the host's position */
    const Coord& getHostPosition(HostRef h)  {return h->pos;}

    /** @brief Returns the number of radio channels (frequencies) simulated */
    const int getNumChannels() {return numChannels;}
    const double getPercentage() {return percentage;}
};

#endif
