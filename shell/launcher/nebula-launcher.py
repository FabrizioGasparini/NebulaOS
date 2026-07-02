#!/usr/bin/env python3
"""NebulaOS Launcher - Spotlight-style application launcher"""

import gi
gi.require_version('Gtk', '4.0')
gi.require_version('Gtk4LayerShell', '1.0')
from gi.repository import Gtk, Gtk4LayerShell, Gdk

class NebulaLauncher(Gtk.Application):
    def __init__(self):
        super().__init__(application_id='org.nebulaos.launcher')

    def do_activate(self):
        window = Gtk.ApplicationWindow(application=self)
        window.set_title("nebula-launcher")
        window.set_default_size(600, 400)

        Gtk4LayerShell.init_for_window(window)
        Gtk4LayerShell.set_layer(window, Gtk4LayerShell.Layer.OVERLAY)
        Gtk4LayerShell.set_keyboard_interactivity(window, True)
        Gtk4LayerShell.set_namespace(window, "nebula-launcher")

        # Center the window
        Gtk4LayerShell.set_anchor(window, Gtk4LayerShell.Edge.TOP, True)
        Gtk4LayerShell.set_anchor(window, Gtk4LayerShell.Edge.BOTTOM, True)
        Gtk4LayerShell.set_anchor(window, Gtk4LayerShell.Edge.LEFT, True)
        Gtk4LayerShell.set_anchor(window, Gtk4LayerShell.Edge.RIGHT, True)

        main_box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=8)
        main_box.set_margin_top(40)
        main_box.set_margin_bottom(40)
        main_box.set_margin_start(100)
        main_box.set_margin_end(100)

        # Search entry
        search_entry = Gtk.SearchEntry()
        search_entry.set_placeholder_text("Search applications, files...")
        main_box.append(search_entry)

        # Results list
        scrolled = Gtk.ScrolledWindow()
        scrolled.set_vexpand(True)
        listbox = Gtk.ListBox()
        listbox.add_css_class("boxed-list")

        apps = [
            "Nebula Files",
            "Nebula Terminal",
            "Nebula Editor",
            "Nebula Settings",
            "Nebula Monitor",
        ]

        for app_name in apps:
            row = Gtk.ListBoxRow()
            label = Gtk.Label(label=app_name)
            label.set_xalign(0)
            row.set_child(label)
            listbox.append(row)

        scrolled.set_child(listbox)
        main_box.append(scrolled)

        window.set_child(main_box)
        window.present()

if __name__ == '__main__':
    app = NebulaLauncher()
    app.run([])
