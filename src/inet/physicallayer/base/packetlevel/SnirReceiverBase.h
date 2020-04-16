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

#ifndef __INET_SNIRRECEIVERBASE_H
#define __INET_SNIRRECEIVERBASE_H

#include "inet/physicallayer/base/packetlevel/ReceiverBase.h"
#include "inet/physicallayer/contract/packetlevel/ISnir.h"

namespace inet {

namespace physicallayer {

class INET_API SnirReceiverBase : public ReceiverBase
{
  protected:
    enum class SnirThresholdMode {
        STM_UNDEFINED = -1,
        STM_MIN,
        STM_MEAN
    };

    double snirThreshold = NaN;
    SnirThresholdMode snirThresholdMode = SnirThresholdMode::STM_UNDEFINED;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;

    virtual double getSNIRThreshold() const { return snirThreshold; }

    virtual bool computeIsReceptionSuccessful(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference, const ISnir *snir) const override;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_SNIRRECEIVERBASE_H

