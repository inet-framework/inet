#ifndef __GNPLIB_IMPL_COMMON_RANDOM_DEFAULTRNG_H
#define __GNPLIB_IMPL_COMMON_RANDOM_DEFAULTRNG_H

/*
 * File: DefaultRng.h
 * Copyright (C) 2009 Philipp Berndt <philipp.berndt@tu-berlin.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

/**
 * The following functions are used by gnplib as default random number source.
 * 
 * @author Philipp Berndt <philipp.berndt@tu-berlin.de>
 * 
 */

#include <gnplib/api/common/random/Rng.h>

namespace gnplib { namespace impl { namespace common { namespace random {

namespace DefaultRng {
    int intrand(int);
    double dblrand(); 
}

} } } } // namespace gnplib::impl:common::random

#endif // not defined __GNPLIB_IMPL_COMMON_RANDOM_DEFAULTRNG_H

