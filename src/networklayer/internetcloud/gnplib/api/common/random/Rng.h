#ifndef __GNPLIB_API_COMMON_RANDOM_RNG_H
#define __GNPLIB_API_COMMON_RANDOM_RNG_H

/*
 * File: Rng.h
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
 * The following functions are used by gnplib as random number source.
 * 
 * @author Philipp Berndt <philipp.berndt@tu-berlin.de>
 * 
 */

namespace gnplib { namespace api { namespace common { namespace random {

namespace Rng {

/**
 * @arg max
 * @return non-negative integers values uniformly distributed between [0.0, max).
 */
    typedef int(*intrand_t)(int);
    extern intrand_t intrand;

/**
 * @return non-negative double-precision floating-point values uniformly distributed between [0.0, 1.0).
 */
    typedef double(*dblrand_t)(); 
    extern dblrand_t dblrand;
}

} } } } // namespace gnplib::api:common::random

#endif // not defined __GNPLIB_API_COMMON_RANDOM_RNG_H

