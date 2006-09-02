//
// Copyright (C) Andras Varga, Levente Meszaros
// Based on the Mobility Framework's SnrEval by Marc Loebbers
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef IEEE80211RADIO_H
#define IEEE80211RADIO_H

#include "AbstractRadio.h"

//FIXME docu

/**
 * @brief Keeps track of the different snir levels when receiving a
 * packet
 *
 * This module keeps track of the noise level of the channel.
 *
 * When receiving a packet this module updates the noise level of the
 * channel. Based on the receive power of the packet it is processed
 * and handed to upper layers or just treated as noise.
 *
 * After the packet is completely received the snir information is
 * attached and it is handed to the decider module.
 *
 * The snir information is a SnrList that lists all different snr
 * levels together with the point of time (simTime()) when they
 * started.
 *
 * On top of that this module manages the RadioState, and posts notifications
 * on NotificationBoard whenever it changes. The radio state gives information
 * about whether this module is sending a packet, receiving a packet or idle.
 * This information can be accessed via the NotificationBoard by other modules,
 * e.g. a CSMAMacLayer.
 *
 * @author Marc Loebbers
 * @ingroup snrEval
 */
class INET_API Ieee80211Radio : public AbstractRadio
{
    double snirThreshold;
  protected:
    /* Redefined from cSimpleModule */
    virtual void initialize(int stage);

    /** Calculates the duration of the AirFrame */
    virtual double calcDuration(AirFrame *);

    /** Calculates the power with which a packet is received.*/
    virtual double calcRcvdPower(double pSend, double distance);

    virtual bool isReceivedCorrectly(AirFrame *af, const SnrList& receivedList);

    // utility
    virtual bool packetOk(double snirMin, int lengthMPDU);
    // utility
    virtual double dB2fraction(double dB);
};

#endif

