//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/util/ColorSet.h"

namespace inet {

namespace visualizer {

void ColorSet::parseColors(const char *colorNames)
{
    colors.clear();
    if (!strcmp(colorNames, "dark")) {
        for (auto color : cFigure::GOOD_DARK_COLORS)
            colors.push_back(color);
    }
    else if (!strcmp(colorNames, "light")) {
        for (auto color : cFigure::GOOD_LIGHT_COLORS)
            colors.push_back(color);
    }
    else {
        cStringTokenizer tokenizer(colorNames, " ,");
        while (tokenizer.hasMoreTokens()) {
            colors.push_back(cFigure::parseColor(tokenizer.nextToken()));
        }
    }
}

cFigure::Color ColorSet::getColor(int index) const
{
    return colors[index % colors.size()];
}

} // namespace visualizer

} // namespace inet

