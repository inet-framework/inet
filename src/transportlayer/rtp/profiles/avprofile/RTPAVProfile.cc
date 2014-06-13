/***************************************************************************
                          RTPAVProfile.cc  -  description
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


#include "RTPAVProfile.h"


Define_Module(RTPAVProfile);


void RTPAVProfile::initialize()
{
    RTPProfile::initialize();
    _profileName = "AVProfile";
    _rtcpPercentage = 5;
    _preferredPort = 5005;
}
