//
// Copyright (C) 2018 OpenSim Ltd.
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

}
