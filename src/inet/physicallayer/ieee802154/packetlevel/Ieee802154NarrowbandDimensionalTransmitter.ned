//
// Copyright (C) 2014 Florian Meier
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

package inet.physicallayer.ieee802154.packetlevel;

import inet.physicallayer.base.packetlevel.DimensionalTransmitterBase;

//
// Some example values for the frequencyGains parameter:
// - default (flat signal): "left s 0dB either e 0dB right"
// - 802.15.4 narrowband spectral mask: "left c-3.5MHz 0dB linear c+3.5MHz 0dB right"
// - 802.15.4 narrowband more realistic power spectral density (based on CC2420 ZigBee transmitter): "left c-5MHz -40dB linear c-2MHz -20dB linear c-1MHz 0dB either c+1MHz 0dB linear c+2MHz -20dB linear c+5MHz -40dB right"
//

module Ieee802154NarrowbandDimensionalTransmitter extends DimensionalTransmitterBase
{
    parameters:
        modulation = default("DSSS-OQPSK-16");
        @class(Ieee802154NarrowbandDimensionalTransmitter);
}

