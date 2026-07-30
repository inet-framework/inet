//
// Copyright (C) 2005 Vojtech Janota
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_UTILS_H
#define __INET_UTILS_H

#include <vector>

#include "inet/networklayer/rsvpte/IntServ_m.h"

namespace inet {

std::string vectorToString(const EroVector& vec);
std::string vectorToString(const EroVector& vec, const char *delim);

} // namespace inet

#endif

