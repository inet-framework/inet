//
// Copyright (C) 2006 Andras Varga
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_MODULATION_H
#define __INET_MODULATION_H

#include "inet/physicallayer/contract/IModulation.h"

namespace inet {

namespace physicallayer {

/**
 * BPSK modulation.
 */
class INET_API BPSKModulation : public IModulation
{
  public:
    virtual const char *getName() override { return "BPSK"; }
    virtual double calculateBER(double snir, double bandwidth, double bitrate) const override;
};

/**
 * 16-QAM modulation.
 */
class INET_API QAM16Modulation : public IModulation
{
  public:
    virtual const char *getName() override { return "16-QAM"; }
    virtual double calculateBER(double snir, double bandwidth, double bitrate) const override;
};

/**
 * 256-QAM modulation.
 */
class INET_API QAM256Modulation : public IModulation
{
  public:
    virtual const char *getName() override { return "256-QAM"; }
    virtual double calculateBER(double snir, double bandwidth, double bitrate) const override;
};

/**
 * DSSS OQPSK-16 modulation used in the 802.15.4 standard
 */
class INET_API DSSSOQPSK16Modulation : public IModulation
{
  public:
    virtual const char *getName() override { return "DSSS-OQPSK-16"; }
    virtual double calculateBER(double snir, double bandwidth, double bitrate) const override;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_MODULATION_H

