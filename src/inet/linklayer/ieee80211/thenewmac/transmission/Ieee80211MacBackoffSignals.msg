//
// Copyright (C) 2015 OpenSim Ltd.
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

namespace inet::ieee80211;

enum Ieee80211MacBackoffSignalType
{
    BkDone = 0;
}

packet Ieee80211MacBackoffSignal
{
    short signalType @enum(Ieee80211MacBackoffSignalType);
}

packet Ieee80211MacBackoffBkDone extends Ieee80211MacBackoffSignal
{
    signalType = BkDone;
    int slotCount;
}
