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

    /**
     * @deprecated Renamed to getNumItems(); override that instead. A figure that
     * still overrides this method keeps working for one release, because the
     * default implementation of getNumItems() calls it.
     */
    [[deprecated("renamed to getNumItems(), override getNumItems() instead")]]
    virtual int getNumSeries() const { return 1; }

    virtual int getNumItems() const {
        // deliberately calls the deprecated method, so that a figure which has not
        // moved its override yet still reports its own number of items
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
        return getNumSeries();
#pragma GCC diagnostic pop
    }

    virtual void setValue(int index, simtime_t timestamp, double value) = 0;
    virtual void refreshDisplay() {}
};

} // namespace inet

#endif

