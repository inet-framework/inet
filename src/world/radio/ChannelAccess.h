/* -*- mode:c++ -*- ********************************************************
 * file:        ChannelAccess.h
 *
 * author:      Marc Loebbers
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
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


#ifndef CHANNEL_ACCESS_H
#define CHANNEL_ACCESS_H

#include <list>
#include <limits>

#include "INETDefs.h"

#include "BasicModule.h"
#include "IChannelControl.h"

// Forward declarations
class AirFrame;

/**
 * @brief Basic class for all physical layers, please don't touch!!
 *
 * This class is not supposed to work on its own, but it contains
 * functions and lists that cooperate with ChannelControl to handle
 * the dynamically created gates. This means EVERY SnrEval (the lowest
 * layer in a host) has to be derived from this class!!!! And please
 * follow the instructions on how to declare a physical layer in a
 * .ned file in "The Design of a Mobility Framework in OMNeT++"
 * paper.
 *
 * Please don't touch this class.
 *
 * @author Marc Loebbers
 * @ingroup channelControl
 * @ingroup phyLayer
 */
class INET_API ChannelAccess : public BasicModule, protected cListener
{
  protected:
    static simsignal_t mobilityStateChangedSignal;
    NotificationBoard *nb; // Cached pointer to the NotificationBoard module
    IChannelControl* cc;  // Pointer to the ChannelControl module
    IChannelControl::RadioRef myRadioRef;  // Identifies this radio in the ChannelControl module
    cModule *hostModule;    // the host that contains this radio model
    Coord radioPos;  // the physical position of the radio (derived from display string or from mobility models)
    bool positionUpdateArrived;

  public:
    ChannelAccess() : cc(NULL), myRadioRef(NULL), hostModule(NULL) {}
    virtual ~ChannelAccess();

    /**
     * @brief Called by the signalling mechanism to inform of changes.
     *
     * ChannelAccess is subscribed to position changes.
     */
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj);

    /** Finds the channelControl module in the network */
    static IChannelControl *getChannelControl();

  protected:
    /** Sends a message to all radios in range */
    virtual void sendToChannel(AirFrame *msg);

    virtual cPar& getChannelControlPar(const char *parName) { return dynamic_cast<cModule *>(cc)->par(parName); }
    const Coord& getRadioPosition() const { return radioPos; }
    cModule *getHostModule() const { return hostModule; }

    /** Register with ChannelControl and subscribe to hostPos*/
    virtual void initialize(int stage);
    virtual int numInitStages() const { return 3; }
};

#endif

