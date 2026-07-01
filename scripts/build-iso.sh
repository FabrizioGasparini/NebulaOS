#!/bin/bash
# NebulaOS ISO Build Script
# Run this inside Cubic's chroot environment

set -e

echo "🌌 Building NebulaOS ISO..."
echo "================================"

# Update system
echo "[1/12] Updating system packages..."
apt update && apt upgrade -y

# Install core desktop
echo "[2/12] Installing GNOME desktop..."
apt install -y gnome gnome-shell ubuntu-desktop gdm3

# Install Calamares
echo "[3/12] Installing Calamares installer..."
apt install -y calamares

# Install GNOME extension support
echo "[4/12] Installing GNOME extension support..."
apt install -y gnome-shell-extensions gnome-shell-extension-manager chrome-gnome-shell

# Install theming tools
echo "[5/12] Installing theming tools..."
apt install -y gnome-tweaks sassc

# Install applications
echo "[6/12] Installing default applications..."
apt install -y \
    nautilus \
    gnome-terminal \
    gnome-text-editor \
    firefox \
    gnome-calculator \
    gnome-screenshot \
    eog \
    gnome-software \
    file-roller \
    vlc \
    libreoffice-common

# Remove unwanted packages
echo "[7/12] Removing unwanted packages..."
apt purge -y thunderbird* libreoffice-* rhythmbox totem

# Install themes
echo "[8/12] Installing NebulaOS themes..."
# Copy theme files (these should be placed in the chroot before running)
# cp -r /tmp/nebula-theme/* /usr/share/themes/Nebula/
# cp -r /tmp/nebula-icons/* /usr/share/icons/Tela-dark/

# Install extensions
echo "[9/12] Installing GNOME extensions..."
# Clone and install Dash to Dock
# git clone https://github.com/micheleg/dash-to-dock.git /tmp/dash-to-dock
# cp -r /tmp/dash-to-dock/dash-to-dock@micxgx.gmail.com /usr/share/gnome-shell/extensions/

# Clone and install Blur My Shell  
# git clone https://github.com/aunetx/blur-my-shell.git /tmp/blur-my-shell
# cp -r /tmp/blur-my-shell/blur-my-shell@aunetx /usr/share/gnome-shell/extensions/

# Apply default settings
echo "[10/12] Applying default settings..."
# cp /tmp/nebulaos-defaults.dconf /usr/share/nebulaos/
# dconf load / < /usr/share/nebulaos/nebulaos-defaults.dconf

# Set up first-boot script
echo "[11/12] Setting up first-boot script..."
# mkdir -p /usr/share/nebulaos
# cp /tmp/nebula-first-boot.sh /usr/share/nebulaos/
# chmod +x /usr/share/nebulaos/nebula-first-boot.sh
# cp /tmp/nebula-first-boot.desktop /etc/xdg/autostart/

# Configure Calamares
echo "[12/12] Configuring Calamares installer..."
# mkdir -p /etc/calamares/branding/nebulaos
# cp /tmp/nebulaos-branding/* /etc/calamares/branding/nebulaos/

# Clean up
echo "Cleaning up..."
apt autoremove -y
apt clean
rm -rf /tmp/*

echo "================================"
echo "✅ NebulaOS build preparation complete!"
echo "🌌 Ready to generate ISO"
