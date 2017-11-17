//
// Copyright (C) 2017 OpenSim Ltd.
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
// author: Zoltan Bojthe
//

import inet.common.INETDefs;

namespace inet;

enum EthernetFcsMode {
    FCS_DECLARED_CORRECT = 1;
    FCS_DECLARED_INCORRECT = 2;
    FCS_COMPUTED = 3;
}

cplusplus {{
INET_API EthernetFcsMode parseEthernetFcsMode(const char *fcsModeString);
}}

