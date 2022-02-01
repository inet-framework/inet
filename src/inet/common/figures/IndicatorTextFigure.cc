//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/figures/IndicatorTextFigure.h"

#include "inet/common/INETUtils.h"

namespace inet {

static const char *PKEY_TEXT_FORMAT = "textFormat";
static const char *PKEY_INITIAL_VALUE = "initialValue";

Register_Figure("indicatorText", IndicatorTextFigure);

const char **IndicatorTextFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = {
            PKEY_TEXT_FORMAT, PKEY_INITIAL_VALUE, nullptr
        };
        concatArrays(keys, cTextFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

void IndicatorTextFigure::parse(cProperty *property)
{
    cTextFigure::parse(property);

    const char *s;
    if ((s = property->getValue(PKEY_TEXT_FORMAT)) != nullptr)
        setTextFormat(s);
    if ((s = property->getValue(PKEY_INITIAL_VALUE)) != nullptr)
        setValue(0, simTime(), utils::atod(s));
}

void IndicatorTextFigure::setValue(int series, simtime_t timestamp, double value)
{
    // Note: we currently ignore timestamp
    ASSERT(series == 0);
    this->value = value;
    refresh();
}

void IndicatorTextFigure::refresh()
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

