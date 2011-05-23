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

#include "gtkoverlay.h"

typedef struct
{
  GtkWidget *child;
  GtkWidget *original;
} ChildContainer;

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

G_DEFINE_TYPE (GtkOverlay, gtk_overlay, GTK_TYPE_CONTAINER)

static ChildContainer *
child_container_new (GtkWidget *child,
                     GtkWidget *original)
{
  ChildContainer *ret;

  ret = g_slice_new (ChildContainer);
  ret->child = child;
  ret->original = original;

  return ret;
}

static void
child_container_free (ChildContainer *container)
{
  g_slice_free (ChildContainer, container);
}

static GtkWidget *
child_container_get_child (ChildContainer *container)
{
  GtkWidget *child;

  if (container->child != NULL)
    child = container->child;
  else
    child = container->original;

  return child;
}

static void
add_toplevel_widget (GtkOverlay *overlay,
                     GtkWidget  *child,
                     GtkWidget  *original)
{
  ChildContainer *container;

  if (child != NULL)
    gtk_widget_set_parent (child, GTK_WIDGET (overlay));
  else
    gtk_widget_set_parent (original, GTK_WIDGET (overlay));

  container = child_container_new (child, original);

  overlay->priv->children = g_slist_append (overlay->priv->children,
                                            container);
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

static GtkWidget *
wrap_child_if_needed (GtkWidget *widget)
{
  GtkWidget *child;

  if (GTK_IS_OVERLAY_CHILD (widget))
    return widget;

  child = GTK_WIDGET (gtk_overlay_child_new (widget));
  gtk_widget_show (child);

  g_signal_connect_swapped (widget,
                            "destroy",
                            G_CALLBACK (gtk_widget_destroy),
                            child);

  return child;
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

        add_toplevel_widget (overlay,
                             NULL,
                             priv->main_widget);
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
gtk_overlay_realize (GtkWidget *widget)
{
  GtkAllocation allocation;
  GdkWindow *window;
  GdkWindowAttr attributes;
  gint attributes_mask;
  GtkStyleContext *context;

  gtk_widget_set_realized (widget, TRUE);

  gtk_widget_get_allocation (widget, &allocation);

  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.x = allocation.x;
  attributes.y = allocation.y;
  attributes.width = allocation.width;
  attributes.height = allocation.height;
  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.visual = gtk_widget_get_visual (widget);
  attributes.event_mask = gtk_widget_get_events (widget);
  attributes.event_mask |= GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK;

  attributes_mask = GDK_WA_X | GDK_WA_Y;

  window = gdk_window_new (gtk_widget_get_parent_window (widget),
                           &attributes, attributes_mask);
  gtk_widget_set_window (widget, window);
  gdk_window_set_user_data (window, widget);

  context = gtk_widget_get_style_context (widget);
  gtk_style_context_set_state (context, GTK_STATE_FLAG_NORMAL);
  gtk_style_context_set_background (context, window);
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
  GtkAllocation main_alloc;
  GSList *l;

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

  for (l = priv->children; l != NULL; l = g_slist_next (l))
    {
      ChildContainer *container = l->data;
      GtkWidget *child;
      GtkRequisition req;
      GtkAllocation alloc;
      guint offset;
      GtkAlign halign, valign;

      child = child_container_get_child (container);

      if (child == priv->main_widget)
        continue;

      gtk_widget_get_preferred_size (child, NULL, &req);
      offset = gtk_overlay_child_get_offset (GTK_OVERLAY_CHILD (child));
      halign = gtk_widget_get_halign (container->original);
      valign = gtk_widget_get_valign (container->original);

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

      gtk_widget_size_allocate (child, &alloc);
    }
}

static GtkOverlayChild *
get_overlay_child (GtkOverlay *overlay,
                   GtkWidget  *widget)
{
  GSList *l;

  for (l = overlay->priv->children; l != NULL; l = g_slist_next (l))
    {
      ChildContainer *container = l->data;

      if (container->original == widget &&
          GTK_IS_OVERLAY_CHILD (container->child))
        {
          return GTK_OVERLAY_CHILD (container->child);
        }
    }

  return NULL;
}

static void
overlay_add (GtkContainer *overlay,
             GtkWidget    *widget)
{
  GtkOverlayChild *child;

  /* check that the widget is not added yet */
  child = get_overlay_child (GTK_OVERLAY (overlay), widget);

  if (child == NULL)
    add_toplevel_widget (GTK_OVERLAY (overlay),
                         wrap_child_if_needed (widget),
                         widget);
}

static void
gtk_overlay_remove (GtkContainer *overlay,
                    GtkWidget    *widget)
{
  GtkOverlayPrivate *priv = GTK_OVERLAY (overlay)->priv;
  GSList *l;

  for (l = priv->children; l != NULL; l = g_slist_next (l))
    {
      ChildContainer *container = l->data;
      GtkWidget *original = container->original;

      if (original == widget)
        {
          gtk_widget_unparent (widget);

          if (container->child != NULL &&
              original != container->child)
            {
              g_signal_handlers_disconnect_by_func (original,
                                                    gtk_widget_destroy,
                                                    container->child);

              gtk_widget_destroy (container->child);
            }

          child_container_free (container);
          priv->children = g_slist_delete_link (priv->children,
                                                l);

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
  GSList *children;

  children = priv->children;

  while (children)
    {
      ChildContainer *container = children->data;
      children = children->next;
      GtkWidget *child;

      child = child_container_get_child (container);

      (* callback) (child, callback_data);
    }
}

static GType
gtk_overlay_child_type (GtkContainer *overlay)
{
  return GTK_TYPE_WIDGET;
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

  widget_class->realize = gtk_overlay_realize;
  widget_class->get_preferred_width = gtk_overlay_get_preferred_width;
  widget_class->get_preferred_height = gtk_overlay_get_preferred_height;
  widget_class->size_allocate = gtk_overlay_size_allocate;

  container_class->add = overlay_add;
  container_class->remove = gtk_overlay_remove;
  container_class->forall = gtk_overlay_forall;
  container_class->child_type = gtk_overlay_child_type;

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

  g_type_class_add_private (object_class, sizeof (GtkOverlayPrivate));
}

static void
gtk_overlay_init (GtkOverlay *overlay)
{
  overlay->priv = G_TYPE_INSTANCE_GET_PRIVATE (overlay, GTK_TYPE_OVERLAY, GtkOverlayPrivate);

  gtk_widget_set_app_paintable (GTK_WIDGET (overlay), TRUE);
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
gtk_overlay_add (GtkOverlay             *overlay,
                 GtkWidget              *widget,
                 guint                   offset)
{
  GtkOverlayChild *child;

  g_return_if_fail (GTK_IS_OVERLAY (overlay));
  g_return_if_fail (GTK_IS_WIDGET (widget));

  gtk_container_add (GTK_CONTAINER (overlay), widget);

  /* NOTE: can we improve this without exposing overlay child? */
  child = get_overlay_child (overlay, widget);
  g_assert (child != NULL);

  gtk_overlay_child_set_offset (child, offset);
}
