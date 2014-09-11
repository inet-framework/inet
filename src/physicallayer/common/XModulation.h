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

#ifndef __INET_XMODULATION_H
#define __INET_XMODULATION_H

#include "IModulationScheme.h"

namespace inet {

namespace physicallayer {

// TODO: rename to Modulation
class INET_API XModulation : public IModulationScheme
{
  public:
    enum Type {
        /** Infrared (IR) (Clause 16) */
        IR,
        /** Frequency-hopping spread spectrum (FHSS) PHY (Clause 14) */
        FHSS,
        /** DSSS PHY (Clause 15) and HR/DSSS PHY (Clause 18) */
        DSSS,
        /** ERP-PBCC PHY (19.6) */
        ERP_PBCC,
        /** DSSS-OFDM PHY (19.7) */
        DSSS_OFDM,
        /** ERP-OFDM PHY (19.5) */
        ERP_OFDM,
        /** OFDM PHY (Clause 17) */
        OFDM,
        /** HT PHY (Clause 20) */
        HT

    };

  protected:
    const Type type;
    const int codeWordLength;
    const int constellationSize;

  public:
    XModulation(const Type type, int codeWordLength, int constellationSize) :
        type(type),
        codeWordLength(codeWordLength),
        constellationSize(constellationSize)
    {}

    virtual void printToStream(std::ostream &stream) const;

    virtual Type getType() const { return type; }

    virtual int getCodeWordLength() const { return codeWordLength; }

    virtual int getConstellationSize() const { return constellationSize; }
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_XMODULATION_H
