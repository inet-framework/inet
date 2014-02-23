//
// Copyright (C) 2014 Andrea Tino
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

//
// Ieee80211xRadioModelUtils.h
//

#ifndef IEEE80211XRADIOMODELUTILS_H_
#define IEEE80211XRADIOMODELUTILS_H_

#include "Ieee80211xRadioModel.h"

/**
 * Internal utilities.
 *
 * @author Andrea Tino
 */
class Ieee80211xRadioModel::Utils {
  public:
    /**
     * @brief TODO.
     */
    static double dB2fraction(double dB);
}; /* Utils */

#endif
