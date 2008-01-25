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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef MODULATION_H
#define MODULATION_H

#include "IModulation.h"

/**
 * Ideal modulation which returns zero bit error rate, regardless of the parameters.
 */
class INET_API NullModulation : public IModulation
{
  public:
    virtual const char *name() {return "no bit errors";}
    virtual double bitErrorRate(double snir, double bandwidth, double bitrate);
};

/**
 * BPSK modulation.
 */
class INET_API BPSKModulation : public IModulation
{
  public:
    virtual const char *name() {return "BPSK";}
    virtual double bitErrorRate(double snir, double bandwidth, double bitrate);
};

/**
 * 16-QAM modulation.
 */
class INET_API QAM16Modulation : public IModulation
{
  public:
    virtual const char *name() {return "16-QAM";}
    virtual double bitErrorRate(double snir, double bandwidth, double bitrate);
};

/**
 * 256-QAM modulation.
 */
class INET_API QAM256Modulation : public IModulation
{
  public:
    virtual const char *name() {return "256-QAM";}
    virtual double bitErrorRate(double snir, double bandwidth, double bitrate);
};


#endif

