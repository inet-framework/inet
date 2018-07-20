/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 The Boeing Company
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Gary Pei <guangyu.pei@boeing.com>
 */

#ifndef __INET_DSSS_ERROR_RATE_MODEL_H
#define __INET_DSSS_ERROR_RATE_MODEL_H
//#include <stdint.h>

#ifdef ENABLE_GSL
#include <gsl/gsl_math.h>
#include <gsl/gsl_integration.h>
#include <gsl/gsl_cdf.h>
#include <gsl/gsl_sf_bessel.h>

#endif // ifdef ENABLE_GSL
namespace inet {

namespace physicallayer {

#ifdef ENABLE_GSL
typedef struct FunctionParameterType
{
    double beta;
    double n;
} FunctionParameters;

double IntegralFunction(double x, void *params);
#endif // ifdef ENABLE_GSL

/**
 * \brief an implementation of DSSS error rate model
 *
 * The 802.11b modulations:
 *    - 1 Mbps mode is based on DBPSK. BER is from equation 5.2-69 from John G. Proakis
 *      Digitial Communications, 2001 edition
 *    - 2 Mbps model is based on DQPSK. Equation 8 from "Tight bounds and accurate
 *      approximations for dqpsk transmission bit error rate", G. Ferrari and G.E. Corazza
 *      ELECTRONICS LETTERS, 40(20):1284-1285, September 2004
 *    - 5.5 Mbps and 11 Mbps are based on equations (18) and (17) from "Properties and
 *      performance of the ieee 802.11b complementarycode-key signal sets",
 *      Michael B. Pursley and Thomas C. Royster. IEEE TRANSACTIONS ON COMMUNICATIONS,
 *      57(2):440-449, February 2009.
 *
 *  This model is designed to run with highest accuracy using the Gnu
 *  Scientific Library (GSL), but if GSL is not installed on the platform,
 *  will fall back to (slightly less accurate) Matlab-derived models for
 *  the CCK modulation types.
 *
 *  More detailed description and validation can be found in
 *      http://www.nsnam.org/~pei/80211b.pdf
 */
class INET_API DsssErrorRateModel
{
  private:
    static const double spectralEfficiency1bit;
    static const double spectralEfficiency2bit;

  public:
    static double DqpskFunction(double x);
    static double GetDsssDbpskSuccessRate(double sinr, uint32_t nbits);
    static double GetDsssDqpskSuccessRate(double sinr, uint32_t nbits);
    static double GetDsssDqpskCck5_5SuccessRate(double sinr, uint32_t nbits);
    static double GetDsssDqpskCck11SuccessRate(double sinr, uint32_t nbits);
#ifdef ENABLE_GSL
    static double SymbolErrorProb16Cck(double e2);    /// equation (18) in Pursley's paper
    static double SymbolErrorProb256Cck(double e1);    /// equation (17) in Pursley's paper
#else // ifdef ENABLE_GSL

  protected:
    static const double WLAN_SIR_PERFECT;
    static const double WLAN_SIR_IMPOSSIBLE;
#endif // ifdef ENABLE_GSL
};

} // namespace physicallayer

} // namespace inet

#endif /* DSSS_ERROR_RATE_MODEL_H */

