//**************************************************************************
// * file:        CSMAFrame.msg
// *
// * author:      Daniel Willkomm
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
// * description: - base class for MAC layer packets
// *              - subclass to create your own MAC layer packets
// **************************************************************************/

cplusplus {{
    #include "inet/linklayer/common/MACFrameBase_m.h"
}}

namespace inet;

class MACFrameBase;

//
// A CSMA mac packet format definition
//
// @author Daniel Willkomm
//
packet CSMAFrame extends MACFrameBase
{
    long       sequenceId; // Sequence Number to detect duplicate messages
}
