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
    const ReceptionIndication *indication;
    const bool isSynchronizationPossible_;
    const bool isSynchronizationAttempted_;
    const bool isSynchronizationSuccessful_;
    const bool isReceptionPossible_;
    const bool isReceptionAttempted_;
    const bool isReceptionSuccessful_;
    const cPacket *macFrame;

  public:
    ReceptionDecision(const IReception *reception, const ReceptionIndication *indication, bool isReceptionPossible, bool isReceptionAttempted, bool isReceptionSuccessful);
    ~ReceptionDecision();

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;

    virtual const IReception *getReception() const { return reception; }
    virtual const ReceptionIndication *getIndication() const { return indication; }

    virtual bool isReceptionPossible() const { return isReceptionPossible_; }
    virtual bool isReceptionAttempted() const { return isReceptionAttempted_; }
    virtual bool isReceptionSuccessful() const { return isReceptionSuccessful_; }
    virtual bool isSynchronizationPossible() const { return isSynchronizationPossible_; }
    virtual bool isSynchronizationAttempted() const { return isSynchronizationAttempted_; }
    virtual bool isSynchronizationSuccessful() const { return isSynchronizationSuccessful_; }

    virtual const cPacket *getPhyFrame() const;
    virtual const cPacket *getMacFrame() const;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_RECEPTIONDECISION_H

