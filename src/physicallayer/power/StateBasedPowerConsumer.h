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

#ifndef __INET_STATEBASEDPOWERCONSUMER_H
#define __INET_STATEBASEDPOWERCONSUMER_H

#include "inet/power/contract/IPowerSource.h"
#include "inet/physicallayer/contract/IRadio.h"

namespace inet {

namespace physicallayer {

using namespace inet::power;

/**
 * This is a simple radio power consumer model. The power consumption is
 * determined by the radio mode, the transmitter state and the receiver state
 * using constant parameters.
 *
 * @author Levente Meszaros
 */
class INET_API StateBasedPowerConsumer : public cSimpleModule, public IPowerConsumer, public cListener
{
  protected:
    // parameters
    W offPowerConsumption;
    W sleepPowerConsumption;
    W switchingPowerConsumption;
    W receiverIdlePowerConsumption;
    W receiverBusyPowerConsumption;
    W receiverSynchronizingPowerConsumption;
    W receiverReceivingPowerConsumption;
    W transmitterIdlePowerConsumption;
    W transmitterTransmittingPowerConsumption;

    // environment
    IRadio *radio;
    IPowerSource *powerSource;

    // internal state
    int powerConsumerId;

  public:
    StateBasedPowerConsumer();

    virtual W getPowerConsumption();

    virtual void receiveSignal(cComponent *source, simsignal_t signalID, long value);

  protected:
    virtual int numInitStages() const { return NUM_INIT_STAGES; }

    virtual void initialize(int stage);
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_STATEBASEDPOWERCONSUMER_H

