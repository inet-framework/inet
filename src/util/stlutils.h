//
// Copyright (C) 2012 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_STLUTILS_H
#define __INET_STLUTILS_H

// various utility functions to make STL containers more usable

#include <vector>
#include <algorithm>

template<typename T>
typename std::vector<T>::iterator find(std::vector<T>& v, T& a) {return std::find(v.begin(), v.end(), a);}

template<typename T>
typename std::vector<T>::const_iterator find(const std::vector<T>& v, T& a) {return std::find(v.begin(), v.end(), a);}

template<typename T>
inline bool contains(const std::vector<T>& v, T& a) {return find(v, a) != v.end();}

#endif


