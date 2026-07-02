# NebulaOS

A custom Linux distribution with a liquid glass desktop experience, built on Arch Linux.

## Features

- **Custom Wayland Compositor**: Built on wlroots with OpenGL ES 3.0 glass blur effects
- **Liquid Glass Design**: Semi-transparent windows with backdrop blur, rounded corners, and noise texture
- **Violet/Purple Theme**: Beautiful violet color palette throughout the desktop
- **macOS-style Dock**: Animated dock with icon magnification
- **Spotlight-style Launcher**: Super+Space to search applications and files
- **Toast Notifications**: Slide-in notifications from the right
- **Custom Applications**: File manager, terminal, text editor, settings, system monitor

## Architecture

```
┌─────────────────────────────────────────────────────┐
│                   Applications                       │
├─────────────────────────────────────────────────────┤
│              GTK4 + Glass CSS Theme                  │
├─────────────────────────────────────────────────────┤
│              Desktop Shell (Python/GTK4)             │
│     Dock │ Top Bar │ Launcher │ Notifications        │
├─────────────────────────────────────────────────────┤
│         nebula-compositor (C + wlroots)              │
│  Window Management │ Glass Blur │ Animations          │
├─────────────────────────────────────────────────────┤
│              Wayland Protocol + layer-shell           │
├─────────────────────────────────────────────────────┤
│              Linux Kernel (Arch Linux)                │
└─────────────────────────────────────────────────────┘
```

## Building

### Prerequisites

- Arch Linux (for building the ISO)
- `archiso` package
- `meson`, `ninja`, `wayland-scanner` (for compositor)
- `wlroots-0.18`, `wayland-protocols`, `libinput`, `xkbcommon`, `mesa` (dev headers)

### Build the Compositor

```bash
cd compositor
meson setup build
ninja -C build
```

### Build the ISO

```bash
# As root:
sudo ./scripts/build-iso.sh
```

### Test in QEMU

```bash
./scripts/run-qemu.sh
```

## Project Structure

```
nebulaos/
├── compositor/              # Wayland compositor (C + wlroots)
│   ├── src/                # Source files
│   ├── shaders/            # GLSL shaders
│   ├── protocol/           # Wayland protocol XML
│   └── meson.build         # Build configuration
├── shell/                   # Desktop shell components (Python)
│   ├── dock/               # macOS-style dock
│   ├── topbar/             # System top bar
│   ├── launcher/           # Spotlight-style launcher
│   └── notifications/      # Toast notification system
├── apps/                    # Custom applications (Python)
│   ├── nebula-files/       # File manager
│   ├── nebula-terminal/    # Terminal emulator
│   ├── nebula-editor/      # Text editor
│   ├── nebula-settings/    # System settings
│   └── nebula-monitor/     # System monitor
├── theme/                   # Global theme
│   └── gtk4/               # GTK4 CSS theme
├── iso/                     # ISO build configuration
│   ├── profiledef.sh       # ISO metadata
│   ├── packages.x86_64     # Package list
│   └── airootfs/           # Root filesystem overlay
├── installer/               # Installation wizard
├── docs/                    # Documentation
└── scripts/                 # Build scripts
```

## Design System

### Colors

- **Primary**: Violet (#8B5CF6)
- **Accent**: Purple (#C084FC)
- **Surface**: Semi-transparent violet
- **Text**: White/Light violet

### Glass Effects

- **Background**: 55% opacity with 30px backdrop blur
- **Border**: 1px white at 12% opacity
- **Shadow**: 0 8px 32px rgba(0,0,0,0.35)
- **Corner Radius**: 16px (windows), 24px (dock)
- **Noise**: 3% grain texture

### Animations

- Window open: 300ms ease-out-expo
- Window close: 200ms ease-in
- Dock magnify: Spring (mass=0.1, stiffness=150)
- Launcher: 250ms ease-out

## License

MIT License
