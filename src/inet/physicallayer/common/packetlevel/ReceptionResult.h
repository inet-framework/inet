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

#ifndef __INET_RECEPTIONRESULT_H
#define __INET_RECEPTIONRESULT_H

#include "inet/physicallayer/contract/packetlevel/IReceptionDecision.h"
#include "inet/physicallayer/contract/packetlevel/IReceptionResult.h"

namespace inet {

namespace physicallayer {

class INET_API ReceptionResult : public IReceptionResult, public cObject
{
  protected:
    const IReception *reception;
    const std::vector<const IReceptionDecision *> *decisions;
    const ReceptionIndication *indication;
    const cPacket *macFrame;

  public:
    ReceptionResult(const IReception *reception, const std::vector<const IReceptionDecision *> *decisions, const ReceptionIndication *indication);
    ~ReceptionResult();

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;

    virtual const IReception *getReception() const override { return reception; }
    virtual const std::vector<const IReceptionDecision *> *getDecisions() const override { return decisions; }
    virtual const ReceptionIndication *getIndication() const override { return indication; }

    virtual const cPacket *getPhyFrame() const override;
    virtual const cPacket *getMacFrame() const override;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_RECEPTIONRESULT_H

