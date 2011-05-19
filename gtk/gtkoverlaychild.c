/*
 * gtk-overlay-child.c
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

#include "gtkoverlaychild.h"

struct _GtkOverlayChildPrivate
{
	GBinding                 *binding;
	guint                     offset;
};

enum
{
	PROP_0,
	PROP_WIDGET,
	PROP_OFFSET
};

G_DEFINE_TYPE (GtkOverlayChild, gtk_overlay_child, GTK_TYPE_BIN)

static void
gtk_overlay_child_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
	GtkOverlayChild *child = GTK_OVERLAY_CHILD (object);

	switch (prop_id)
	{
		case PROP_WIDGET:
			g_value_set_object (value, gtk_bin_get_child (GTK_BIN (child)));
			break;
		case PROP_OFFSET:
			g_value_set_uint (value, child->priv->offset);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gtk_overlay_child_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
	GtkOverlayChild *child = GTK_OVERLAY_CHILD (object);

	switch (prop_id)
	{
		case PROP_WIDGET:
			gtk_container_add (GTK_CONTAINER (child),
			                   g_value_get_object (value));
			break;
		case PROP_OFFSET:
			child->priv->offset = g_value_get_uint (value);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gtk_overlay_child_realize (GtkWidget *widget)
{
	GdkWindowAttr attributes;
	GdkWindow *parent_window;
	GdkWindow *window;
	GtkStyleContext *context;

	gtk_widget_set_realized (widget, TRUE);

	parent_window = gtk_widget_get_parent_window (widget);
	context = gtk_widget_get_style_context (widget);

	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.event_mask = GDK_EXPOSURE_MASK;
	attributes.width = 0;
	attributes.height = 0;

	window = gdk_window_new (parent_window, &attributes, 0);
	gdk_window_set_user_data (window, widget);
	gtk_widget_set_window (widget, window);
	gtk_style_context_set_state (context, GTK_STATE_FLAG_NORMAL);
	gtk_style_context_set_background (context, window);
}

static void
gtk_overlay_child_get_size (GtkWidget     *widget,
                              GtkOrientation orientation,
                              gint          *minimum,
                              gint          *natural)
{
	GtkOverlayChild *overlay_child = GTK_OVERLAY_CHILD (widget);
	GtkWidget *child;
	gint child_min = 0, child_nat = 0;

	child = gtk_bin_get_child (GTK_BIN (overlay_child));

	if (child != NULL)
	{
		if (orientation == GTK_ORIENTATION_HORIZONTAL)
		{
			gtk_widget_get_preferred_width (child,
			                                &child_min, &child_nat);
		}
		else
		{
			gtk_widget_get_preferred_height (child,
			                                 &child_min, &child_nat);
		}
	}

	*minimum = child_min;
	*natural = child_nat;
}

static void
gtk_overlay_child_get_preferred_width (GtkWidget *widget,
                                         gint      *minimum,
                                         gint      *natural)
{
	gtk_overlay_child_get_size (widget, GTK_ORIENTATION_HORIZONTAL, minimum, natural);
}

static void
gtk_overlay_child_get_preferred_height (GtkWidget *widget,
                                          gint      *minimum,
                                          gint      *natural)
{
	gtk_overlay_child_get_size (widget, GTK_ORIENTATION_VERTICAL, minimum, natural);
}

static void
gtk_overlay_child_size_allocate (GtkWidget     *widget,
                                   GtkAllocation *allocation)
{
	GtkOverlayChild *overlay_child = GTK_OVERLAY_CHILD (widget);
	GtkWidget *child;
	GtkAllocation tmp;

	tmp.width = allocation->width;
	tmp.height = allocation->height;
	tmp.x = tmp.y = 0;

	GTK_WIDGET_CLASS (gtk_overlay_child_parent_class)->size_allocate (widget, allocation);

	child = gtk_bin_get_child (GTK_BIN (overlay_child));

	if (child != NULL)
	{
		gtk_widget_size_allocate (child, &tmp);
	}
}

static void
gtk_overlay_child_add (GtkContainer *container,
                         GtkWidget    *widget)
{
	GtkOverlayChild *overlay_child = GTK_OVERLAY_CHILD (container);

	overlay_child->priv->binding = g_object_bind_property (G_OBJECT (widget), "visible",
							       G_OBJECT (container), "visible",
							       G_BINDING_BIDIRECTIONAL);

	GTK_CONTAINER_CLASS (gtk_overlay_child_parent_class)->add (container, widget);
}

static void
gtk_overlay_child_remove (GtkContainer *container,
                            GtkWidget    *widget)
{
	GtkOverlayChild *child = GTK_OVERLAY_CHILD (container);

	g_object_unref (child->priv->binding);

	GTK_CONTAINER_CLASS (gtk_overlay_child_parent_class)->remove (container, widget);
}

static void
gtk_overlay_child_class_init (GtkOverlayChildClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
	GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);
	
	object_class->get_property = gtk_overlay_child_get_property;
	object_class->set_property = gtk_overlay_child_set_property;

	widget_class->realize = gtk_overlay_child_realize;
	widget_class->get_preferred_width = gtk_overlay_child_get_preferred_width;
	widget_class->get_preferred_height = gtk_overlay_child_get_preferred_height;
	widget_class->size_allocate = gtk_overlay_child_size_allocate;

	container_class->add = gtk_overlay_child_add;
	container_class->remove = gtk_overlay_child_remove;

	g_object_class_install_property (object_class, PROP_WIDGET,
	                                 g_param_spec_object ("widget",
	                                                      "Widget",
	                                                      "The Widget",
	                                                      GTK_TYPE_WIDGET,
	                                                      G_PARAM_READWRITE |
	                                                      G_PARAM_CONSTRUCT_ONLY |
	                                                      G_PARAM_STATIC_STRINGS));

	g_object_class_install_property (object_class, PROP_OFFSET,
	                                 g_param_spec_uint ("offset",
	                                                    "Offset",
	                                                    "The Widget Offset",
	                                                    0,
	                                                    G_MAXUINT,
	                                                    0,
	                                                    G_PARAM_READWRITE |
	                                                    G_PARAM_CONSTRUCT |
	                                                    G_PARAM_STATIC_STRINGS));

	g_type_class_add_private (object_class, sizeof (GtkOverlayChildPrivate));
}

static void
gtk_overlay_child_init (GtkOverlayChild *child)
{
	child->priv = G_TYPE_INSTANCE_GET_PRIVATE (child,
	                                           GTK_TYPE_OVERLAY_CHILD,
	                                           GtkOverlayChildPrivate);

	gtk_widget_set_has_window (GTK_WIDGET (child), TRUE);
}

/**
 * gtk_overlay_child_new:
 * @widget: a #GtkWidget
 *
 * Creates a new #GtkOverlayChild object
 *
 * Returns: a new #GtkOverlayChild object
 */
GtkOverlayChild *
gtk_overlay_child_new (GtkWidget *widget)
{
	return g_object_new (GTK_TYPE_OVERLAY_CHILD,
	                     "widget", widget,
	                     NULL);
}

/**
 * gtk_overlay_child_get_offset:
 * @child: a #GtkOverlayChild
 *
 * Gets the offset for @child. The offset is usually used by #GtkOverlay
 * to not place the widget directly in the border of the container
 *
 * Returns: the offset for @child
 */
guint
gtk_overlay_child_get_offset (GtkOverlayChild *child)
{
	g_return_val_if_fail (GTK_IS_OVERLAY_CHILD (child), 0);

	return child->priv->offset;
}

/**
 * gtk_overlay_child_set_offset:
 * @child: a #GtkOverlayChild
 * @offset: the offset for @child
 *
 * Sets the new offset for @child
 */
void
gtk_overlay_child_set_offset (GtkOverlayChild *child,
                                guint              offset)
{
	g_return_if_fail (GTK_IS_OVERLAY_CHILD (child));

	if (child->priv->offset != offset)
	{
		child->priv->offset = offset;

		g_object_notify (G_OBJECT (child), "offset");
	}
}

/* ex:set ts=8 noet: */
