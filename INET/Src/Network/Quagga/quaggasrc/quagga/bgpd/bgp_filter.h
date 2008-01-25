/* AS path filter list.
   Copyright (C) 1999 Kunihiro Ishiguro

This file is part of GNU Zebra.

GNU Zebra is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

GNU Zebra is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Zebra; see the file COPYING.  If not, write to the Free
Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA.  */

#ifndef __BGPFILTER_H__
#define __BGPFILTER_H__

enum as_filter_type
{
  AS_FILTER_DENY,
  AS_FILTER_PERMIT
};

enum as_filter_type as_list_apply (struct as_list *, void *);

struct as_list *as_list_lookup (const char *);
void as_list_add_hook (void (*func) ());
void as_list_delete_hook (void (*func) ());

/* Element of AS path filter. */
struct as_filter
{
  struct as_filter *next;
  struct as_filter *prev;

  enum as_filter_type type;

  regex_t *reg;
  char *reg_str;
};

enum as_list_type
{
  ACCESS_TYPE_BGPD_STRING,
  ACCESS_TYPE_BGPD_NUMBER
};

/* AS path filter list. */
struct as_list
{
  char *name;

  enum as_list_type type;

  struct as_list *next;
  struct as_list *prev;

  struct as_filter *head;
  struct as_filter *tail;
};

/* List of AS filter list. */
struct as_list_list
{
  struct as_list *head;
  struct as_list *tail;
};

/* AS path filter master. */
struct_as_list_master
{
  /* List of access_list which name is number. */
  struct as_list_list num;

  /* List of access_list which name is string. */
  struct as_list_list str;

  /* Hook function which is executed when new access_list is added. */
  void (*add_hook) ();

  /* Hook function which is executed when access_list is deleted. */
  void (*delete_hook) ();
};

#endif
