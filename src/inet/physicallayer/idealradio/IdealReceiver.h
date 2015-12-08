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

#ifndef __INET_IDEALRECEIVER_H
#define __INET_IDEALRECEIVER_H

#include "inet/physicallayer/base/packetlevel/ReceiverBase.h"

namespace inet {

namespace physicallayer {

/**
 * Implements the IdealReceiver model, see the NED file for details.
 */
class INET_API IdealReceiver : public ReceiverBase
{
  protected:
    bool ignoreInterference;

  protected:
    virtual void initialize(int stage) override;

    virtual const ReceptionIndication *computeReceptionIndication(const ISNIR *snir) const override;

  public:
    IdealReceiver();

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;

    virtual bool computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const override;
    virtual bool computeIsReceptionAttempted(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference) const override;
    virtual bool computeIsReceptionSuccessful(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference, const ISNIR *snir) const override;

    virtual const IListening *createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition) const override;
    virtual const IListeningDecision *computeListeningDecision(const IListening *listening, const IInterference *interference) const override;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_IDEALRECEIVER_H

