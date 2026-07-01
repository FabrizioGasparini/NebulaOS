# NebulaOS

A custom Linux distribution with liquid glass aesthetics, built on Arch Linux with a custom Wayland compositor.

## Features

- **Custom Wayland Compositor** - Built on wlroots with blur and glass effects
- **macOS-style Dock** - With cosine magnification animation
- **Glass Topbar** - Clock, system tray, app menu, workspace indicators
- **Violet Glass Theme** - Apple-like liquid glass aesthetic
- **Custom Applications** - File manager, terminal, settings, calculator, text editor
- **Mission Control** - Workspace overview (like macOS Expose)
- **Notification Daemon** - macOS-style slide-in blur notifications
- **Full Branding** - Plymouth boot splash, GRUB theme, greetd login

## Architecture

```
nebulaos/
├── compositor/          # Custom wlroots Wayland compositor (C)
├── dock/               # macOS-style dock with magnification
├── topbar/             # Top panel with clock, tray, app menu
├── file-manager/       # Custom file manager
├── terminal/           # Terminal emulator (VTE)
├── settings/           # Settings application
├── calculator/         # Calculator
├── text-editor/        # Text editor
├── notification-daemon/# Notification system
├── mission-control/    # Workspace overview
├── themes/             # GTK4 violet glass theme
├── branding/           # Plymouth, GRUB, greetd themes
└── iso-builder/        # archiso configuration
```

## Building

### Prerequisites

- Arch Linux (or derivative)
- `base-devel`, `meson`, `ninja`, `pkg-config`
- Wayland development libraries

### Build Components

```bash
# Install dependencies
sudo pacman -S meson ninja pkg-config wayland wlroots gtk4 \
  gtk4-layer-shell vte3 wayland-protocols xkbcommon libinput \
  pixman libdrm mesa

# Build all components
meson setup builddir
ninja -C builddir
```

### Build ISO

```bash
cd iso-builder
sudo ./build.sh
```

The ISO will be in `iso-builder/output/`.

## Default Applications

| Application | Command |
|-------------|---------|
| Terminal | `nebula-terminal` |
| File Manager | `nebula-file-manager` |
| Text Editor | `nebula-text-editor` |
| Settings | `nebula-settings` |
| Calculator | `nebula-calculator` |

## Keybindings

| Shortcut | Action |
|----------|--------|
| `Super + Enter` | Open terminal |
| `Super + Q` | Close window |
| `Super + H` | Toggle maximize |
| `Super + F` | Toggle fullscreen |
| `Super + 1-0` | Switch workspace |
| `Super + Escape` | Close focused window |

## Theme

NebulaOS uses a violet glass theme with:
- Deep purple backgrounds (`#0f0520`)
- Glass transparency with blur effects
- Violet accent color (`#8b5cf6`)
- Smooth 200ms transitions
- Rounded corners (8-16px)

## License

MIT License - See LICENSE file for details.
