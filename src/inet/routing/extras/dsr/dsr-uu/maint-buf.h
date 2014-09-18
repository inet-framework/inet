/* Copyright (C) Uppsala University
 *
 * This file is distributed under the terms of the GNU general Public
 * License (GPL), see the file LICENSE
 *
 * Author: Erik Nordström, <erikn@it.uu.se>
 */
#ifndef _MAINT_BUF_H
#define _MAINT_BUF_H

#ifndef NO_DECLS

int maint_buf_init(void);
void maint_buf_cleanup(void);

void maint_buf_set_max_len(unsigned int max_len);
int maint_buf_add(struct dsr_pkt *dp);
int maint_buf_del_all(struct in_addr nxt_hop);
int maint_buf_del_all_id(struct in_addr nxt_hop, unsigned short id);
int maint_buf_del_addr(struct in_addr nxt_hop);
void maint_buf_set_timeout(void);
void maint_buf_timeout(unsigned long data);
int maint_buf_salvage(struct dsr_pkt *dp);

#endif              /* NO_DECLS */

#endif              /* _MAINT_BUF_H */
