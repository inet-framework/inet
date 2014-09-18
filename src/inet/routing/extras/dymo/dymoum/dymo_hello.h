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

#ifndef __DYMO_HELLO_H__
#define __DYMO_HELLO_H__

#ifndef NS_NO_GLOBALS
#ifndef OMNETPP
#include <sys/types.h>
#include <netinet/in.h>
#else
#include "inet/routing/extras/base/compatibility.h"

#endif
#ifndef OMNETPP
namespace inet {

namespace inetmanet {

/* HELLO message */
typedef struct      // FIXME: adjust byte ordering
{
    u_int32_t   m : 1;
    u_int32_t   h : 2;
    u_int32_t   type : 5;
    u_int32_t   len : 12;
    u_int32_t   ttl : 6;
    u_int32_t   i : 1;
    u_int32_t   res : 5;
} HELLO;

#define HELLO_BASIC_SIZE    sizeof(HELLO)

} // namespace inetmanet

} // namespace inet

#endif


#endif  /* NS_NO_GLOBALS */

#ifndef NS_NO_DECLARATIONS
/* Start sending of HELLO messages */
void hello_init(void);

/* Finish sending of HELLO messages */
void hello_fini(void);

/* Return a HELLO message */
HELLO *hello_create(void);

/* Send a HELLO message and schedule the next. Use hello_init() and
   hello_fini() instead.*/
void hello_send(void *arg);

/* Process a HELLO message */
void hello_process(HELLO *hello,struct in_addr ip_src, u_int32_t ifindex);

/* Return a random jitter */
long hello_jitter(void);

#endif  /* NS_NO_DECLARATIONS */

#endif  /* __DYMO_HELLO_H__ */

