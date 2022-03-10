//
// Copyright (c) 2010 The Boeing Company
// Copyright (C) 2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

// Author: Gary Pei <guangyu.pei@boeing.com>
//

#ifndef __INET_IEEE80211NISTERRORMODEL_H
#define __INET_IEEE80211NISTERRORMODEL_H

#include "inet/physicallayer/wireless/common/base/packetlevel/ApskModulationBase.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/ConvolutionalCode.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/errormodel/Ieee80211ErrorModelBase.h"

namespace inet {
namespace physicallayer {

/**
 * A model for the error rate for different modulations.  For OFDM modulation,
 * the model description and validation can be found in
 * http://www.nsnam.org/~pei/80211ofdm.pdf.  For DSSS modulations (802.11b),
 * the model uses a separate error model.
 */
class INET_API Ieee80211NistErrorModel : public Ieee80211ErrorModelBase
{
  protected:
    double calculatePe(double p, uint32_t bValue) const;
    double getBpskBer(double snr) const;
    double getQpskBer(double snr) const;
    double get16QamBer(double snr) const;
    double get64QamBer(double snr) const;
    double get256QamBer(double snr) const;
    double get1024QamBer(double snr) const;
    double getFecBpskBer(double snr, double nbits, uint32_t bValue) const;
    double getFecQpskBer(double snr, double nbits, uint32_t bValue) const;
    double getFec16QamBer(double snr, uint32_t nbits, uint32_t bValue) const;
    double getFec64QamBer(double snr, uint32_t nbits, uint32_t bValue) const;
    double getFec256QamBer(double snr, uint64_t nbits, uint32_t bValue) const;
    double getFec1024QamBer(double snr, uint64_t nbits, uint32_t bValue) const;

    virtual double getOFDMAndERPOFDMChunkSuccessRate(const ApskModulationBase *subcarrierModulation, const ConvolutionalCode *convolutionalCode, unsigned int bitLength, double snr) const;
    virtual double getDSSSAndHrDSSSChunkSuccessRate(bps bitrate, unsigned int bitLength, double snr) const;

    virtual double getHeaderSuccessRate(const IIeee80211Mode *mode, unsigned int headerBitLength, double snr) const override;
    virtual double getDataSuccessRate(const IIeee80211Mode *mode, unsigned int bitLength, double snr) const override;

  public:
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override { return stream << "Ieee80211NistErrorModel"; }
};

} // namespace physicallayer
} // namespace inet

#endif

