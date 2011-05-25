/*
 * gtkoverlay.c
 * This file is part of gtk
 *
 * Copyright (C) 2011 - Ignacio Casal Quinteiro, Mike Krüger
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

#include "config.h"

#include "gtkoverlay.h"

#include "gtkprivate.h"
#include "gtkintl.h"

struct _GtkOverlayPrivate
{
  GtkWidget *main_widget;
  GtkWidget *relative_widget;
  GSList    *children;
};

enum
{
  PROP_0,
  PROP_MAIN_WIDGET,
  PROP_RELATIVE_WIDGET
};

enum
{
  CHILD_PROP_0,
  CHILD_PROP_OFFSET
};

G_DEFINE_TYPE (GtkOverlay, gtk_overlay, GTK_TYPE_CONTAINER)

static GtkOverlayChild *
get_child (GtkOverlay *overlay,
           GtkWidget  *widget)
{
  GtkOverlayPrivate *priv = overlay->priv;
  GtkOverlayChild *child;
  GSList *children;

  for (children = priv->children; children; children = g_slist_next (children))
    {
      child = children->data;

      if (child->widget == widget)
        return child;
    }

  return NULL;
}

static void
gtk_overlay_dispose (GObject *object)
{
  G_OBJECT_CLASS (gtk_overlay_parent_class)->dispose (object);
}

static void
gtk_overlay_get_property (GObject    *object,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  GtkOverlay *overlay = GTK_OVERLAY (object);
  GtkOverlayPrivate *priv = overlay->priv;

  switch (prop_id)
    {
      case PROP_MAIN_WIDGET:
        g_value_set_object (value, priv->main_widget);
        break;
      case PROP_RELATIVE_WIDGET:
        g_value_set_object (value, priv->relative_widget);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
gtk_overlay_set_property (GObject      *object,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  GtkOverlay *overlay = GTK_OVERLAY (object);
  GtkOverlayPrivate *priv = overlay->priv;

  switch (prop_id)
    {
      case PROP_MAIN_WIDGET:
        priv->main_widget = g_value_get_object (value);
        gtk_overlay_add (overlay, priv->main_widget, 0);
        break;
      case PROP_RELATIVE_WIDGET:
        priv->relative_widget = g_value_get_object (value);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
gtk_overlay_get_preferred_width (GtkWidget *widget,
                                 gint      *minimum,
                                 gint      *natural)
{
  GtkOverlayPrivate *priv = GTK_OVERLAY (widget)->priv;

  *minimum = 0;
  *natural = 0;

  if (priv->main_widget)
    gtk_widget_get_preferred_width (priv->main_widget, minimum, natural);
}

static void
gtk_overlay_get_preferred_height (GtkWidget *widget,
                                  gint      *minimum,
                                  gint      *natural)
{
  GtkOverlayPrivate *priv = GTK_OVERLAY (widget)->priv;

  *minimum = 0;
  *natural = 0;

  if (priv->main_widget)
    gtk_widget_get_preferred_height (priv->main_widget, minimum, natural);
}

static void
gtk_overlay_size_allocate (GtkWidget     *widget,
                             GtkAllocation *allocation)
{
  GtkOverlay *overlay = GTK_OVERLAY (widget);
  GtkOverlayPrivate *priv = overlay->priv;
  GtkOverlayChild *child;
  GtkAllocation main_alloc;
  GSList *children;

  GTK_WIDGET_CLASS (gtk_overlay_parent_class)->size_allocate (widget, allocation);

  /* main widget allocation */
  main_alloc.x = 0;
  main_alloc.y = 0;
  main_alloc.width = allocation->width;
  main_alloc.height = allocation->height;

  gtk_widget_size_allocate (overlay->priv->main_widget, &main_alloc);

  /* if a relative widget exists place the floating widgets in relation to it */
  if (priv->relative_widget)
    gtk_widget_get_allocation (priv->relative_widget, &main_alloc);

  for (children = priv->children; children; children = g_slist_next (children))
    {
      GtkRequisition req;
      GtkAllocation alloc;
      guint offset;
      GtkAlign halign, valign;

      child = children->data;

      if (child->widget == priv->main_widget)
        continue;

      gtk_widget_get_preferred_size (child->widget, NULL, &req);
      halign = gtk_widget_get_halign (child->widget);
      valign = gtk_widget_get_valign (child->widget);
      offset = child->offset;

      /* FIXME: Add all the positions here */
      switch (halign)
        {
          case GTK_ALIGN_END:
            switch (valign)
              {
                case GTK_ALIGN_START:
                  alloc.x = MAX (main_alloc.x, main_alloc.width - req.width - (gint) offset);
                  alloc.y = 0;
                  break;

                case GTK_ALIGN_END:
                  alloc.x = MAX (main_alloc.x, main_alloc.width - req.width - (gint) offset);
                  alloc.y = MAX (main_alloc.y, main_alloc.height - req.height);
                  break;

                default:
                  break;
              }
              break;

          case GTK_ALIGN_START:
            switch (valign)
              {
                case GTK_ALIGN_START:
                  alloc.x = offset;
                  alloc.y = 0;
                  break;

                case GTK_ALIGN_END:
                  alloc.x = offset;
                  alloc.y = MAX (main_alloc.y, main_alloc.height - req.height);
                  break;

                default:
                  break;
              }
              break;

          default:
            alloc.x = 0;
            alloc.y = 0;
        }

      alloc.width = MIN (main_alloc.width, req.width);
      alloc.height = MIN (main_alloc.height, req.height);

      gtk_widget_size_allocate (child->widget, &alloc);
    }
}

static void
overlay_add (GtkContainer *overlay,
             GtkWidget    *widget)
{
  gtk_overlay_add (GTK_OVERLAY (overlay), widget, 0);
}

static void
gtk_overlay_remove (GtkContainer *overlay,
                    GtkWidget    *widget)
{
  GtkOverlayPrivate *priv = GTK_OVERLAY (overlay)->priv;
  GtkOverlayChild *child;
  GSList *children;

  for (children = priv->children; children; children = g_slist_next (children))
    {
      child = children->data;

      if (child->widget == widget)
        {
          gtk_widget_unparent (widget);

          priv->children = g_slist_delete_link (priv->children,
                                                children);
          g_slice_free (GtkOverlayChild, child);

          break;
        }
    }
}

static void
gtk_overlay_forall (GtkContainer *overlay,
                    gboolean      include_internals,
                    GtkCallback   callback,
                    gpointer      callback_data)
{
  GtkOverlayPrivate *priv = GTK_OVERLAY (overlay)->priv;
  GtkOverlayChild *child;
  GSList *children;

  children = priv->children;

  while (children)
    {
      child = children->data;
      children = children->next;

      (* callback) (child->widget, callback_data);
    }
}

static GType
gtk_overlay_child_type (GtkContainer *overlay)
{
  return GTK_TYPE_WIDGET;
}

static void
gtk_overlay_set_offset_internal (GtkOverlay      *overlay,
                                 GtkOverlayChild *child,
                                 gint             offset)
{
  g_return_if_fail (gtk_widget_get_parent (child->widget) == GTK_WIDGET (overlay));

  gtk_widget_freeze_child_notify (child->widget);

  if (child->offset != offset)
    {
      child->offset = offset;
      gtk_widget_child_notify (child->widget, "offset");
    }

  gtk_widget_thaw_child_notify (child->widget);

  if (gtk_widget_get_visible (child->widget) &&
      gtk_widget_get_visible (GTK_WIDGET (overlay)))
    gtk_widget_queue_resize (GTK_WIDGET (overlay));
}

static void
gtk_overlay_set_child_property (GtkContainer *container,
                                GtkWidget    *child,
                                guint         property_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  GtkOverlay *overlay = GTK_OVERLAY (container);
  GtkOverlayChild *overlay_child;

  overlay_child = get_child (overlay, child);

  switch (property_id)
    {
    case CHILD_PROP_OFFSET:
      gtk_overlay_set_offset_internal (overlay,
                                       overlay_child,
                                       g_value_get_int (value));
      break;
    default:
      GTK_CONTAINER_WARN_INVALID_CHILD_PROPERTY_ID (container, property_id, pspec);
      break;
    }
}

static void
gtk_overlay_get_child_property (GtkContainer *container,
                                GtkWidget    *child,
                                guint         property_id,
                                GValue       *value,
                                GParamSpec   *pspec)
{
  GtkOverlayChild *overlay_child;

  overlay_child = get_child (GTK_OVERLAY (container), child);

  switch (property_id)
    {
    case CHILD_PROP_OFFSET:
      g_value_set_int (value, overlay_child->offset);
      break;
    default:
      GTK_CONTAINER_WARN_INVALID_CHILD_PROPERTY_ID (container, property_id, pspec);
      break;
    }
}

static void
gtk_overlay_class_init (GtkOverlayClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);

  object_class->dispose = gtk_overlay_dispose;
  object_class->get_property = gtk_overlay_get_property;
  object_class->set_property = gtk_overlay_set_property;

  widget_class->get_preferred_width = gtk_overlay_get_preferred_width;
  widget_class->get_preferred_height = gtk_overlay_get_preferred_height;
  widget_class->size_allocate = gtk_overlay_size_allocate;

  container_class->add = overlay_add;
  container_class->remove = gtk_overlay_remove;
  container_class->forall = gtk_overlay_forall;
  container_class->child_type = gtk_overlay_child_type;
  container_class->set_child_property = gtk_overlay_set_child_property;
  container_class->get_child_property = gtk_overlay_get_child_property;

  g_object_class_install_property (object_class, PROP_MAIN_WIDGET,
                                   g_param_spec_object ("main-widget",
                                                        "Main Widget",
                                                        "The Main Widget",
                                                        GTK_TYPE_WIDGET,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY |
                                                        G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_RELATIVE_WIDGET,
                                   g_param_spec_object ("relative-widget",
                                                        "Relative Widget",
                                                        "Widget on which the floating widgets are placed",
                                                        GTK_TYPE_WIDGET,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_STATIC_STRINGS));

  gtk_container_class_install_child_property (container_class,
                                              CHILD_PROP_OFFSET,
                                              g_param_spec_int ("offset",
                                                                P_("Offset"),
                                                                P_("The offset of child widget"),
                                                                G_MININT, G_MAXINT, 0,
                                                                GTK_PARAM_READWRITE));

  g_type_class_add_private (object_class, sizeof (GtkOverlayPrivate));
}

static void
gtk_overlay_init (GtkOverlay *overlay)
{
  overlay->priv = G_TYPE_INSTANCE_GET_PRIVATE (overlay, GTK_TYPE_OVERLAY, GtkOverlayPrivate);

  gtk_widget_set_has_window (GTK_WIDGET (overlay), FALSE);
}

/**
 * gtk_overlay_new:
 * @main_widget: a #GtkWidget
 * @relative_widget: (allow-none): a #Gtkwidget
 *
 * Creates a new #GtkOverlay. If @relative_widget is not %NULL the floating
 * widgets will be placed in relation to it, if not @main_widget will be use
 * for this purpose.
 *
 * Returns: a new #GtkOverlay object.
 */
GtkWidget *
gtk_overlay_new (GtkWidget *main_widget,
                 GtkWidget *relative_widget)
{
  g_return_val_if_fail (GTK_IS_WIDGET (main_widget), NULL);

  return GTK_WIDGET (g_object_new (GTK_TYPE_OVERLAY,
                                   "main-widget", main_widget,
                                   "relative-widget", relative_widget,
                                   NULL));
}

/**
 * gtk_overlay_add:
 * @overlay: a #GtkOverlay
 * @widget: a #GtkWidget to be added to the container
 * @offset: offset for @widget
 *
 * Adds @widget to @overlay in a specific position.
 */
void
gtk_overlay_add (GtkOverlay *overlay,
                 GtkWidget  *widget,
                 guint       offset)
{
  GtkOverlayPrivate *priv = overlay->priv;
  GtkOverlayChild *child;

  g_return_if_fail (GTK_IS_OVERLAY (overlay));
  g_return_if_fail (GTK_IS_WIDGET (widget));

  child = g_slice_new (GtkOverlayChild);
  child->widget = widget;
  child->offset = offset;

  gtk_widget_set_parent (widget, GTK_WIDGET (overlay));

  priv->children = g_slist_append (priv->children, child);
}

void
gtk_overlay_set_offset (GtkOverlay *overlay,
                        GtkWidget  *widget,
                        gint        offset)
{
  g_return_if_fail (GTK_IS_OVERLAY (overlay));

  gtk_overlay_set_offset_internal (overlay, get_child (overlay, widget), offset);
}
