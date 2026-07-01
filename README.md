# NebulaOS

A custom Linux distribution with liquid glass aesthetics, built on Ubuntu 24.04 LTS with GNOME.

## Features

- **Liquid Glass Design** - Violet/purple glassmorphism throughout the desktop
- **macOS-like Experience** - Bottom dock, polished UI, Apple-inspired workflow
- **Nebula Theme** - Dark violet GTK/Shell theme with glass effects
- **Dash to Dock** - Floating bottom dock with blur and transparency
- **Blur My Shell** - Frosted glass effects on panels and overview
- **Custom Branding** - Boot splash, GRUB, login screen, installer
- **Calamares Installer** - Beautiful branded installation experience
- **First Boot Setup** - Automatic configuration on first login

## Color Palette

| Role | Hex | Usage |
|------|-----|-------|
| Deep Space | `#0f0a1a` | Backgrounds |
| Nebula Purple | `#7c3aed` | Primary accent |
| Violet Glow | `#a855f7` | Hover/secondary |
| Cosmic Lavender | `#e2d9f3` | Text |
| Glass Surface | `rgba(124,58,237,0.08)` | Panels/cards |
| Stardust | `#c4b5fd` | Highlights |

## Project Structure

```
nebulaos/
├── themes/nebula-gtk/        # GTK3/4 + GNOME Shell theme
│   ├── index.theme
│   ├── gtk-3.0/gtk.css       # 1949 lines of comprehensive GTK3 CSS
│   ├── gtk-4.0/gtk.css       # 1727 lines of libadwaita overrides
│   └── gnome-shell/          # GNOME Shell theme
├── assets/
│   ├── wallpapers/           # Nebula wallpapers (4K, 1440p, 1080p)
│   └── branding/             # GRUB background image
├── calamares/branding/       # Installer branding & slideshow
├── dconf/                    # Default GNOME settings
├── terminal/                 # Terminal color profile
├── grub/                     # GRUB boot menu theme
├── plymouth/                 # Boot splash theme
├── gdm/                      # Login screen CSS
├── scripts/
│   ├── cubic-build.sh        # Main Cubic build script (run in chroot)
│   ├── nebula-first-boot.sh  # First boot setup script
│   └── generate_wallpaper.py # Wallpaper generator
└── iso-builder/              # ISO build configuration
```

## Building the ISO

### Prerequisites

- Ubuntu 24.04 LTS (recommended build environment)
- [Cubic](https://github.com/PJ-Singh-001/Cubic) installed
- 20GB+ free disk space
- Internet connection

### Step 1: Install Cubic

```bash
sudo apt-add-repository universe
sudo apt-add-repository ppa:cubic-wizard/release
sudo apt update
sudo apt install cubic
```

### Step 2: Create Cubic Project

1. Open Cubic
2. Create new project directory (e.g., `~/nebulaos-build`)
3. Select **Ubuntu 24.04 LTS Noble Numbat** as the base ISO
4. Set ISO filename: `NebulaOS-1.0-amd64.iso`
5. Click **Next**

### Step 3: Copy Assets to Chroot

In Cubic's **Terminal** page, copy the nebulaos directory into the chroot:

```bash
# From the Cubic terminal, mount and copy:
cp -r /path/to/nebulaos /tmp/nebulaos
```

Or use Cubic's file copy feature to copy the `nebulaos/` directory.

### Step 4: Run Build Script

In the Cubic terminal:

```bash
bash /tmp/nebulaos/cubic-build.sh
```

This will:
- Install GNOME desktop and all packages
- Install Dash to Dock, Blur My Shell, and other extensions
- Install the Nebula GTK/Shell theme
- Configure Calamares installer with NebulaOS branding
- Set up GRUB, Plymouth, and GDM themes
- Apply all default settings
- Clean up unnecessary packages

### Step 5: Generate ISO

1. Click **Next** in Cubic
2. Review the package list
3. Click **Generate** to create the ISO
4. Choose compression (recommend `gzip` for speed, `xz` for smaller size)
5. Wait for generation to complete

### Step 6: Test

Use Cubic's built-in QEMU to test, or write to USB:

```bash
sudo dd if=NebulaOS-1.0-amd64.iso of=/dev/sdX bs=4M status=progress
```

## What Gets Installed

### Desktop Environment
- GNOME 46+ with extensions
- Dash to Dock (bottom floating, macOS-style)
- Blur My Shell (frosted glass effects)
- AppIndicator (system tray)
- Just Perfection (UI customization)
- User Themes (custom shell theme)

### Applications
- Nautilus (file manager)
- GNOME Terminal (custom nebula color scheme)
- GNOME Text Editor
- Firefox (default browser)
- GNOME Calculator
- GNOME Screenshot
- Eye of GNOME (image viewer)
- GNOME Software (app store)
- File Roller (archive manager)
- VLC (media player)
- LibreOffice (office suite)

### Theming
- **Nebula** GTK3/4 theme (dark violet glassmorphism)
- **Nebula** GNOME Shell theme
- Custom terminal color scheme
- Custom GRUB boot menu
- Custom Plymouth boot splash
- Custom GDM login screen
- NebulaOS wallpapers

## Customization

### After Installation

Settings can be tweaked via:
- **GNOME Settings** - Standard desktop settings
- **GNOME Tweaks** - Advanced theme and font settings
- **Extension Manager** - Enable/disable/configure extensions
- **dconf Editor** - Low-level settings

### Key dconf Paths

```
org/gnome/shell/extensions/dash-to-dock/    # Dock settings
org/gnome/shell/extensions/blur-my-shell/   # Blur settings
org/gnome/desktop/interface/                 # Theme and fonts
org/gnome/desktop/background/               # Wallpaper
```

## License

MIT License
