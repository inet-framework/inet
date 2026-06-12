//
// Copyright (C) 2007
// Faqir Zarrar Yousaf
// Communication Networks Institute, University of Dortmund, Germany.
// Christian Bauer
// Institute of Communications and Navigation, German Aerospace Center (DLR)
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_MOBILITYCONSTANTS_H
#define __INET_MOBILITYCONSTANTS_H

namespace inet {

// Return Routability tokens and cookies
constexpr int UNDEFINED_TOKEN           = 0;
constexpr int UNDEFINED_COOKIE          = 0;
constexpr int UNDEFINED_BIND_AUTH_DATA  = 0;
constexpr int HO_COOKIE                 = 11;
constexpr int HO_TOKEN                  = 1101;
constexpr int CO_COOKIE                 = 21;
constexpr int CO_TOKEN                  = 2101;

// Amount of seconds before BUL expiry that indicate that a binding will shortly expire
constexpr int PRE_BINDING_EXPIRY        = 2;

} // namespace inet

#endif
