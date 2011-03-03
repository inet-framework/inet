//
// Copyright (C) 2011 Philipp Berndt
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_CDATARATECHANNEL2_H
#define __INET_CDATARATECHANNEL2_H


#include <omnetpp.h>

/**
 * cDatarateChannel2 is a workaround
 * for defunct accessors in OMNeT++'s cDatarateChannel.
 */

class cDatarateChannel2 : public cDatarateChannel
{
public:

    /**
     * Constructor. This is only for internal purposes, and should not
     * be used when creating channels dynamically; use the create()
     * factory method instead.
     */
    explicit cDatarateChannel2(const char *name=NULL);

    /**
     * Sets the propagation delay of the channel, in seconds.
     */
    virtual void setDelay(double d);

    /**
     * Sets the data rate of the channel, in bit/second.
     *
     * @see isBusy(), getTransmissionFinishTime()
     */
    virtual void setDatarate(double d);

    /**
     * Sets the bit error rate (BER) of the channel.
     *
     * @see cMessage::hasBitError()
     */
    virtual void setBitErrorRate(double d);

    /**
     * Sets the packet error rate (PER) of the channel.
     *
     * @see cMessage::hasBitError()
     */
    virtual void setPacketErrorRate(double d);

    /**
     * Disables or enables the channel.
     */
    virtual void setDisabled(bool d);
};

#endif