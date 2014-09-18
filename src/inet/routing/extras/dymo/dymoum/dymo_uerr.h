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

#ifndef __DYMO_UERR_H__
#define __DYMO_UERR_H__

#ifndef NS_NO_GLOBALS

#include "inet/routing/extras/dymo/dymoum/defs_dymo.h"
#include "inet/routing/extras/dymo/dymoum/dymo_generic.h"

#ifndef OMNETPP
#include <sys/types.h>
#include <netinet/in.h>
#else
#include "inet/routing/extras/base/compatibility.h"
#endif

#ifndef OMNETPP
/* UERR message */
typedef struct      // FIXME: adjust byte ordering
{
    u_int32_t   m : 1;
    u_int32_t   h : 2;
    u_int32_t   type : 5;
    u_int32_t   len : 12;
    u_int32_t   ttl : 6;
    u_int32_t   i : 1;
    u_int32_t   res : 5;

    u_int32_t   target_addr;
    u_int32_t   uelem_target_addr;
    u_int32_t   uerr_node_addr;

    u_int8_t    uelem_type;
} UERR;
#define UERR_SIZE   sizeof(UERR)
#endif

#endif  /* NS_NO_GLOBALS */

#ifndef NS_NO_DECLARATIONS
/* Create a UERR message */
UERR *uerr_create(struct in_addr target_addr, struct in_addr uelem_target_addr,
                  struct in_addr uerr_node_addr, u_int8_t uelem_type, u_int8_t ttl);

/* Send a UERR message given the unsupported message which was received */
void uerr_send(DYMO_element *e, u_int32_t ifindex);

/* Process a UERR message */
void uerr_process(UERR *e, struct in_addr ip_src, u_int32_t ifindex);
#endif  /* NS_NO_DECLARATIONS */




#endif  /* __DYMO_UERR_H__ */
