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

  public:
    Ieee80211ModeBase(const char *name) : name(name) {}
    virtual const char *getName() const override { return name.c_str(); }
};

} /* namespace physicallayer */
} /* namespace inet */

#endif

