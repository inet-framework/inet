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

#ifndef __INET_SCALARSNIR_H
#define __INET_SCALARSNIR_H

#include "inet/physicallayer/base/packetlevel/SnirBase.h"

namespace inet {

namespace physicallayer {

class INET_API ScalarSnir : public SnirBase
{
  protected:
    mutable double minSNIR;
    mutable double maxSNIR;
    mutable double meanSNIR;

  protected:
    virtual double computeMin() const;
    virtual double computeMax() const;
    virtual double computeMean() const;

  public:
    ScalarSnir(const IReception *reception, const INoise *noise);

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;

    virtual double getMin() const override;
    virtual double getMax() const override;
    virtual double getMean() const override;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_SCALARSNIR_H

