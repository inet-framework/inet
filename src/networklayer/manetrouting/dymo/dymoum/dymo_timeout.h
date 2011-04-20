/***************************************************************************
 *   Copyright (C) 2005 by Francisco J. Ros                                *
 *   fjrm@dif.um.es                                                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef __DYMO_TIMEOUT_H__
#define __DYMO_TIMEOUT_H__

#ifndef NS_NO_GLOBALS
#include "defs_dymo.h"
#endif  /* NS_NO_GLOBALS */

#ifndef NS_NO_DECLARATIONS

/* Handler function called when a routing table entry becomes invalid */
void route_valid_timeout(void *arg);

/* Handler function called when a routing table entry must be deleted */
void route_del_timeout(void *arg);

/* Handler function called when no RREP has been received after sending a
   RREQ */
void route_discovery_timeout(void *arg);

/* Handler function called when an entry in the blacklist expired */
void blacklist_timeout(void *arg);

/* Handler function called when an entry in the neighbor list expired */
void nb_timeout(void *arg);

#endif  /* NS_NO_DECLARATIONS */

#endif  /* __DYMO_TIMEOUT_H__ */
