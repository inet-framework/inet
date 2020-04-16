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

#ifndef __INET_IEEE80211UNITDISKRECEIVER_H
#define __INET_IEEE80211UNITDISKRECEIVER_H

#include "inet/physicallayer/unitdisk/UnitDiskReceiver.h"

namespace inet {

namespace physicallayer {

// TODO: Ieee80211ReceiverBase
class INET_API Ieee80211UnitDiskReceiver : public UnitDiskReceiver
{
  protected:
    virtual void initialize(int stage) override;

    virtual bool computeIsReceptionPossible(const IListening *listening, const ITransmission *transmission) const override;
    virtual bool computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const override;

  public:
    Ieee80211UnitDiskReceiver();

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;

    virtual const IReceptionResult *computeReceptionResult(const IListening *listening, const IReception *reception, const IInterference *interference, const ISnir *snir, const std::vector<const IReceptionDecision *> *decisions) const override;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_IEEE80211UNITDISKRECEIVER_H

