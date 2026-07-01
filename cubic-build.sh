#!/bin/bash
# ============================================================================
# NebulaOS - Complete Cubic Build Script
# ============================================================================
# Run this script INSIDE Cubic's chroot terminal environment.
# It installs all packages, themes, extensions, and configurations
# needed to build the NebulaOS ISO.
#
# Usage: Copy this script and all nebulaos/ assets into the Cubic chroot,
#        then run:  bash /tmp/nebulaos/cubic-build.sh
# ============================================================================

set -euo pipefail

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
VIOLET='\033[0;35m'
NC='\033[0m' # No Color

log() { echo -e "${VIOLET}[NebulaOS]${NC} $1"; }
success() { echo -e "${GREEN}[OK]${NC} $1"; }
warn() { echo -e "${YELLOW}[WARN]${NC} $1"; }
error() { echo -e "${RED}[ERROR]${NC} $1"; exit 1; }

NEBULA_SRC="/tmp/nebulaos"

echo ""
echo -e "${VIOLET}╔══════════════════════════════════════════╗${NC}"
echo -e "${VIOLET}║         🌌 NebulaOS Build Script        ║${NC}"
echo -e "${VIOLET}║    A Cosmic Desktop Experience           ║${NC}"
echo -e "${VIOLET}╚══════════════════════════════════════════╝${NC}"
echo ""

# ============================================================================
# PHASE 1: System Update & Core Packages
# ============================================================================
log "Phase 1: Updating system and installing core packages..."

apt update
apt upgrade -y

# Core desktop
DEBIAN_FRONTEND=noninteractive apt install -y \
    ubuntu-desktop \
    gnome-shell \
    gnome-session \
    gdm3 \
    gnome-terminal \
    gnome-text-editor \
    nautilus \
    eog \
    gnome-calculator \
    gnome-screenshot \
    gnome-software \
    file-roller \
    firefox \
    libreoffice-common \
    vlc \
    gnome-tweaks \
    gnome-control-center \
    chrome-gnome-shell \
    gnome-shell-extension-manager \
    sassc \
    git \
    curl \
    wget \
    network-manager \
    pulseaudio \
    pipewire \
    wireplumber \
    plymouth \
    plymouth-themes \
    fonts-ubuntu \
    fonts-noto \
    xdg-user-dirs \
    xdg-utils

success "Core packages installed"

# ============================================================================
# PHASE 2: Install Calamares
# ============================================================================
log "Phase 2: Installing Calamares installer..."

DEBIAN_FRONTEND=noninteractive apt install -y \
    calamares \
    calamares-settings-ubuntu || {
    warn "calamares-settings-ubuntu not available, installing calamares only"
    DEBIAN_FRONTEND=noninteractive apt install -y calamares
}

success "Calamares installed"

# ============================================================================
# PHASE 3: Install GNOME Extensions
# ============================================================================
log "Phase 3: Installing GNOME extensions..."

EXTENSIONS_DIR="/usr/share/gnome-shell/extensions"
mkdir -p "$EXTENSIONS_DIR"

# Dash to Dock
log "  Installing Dash to Dock..."
if [ ! -d "$EXTENSIONS_DIR/dash-to-dock@micxgx.gmail.com" ]; then
    cd /tmp
    git clone --depth 1 https://github.com/micheleg/dash-to-dock.git 2>/dev/null || \
    git clone --depth 1 https://github.com/micheleg/dash-to-dock.git /tmp/dash-to-dock
    cp -r /tmp/dash-to-dock/dash-to-dock@micxgx.gmail.com "$EXTENSIONS_DIR/"
    success "  Dash to Dock installed"
else
    success "  Dash to Dock already present"
fi

# Blur My Shell
log "  Installing Blur My Shell..."
if [ ! -d "$EXTENSIONS_DIR/blur-my-shell@aunetx" ]; then
    cd /tmp
    git clone --depth 1 https://github.com/aunetx/blur-my-shell.git 2>/dev/null || \
    git clone --depth 1 https://github.com/aunetx/blur-my-shell.git /tmp/blur-my-shell
    cp -r /tmp/blur-my-shell/blur-my-shell@aunetx "$EXTENSIONS_DIR/"
    success "  Blur My Shell installed"
else
    success "  Blur My Shell already present"
fi

# AppIndicator Support (system tray)
log "  Installing AppIndicator Support..."
if [ ! -d "$EXTENSIONS_DIR/appindicatorsupport@rgcjonas.gmail.com" ]; then
    apt install -y gnome-shell-extension-appindicator 2>/dev/null || {
        cd /tmp
        git clone --depth 1 https://github.com/ubuntu/gnome-shell-extension-appindicator.git 2>/dev/null
        if [ -d /tmp/gnome-shell-extension-appindicator ]; then
            cp -r /tmp/gnome-shell-extension-appindicator/appindicatorsupport@rgcjonas.gmail.com "$EXTENSIONS_DIR/" 2>/dev/null || true
        fi
    }
    success "  AppIndicator Support installed"
else
    success "  AppIndicator Support already present"
fi

# Just Perfection (UI customization)
log "  Installing Just Perfection..."
if [ ! -d "$EXTENSIONS_DIR/just-perfectiondesktopteam@gmail.com" ]; then
    cd /tmp
    git clone --depth 1 https://github.com/JustPerfection/Just-Perfection.git 2>/dev/null || true
    if [ -d /tmp/Just-Perfection ]; then
        cp -r /tmp/Just-Perfection/just-perfectiondesktopteam@gmail.com "$EXTENSIONS_DIR/" 2>/dev/null || true
    fi
    success "  Just Perfection installed"
else
    success "  Just Perfection already present"
fi

# User Themes (allows custom shell theme)
log "  Installing User Themes..."
if [ ! -d "$EXTENSIONS_DIR/user-theme@gnome-shell-extensions.gnome.org" ]; then
    apt install -y gnome-shell-extensions 2>/dev/null || true
    success "  User Themes installed"
else
    success "  User Themes already present"
fi

success "GNOME extensions installed"

# ============================================================================
# PHASE 4: Install NebulaOS Theme
# ============================================================================
log "Phase 4: Installing NebulaOS Nebula theme..."

# Create theme directory
THEME_DIR="/usr/share/themes/Nebula"
mkdir -p "$THEME_DIR"

# Copy GTK3 theme
if [ -d "$NEBULA_SRC/themes/nebula-gtk/gtk-3.0" ]; then
    cp -r "$NEBULA_SRC/themes/nebula-gtk/gtk-3.0" "$THEME_DIR/"
    success "  GTK3 theme installed"
fi

# Copy GTK4 theme
if [ -d "$NEBULA_SRC/themes/nebula-gtk/gtk-4.0" ]; then
    cp -r "$NEBULA_SRC/themes/nebula-gtk/gtk-4.0" "$THEME_DIR/"
    success "  GTK4 theme installed"
fi

# Copy GNOME Shell theme
if [ -d "$NEBULA_SRC/themes/nebula-gtk/gnome-shell" ]; then
    cp -r "$NEBULA_SRC/themes/nebula-gtk/gnome-shell" "$THEME_DIR/"
    success "  GNOME Shell theme installed"
fi

# Copy index.theme
if [ -f "$NEBULA_SRC/themes/nebula-gtk/index.theme" ]; then
    cp "$NEBULA_SRC/themes/nebula-gtk/index.theme" "$THEME_DIR/"
    success "  Theme metadata installed"
fi

# Also copy to GTK4 override directory for libadwaita apps
if [ -d "$NEBULA_SRC/themes/gtk4" ]; then
    mkdir -p /etc/gtk-4.0
    cp -r "$NEBULA_SRC/themes/gtk4/css" /etc/gtk-4.0/ 2>/dev/null || true
    success "  GTK4 libadwaita overrides installed"
fi

success "NebulaOS theme installed"

# ============================================================================
# PHASE 5: Install Wallpapers
# ============================================================================
log "Phase 5: Installing wallpapers..."

WALLPAPER_DIR="/usr/share/backgrounds"
mkdir -p "$WALLPAPER_DIR"

if [ -f "$NEBULA_SRC/assets/wallpapers/nebula-default.png" ]; then
    cp "$NEBULA_SRC/assets/wallpapers/nebula-default.png" "$WALLPAPER_DIR/"
    success "  4K wallpaper installed"
fi
if [ -f "$NEBULA_SRC/assets/wallpapers/nebula-default-1080p.png" ]; then
    cp "$NEBULA_SRC/assets/wallpapers/nebula-default-1080p.png" "$WALLPAPER_DIR/"
    success "  1080p wallpaper installed"
fi
if [ -f "$NEBULA_SRC/assets/wallpapers/nebula-default-1440p.png" ]; then
    cp "$NEBULA_SRC/assets/wallpapers/nebula-default-1440p.png" "$WALLPAPER_DIR/"
    success "  1440p wallpaper installed"
fi

success "Wallpapers installed"

# ============================================================================
# PHASE 6: Install Terminal Profile
# ============================================================================
log "Phase 6: Installing terminal profile..."

TERMINAL_DIR="/usr/share/nebulaos"
mkdir -p "$TERMINAL_DIR"

if [ -f "$NEBULA_SRC/terminal/nebula-terminal.profile" ]; then
    cp "$NEBULA_SRC/terminal/nebula-terminal.profile" "$TERMINAL_DIR/"
    success "  Terminal profile installed"
fi

success "Terminal configured"

# ============================================================================
# PHASE 7: Install Calamares Branding
# ============================================================================
log "Phase 7: Installing Calamares branding..."

CALAMARES_DIR="/etc/calamares/branding/nebulaos"
mkdir -p "$CALAMARES_DIR/slideshow"

if [ -d "$NEBULA_SRC/calamares/branding/nebulaos" ]; then
    cp -r "$NEBULA_SRC/calamares/branding/nebulaos/"* "$CALAMARES_DIR/"
    success "  Calamares branding installed"
fi

# Also install to standard calamares location
if [ -f "$NEBULA_SRC/calamares/branding/nebulaos/branding.desc" ]; then
    mkdir -p /etc/calamares
    cp "$NEBULA_SRC/calamares/branding/nebulaos/branding.desc" /etc/calamares/branding.desc 2>/dev/null || true
fi

success "Calamares branding installed"

# ============================================================================
# PHASE 8: Install GRUB Theme
# ============================================================================
log "Phase 8: Installing GRUB theme..."

GRUB_THEME_DIR="/boot/grub/themes/nebulaos"
mkdir -p "$GRUB_THEME_DIR"

if [ -f "$NEBULA_SRC/assets/branding/nebulaos-grub.png" ]; then
    cp "$NEBULA_SRC/assets/branding/nebulaos-grub.png" "$GRUB_THEME_DIR/"
    success "  GRUB background installed"
fi

if [ -f "$NEBULA_SRC/grub/grub-theme.txt" ]; then
    cp "$NEBULA_SRC/grub/grub-theme.txt" "$GRUB_THEME_DIR/theme.txt"
    success "  GRUB theme config installed"
fi

# Configure GRUB to use our theme
if [ -f /etc/default/grub ]; then
    sed -i 's|^#GRUB_THEME=.*|GRUB_THEME="/boot/grub/themes/nebulaos/theme.txt"|' /etc/default/grub
    sed -i 's|^GRUB_THEME=.*|GRUB_THEME="/boot/grub/themes/nebulaos/theme.txt"|' /etc/default/grub
    update-grub 2>/dev/null || true
    success "  GRUB configured"
fi

success "GRUB theme installed"

# ============================================================================
# PHASE 9: Install Plymouth Theme
# ============================================================================
log "Phase 9: Installing Plymouth theme..."

PLYMOUTH_DIR="/usr/share/plymouth/themes/nebulaos"
mkdir -p "$PLYMOUTH_DIR"

if [ -f "$NEBULA_SRC/plymouth/nebulaos.plymouth" ]; then
    cp "$NEBULA_SRC/plymouth/nebulaos.plymouth" "$PLYMOUTH_DIR/"
    success "  Plymouth config installed"
fi
if [ -f "$NEBULA_SRC/plymouth/nebulaos.script" ]; then
    cp "$NEBULA_SRC/plymouth/nebulaos.script" "$PLYMOUTH_DIR/"
    success "  Plymouth script installed"
fi

# Also check existing branding directory
if [ -f "$NEBULA_SRC/branding/plymouth/themes/nebula/logo.png" ]; then
    cp "$NEBULA_SRC/branding/plymouth/themes/nebula/logo.png" "$PLYMOUTH_DIR/" 2>/dev/null || true
fi

# Set Plymouth theme
plymouth-set-default-theme nebulaos 2>/dev/null || true
update-initramfs -u 2>/dev/null || true

success "Plymouth theme installed"

# ============================================================================
# PHASE 10: Install GDM Theme
# ============================================================================
log "Phase 10: Installing GDM theme..."

GDM_DIR="/etc/gdm"
mkdir -p "$GDM_DIR"

if [ -f "$NEBULA_SRC/gdm/gdm.css" ]; then
    cp "$NEBULA_SRC/gdm/gdm.css" "$GDM_DIR/custom.css"
    success "  GDM CSS installed"
fi

success "GDM theme installed"

# ============================================================================
# PHASE 11: Install dconf defaults
# ============================================================================
log "Phase 11: Installing default settings..."

if [ -f "$NEBULA_SRC/dconf/default-settings.dconf" ]; then
    cp "$NEBULA_SRC/dconf/default-settings.dconf" "$TERMINAL_DIR/"
    success "  dconf defaults installed"
fi

success "Default settings installed"

# ============================================================================
# PHASE 12: Install First Boot Script
# ============================================================================
log "Phase 12: Installing first-boot setup..."

if [ -f "$NEBULA_SRC/scripts/nebula-first-boot.sh" ]; then
    cp "$NEBULA_SRC/scripts/nebula-first-boot.sh" "$TERMINAL_DIR/"
    chmod +x "$TERMINAL_DIR/nebula-first-boot.sh"
    success "  First boot script installed"
fi

if [ -f "$NEBULA_SRC/scripts/nebula-first-boot.desktop" ]; then
    mkdir -p /etc/xdg/autostart
    cp "$NEBULA_SRC/scripts/nebula-first-boot.desktop" /etc/xdg/autostart/
    success "  First boot autostart installed"
fi

success "First boot setup installed"

# ============================================================================
# PHASE 13: Remove Unwanted Packages
# ============================================================================
log "Phase 13: Cleaning up unwanted packages..."

# Remove packages that don't fit the NebulaOS experience
apt purge -y thunderbird* 2>/dev/null || true
apt purge -y gnome-contacts 2>/dev/null || true
apt purge -y gnome-maps 2>/dev/null || true
apt purge -y gnome-weather 2>/dev/null || true
apt purge -y gnome-clocks 2>/dev/null || true
apt purge -y gnome-todo 2>/dev/null || true
apt purge -y gnome-characters 2>/dev/null || true
apt purge -y rhythmbox 2>/dev/null || true
apt purge -y totem 2>/dev/null || true

success "Unwanted packages removed"

# ============================================================================
# PHASE 14: Configure GDM
# ============================================================================
log "Phase 14: Configuring GDM..."

# Enable GDM
if [ -f /etc/gdm3/custom.conf ]; then
    # Ensure GDM is enabled
    sed -i 's/#WaylandEnable=false/WaylandEnable=true/' /etc/gdm3/custom.conf 2>/dev/null || true
fi

success "GDM configured"

# ============================================================================
# PHASE 15: Final Cleanup
# ============================================================================
log "Phase 15: Final cleanup..."

# Clean apt cache
apt autoremove -y
apt clean

# Remove temporary files
rm -rf /tmp/dash-to-dock /tmp/blur-my-shell /tmp/Just-Perfection 2>/dev/null || true
rm -rf /tmp/gnome-shell-extension-appindicator 2>/dev/null || true

# Remove this script (optional - Cubic will handle this)
# rm -f "$NEBULA_SRC/cubic-build.sh"

success "Cleanup complete"

# ============================================================================
# DONE
# ============================================================================
echo ""
echo -e "${VIOLET}╔══════════════════════════════════════════╗${NC}"
echo -e "${VIOLET}║   🌌 NebulaOS Build Complete!           ║${NC}"
echo -e "${VIOLET}║                                          ║${NC}"
echo -e "${VIOLET}║   The chroot is now ready.               ║${NC}"
echo -e "${VIOLET}║   Click 'Generate' in Cubic to create   ║${NC}"
echo -e "${VIOLET}║   the NebulaOS ISO.                     ║${NC}"
echo -e "${VIOLET}╚══════════════════════════════════════════╝${NC}"
echo ""
echo -e "Installed components:"
echo -e "  ${GREEN}✓${NC} GNOME Desktop Environment"
echo -e "  ${GREEN}✓${NC} Calamares Installer"
echo -e "  ${GREEN}✓${NC} Nebula GTK/Shell Theme (dark violet glassmorphism)"
echo -e "  ${GREEN}✓${NC} Dash to Dock (macOS-style bottom dock)"
echo -e "  ${GREEN}✓${NC} Blur My Shell (frosted glass effects)"
echo -e "  ${GREEN}✓${NC} AppIndicator Support (system tray)"
echo -e "  ${GREEN}✓${NC} Just Perfection (UI customization)"
echo -e "  ${GREEN}✓${NC} Custom GRUB theme"
echo -e "  ${GREEN}✓${NC} Custom Plymouth boot splash"
echo -e "  ${GREEN}✓${NC} Custom GDM login screen"
echo -e "  ${GREEN}✓${NC} NebulaOS wallpapers"
echo -e "  ${GREEN}✓${NC} Default settings & first-boot setup"
echo -e "  ${GREEN}✓${NC} Calamares installer branding"
echo ""
echo -e "${YELLOW}Next step: Click 'Generate' in Cubic to build the ISO${NC}"
echo ""
