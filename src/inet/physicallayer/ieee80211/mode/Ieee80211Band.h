//
// Copyright (C) 2014 OpenSim Ltd.
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

#ifndef __INET_IEEE80211BAND_H
#define __INET_IEEE80211BAND_H

#include "inet/physicallayer/contract/packetlevel/IPrintableObject.h"

namespace inet {

namespace physicallayer {

class INET_API IIeee80211Band : public cObject, public IPrintableObject
{
  public:
    virtual std::ostream& printToStream(std::ostream& stream, int level) const override { return stream << "Ieee80211Band, name = " << getName(); }
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

#endif // ifndef __INET_IEEE80211BAND_H

