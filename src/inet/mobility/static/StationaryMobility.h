//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_STATIONARYMOBILITY_H
#define __INET_STATIONARYMOBILITY_H

#include "inet/mobility/base/StationaryMobilityBase.h"

namespace inet {

/**
 * This mobility module does not move at all; it can be used for standalone stationary nodes.
 *
 * @ingroup mobility
 */
class INET_API StationaryMobility : public StationaryMobilityBase
{
  protected:
    bool updateFromDisplayString;
    // The p tag arguments of the subject module's display string, remembered in
    // INITSTAGE_SINGLE_MOBILITY right after MobilityBase writes them and at the end of each
    // refreshDisplay(); a change from these was written by someone else, e.g. by the user
    // dragging the icon
    mutable std::string displayStringPositionX;
    mutable std::string displayStringPositionY;

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;
    virtual void updateMobilityStateFromDisplayString();
    void rememberDisplayStringPosition() const;
};

} // namespace inet

#endif

