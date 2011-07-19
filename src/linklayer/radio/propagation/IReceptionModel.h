//
// Copyright (C) 2006 Andras Varga
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef IRECEPTIONMODEL_H
#define IRECEPTIONMODEL_H

#include "INETDefs.h"


/**
 * Abstract class to encapsulate the calculation of received power of a
 * radio transmission. The calculation may include the effects of
 * path loss, antenna gain, etc.
 */
class INET_API IReceptionModel : public cObject
{
  public:
    /**
     * Allows parameters to be read from the module parameters of a
     * module that contains this object.
     */
    virtual void initializeFrom(cModule *radioModule) = 0;

    /**
     * To be redefined to calculate the received power of a transmission.
     */
    virtual double calculateReceivedPower(double pSend, double carrierFrequency, double distance) = 0;

    /**
     * Virtual destructor.
     */
    virtual ~IReceptionModel() {}
};

#endif

