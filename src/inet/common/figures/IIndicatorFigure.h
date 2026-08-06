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

    /**
     * Returns the index of the item identified by the given label, or -1 if
     * this figure has no such item. Figures that display a dynamically changing
     * set of items (e.g. one per peer or per flow of a demultiplexed statistic)
     * create the item when createIfMissing is true. The default implementation
     * maps the empty label to the first item, which is what figures displaying
     * a single item need.
     */
    virtual int getItemIndex(const char *label, bool createIfMissing = false) { return opp_isempty(label) ? 0 : -1; }

    virtual void setValue(int series, simtime_t timestamp, double value) = 0;

    /**
     * Called after the values have been updated, before the figure is
     * displayed. Figures that compute their layout from all of their values
     * (and thus cannot do it in setValue()) do it here.
     */
    virtual void refreshDisplay() {}
};

} // namespace inet

#endif

