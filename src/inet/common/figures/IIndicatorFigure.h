//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IINDICATORFIGURE_H
#define __INET_IINDICATORFIGURE_H

#include "inet/common/INETDefs.h"

namespace inet {

class INET_API IIndicatorFigure
{
  public:
    virtual ~IIndicatorFigure() {}
    virtual const cFigure::Point getSize() const = 0;
    virtual int getNumSeries() const { return 1; }
    virtual void setValue(int series, simtime_t timestamp, double value) = 0;
    virtual void refreshDisplay() {}
};

} // namespace inet

#endif

