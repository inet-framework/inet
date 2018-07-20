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

#ifndef __INET_IEEE80211ERPOFDMMODE_H
#define __INET_IEEE80211ERPOFDMMODE_H

#include "inet/physicallayer/ieee80211/mode/Ieee80211OfdmMode.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee80211ErpOfdmMode : public Ieee80211OfdmMode
{
    protected:
        bool isErpOnly;

    public:
        Ieee80211ErpOfdmMode(const char *name, bool isErpOnly, const Ieee80211OfdmPreambleMode *preambleMode, const Ieee80211OfdmSignalMode *signalMode, const Ieee80211OfdmDataMode *dataMode);

        virtual std::ostream& printToStream(std::ostream& stream, int level) const override { return stream << "Ieee80211ErpOfdmMode"; }

        // The slot time is 20 μs in accordance with 17.3.3, except that an optional 9 μs
        // slot time may be used when the BSS consists of only ERP STAs.
        virtual inline const simtime_t getSlotTime() const override { return isErpOnly ? 9E-6 : 20E-6; }
        const simtime_t getRifsTime() const override;
        // SIFS time is 10 μs in accordance with 17.3.3. See 19.3.2.4 for more detail.
        virtual inline const simtime_t getSifsTime() const override { return 10E-6; }
        // For ERP-OFDM modes, an ERP packet is followed by a period of no transmission
        // with a length of 6 μs called the signal extension.
        virtual inline const simtime_t getDuration(b dataLength) const override { return preambleMode->getDuration() + signalMode->getDuration() + dataMode->getDuration(dataLength) + 6E-6; }
};

class INET_API Ieee80211ErpOfdmCompliantModes
{
    public:
        static const Ieee80211ErpOfdmMode erpOfdmMode6Mbps;
        static const Ieee80211ErpOfdmMode erpOfdmMode9Mbps;
        static const Ieee80211ErpOfdmMode erpOfdmMode12Mbps;
        static const Ieee80211ErpOfdmMode erpOfdmMode18Mbps;
        static const Ieee80211ErpOfdmMode erpOfdmMode24Mbps;
        static const Ieee80211ErpOfdmMode erpOfdmMode36Mbps;
        static const Ieee80211ErpOfdmMode erpOfdmMode48Mbps;
        static const Ieee80211ErpOfdmMode erpOfdmMode54Mbps;

        static const Ieee80211ErpOfdmMode erpOnlyOfdmMode6Mbps;
        static const Ieee80211ErpOfdmMode erpOnlyOfdmMode9Mbps;
        static const Ieee80211ErpOfdmMode erpOnlyOfdmMode12Mbps;
        static const Ieee80211ErpOfdmMode erpOnlyOfdmMode18Mbps;
        static const Ieee80211ErpOfdmMode erpOnlyOfdmMode24Mbps;
        static const Ieee80211ErpOfdmMode erpOnlyOfdmMode36Mbps;
        static const Ieee80211ErpOfdmMode erpOnlyOfdmMode48Mbps;
        static const Ieee80211ErpOfdmMode erpOnlyOfdmMode54Mbps;
};

} /* namespace physicallayer */

} /* namespace inet */

#endif // ifndef __INET_IEEE80211ERPOFDMMODE_H

