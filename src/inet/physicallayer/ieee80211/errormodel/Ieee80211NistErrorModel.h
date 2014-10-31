/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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
#ifndef __INET_NISTERRORRATEMODEL_H
#define __INET_NISTERRORRATEMODEL_H

#include "inet/physicallayer/ieee80211/errormodel/Ieee80211ErrorModelBase.h"
#include "inet/physicallayer/ieee80211/errormodel/dsss-error-rate-model.h"

namespace inet {

namespace physicallayer {

/**
 * A model for the error rate for different modulations.  For OFDM modulation,
 * the model description and validation can be found in
 * http://www.nsnam.org/~pei/80211ofdm.pdf.  For DSSS modulations (802.11b),
 * the model uses the DsssErrorRateModel.
 */
class Ieee80211NistErrorModel : public Ieee80211ErrorModelBase
{
  public:

    Ieee80211NistErrorModel();
    virtual ~Ieee80211NistErrorModel();

    virtual void printToStream(std::ostream& stream) const { stream << "Ieee80211NistErrorModel"; }
    virtual double GetChunkSuccessRate(ModulationType mode, double snr, uint32_t nbits) const;

  private:
    double CalculatePe(double p, uint32_t bValue) const;
    double GetBpskBer(double snr) const;
    double GetQpskBer(double snr) const;
    double Get16QamBer(double snr) const;
    double Get64QamBer(double snr) const;
    double GetFecBpskBer(double snr, double nbits,
            uint32_t bValue) const;
    double GetFecQpskBer(double snr, double nbits,
            uint32_t bValue) const;
    double GetFec16QamBer(double snr, uint32_t nbits,
            uint32_t bValue) const;
    double GetFec64QamBer(double snr, uint32_t nbits,
            uint32_t bValue) const;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_NISTERRORRATEMODEL_H

