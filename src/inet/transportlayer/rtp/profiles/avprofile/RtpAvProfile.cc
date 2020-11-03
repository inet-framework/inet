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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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

