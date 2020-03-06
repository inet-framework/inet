//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ISIGNALSYMBOLMODEL_H
#define __INET_ISIGNALSYMBOLMODEL_H

#include "inet/common/IPrintableObject.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISymbol.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IModulation.h"

namespace inet {
namespace physicallayer {

/**
 * This purely virtual interface provides an abstraction for different radio
 * signal models in the waveform or symbol domain.
 */
class INET_API ISignalSymbolModel : public IPrintableObject
{
  public:
    /**
     * Return the number of symbols in the PHY frame header part.
     */
    virtual int getHeaderSymbolLength() const = 0;

    /**
     * Return the symbol rate of the PHY frame header part.
     */
    virtual double getHeaderSymbolRate() const = 0;

    /**
     * Return the number of symbols in the PHY frame data part.
     */
    virtual int getDataSymbolLength() const = 0;

    /**
     * Return the symbol rate of the PHY frame data part.
     */
    virtual double getDataSymbolRate() const = 0;

    /**
     * Returns the all symbols of the PHY frame header and data parts.
     */
    virtual const std::vector<const ISymbol *> *getAllSymbols() const = 0;
};

class INET_API ITransmissionSymbolModel : public virtual ISignalSymbolModel
{
  public:
    virtual const IModulation *getHeaderModulation() const = 0;
    virtual const IModulation *getDataModulation() const = 0;
};

class INET_API IReceptionSymbolModel : public virtual ISignalSymbolModel
{
  public:
    virtual double getSymbolErrorRate() const = 0;
};

} // namespace physicallayer
} // namespace inet

#endif

