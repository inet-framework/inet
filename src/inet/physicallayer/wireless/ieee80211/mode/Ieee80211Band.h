//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211BAND_H
#define __INET_IEEE80211BAND_H

#include "inet/common/IPrintableObject.h"
#include "inet/common/Units.h"

namespace inet {

namespace physicallayer {

using namespace inet::units::values;

class INET_API IIeee80211Band : public cObject, public IPrintableObject
{
  public:
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override { return stream << "Ieee80211Band, name = " << getName(); }
    virtual const char *getName() const override = 0;
    virtual int getNumChannels() const = 0;
    virtual Hz getCenterFrequency(int channelNumber) const = 0;
    virtual Hz getSpacing() const = 0;
};

class INET_API Ieee80211BandBase : public IIeee80211Band
{
  protected:
    const char *name;

  public:
    Ieee80211BandBase(const char *name);

    virtual const char *getName() const override { return name; }
};

class INET_API Ieee80211EnumeratedBand : public Ieee80211BandBase
{
  protected:
    std::vector<Hz> centers;

  public:
    Ieee80211EnumeratedBand(const char *name, const std::vector<Hz> centers);

    virtual int getNumChannels() const override { return centers.size(); }
    virtual Hz getCenterFrequency(int channelNumber) const override;
    virtual Hz getSpacing() const override { return Hz(NaN); }
};

class INET_API Ieee80211ArithmeticalBand : public Ieee80211BandBase
{
  protected:
    Hz start;
    Hz spacing;
    int numChannels;

  public:
    Ieee80211ArithmeticalBand(const char *name, Hz start, Hz spacing, int numChannels);

    virtual int getNumChannels() const override { return numChannels; }
    virtual Hz getCenterFrequency(int channelNumber) const override;
    virtual Hz getSpacing() const override { return spacing; }
};

class INET_API Ieee80211CompliantBands
{
  protected:
    static const std::vector<const IIeee80211Band *> bands;

  public:
    static const Ieee80211EnumeratedBand band2_4GHz;
    static const Ieee80211ArithmeticalBand band5GHz;
    static const Ieee80211ArithmeticalBand band5GHz20MHz;
    static const Ieee80211ArithmeticalBand band5GHz40MHz;
    static const Ieee80211ArithmeticalBand band5GHz80MHz;
    static const Ieee80211ArithmeticalBand band5GHz160MHz;
    static const Ieee80211ArithmeticalBand band5_9GHz;

    static const IIeee80211Band *findBand(const char *name);
    static const IIeee80211Band *getBand(const char *name);
};

} // namespace physicallayer

} // namespace inet

#endif

