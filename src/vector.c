/* $Id: vector.c,v 1.2 2002-04-09 16:28:13 rjkaes Exp $
 *
 * A vector implementation.  The vector can be of an arbritrary length, and
 * the data for each entry is an lump of data (the size is stored in the
 * vector.)
 *
 * Copyright (C) 2002  Robert James Kaes (rjkaes@flarenet.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#if defined(HAVE_CONFIG_H)
#  include <config.h>
#endif

#include <sys/types.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "vector.h"

/*
 * These structures are the storage for the "vector".  Entries are
 * stored in struct vectorentry_s (the data and the length), and the
 * "vector" structure is implemented as a linked-list.  The struct
 * vector_s stores a pointer to the first vector (vector[0]) and a
 * count of the number of enteries (or how long the vector is.)
 */
struct vectorentry_s {
	void *data;
	size_t len;

	struct vectorentry_s *next;
};

struct vector_s {
	size_t num_entries;
	struct vectorentry_s *vector;
};

/*
 * Create an vector.  The vector initially has no elements and no
 * storage has been allocated for the entries.
 *
 * A NULL is returned if memory could not be allocated for the
 * vector.
 */
vector_t
vector_create(void)
{
	vector_t vector;

	vector = malloc(sizeof(struct vector_s));
	if (!vector)
		return NULL;

	vector->num_entries = 0;
	vector->vector = NULL;
	
	return vector;
}

/*
 * Deletes an vector.  All the enteries when this function is run.
 *
 * Returns: 0 on success
 *          negative if a NULL vector is supplied
 */
int
vector_delete(vector_t vector)
{
	struct vectorentry_s *ptr, *next;

	if (!vector)
		return -EINVAL;
       
	ptr = vector->vector;
	while (ptr) {
		next = ptr->next;
		free(ptr->data);
		free(ptr);

		ptr = next;	
	}

	free(vector);

	return 0;
}

/*
 * Inserts an entry into the vector.  The entry is an arbitrary
 * collection of bytes of _len_ octets.  The data is copied into the
 * vector, so the original data must be freed to avoid a memory leak.
 * The "data" must be non-NULL and the "len" must be greater than zero.
 *
 * Returns: 0 on success
 *          negative number if there are errors
 */
int
vector_insert(vector_t vector, void *data, ssize_t len)
{
	struct vectorentry_s *entry, **ptr;

	if (!vector || !data || len <= 0)
		return -EINVAL;

	entry = malloc(sizeof(struct vectorentry_s));
	if (!entry)
		return -ENOMEM;

	entry->data = malloc(len);
	if (!entry->data) {
		free(entry);
		return -ENOMEM;
	}

	memcpy(entry->data, data, len);
	entry->len = len;
	entry->next = NULL;

	ptr = &vector->vector;
	while (*ptr)
		ptr = &((*ptr)->next);

	*ptr = entry;
	vector->num_entries++;

	return 0;
}

/*
 * A pointer to the data at position "pos" (zero based) is returned in the
 * "data" pointer.  If the vector is out of bound, data is set to NULL.
 *
 * Returns: negative upon an error
 *          length of data if position is valid
 */
ssize_t
vector_getentry(vector_t vector, size_t pos, void **data)
{
	struct vectorentry_s *ptr;
	size_t loc;

	if (!vector || !data)
		return -EINVAL;

	if (pos < 0 || pos >= vector->num_entries)
		return -ERANGE;

	loc = 0;
	ptr = vector->vector;

	while (loc != pos) {
		ptr = ptr->next;
		loc++;
	}

	*data = ptr->data;
	return ptr->len;
}

/*
 * Returns the number of enteries (or the length) of the vector.
 *
 * Returns: negative if vector is not valid
 *          positive length of vector otherwise
 */
ssize_t
vector_length(vector_t vector)
{
	if (!vector)
		return -EINVAL;

	return vector->num_entries;
}