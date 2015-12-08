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

#ifndef __INET_RECEPTIONDECISION_H
#define __INET_RECEPTIONDECISION_H

#include "inet/physicallayer/contract/packetlevel/IReceptionDecision.h"

namespace inet {

namespace physicallayer {

class INET_API ReceptionDecision : public IReceptionDecision, public cObject
{
  protected:
    const IReception *reception;
    IRadioSignal::SignalPart part;
    const bool isReceptionPossible_;
    const bool isReceptionAttempted_;
    const bool isReceptionSuccessful_;

  public:
    ReceptionDecision(const IReception *reception, IRadioSignal::SignalPart part, bool isReceptionPossible, bool isReceptionAttempted, bool isReceptionSuccessful);

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;

    virtual const IReception *getReception() const override { return reception; }
    virtual IRadioSignal::SignalPart getSignalPart() const override { return part; }

    virtual bool isReceptionPossible() const override { return isReceptionPossible_; }
    virtual bool isReceptionAttempted() const override { return isReceptionAttempted_; }
    virtual bool isReceptionSuccessful() const override { return isReceptionSuccessful_; }
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_RECEPTIONDECISION_H

