//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.errormodel.packetlevel;

import inet.physicallayer.wireless.common.base.packetlevel.ErrorModelBase;

//
// This error model provides parameters to specify the constant packet error
// rate, bit error rate, and symbol error rate for receptions independent of
// any interfering transmission or noise. All three parameters are optional.
//
// The three rates are related under the assumption that bit errors are
// independent:
//
//   PER = 1 - (1 - BER)^L        (L: length of the packet in bits)
//   SER = 1 - (1 - BER)^k        (k: bits per symbol of the modulation)
//
// Each of the three rates the error model returns is the value of its own
// parameter if that is set, and is derived from the others if it is not:
//
// <pre>
//   packet error rate  = packetErrorRate if set (whole packet), otherwise
//                        computed from the bit error rate over the length of
//                        the header and of the data part
//   bit error rate     = bitErrorRate if set, otherwise computed from
//                        symbolErrorRate if that is set and the modulation has
//                        bits per symbol, otherwise computed from
//                        packetErrorRate over the header and data bits
//   symbol error rate  = symbolErrorRate if set, otherwise computed from the
//                        bit error rate
// </pre>
//
// So a rate that is set is returned as is, and when several parameters are set
// the bit error rate takes precedence over the symbol error rate, which takes
// precedence over the packet error rate for the rates that are derived. A rate
// that cannot be derived (for example no parameter is set) is NaN.
//
// The packet error rate of the HEADER and DATA parts of a reception is
// computed from the length of the part. If packetErrorRate is set, the rate
// of a part is its share (1 - (1 - PER)^(L_part / L)) of the whole, so that
// the parts compose to the packetErrorRate. Otherwise the error rate of the
// whole packet is composed from the rates of the two parts, using the
// modulation of the header for the header and the modulation of the data for
// the data, in the same way as the other error models do:
// PER = 1 - (1 - PER_header) * (1 - PER_data). The bit error rate and the
// symbol error rate of the whole packet, which are only reported, use the
// modulation of the data part.
//
// The header and data lengths and modulations come from the transmission if
// it implements IFlatTransmission (IEEE 802.11, IEEE 802.15.4 narrowband and
// APSK), otherwise from the bit model of the transmission if there is one,
// otherwise the whole packet is data without a modulation. A length of a
// transmission that is negative (for example in IEEE 802.15.4 UWB-IR) is
// unknown: the lengths are then taken from the bit model if there is one,
// otherwise the whole packet is data.
//
// Approximations: the relation between the symbol and bit error rates is that
// of uncoded transmission. The coding rate of the transmission is ignored,
// that is, k is the number of coded bits that a constellation symbol carries,
// while L counts the bits of the packet after decoding (including tail and
// padding bits for IEEE 802.11 OFDM). For OFDM the symbol is a symbol of the
// subcarrier modulation (for example a 16-QAM symbol), not an OFDM symbol.
// The symbol error rate is therefore not the symbol error rate of the decoded
// packet.
//
// Modulations that have no number of bits per symbol (the CCK modulation of
// the 5.5 and 11 Mbit/s rates of IEEE 802.11b and g (HR/DSSS), IEEE 802.11
// FHSS, and the FSK, PPM and MFSK modulations in general) give no bit error
// rate from symbolErrorRate, and no symbol error rate from the other rates.
// If symbolErrorRate is the only parameter that is set, the packet error
// rate cannot be computed for these, and the error model stops the
// simulation with an error that names the modulation. Set bitErrorRate or
// packetErrorRate for these modes.
//
module StochasticErrorModel extends ErrorModelBase
{
    parameters:
        double packetErrorRate = default(nan); // A probability in the range [0, 1] or NaN if not set
        double bitErrorRate = default(nan);    // A probability in the range [0, 1] or NaN if not set
        double symbolErrorRate = default(nan); // A probability in the range [0, 1] or NaN if not set
        @class(StochasticErrorModel);
}

