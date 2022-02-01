//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SIGNALSYMBOLMODEL_H
#define __INET_SIGNALSYMBOLMODEL_H

#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalSymbolModel.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IModulation.h"

namespace inet {
namespace physicallayer {

class INET_API SignalSymbolModel : public virtual ISignalSymbolModel
{
  protected:
    const int headerSymbolLength;
    const int payloadSymbolLength;
    const double headerSymbolRate;
    const double payloadSymbolRate;
    const std::vector<const ISymbol *> *symbols;

  public:
    SignalSymbolModel(int headerSymbolLength, double headerSymbolRate, int payloadSymbolLength, double payloadSymbolRate, const std::vector<const ISymbol *> *symbols);
    virtual ~SignalSymbolModel();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual int getPayloadSymbolLength() const override { return payloadSymbolLength; }
    virtual double getPayloadSymbolRate() const override { return payloadSymbolRate; }
    virtual int getHeaderSymbolLength() const override { return headerSymbolLength; }
    virtual double getHeaderSymbolRate() const override { return headerSymbolRate; }
    virtual const std::vector<const ISymbol *> *getSymbols() const override { return symbols; }
};

class INET_API TransmissionSymbolModel : public SignalSymbolModel, public virtual ITransmissionSymbolModel
{
  protected:
    const IModulation *headerModulation;
    const IModulation *payloadModulation;

  public:
    TransmissionSymbolModel(int headerSymbolLength, double headerSymbolRate, int payloadSymbolLength, double payloadSymbolRate, const std::vector<const ISymbol *> *symbols, const IModulation *headerModulation, const IModulation *payloadModulation);

    virtual const IModulation *getHeaderModulation() const override { return headerModulation; }
    virtual const IModulation *getPayloadModulation() const override { return payloadModulation; }
};

class INET_API ReceptionSymbolModel : public SignalSymbolModel, public virtual IReceptionSymbolModel
{
  protected:
    double symbolErrorRate;

  public:
    ReceptionSymbolModel(int headerSymbolLength, double headerSymbolRate, int payloadSymbolLength, double payloadSymbolRate, const std::vector<const ISymbol *> *symbols, double symbolErrorRate);

    virtual double getSymbolErrorRate() const override { return symbolErrorRate; }
};

} // namespace physicallayer
} // namespace inet

#endif

