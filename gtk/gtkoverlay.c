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

typedef struct _GtkOverlayChild GtkOverlayChild;

struct _GtkOverlayChild
{
  GtkWidget *widget;
  GdkWindow *window;
  gint x_offset;
  gint y_offset;
};

enum
{
  PROP_0,
  PROP_RELATIVE_WIDGET
};

enum
{
  CHILD_PROP_0,
  CHILD_PROP_X_OFFSET,
  CHILD_PROP_Y_OFFSET
};

G_DEFINE_TYPE (GtkOverlay, gtk_overlay, GTK_TYPE_CONTAINER)

/* the reason for this is that the main widget doesn't need to set an offset
   and it doesn't need an extra window */
static void
add_child (GtkOverlay *overlay,
           GtkWidget  *widget)
{
  GtkOverlayPrivate *priv = overlay->priv;
  GtkOverlayChild *child;

  child = g_slice_new0 (GtkOverlayChild);
  child->widget = widget;

  gtk_widget_set_parent (widget, GTK_WIDGET (overlay));

  priv->children = g_slist_append (priv->children, child);
}

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

static GdkWindow *
gtk_overlay_create_child_window (GtkOverlay *overlay,
                                 GtkWidget  *child)
{
  GtkWidget *widget = GTK_WIDGET (overlay);
  GtkAllocation allocation;
  GdkWindow *window;
  GdkWindowAttr attributes;
  gint attributes_mask;

  gtk_widget_get_allocation (child, &allocation);

  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.width = allocation.width;
  attributes.height = allocation.height;
  attributes.x = allocation.x;
  attributes.y = allocation.y;
  attributes_mask = GDK_WA_X | GDK_WA_Y;
  attributes.event_mask = gtk_widget_get_events (widget) | GDK_EXPOSURE_MASK;

  window = gdk_window_new (gtk_widget_get_window (widget),
                           &attributes, attributes_mask);
  gdk_window_set_user_data (window, overlay);
  gtk_style_context_set_background (gtk_widget_get_style_context (widget), window);

  gtk_widget_set_parent_window (child, window);

  return window;
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
    case PROP_RELATIVE_WIDGET:
      {
        GtkWidget *relative_widget;

        relative_widget = g_value_get_object (value);

        if (priv->main_widget == NULL ||
            (relative_widget != NULL &&
             !gtk_widget_is_ancestor (priv->main_widget, relative_widget)))
          {
            g_warning ("relative_widget must be a child of the main widget");
            break;
          }
        priv->relative_widget = relative_widget;
        gtk_widget_queue_resize_no_redraw (GTK_WIDGET (overlay));
      }
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
gtk_overlay_child_allocate (GtkWidget           *child,
                            GdkWindow           *child_window, /* can be NULL */
                            const GtkAllocation *window_allocation,
                            GtkAllocation       *child_allocation)
{
  if (child_window)
    gdk_window_move_resize (child_window,
                            window_allocation->x, window_allocation->y,
                            window_allocation->width, window_allocation->height);

  gtk_widget_size_allocate (child, child_allocation);
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
      GtkAllocation alloc, child_alloc;
      guint x_offset, y_offset;
      GtkAlign halign, valign;

      child = children->data;

      if (child->widget == priv->main_widget ||
          !gtk_widget_get_visible (child->widget))
        continue;

      gtk_widget_get_preferred_size (child->widget, NULL, &req);
      halign = gtk_widget_get_halign (child->widget);
      valign = gtk_widget_get_valign (child->widget);
      x_offset = child->x_offset;
      y_offset = child->y_offset;

      /* FIXME: Add all the positions here */
      switch (halign)
        {
        case GTK_ALIGN_END:
          switch (valign)
            {
            case GTK_ALIGN_START:
              alloc.x = MAX (main_alloc.x, main_alloc.width - req.width + x_offset);
              alloc.y = y_offset;
              break;

            case GTK_ALIGN_END:
              alloc.x = MAX (main_alloc.x, main_alloc.width - req.width + x_offset);
              alloc.y = MAX (main_alloc.y, main_alloc.height - req.height + y_offset);
              break;

            default:
              break;
            }
            break;

        case GTK_ALIGN_START:
          switch (valign)
            {
            case GTK_ALIGN_START:
              alloc.x = x_offset;
              alloc.y = y_offset;
              break;

            case GTK_ALIGN_END:
              alloc.x = x_offset;
              alloc.y = MAX (main_alloc.y, main_alloc.height - req.height + y_offset);
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

      child_alloc.x = child_alloc.y = 0;
      child_alloc.width = alloc.width;
      child_alloc.height = alloc.height;

      gtk_overlay_child_allocate (child->widget, child->window, &alloc, &child_alloc);
    }
}

static void
gtk_overlay_realize (GtkWidget *widget)
{
  GtkOverlay *overlay = GTK_OVERLAY (widget);
  GtkOverlayPrivate *priv = overlay->priv;
  GtkOverlayChild *child;
  GSList *children;

  GTK_WIDGET_CLASS (gtk_overlay_parent_class)->realize (widget);

  for (children = priv->children; children; children = g_slist_next (children))
    {
      child = children->data;

      if (child->widget == priv->main_widget)
        {
          child->window = gtk_widget_get_window (priv->main_widget);
        }
      else if (child->window == NULL)
        {
          child->window = gtk_overlay_create_child_window (overlay, child->widget);
        }
    }
}

static void
gtk_overlay_unrealize (GtkWidget *widget)
{
  GtkOverlay *overlay = GTK_OVERLAY (widget);
  GtkOverlayPrivate *priv = overlay->priv;
  GtkOverlayChild *child;
  GSList *children;

  for (children = priv->children; children; children = g_slist_next (children))
    {
      child = children->data;

      if (child->widget == priv->main_widget)
        child->window = NULL;
      else
        {
          gtk_widget_set_parent_window (child->widget, NULL);
          gdk_window_set_user_data (child->window, NULL);
          gdk_window_destroy (child->window);
          child->window = NULL;
        }
    }

  GTK_WIDGET_CLASS (gtk_overlay_parent_class)->unrealize (widget);
}

static void
gtk_overlay_map (GtkWidget *widget)
{
  GtkOverlay *overlay = GTK_OVERLAY (widget);
  GtkOverlayPrivate *priv = overlay->priv;
  GtkOverlayChild *child;
  GSList *children;

  GTK_WIDGET_CLASS (gtk_overlay_parent_class)->map (widget);

  for (children = priv->children; children; children = g_slist_next (children))
    {
      child = children->data;

      if (child->window != NULL && gtk_widget_get_visible (child->widget) && gtk_widget_get_child_visible (child->widget))
        gdk_window_show (child->window);
    }
}

static void
gtk_overlay_unmap (GtkWidget *widget)
{
  GtkOverlay *overlay = GTK_OVERLAY (widget);
  GtkOverlayPrivate *priv = overlay->priv;
  GtkOverlayChild *child;
  GSList *children;

  for (children = priv->children; children; children = g_slist_next (children))
    {
      child = children->data;

      if (child->window != NULL && gdk_window_is_visible (child->window))
        gdk_window_hide (child->window);
    }

  GTK_WIDGET_CLASS (gtk_overlay_parent_class)->unmap (widget);
}

static gboolean
gtk_overlay_draw (GtkWidget *widget,
                  cairo_t   *cr)
{
  GtkOverlay *overlay = GTK_OVERLAY (widget);
  GtkOverlayPrivate *priv = overlay->priv;
  GtkOverlayChild *child;
  GSList *children;

  for (children = priv->children; children; children = g_slist_next (children))
    {
      child = children->data;

      if (child->widget == priv->main_widget)
        continue;

      if (gtk_cairo_should_draw_window (cr, child->window))
        {
          cairo_save (cr);
          gtk_cairo_transform_to_window (cr, widget, child->window);
          gtk_render_background (gtk_widget_get_style_context (widget),
                                 cr,
                                 0, 0,
                                 gdk_window_get_width (child->window),
                                 gdk_window_get_height (child->window));
          cairo_restore (cr);
        }
    }

  /* Chain up to draw children */
  GTK_WIDGET_CLASS (gtk_overlay_parent_class)->draw (widget, cr);
  
  return FALSE;
}

static void
overlay_add (GtkContainer *container,
             GtkWidget    *widget)
{
  GtkOverlay *overlay = GTK_OVERLAY (container);
  GtkOverlayPrivate *priv = overlay->priv;

  if (priv->main_widget != NULL)
    {
      g_warning ("Attempting to add a widget with type %s to a %s, "
                 "but as a GtkOverlay subclass a %s can only contain one main widget at a time; "
                 "it already contains a widget of type %s",
                 g_type_name (G_OBJECT_TYPE (widget)),
                 g_type_name (G_OBJECT_TYPE (container)),
                 g_type_name (G_OBJECT_TYPE (container)),
                 g_type_name (G_OBJECT_TYPE (priv->main_widget)));
      return;
    }

  priv->main_widget = widget;
  add_child (overlay, widget);
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
          if (child->window != NULL && child->widget != priv->main_widget)
            {
              gdk_window_set_user_data (child->window, NULL);
              gdk_window_destroy (child->window);
            }

          if (widget == priv->main_widget)
            priv->main_widget = NULL;

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
                                 gint             x_offset,
                                 gint             y_offset)
{
  g_return_if_fail (gtk_widget_get_parent (child->widget) == GTK_WIDGET (overlay));

  gtk_widget_freeze_child_notify (child->widget);

  if (child->x_offset != x_offset)
    {
      child->x_offset = x_offset;
      gtk_widget_child_notify (child->widget, "x-offset");
    }

  if (child->y_offset != y_offset)
    {
      child->y_offset = y_offset;
      gtk_widget_child_notify (child->widget, "y-offset");
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
    case CHILD_PROP_X_OFFSET:
      gtk_overlay_set_offset_internal (overlay,
                                       overlay_child,
                                       g_value_get_int (value),
                                       overlay_child->y_offset);
      break;
    case CHILD_PROP_Y_OFFSET:
      gtk_overlay_set_offset_internal (overlay,
                                       overlay_child,
                                       overlay_child->x_offset,
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
    case CHILD_PROP_X_OFFSET:
      g_value_set_int (value, overlay_child->x_offset);
      break;
    case CHILD_PROP_Y_OFFSET:
      g_value_set_int (value, overlay_child->y_offset);
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
  widget_class->realize = gtk_overlay_realize;
  widget_class->unrealize = gtk_overlay_unrealize;
  widget_class->map = gtk_overlay_map;
  widget_class->unmap = gtk_overlay_unmap;
  widget_class->draw = gtk_overlay_draw;

  container_class->add = overlay_add;
  container_class->remove = gtk_overlay_remove;
  container_class->forall = gtk_overlay_forall;
  container_class->child_type = gtk_overlay_child_type;
  container_class->set_child_property = gtk_overlay_set_child_property;
  container_class->get_child_property = gtk_overlay_get_child_property;

  g_object_class_install_property (object_class, PROP_RELATIVE_WIDGET,
                                   g_param_spec_object ("relative-widget",
                                                        "Relative Widget",
                                                        "Widget on which the floating widgets are placed",
                                                        GTK_TYPE_WIDGET,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_STATIC_STRINGS));

  gtk_container_class_install_child_property (container_class,
                                              CHILD_PROP_X_OFFSET,
                                              g_param_spec_int ("x-offset",
                                                                P_("X Offset"),
                                                                P_("The x offset of child widget"),
                                                                G_MININT, G_MAXINT, 0,
                                                                GTK_PARAM_READWRITE));

  gtk_container_class_install_child_property (container_class,
                                              CHILD_PROP_Y_OFFSET,
                                              g_param_spec_int ("y-offset",
                                                                P_("Y Offset"),
                                                                P_("The y offset of child widget"),
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
 *
 * Creates a new #GtkOverlay.
 *
 * Returns: a new #GtkOverlay object.
 */
GtkWidget *
gtk_overlay_new (void)
{
  return g_object_new (GTK_TYPE_OVERLAY, NULL);
}

/**
 * gtk_overlay_set_relative_widget:
 * @overlay: a #GtkOverlay
 * @relative_widget: (allow-none): a child of the main widget
 *
 * Sets the relative widget where static widgets will be placed. This
 * widget must be a child of the widget added by gtk_container_add()
 */
void
gtk_overlay_set_relative_widget (GtkOverlay *overlay,
                                 GtkWidget  *relative_widget)
{
  GtkOverlayPrivate *priv;

  g_return_if_fail (GTK_IS_OVERLAY (overlay));

  priv = overlay->priv;

  if (priv->relative_widget != relative_widget)
    {
      priv->relative_widget = relative_widget;

      g_object_notify (G_OBJECT (overlay), "relative-widget");
    }
}

/**
 * gtk_overlay_get_relative_widget:
 * @overlay: a #GtkOverlay
 *
 * Gets the relative widget to the main widget added by gtk_container_add()
 */
GtkWidget *
gtk_overlay_get_relative_widget (GtkOverlay *overlay)
{
  g_return_val_if_fail (GTK_IS_OVERLAY (overlay), NULL);

  return overlay->priv->relative_widget;
}

/**
 * gtk_overlay_add:
 * @overlay: a #GtkOverlay
 * @widget: a #GtkWidget to be added to the container
 *
 * Adds @widget to @overlay.
 */
void
gtk_overlay_add (GtkOverlay *overlay,
                 GtkWidget  *widget)
{
  GtkOverlayChild *child;

  g_return_if_fail (GTK_IS_OVERLAY (overlay));
  g_return_if_fail (GTK_IS_WIDGET (widget));

  add_child (overlay, widget);

  child = get_child (overlay, widget);

  if (gtk_widget_get_realized (GTK_WIDGET (overlay)))
    child->window = gtk_overlay_create_child_window (overlay, widget);
}

/**
 * gtk_overlay_set_offset:
 * @overlay: a #GtkOverlay
 * @widget: a child of @overlay
 * @x_offset: the new x offset for @widget
 * @y_offset: the new y offset for @widget
 *
 * Sets the offset for @widget
 */
void
gtk_overlay_set_offset (GtkOverlay *overlay,
                        GtkWidget  *widget,
                        gint        x_offset,
                        gint        y_offset)
{
  g_return_if_fail (GTK_IS_OVERLAY (overlay));

  gtk_overlay_set_offset_internal (overlay, get_child (overlay, widget), x_offset, y_offset);
}

/**
 * gtk_overlay_get_offset:
 * @overlay: a #GtkOverlay
 * @widget: a child of @overlay
 * @x_offset: (out) (allow-none): returns the x offset of @widget
 * @y_offset: (out) (allow-none): returns the y offset of @widget
 *
 * Gets the offset for @widget
 */
void
gtk_overlay_get_offset (GtkOverlay *overlay,
                        GtkWidget  *widget,
                        gint       *x_offset,
                        gint       *y_offset)
{
  GtkOverlayChild *child;

  g_return_if_fail (GTK_IS_OVERLAY (overlay));

  child = get_child (overlay, widget);

  if (x_offset != NULL)
    *x_offset = child->x_offset;

  if (y_offset != NULL)
    *y_offset = child->y_offset;
}
