//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CABLE_H
#define __INET_CABLE_H

#include "inet/common/INETDefs.h"

namespace inet {

class INET_API Cable : public cDatarateChannel
{
  protected:
    const char *enabledLineStyle = nullptr;
    const char *disabledLineStyle = nullptr;
    cIconFigure *disabledFigure = nullptr;

  protected:
    virtual void initialize() override;
    virtual void refreshDisplay() const override;
};

}  // namespace inet

#endif
