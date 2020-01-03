/*
 * Copyright (C) 2017 OpenSim Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * author: Zoltan Bojthe
 */

#include <string.h>

#include "inet/common/INETDefs.h"
#include "inet/transportlayer/common/CrcMode_m.h"

namespace inet {

CrcMode parseCrcMode(const char *crcModeString, bool allowDisable)
{
    if (!strcmp(crcModeString, "disabled")) {
        if (allowDisable)
            return CRC_DISABLED;
        else
            throw cRuntimeError("The 'disabled' CRC mode not allowed");
    }
    else if (!strcmp(crcModeString, "declared"))
        return CRC_DECLARED_CORRECT;
    else if (!strcmp(crcModeString, "declaredCorrect"))
        return CRC_DECLARED_CORRECT;
    else if (!strcmp(crcModeString, "declaredIncorrect"))
        return CRC_DECLARED_INCORRECT;
    else if (!strcmp(crcModeString, "computed"))
        return CRC_COMPUTED;
    else
        throw cRuntimeError("Unknown CRC mode: '%s'", crcModeString);
}

} // namespace inet

