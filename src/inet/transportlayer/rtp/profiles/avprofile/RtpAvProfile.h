//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

/***************************************************************************
                          RtpAvProfile.h  -  description
                             -------------------
    begin                : Thu Nov 29 2001
    copyright            : (C) 2001 by Matthias Oppitz
    email                : Matthias.Oppitz@gmx.de
***************************************************************************/

#ifndef __INET_RTPAVPROFILE_H
#define __INET_RTPAVPROFILE_H

#include "inet/transportlayer/rtp/RtpProfile.h"

namespace inet {

namespace rtp {

/**
 * The class RtpAvProfile is a subclass of RtpProfile. It does not extend
 * the functionality of its super class, it just sets some values in
 * its initialize() method.
 * For for information about the rtp audio/video profile consult
 * rfc 1890.
 */
class INET_API RtpAvProfile : public RtpProfile
{
  protected:
    /**
     * This initialisation method sets following values:
     * name, rtcpPercentage and preferredPort.
     */
    virtual void initialize() override;
};

} // namespace rtp

} // namespace inet

#endif

