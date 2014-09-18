/* Copyright (C) Uppsala University
 *
 * This file is distributed under the terms of the GNU general Public
 * License (GPL), see the file LICENSE
 *
 * Author: Erik Nordström, <erikn@it.uu.se>
 */
#ifndef _DSR_IO_H
#define _DSR_IO_H

int dsr_recv(struct dsr_pkt *dp);
void dsr_start_xmit(struct dsr_pkt *dp);

#endif              /* _DSR_IO_H */
