/***************************************************************************
                          RtpAvProfile.h  -  description
                             -------------------
    begin                : Thu Nov 29 2001
    copyright            : (C) 2001 by Matthias Oppitz
    email                : Matthias.Oppitz@gmx.de
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "inet/common/INETDefs.h"

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

