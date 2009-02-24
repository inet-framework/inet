//**************************************************************************
// * file:        TransmComplete.msg
// *
// * author:      Marc Loebbers
// *
// * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
// *              Technische Universitaet Berlin, Germany.
// *
// *              This program is free software; you can redistribute it
// *              and/or modify it under the terms of the GNU General Public
// *              License as published by the Free Software Foundation; either
// *              version 2 of the License, or (at your option) any later
// *              version.
// *              For further information see file COPYING
// *              in the top level directory
// **************************************************************************
// * part of:     framework implementation developed by tkn
// **************************************************************************/

cplusplus {{
#include "INETDefs.h"
}}


//
// Timer to indicate a completely received frame
//
// This message class is used within BasicSnrEval to indicate that an
// AirFrame is completely received. It has no parameters and was only
// introduced to differentiate this timer from other self messages
//
// This component has been taken over from Mobility Framework 1.0a5.
//
// @author Daniel Willkomm
//
packet TransmComplete
{
}
