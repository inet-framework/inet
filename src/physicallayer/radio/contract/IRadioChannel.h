//
// Copyright (C) 2013 OpenSim Ltd.
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

#ifndef __INET_IRADIOCHANNEL_H_
#define __INET_IRADIOCHANNEL_H_

#include "IRadio.h"
#include "IRadioFrame.h"

/**
 * This purely virtual interface provides an abstraction for different radio channels.
 */
class INET_API IRadioChannel
{
  public:
    virtual ~IRadioChannel() { }

    /**
     * Returns the number of available radio channels.
     */
    virtual int getNumChannels() = 0;

//    /**
//     * Adds a new radio to the radio channel and returns its id.
//     */
//    virtual int addRadio(IRadio *radio) = 0;

//    /**
//     * Removes a previously added radio from the radio channel.
//     */
//    virtual void removeRadio(int id) = 0;

//    /**
//     * Transmits a radio frame through the radio channel to all radios within
//     * interference distance.
//     */
//    virtual void transmitRadioFrame(int id, IRadioFrame *radioFrame) = 0;

//    /**
//     * Returns the radio frames of all ongoing transmissions for the provided channel.
//     */
//    virtual std::vector<IRadioFrame *>& getOngoingTransmissions(int radioChannel) = 0;
};

#endif
