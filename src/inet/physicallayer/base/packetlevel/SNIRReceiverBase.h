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

#include "inet/physicallayer/contract/packetlevel/ISNIR.h"
#include "inet/physicallayer/base/packetlevel/ReceiverBase.h"

namespace inet {

namespace physicallayer {

class INET_API SNIRReceiverBase : public ReceiverBase
{
  protected:
    double snirThreshold;

  protected:
    virtual void initialize(int stage) override;

    /**
     * Returns the physical properties of the reception including noise and
     * signal related measures, error probabilities, actual error counts, etc.
     * This function must be purely functional and support optimistic parallel
     * computation.
     */
    virtual const ReceptionIndication *computeReceptionIndication(const ISNIR *snir) const override;

  public:
    SNIRReceiverBase();

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;

    virtual double getSNIRThreshold() const { return snirThreshold; }

    virtual bool computeIsReceptionSuccessful(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference, const ISNIR *snir) const override;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_SNIRRECEIVERBASE_H

