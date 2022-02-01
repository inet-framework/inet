//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/figures/SimTimeTextFigure.h"

namespace inet {

Register_Figure("simTimeText", SimTimeTextFigure);

static const char *PKEY_PREFIX = "prefix";

void SimTimeTextFigure::refreshDisplay() {
    setText((prefix + simTime().format(SimTime::getScaleExp(), ".", "'", true, "", " ")).c_str());
}

void SimTimeTextFigure::parse(cProperty *property)
{
    cTextFigure::parse(property);

    const char *s;

    if ((s = property->getValue(PKEY_PREFIX)) != nullptr)
        prefix = s;
}

const char **SimTimeTextFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = {
            PKEY_PREFIX, nullptr
        };
        concatArrays(keys, cTextFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

} // namespace inet

