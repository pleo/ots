/* GLIB - Library of useful routines for C programming
 * Copyright (C) 1995-1997, 2002  Peter Mattis, Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __G_PRINTF_H__
#define __G_PRINTF_H__

#include "glib.h"
#include <stdio.h>
#include <stdarg.h>

G_BEGIN_DECLS

gint                  g_printf    (gchar const *format,
                                   ...) G_GNUC_PRINTF (1, 2);
gint                  g_fprintf   (FILE        *file,
				   gchar const *format,
				   ...) G_GNUC_PRINTF (2, 3);
gint                  g_sprintf   (gchar       *string,
				   gchar const *format,
				   ...) G_GNUC_PRINTF (2, 3);

gint                  g_vprintf   (gchar const *format,
                                   va_list      args);
gint                  g_vfprintf  (FILE        *file,
				   gchar const *format,
				   va_list      args);
gint                  g_vsprintf  (gchar       *string,
				   gchar const *format,
				   va_list      args);
gint                  g_vasprintf (gchar      **string,
				   gchar const *format,
				   va_list      args);

G_END_DECLS

#endif /* __G_PRINTF_H__ */
