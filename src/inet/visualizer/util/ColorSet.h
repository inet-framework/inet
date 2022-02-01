//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_COLORSET_H
#define __INET_COLORSET_H

#include "inet/common/INETDefs.h"

namespace inet {

namespace visualizer {

class INET_API ColorSet
{
  protected:
    std::vector<cFigure::Color> colors;

  public:
    void parseColors(const char *colorNames);
    size_t getSize() const { return colors.size(); }
    cFigure::Color getColor(int index) const;
};

} // namespace visualizer

} // namespace inet

#endif

