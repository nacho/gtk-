/* testexpand.c
 * Copyright (C) 2010 Havoc Pennington
 *
 * Author:
 *      Havoc Pennington <hp@pobox.com>
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

#include <gtk/gtk.h>

static void
on_toggle_hexpand (GtkToggleButton *toggle,
                   void            *data)
{
  GtkWidget *parent;

  /* get the event box with color set on it */
  parent = gtk_widget_get_parent (gtk_widget_get_parent (GTK_WIDGET (toggle)));

  g_object_set (toggle,
                "hexpand", gtk_toggle_button_get_active (toggle),
                NULL);
}

static void
on_toggle_vexpand (GtkToggleButton *toggle,
                   void            *data)
{
  GtkWidget *parent;

  /* get the event box with color set on it */
  parent = gtk_widget_get_parent (gtk_widget_get_parent (GTK_WIDGET (toggle)));

  g_object_set (toggle,
                "vexpand", gtk_toggle_button_get_active (toggle),
                NULL);
}

static GtkWidget*
create_window (void)
{
  GtkWidget *window;
  GtkWidget *box1, *box2, *box3;
  GtkWidget *toggle;
  GtkWidget *alignment;
  GtkWidget *colorbox;
  GdkColor red, blue;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  box1 = gtk_box_new (GTK_ORIENTATION_VERTICAL, FALSE, 0);
  box2 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, FALSE, 0);
  box3 = gtk_box_new (GTK_ORIENTATION_VERTICAL, FALSE, 0);

  gtk_box_pack_start (GTK_BOX (box1),
                      gtk_label_new ("VBox 1 Top"),
                      FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (box1),
                      box2,
                      FALSE, TRUE, 0);
  gtk_box_pack_end (GTK_BOX (box1),
                    gtk_label_new ("VBox 1 Bottom"),
                    FALSE, FALSE, 0);

  gtk_box_pack_start (GTK_BOX (box2),
                      gtk_label_new ("HBox 2 Left"),
                      FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (box2),
                      box3,
                      FALSE, TRUE, 0);
  gtk_box_pack_end (GTK_BOX (box2),
                    gtk_label_new ("HBox 2 Right"),
                    FALSE, FALSE, 0);

  gtk_box_pack_start (GTK_BOX (box3),
                      gtk_label_new ("VBox 3 Top"),
                      FALSE, FALSE, 0);
  gtk_box_pack_end (GTK_BOX (box3),
                    gtk_label_new ("VBox 3 Bottom"),
                    FALSE, FALSE, 0);

  gdk_color_parse ("red", &red);
  gdk_color_parse ("blue", &blue);

  colorbox = gtk_event_box_new ();
  gtk_widget_modify_bg (colorbox, GTK_STATE_NORMAL, &red);

  alignment = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);
  gtk_container_add (GTK_CONTAINER (colorbox), alignment);

  toggle = gtk_toggle_button_new_with_label ("H Expand");
  g_signal_connect (G_OBJECT (toggle), "toggled",
                    G_CALLBACK (on_toggle_hexpand), NULL);
  gtk_container_add (GTK_CONTAINER (alignment), toggle);

  gtk_box_pack_start (GTK_BOX (box3),
                      colorbox,
                      FALSE, TRUE, 0);

  colorbox = gtk_event_box_new ();
  gtk_widget_modify_bg (colorbox, GTK_STATE_NORMAL, &blue);

  alignment = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);
  gtk_container_add (GTK_CONTAINER (colorbox), alignment);

  toggle = gtk_toggle_button_new_with_label ("V Expand");
  g_signal_connect (G_OBJECT (toggle), "toggled",
                    G_CALLBACK (on_toggle_vexpand), NULL);
  gtk_container_add (GTK_CONTAINER (alignment), toggle);
  gtk_box_pack_start (GTK_BOX (box3),
                      colorbox,
                      FALSE, TRUE, 0);

  gtk_container_add (GTK_CONTAINER (window),
                     box1);
  gtk_widget_show_all (box1);

  return window;
}

int
main (int argc, char *argv[])
{
  GtkWidget *window;

  gtk_init (&argc, &argv);

  window = create_window ();

  g_signal_connect (window, "delete-event",
                    G_CALLBACK (gtk_main_quit), window);

  gtk_widget_show (window);

  gtk_main ();

  return 0;
}