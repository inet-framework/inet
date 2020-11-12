//
// Copyright (C) 2016 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "inet/common/figures/IndicatorLabelFigure.h"

#include "inet/common/INETUtils.h"

namespace inet {

static const char *PKEY_TEXT_FORMAT = "textFormat";
static const char *PKEY_INITIAL_VALUE = "initialValue";

Register_Figure("indicatorLabel", IndicatorLabelFigure);

const char **IndicatorLabelFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = {
            PKEY_TEXT_FORMAT, PKEY_INITIAL_VALUE, nullptr
        };
        concatArrays(keys, cLabelFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

void IndicatorLabelFigure::parse(cProperty *property)
{
    cLabelFigure::parse(property);

    const char *s;
    if ((s = property->getValue(PKEY_TEXT_FORMAT)) != nullptr)
        setTextFormat(s);
    if ((s = property->getValue(PKEY_INITIAL_VALUE)) != nullptr)
        setValue(0, simTime(), utils::atod(s));
}

void IndicatorLabelFigure::setValue(int series, simtime_t timestamp, double value)
{
    // Note: we currently ignore timestamp
    ASSERT(series == 0);
    this->value = value;
    refresh();
}

void IndicatorLabelFigure::refresh()
{
    if (std::isnan(value)) {
        setText("");
    }
    else {
        char buf[64];
        sprintf(buf, textFormat.c_str(), value);
        setText(buf);
    }
}

} // namespace inet

