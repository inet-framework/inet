//
// Copyright (C) 2006 Andras Varga
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

#ifndef IRADIOMODEL_H
#define IRADIOMODEL_H

#include "INETDefs.h"
#include "AirFrame_m.h"
#include "SnrList.h"

/**
 * Abstract class to encapsulate the calculation of received power of a
 * radio transmission. The calculation may include the effects of
 * path loss, antenna gain, etc.
 */
class INET_API IRadioModel : public cPolymorphic
{
  public:
    /**
     * Allows parameters to be read from the module parameters of a
     * module that contains this object.
     */
    virtual void initializeFrom(cModule *radioModule) = 0;

    /**
     * Virtual destructor.
     */
    virtual ~IRadioModel() {}

    /**
     * Should be defined to calculate the duration of the AirFrame.
     * Usually the duration is just the frame length divided by the
     * bitrate. However, in some cases, notably IEEE 802.11, the header
     * has a different modulation (and thus a different bitrate) than the
     * rest of the message.
     */
    virtual double calculateDuration(AirFrame *) = 0;

    /**
     * Should be defined to calculate whether the frame has been received
     * correctly. Input is the signal-noise ratio over the duration of the
     * frame. The calculation may take into account the modulation scheme,
     * possible error correction code, etc.
     */
    virtual bool isReceivedCorrectly(AirFrame *airframe, const SnrList& receivedList) = 0;
};

#endif

