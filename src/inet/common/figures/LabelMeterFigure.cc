//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "LabelMeterFigure.h"

// namespace inet {

static const char *TEXT_FORMAT_PROPERTY = "textFormat";

Register_Class(LabelMeterFigure);

const char **LabelMeterFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = { TEXT_FORMAT_PROPERTY, nullptr};
        concatArrays(keys, cLabelFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

void LabelMeterFigure::parse(cProperty *property)
{
    cLabelFigure::parse(property);

    const char *s;
    if ((s = property->getValue(TEXT_FORMAT_PROPERTY)) != nullptr)
        setTextFormat(s);
}

void LabelMeterFigure::setValue(int series, simtime_t timestamp, double value)
{
    // Note: we currently ignore timestamp
    ASSERT(series == 0);
    this->value = value;
    refresh();
}

void LabelMeterFigure::refresh()
{
    char buf[64];
    sprintf(buf, textFormat.c_str(), value);
    setText(buf);
}


// } // namespace inet

