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

#ifndef __INET_RECEIVERBASE_H
#define __INET_RECEIVERBASE_H

#include "inet/physicallayer/contract/packetlevel/IReceiver.h"
#include "inet/physicallayer/contract/packetlevel/IReception.h"
#include "inet/physicallayer/contract/packetlevel/ITransmission.h"

namespace inet {

namespace physicallayer {

class INET_API ReceiverBase : public cModule, public virtual IReceiver
{
  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

  public:
    ReceiverBase() { }

    virtual W getMinInterferencePower() const override { return W(NaN); }
    virtual W getMinReceptionPower() const override { return W(NaN); }

    virtual bool computeIsReceptionPossible(const IListening *listening, const ITransmission *transmission) const override;

    virtual bool computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const override;
    virtual bool computeIsReceptionAttempted(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference) const override;

    virtual const IReceptionDecision *computeReceptionDecision(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference, const ISNIR *snir) const override;
    virtual const IReceptionResult *computeReceptionResult(const IListening *listening, const IReception *reception, const IInterference *interference, const ISNIR *snir) const override;

    /**
     * Returns an empty reception indication (control info).
     *
     * This function called from computeReceptionIndication().
     */
    virtual ReceptionIndication *createReceptionIndication() const;

    virtual const ReceptionIndication *computeReceptionIndication(const ISNIR *snir) const override;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_RECEIVERBASE_H

