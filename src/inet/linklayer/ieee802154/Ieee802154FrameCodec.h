// SPDX-License-Identifier: LGPL-3.0-or-later
//
#ifndef __INET_IEEE802154FRAMECODEC_H
#define __INET_IEEE802154FRAMECODEC_H

#include <vector>
#include "inet/linklayer/ieee802154/Ieee802154Frame_m.h"

namespace inet {

class INET_API Ieee802154FrameCodec
{
  public:
    enum DecodeStatus { OK, TRUNCATED, MALFORMED, UNSUPPORTED };
    static uint16_t calculateFcs(const std::vector<uint8_t>& bytes);
    static std::vector<uint8_t> encode(const Ieee802154Frame& frame);
    static Ptr<Ieee802154Frame> decode(const std::vector<uint8_t>& bytes, bool fcsPresent = true);
    static void finalize(Ieee802154Frame& frame);
};

} // namespace inet
#endif
