//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_APSKMODULATIONBASE_H
#define __INET_APSKMODULATIONBASE_H

#include "inet/common/ShortBitVector.h"
#include "inet/physicallayer/wireless/apsk/bitlevel/ApskSymbol.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IApskModulation.h"

namespace inet {

namespace physicallayer {

/**
 * Base class for modulations using various amplitude and phase-shift keying.
 */
class INET_API ApskModulationBase : public IApskModulation
{
  protected:
    const std::vector<ApskSymbol> *constellation;
    const unsigned int codeWordSize;
    const unsigned int constellationSize;

  public:
    ApskModulationBase(const std::vector<ApskSymbol> *constellation);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    const static ApskModulationBase *findModulation(const char *name);

    virtual const std::vector<ApskSymbol> *getConstellation() const { return constellation; }
    virtual unsigned int getConstellationSize() const override { return constellationSize; }
    virtual unsigned int getCodeWordSize() const override { return codeWordSize; }

    virtual const ApskSymbol *mapToConstellationDiagram(const ShortBitVector& symbol) const;
    virtual ShortBitVector demapToBitRepresentation(const ApskSymbol *symbol) const;
};

} // namespace physicallayer

} // namespace inet

#endif

