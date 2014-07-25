//
// Copyright (C) 2006 Andras Varga, Levente Meszaros
// Based on the Mobility Framework's SnrEval by Marc Loebbers
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef __INET_BERPARSEFILE_H
#define __INET_BERPARSEFILE_H

#include <vector>
#include <math.h>

namespace inet {

namespace physicallayer {

/**
 * Radio model for IEEE 802.11. The implementation is largely based on the
 * Mobility Framework's SnrEval80211 and Decider80211 modules.
 * See the NED file for more info.
 */
class BerParseFile
{
  protected:
    struct SnrBer
    {
        double snr;
        double ber;
        inline SnrBer& operator=(const SnrBer& m)
        {
            snr = m.snr;
            ber = m.ber;
            return *this;
        }

        bool operator<(SnrBer const& o) const
        {
            return (snr < o.snr) ? true : false;
        }
    };
    typedef std::vector<SnrBer> SnrBerList;
    struct LongBer
    {
        int longpkt;
        SnrBerList snrlist;
    };

    typedef std::vector<LongBer *> BerList;
// A and G
    typedef std::vector<BerList> BerTable;
    BerTable berTable;
    char phyOpMode;
    bool fileBer;

    int getTablePosition(double speed);
    void clearBerTable();
    double dB2fraction(double dB)
    {
        return pow(10.0, (dB / 10));
    }

  public:
    void parseFile(const char *filename);
    bool isFile() { return fileBer; }
    void setPhyOpMode(char p);
    double getPer(double speed, double tsnr, int tlen);
    BerParseFile(char p) { setPhyOpMode(p); fileBer = false; }
    ~BerParseFile();
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_BERPARSEFILE_H

