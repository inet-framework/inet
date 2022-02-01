//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

// Based on the Mobility Framework's SnrEval by Marc Loebbers
//

#ifndef __INET_BERPARSEFILE_H
#define __INET_BERPARSEFILE_H

#include <math.h>

#include <vector>

#include "inet/common/INETDefs.h"

namespace inet {
namespace physicallayer {

/**
 * Radio model for IEEE 802.11. The implementation is largely based on the
 * Mobility Framework's SnrEval80211 and Decider80211 modules.
 * See the NED file for more info.
 */
class INET_API BerParseFile
{
  protected:
    struct SnrBer {
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
    struct LongBer {
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

#endif

