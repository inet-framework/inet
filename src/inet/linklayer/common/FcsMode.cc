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
#include "inet/linklayer/common/FcsMode_m.h"

namespace inet {

FcsMode parseFcsMode(const char *fcsModeString)
{
    if (!strcmp(fcsModeString, "disabled"))
        return FCS_DISABLED;
    else if (!strcmp(fcsModeString, "declared"))
        return FCS_DECLARED_CORRECT;
    else if (!strcmp(fcsModeString, "declaredCorrect"))
        return FCS_DECLARED_CORRECT;
    else if (!strcmp(fcsModeString, "declaredIncorrect"))
        return FCS_DECLARED_INCORRECT;
    else if (!strcmp(fcsModeString, "computed"))
        return FCS_COMPUTED;
    else
        throw cRuntimeError("Unknown FCS mode: '%s'", fcsModeString);
}

} // namespace inet

