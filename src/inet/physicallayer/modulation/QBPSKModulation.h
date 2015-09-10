//
// Copyright (C) 2015 OpenSim Ltd.
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

#ifndef __INET_QBPSKMODULATION_H
#define __INET_QBPSKMODULATION_H

#include "inet/physicallayer/base/packetlevel/MQAMModulationBase.h"

namespace inet {

namespace physicallayer {

/**
 * This class implements quadrature binary phase-shift keying defined
 * in 20.3.9.4.3 HT-SIG definition.
 */
class INET_API QBPSKModulation : public MQAMModulationBase
{
  public:
    static const QBPSKModulation singleton;

  protected:
    static const std::vector<APSKSymbol> constellation;

  public:
    QBPSKModulation();

    virtual void printToStream(std::ostream& stream) const { stream << "QBPSKModulation"; }

    virtual double calculateSER(double snir, Hz bandwidth, bps bitrate) const;
    virtual double calculateBER(double snir, Hz bandwidth, bps bitrate) const;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_QBPSKMODULATION_H

