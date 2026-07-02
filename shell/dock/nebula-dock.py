#!/usr/bin/env python3
"""NebulaOS Dock - macOS-style application dock"""

import gi
gi.require_version('Gtk', '4.0')
gi.require_version('Gtk4LayerShell', '1.0')
from gi.repository import Gtk, Gtk4LayerShell, Gdk, GLib
import math

class DockItem(Gtk.Box):
    def __init__(self, app_name, icon_name):
        super().__init__(orientation=Gtk.Orientation.VERTICAL, spacing=2)
        self.app_name = app_name
        self.base_size = 48
        self.current_size = self.base_size

        self.icon = Gtk.Image.new_from_icon_name(icon_name)
        self.icon.set_pixel_size(self.base_size)
        self.append(self.icon)

        # Running indicator dot
        self.dot = Gtk.DrawingArea()
        self.dot.set_content_width(6)
        self.dot.set_content_height(6)
        self.dot.set_visible(False)
        self.append(self.dot)

    def set_scale(self, scale):
        size = int(self.base_size * scale)
        self.icon.set_pixel_size(size)

class NebulaDock(Gtk.Application):
    def __init__(self):
        super().__init__(application_id='org.nebulaos.dock')
        self.items = []

    def do_activate(self):
        window = Gtk.ApplicationWindow(application=self)
        window.set_title("nebula-dock")

        Gtk4LayerShell.init_for_window(window)
        Gtk4LayerShell.set_layer(window, Gtk4LayerShell.Layer.BOTTOM)
        Gtk4LayerShell.set_anchor(window, Gtk4LayerShell.Edge.BOTTOM, True)
        Gtk4LayerShell.set_anchor(window, Gtk4LayerShell.Edge.LEFT, True)
        Gtk4LayerShell.set_anchor(window, Gtk4LayerShell.Edge.RIGHT, True)
        Gtk4LayerShell.set_keyboard_interactivity(window, False)
        Gtk4LayerShell.set_exclusive_zone(window, 64)
        Gtk4LayerShell.set_namespace(window, "nebula-dock")

        window.set_default_size(400, 64)
        window.set_size_request(400, 64)

        # Dock container
        dock_box = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=4)
        dock_box.set_halign(Gtk.Align.CENTER)
        dock_box.set_valign(Gtk.Align.CENTER)
        dock_box.set_margin_top(4)
        dock_box.set_margin_bottom(4)

        # Add dock items
        apps = [
            ("Files", "system-file-manager"),
            ("Terminal", "utilities-terminal"),
            ("Editor", "accessories-text-editor"),
            ("Browser", "internet-web-browser"),
            ("Settings", "preferences-system"),
        ]

        for app_name, icon_name in apps:
            item = DockItem(app_name, icon_name)
            dock_box.append(item)
            self.items.append(item)

        window.set_child(dock_box)

        # Motion controller for magnification
        motion = Gtk.EventControllerMotion()
        motion.connect("motion", self.on_motion)
        window.add_controller(motion)

        window.present()

    def on_motion(self, controller, x, y):
        dock_width = 400
        center_x = dock_width / 2

        for i, item in enumerate(self.items):
            item_center_x = (i * 56) + 28
            dx = x - item_center_x
            dist = abs(dx)

            max_distance = 120.0
            sigma = max_distance / 2.5
            gaussian = math.exp(-(dx * dx) / (2.0 * sigma * sigma))

            scale = 1.0 + (0.5 * gaussian)
            item.set_scale(scale)

if __name__ == '__main__':
    app = NebulaDock()
    app.run([])
