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

#ifndef __INET_STOCHASTICERRORMODEL_H
#define __INET_STOCHASTICERRORMODEL_H

#include "inet/physicallayer/base/packetlevel/ErrorModelBase.h"

namespace inet {

namespace physicallayer {

/**
 * Implements the StochasticErrorModel model, see the NED file for details.
 */
class INET_API StochasticErrorModel : public ErrorModelBase
{
  protected:
    double packetErrorRate;
    double bitErrorRate;
    double symbolErrorRate;

  protected:
    virtual void initialize(int stage) override;

  public:
    StochasticErrorModel();

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;

    virtual double computePacketErrorRate(const ISNIR *snir, IRadioSignal::SignalPart part) const override;
    virtual double computeBitErrorRate(const ISNIR *snir, IRadioSignal::SignalPart part) const override;
    virtual double computeSymbolErrorRate(const ISNIR *snir, IRadioSignal::SignalPart part) const override;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_STOCHASTICERRORMODEL_H

