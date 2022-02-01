//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

/***************************************************************************
                          RtpAvProfile.cc  -  description
                             -------------------
    begin                : Thu Nov 29 2001
    copyright            : (C) 2001 by Matthias Oppitz
    email                : Matthias.Oppitz@gmx.de
***************************************************************************/

#include "inet/transportlayer/rtp/profiles/avprofile/RtpAvProfile.h"

namespace inet {

namespace rtp {

Define_Module(RtpAvProfile);

void RtpAvProfile::initialize()
{
    RtpProfile::initialize();
    _profileName = "AvProfile";
    _rtcpPercentage = 5;
    _preferredPort = 5005;
}

} // namespace rtp

} // namespace inet

