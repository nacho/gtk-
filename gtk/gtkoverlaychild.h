/*
 * gtk-overlay-child.h
 * This file is part of gtk
 *
 * Copyright (C) 2011 - Ignacio Casal Quinteiro
 *
 * gtk is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * gtk is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __GTK_OVERLAY_CHILD_H__
#define __GTK_OVERLAY_CHILD_H__

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GTK_TYPE_OVERLAY_CHILD		(gtk_overlay_child_get_type ())
#define GTK_OVERLAY_CHILD(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_OVERLAY_CHILD, GtkOverlayChild))
#define GTK_OVERLAY_CHILD_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_OVERLAY_CHILD, GtkOverlayChild const))
#define GTK_OVERLAY_CHILD_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_OVERLAY_CHILD, GtkOverlayChildClass))
#define GTK_IS_OVERLAY_CHILD(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_OVERLAY_CHILD))
#define GTK_IS_OVERLAY_CHILD_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_OVERLAY_CHILD))
#define GTK_OVERLAY_CHILD_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_OVERLAY_CHILD, GtkOverlayChildClass))

typedef struct _GtkOverlayChild		GtkOverlayChild;
typedef struct _GtkOverlayChildClass		GtkOverlayChildClass;
typedef struct _GtkOverlayChildPrivate	GtkOverlayChildPrivate;

struct _GtkOverlayChild
{
	GtkBin parent;

	GtkOverlayChildPrivate *priv;
};

struct _GtkOverlayChildClass
{
	GtkBinClass parent_class;
};

GType                     gtk_overlay_child_get_type     (void) G_GNUC_CONST;

GtkOverlayChild        *gtk_overlay_child_new          (GtkWidget *widget);

guint                     gtk_overlay_child_get_offset   (GtkOverlayChild *child);

void                      gtk_overlay_child_set_offset   (GtkOverlayChild *child,
                                                            guint              offset);

gboolean                  gtk_overlay_child_get_fixed    (GtkOverlayChild *child);

void                      gtk_overlay_child_set_fixed    (GtkOverlayChild *child,
                                                            gboolean           fixed);

G_END_DECLS

#endif /* __GTK_OVERLAY_CHILD_H__ */
