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
#include "Coord.h"

//#include "ChannelControl.h"


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
class AirFrame;


class INET_API ChannelControlExtended : public cSimpleModule
{
  protected:
    class HostEntry;
    typedef std::list<HostEntry> HostList;


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
    typedef HostEntry *HostRef; // handle for ChannelControl's clients
    typedef std::vector<HostRef> HostRefVector;
    typedef std::list<AirFrame *> TransmissionList;


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

    struct HostEntry
    {
    public:

        cModule *host;
        int regCnt; // counter of registration
        cGate *radioInGate;
        int channel;
        Coord pos; // cached

        // we cache neighbors set in an std::vector, because std::set iteration is slow;
        // std::vector is created and updated on demand
        HostRefVector neighborList;
        bool isNeighborListValid;
        virtual bool getIsModuleListValid(){return isNeighborListValid;}
// extended
    	std::set<HostRef> neighbors;  // cached neighbor list
        double maxInterferenceDist;

        RadioList radioList;

        double carrierFrequency;
        double percentage;
        int numChannles;
        HostEntry () {percentage = carrierFrequency = -1;}

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

    /** @brief used to clear the transmission list from time to time */
    simtime_t lastOngoingTransmissionsUpdate;
    /** @brief Set debugging for the basic module*/
    bool coreDebug;
    double carrierFrequency;
    double percentage;
    double maxInterferenceDistance; // the biggest interference distance in the network
    int numChannels;  // the number of controlled channels

  protected:
    virtual void updateConnections(HostRef h);

    /** @brief Calculate interference distance*/
    virtual double calcInterfDistExtended(double);
    virtual double calcInterfDist();

    virtual void initialize();

    /** @brief Throws away expired transmissions. */
    virtual void purgeOngoingTransmissions();
    /** @brief Validate the channel identifier */
    virtual void checkChannel(const int channel);

    friend std::ostream& operator<<(std::ostream&, const HostEntry&);
    friend std::ostream& operator<<(std::ostream&, const TransmissionList&);

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

    /** @brief To be called when the host moved; updates proximity info */
    virtual void updateHostPosition(HostRef h, const Coord& pos);

    /** JcM Add: Get a host reference by its radio **/
    cModule* getHostByRadio(AbstractRadio* r);

    /** @brief Provides a list of transmissions currently on the air */
    const TransmissionList& getOngoingTransmissions(const int channel);

    /** @brief Notifies the channel control with an ongoing transmission */
    virtual void addOngoingTransmission(HostRef h, AirFrame *frame);

    /** @brief Returns the host's position */
    const Coord& getHostPosition(HostRef h)  {return h->pos;}


    /** @brief Reads init parameters and calculates a maximal interference distance*/
    virtual double getCommunicationRange(HostRef h) {
        return maxInterferenceDistance;
    }
    /** @brief Returns the number of radio channels (frequencies) simulated */
    const int getNumChannels() {return numChannels;}
    const double getPercentage() {return percentage;}
};

#endif
