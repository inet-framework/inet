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

#ifndef __INET_RADIOPOWERCONSUMER_H_
#define __INET_RADIOPOWERCONSUMER_H_

#include "IPowerSource.h"
#include "IRadio.h"

/**
 * This is radio power consumer model.
 *
 * @author Levente Meszaros
 */
class INET_API RadioPowerConsumer : public cSimpleModule, public IPowerConsumer, public cListener
{
  protected:
    // parameters
    double sleepModePowerConsumption;
    double receiverModeFreeChannelPowerConsumption;
    double receiverModeBusyChannelPowerConsumption;
    double receiverModeReceivingPowerConsumption;
    double transmitterModeIdlePowerConsumption;
    double transmitterModeTransmittingPowerConsumption;

    // environment
    IRadio *radio;
    IPowerSource *powerSource;

    // internal state
    int powerConsumerId;

  public:
    RadioPowerConsumer();

    virtual double getPowerConsumption();

    virtual void receiveSignal(cComponent *source, simsignal_t signalID, long value);

  protected:
    virtual int numInitStages() const { return NUM_INIT_STAGES; }

    virtual void initialize(int stage);
};

#endif
