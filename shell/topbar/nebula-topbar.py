#!/usr/bin/env python3
"""NebulaOS Top Bar - System panel for the desktop"""

import gi
gi.require_version('Gtk', '4.0')
gi.require_version('Gtk4LayerShell', '1.0')
from gi.repository import Gtk, Gtk4LayerShell, GLib
import time

class NebulaTopBar(Gtk.Application):
    def __init__(self):
        super().__init__(application_id='org.nebulaos.topbar')

    def do_activate(self):
        window = Gtk.ApplicationWindow(application=self)
        window.set_title("nebula-topbar")

        Gtk4LayerShell.init_for_window(window)
        Gtk4LayerShell.set_layer(window, Gtk4LayerShell.Layer.TOP)
        Gtk4LayerShell.set_anchor(window, Gtk4LayerShell.Edge.TOP, True)
        Gtk4LayerShell.set_anchor(window, Gtk4LayerShell.Edge.LEFT, True)
        Gtk4LayerShell.set_anchor(window, Gtk4LayerShell.Edge.RIGHT, True)
        Gtk4LayerShell.set_keyboard_interactivity(window, False)
        Gtk4LayerShell.set_exclusive_zone(window, 32)
        Gtk4LayerShell.set_namespace(window, "nebula-topbar")

        window.set_size_request(-1, 32)

        box = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=0)
        box.set_margin_start(8)
        box.set_margin_end(8)

        # Left: App menu
        menu_btn = Gtk.Button(label="Nebula")
        menu_btn.add_css_class("flat")
        box.append(menu_btn)

        # Center: Window title (spacer)
        spacer = Gtk.Box()
        spacer.set_hexpand(True)
        box.append(spacer)

        # Right: Clock
        clock_label = Gtk.Label(label=time.strftime("%H:%M"))
        box.append(clock_label)

        window.set_child(box)

        # Update clock every minute
        def update_clock():
            clock_label.set_text(time.strftime("%H:%M"))
            return True
        GLib.timeout_add_seconds(60, update_clock)

        window.present()

if __name__ == '__main__':
    app = NebulaTopBar()
    app.run([])
