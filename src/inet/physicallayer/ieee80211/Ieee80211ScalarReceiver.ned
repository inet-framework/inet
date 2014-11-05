//
// Copyright (C) 2013 OpenSim Ltd
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

package inet.physicallayer.ieee80211;

import inet.physicallayer.apsk.APSKScalarReceiver;

module Ieee80211ScalarReceiver extends APSKScalarReceiver
{
    parameters:
        string opMode @enum("b","g","a","p");
        string preambleMode @enum("long", "short");
        errorModelType = default("Ieee80211NistErrorModel");
        // TODO: remove when opMode is removed from the error model
        *.opMode = opMode;
        @class(Ieee80211ScalarReceiver);
}
