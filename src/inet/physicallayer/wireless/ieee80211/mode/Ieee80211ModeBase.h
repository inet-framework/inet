//
// Copyright (C) 2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211MODEBASE_H
#define __INET_IEEE80211MODEBASE_H

#include "inet/physicallayer/wireless/ieee80211/mode/IIeee80211Mode.h"

namespace inet {
namespace physicallayer {

class INET_API Ieee80211ModeBase : public IIeee80211Mode
{
  private:
    std::string name;

  protected:
    // Maps stream 1 modulation and code rate; returns NaN for an unsupported pair.
    static bps computeNonHtReferenceRate(unsigned int constellationSize, double codeRate);

  public:
    Ieee80211ModeBase(const char *name) : name(name) {}
    virtual bps getNonHtReferenceRate() const override { return bps(NaN); }
    virtual ModulationClass getModulationClass() const override { return ModulationClass::UNKNOWN; }
    virtual PreambleType getLegacyPreambleType() const override { return PreambleType::UNKNOWN; }
    virtual int getHtMcsIndex() const override { return -1; }
    virtual bool isHtShortGuardInterval() const override { return false; }
    virtual const char *getName() const override { return name.c_str(); }
};

} /* namespace physicallayer */
} /* namespace inet */

#endif
