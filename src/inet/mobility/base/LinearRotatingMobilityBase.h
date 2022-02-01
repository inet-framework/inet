//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_LINEARROTATINGMOBILITYBASE_H
#define __INET_LINEARROTATINGMOBILITYBASE_H

#include "inet/mobility/base/RotatingMobilityBase.h"

namespace inet {

class INET_API LinearRotatingMobilityBase : public RotatingMobilityBase
{
  protected:
    /** @brief End position of current linear movement. */
    Quaternion targetOrientation;

  protected:
    virtual void initializeOrientation() override;

    virtual void rotate() override;

    /**
     * @brief Should be redefined in subclasses. This method gets called
     * when targetOrientation and nextChange has been reached, and its task is
     * to set a new targetOrientation and nextChange. At the end of the movement
     * sequence, it should set nextChange to -1.
     */
    virtual void setTargetOrientation() = 0;

  public:
    LinearRotatingMobilityBase();
};

} // namespace inet

#endif

