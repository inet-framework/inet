/* -*- mode:c++ -*- ********************************************************
 * file:        IMobility.cc
 *
 * author:      Levente Meszaros
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              (C) 2005 Andras Varga
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 *************************************************************************/

#include "IMobility.h"

simsignal_t IMobility::mobilityStateChangedSignal = cComponent::registerSignal("mobilityStateChanged");
