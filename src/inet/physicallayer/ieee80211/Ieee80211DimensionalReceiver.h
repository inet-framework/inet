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

#ifndef __INET_IEEE80211DIMENSIONALRECEIVER_H
#define __INET_IEEE80211DIMENSIONALRECEIVER_H

#include "inet/physicallayer/apsk/APSKDimensionalReceiver.h"
#include "inet/physicallayer/ieee80211/BerParseFile.h"
#include "inet/physicallayer/ieee80211/Ieee80211Modulation.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee80211DimensionalReceiver : public APSKDimensionalReceiver
{
  protected:
    char opMode;
    Ieee80211PreambleMode preambleMode;

  protected:
    virtual void initialize(int stage);
    virtual bool computeIsReceptionPossible(const ITransmission *transmission) const;

  public:
    Ieee80211DimensionalReceiver();
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_IEEE80211DIMENSIONALRECEIVER_H

