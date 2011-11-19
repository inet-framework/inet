/*
 *  Copyright (C) 2005 Mohamed Louizi
 *  Copyright (C) 2006,2007 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/**
 * Helper functions for Tkenv to display our user-defined vectors
 */
#ifndef INSPECTOR_HELPERS_H_98345979
#define INSPECTOR_HELPERS_H_98345979

#include <iostream>
#include "DYMO_RM_m.h"
#include "DYMO_RERR_m.h"

std::ostream& operator<<(std::ostream& os, const std::vector<DYMO_AddressBlock>& abs);
std::ostream& operator<<(std::ostream& os, const DYMO_AddressBlock& ab);

#endif
