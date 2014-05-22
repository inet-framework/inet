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

#ifndef __INET_RADIOBASE_H_
#define __INET_RADIOBASE_H_

#include "IRadio.h"
#include "IMobility.h"
#include "PhysicalLayerBase.h"

/**
 * This is an abstract base class for different radios.
 *
 * @author Levente Meszaros
 */
class INET_API RadioBase : public PhysicalLayerBase, public virtual IRadio
{
  protected:
    /** Internal state */
    //@{
    RadioMode radioMode;
    ReceptionState receptionState;
    TransmissionState transmissionState;
    // TODO: delme
    int radioChannel;
    //@}

    /** Environment */
    //@{
    // TODO: delme
    IMobility *mobility;
    //@}

    /** Gates */
    //@{
    cGate *upperLayerOut;
    cGate *upperLayerIn;
    cGate *radioIn;
    //@}

  public:
    RadioBase();

    virtual IMobility *getMobility() const { return mobility; }

    virtual const cGate *getRadioGate() const { return radioIn; }

    virtual RadioMode getRadioMode() const { return radioMode; }

    virtual void setRadioMode(RadioMode newRadioMode);

    virtual ReceptionState getReceptionState() const { return receptionState; }

    virtual TransmissionState getTransmissionState() const { return transmissionState; }

    virtual int getOldRadioChannel() const { return radioChannel; }

    virtual void setOldRadioChannel(int newRadioChannel);

  protected:
    virtual int numInitStages() const { return NUM_INIT_STAGES; }

    virtual void initialize(int stage);
};

#endif
