/* gtkcellareaboxiter.h
 *
 * Copyright (C) 2010 Openismus GmbH
 *
 * Authors:
 *      Tristan Van Berkom <tristanvb@openismus.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#if !defined (__GTK_H_INSIDE__) && !defined (GTK_COMPILATION)
#error "Only <gtk/gtk.h> can be included directly."
#endif

#ifndef __GTK_CELL_AREA_BOX_ITER_H__
#define __GTK_CELL_AREA_BOX_ITER_H__

#include <gtk/gtkcellareaiter.h>
#include <gtk/gtkcellrenderer.h>

G_BEGIN_DECLS

#define GTK_TYPE_CELL_AREA_BOX_ITER		  (gtk_cell_area_box_iter_get_type ())
#define GTK_CELL_AREA_BOX_ITER(obj)		  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_CELL_AREA_BOX_ITER, GtkCellAreaBoxIter))
#define GTK_CELL_AREA_BOX_ITER_CLASS(klass)	  (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_CELL_AREA_BOX_ITER, GtkCellAreaBoxIterClass))
#define GTK_IS_CELL_AREA_BOX_ITER(obj)	  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_CELL_AREA_BOX_ITER))
#define GTK_IS_CELL_AREA_BOX_ITER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_CELL_AREA_BOX_ITER))
#define GTK_CELL_AREA_BOX_ITER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_CELL_AREA_BOX_ITER, GtkCellAreaBoxIterClass))

typedef struct _GtkCellAreaBoxIter              GtkCellAreaBoxIter;
typedef struct _GtkCellAreaBoxIterClass         GtkCellAreaBoxIterClass;
typedef struct _GtkCellAreaBoxIterPrivate       GtkCellAreaBoxIterPrivate;

struct _GtkCellAreaBoxIter
{
  GtkCellAreaIter parent_instance;

  GtkCellAreaBoxIterPrivate *priv;
};

struct _GtkCellAreaBoxIterClass
{
  GtkCellAreaIterClass parent_class;

};

GType   gtk_cell_area_box_iter_get_type               (void) G_GNUC_CONST;


/* Update cell alignments */
void    gtk_cell_area_box_iter_push_cell_width             (GtkCellAreaBoxIter *box_iter,
							    GtkCellRenderer    *renderer,
							    gint                minimum_width,
							    gint                natural_width);

void    gtk_cell_area_box_iter_push_cell_height_for_width  (GtkCellAreaBoxIter *box_iter,
							    GtkCellRenderer    *renderer,
							    gint                for_width,
							    gint                minimum_height,
							    gint                natural_height);

void    gtk_cell_area_box_iter_push_cell_height            (GtkCellAreaBoxIter *box_iter,
							    GtkCellRenderer    *renderer,
							    gint                minimum_height,
							    gint                natural_height);

void    gtk_cell_area_box_iter_push_cell_width_for_height  (GtkCellAreaBoxIter *box_iter,
							    GtkCellRenderer    *renderer,
							    gint                for_height,
							    gint                minimum_width,
							    gint                natural_width);

/* Fetch cell alignments */
void    gtk_cell_area_box_iter_get_cell_width              (GtkCellAreaBoxIter *box_iter,
							    GtkCellRenderer    *renderer,
							    gint               *minimum_width,
							    gint               *natural_width);

void    gtk_cell_area_box_iter_get_cell_height_for_width   (GtkCellAreaBoxIter *box_iter,
							    GtkCellRenderer    *renderer,
							    gint                for_width,
							    gint               *minimum_height,
							    gint               *natural_height);

void    gtk_cell_area_box_iter_get_cell_height             (GtkCellAreaBoxIter *box_iter,
							    GtkCellRenderer    *renderer,
							    gint               *minimum_height,
							    gint               *natural_height);

void    gtk_cell_area_box_iter_get_cell_width_for_height   (GtkCellAreaBoxIter *box_iter,
							    GtkCellRenderer    *renderer,
							    gint                for_height,
							    gint               *minimum_width,
							    gint               *natural_width);

G_END_DECLS

#endif /* __GTK_CELL_AREA_BOX_ITER_H__ */