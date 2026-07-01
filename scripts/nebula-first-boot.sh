#!/bin/bash
# NebulaOS First Boot Setup Script
# Applies all default settings and configurations

set -e

NEBULA_USER="${SUDO_USER:-$USER}"
NEBULA_HOME=$(eval echo "~$NEBULA_USER")

echo "🌌 Setting up NebulaOS..."

# Wait for desktop to be ready
sleep 3

# Apply dconf defaults
echo "Applying default settings..."
if [ -f /usr/share/nebulaos/default-settings.dconf ]; then
    dconf load / < /usr/share/nebulaos/default-settings.dconf
fi

# Set wallpaper
echo "Setting wallpaper..."
gsettings set org.gnome.desktop.background picture-uri "file:///usr/share/backgrounds/nebula-default.png"
gsettings set org.gnome.desktop.background picture-uri-dark "file:///usr/share/backgrounds/nebula-default.png"
gsettings set org.gnome.desktop.background picture-options "zoom"
gsettings set org.gnome/desktop/background primary-color "#0f0a1a"

# Set theme
echo "Applying NebulaOS theme..."
gsettings set org.gnome.desktop.interface gtk-theme "Nebula"
gsettings set org.gnome.desktop.interface icon-theme "Tela-dark"
gsettings set org.gnome.desktop.interface cursor-theme "Bibata-Modern-Ice"
gsettings set org.gnome.desktop.interface color-scheme "prefer-dark"
gsettings set org.gnome/desktop/wm.preferences theme "Nebula"

# Enable extensions
echo "Enabling GNOME extensions..."
gsettings set org.gnome.shell enabled-extensions "['dash-to-dock@micxgx.gmail.com', 'blur-my-shell@aunetx', 'appindicatorsupport@rgcjonas.gmail.com', 'user-theme@gnome-shell-extensions.gnome.org', 'just-perfectiondesktopteam@gmail.com']"

# Configure Dash to Dock
echo "Configuring dock..."
gsettings set org.gnome.shell.extensions.dash-to-dock dock-position "BOTTOM"
gsettings set org.gnome.shell.extensions.dash-to-dock transparency-mode "DYNAMIC"
gsettings set org.gnome.shell.extensions.dash-to-dock background-opacity 0.65
gsettings set org.gnome.shell.extensions.dash-to-dock dash-max-icon-size 52
gsettings set org.gnome.shell.extensions.dash-to-dock dock-fixed true
gsettings set org.gnome.shell.extensions.dash-to-dock custom-theme-shrink true
gsettings set org.gnome.shell.extensions.dash-to-dock custom-theme-customize-running-dots true
gsettings set org.gnome.shell.extensions.dash-to-dock custom-theme-running-dots-color "#7c3aed"
gsettings set org.gnome.shell.extensions.dash-to-dock show-favorites true
gsettings set org.gnome.shell.extensions.dash-to-dock show-running true
gsettings set org.gnome.shell.extensions.dash-to-dock autohide false

# Configure Blur My Shell
echo "Configuring blur effects..."
gsettings set org.gnome.shell.extensions.blur-my-shell sigma 30
gsettings set org.gnome.shell.extensions.blur-my-shell brightness 0.6

# Configure terminal profile
echo "Setting up terminal..."
TERMINAL_PROFILE=$(gsettings get org.gnome.terminal.legacy profiles: | tr -d "[]'," | head -c -1)
if [ -z "$TERMINAL_PROFILE" ] || [ "$TERMINAL_PROFILE" = "''" ]; then
    TERMINAL_PROFILE=$(uuidgen)
    gsettings set org.gnome.terminal.legacy profiles: "['$TERMINAL_PROFILE']"
fi
gsettings set org.gnome.terminal.legacy/profiles:/:$TERMINAL_PROFILE visible-name "NebulaOS"
gsettings set org.gnome.terminal.legacy/profiles:/:$TERMINAL_PROFILE use-theme-colors false
gsettings set org.gnome.terminal.legacy/profiles:/:$TERMINAL_PROFILE foreground-color "#e2d9f3"
gsettings set org.gnome.terminal.legacy/profiles:/:$TERMINAL_PROFILE background-color "#0f0a1a"
gsettings set org.gnome.terminal.legacy/profiles:/:$TERMINAL_PROFILE bold-color "#c4b5fd"
gsettings set org.gnome.terminal.legacy/profiles:/:$TERMINAL_PROFILE cursor-colors-set true
gsettings set org.gnome.terminal.legacy/profiles:/:$TERMINAL_PROFILE cursor-foreground-color "#e2d9f3"
gsettings set org.gnome.terminal.legacy/profiles:/:$TERMINAL_PROFILE cursor-background-color "#7c3aed"
gsettings set org.gnome.terminal.legacy/profiles:/:$TERMINAL_PROFILE font "Ubuntu Mono 13"
gsettings set org.gnome.terminal.legacy/profiles:/:$TERMINAL_PROFILE use-system-font false

# Restart GNOME Shell to apply theme changes
echo "Restarting GNOME Shell..."
if [ "$XDG_SESSION_TYPE" = "x11" ]; then
    dbus-send --type=method_call --dest=org.gnome.Shell /org/gnome/Shell org.gnome.Shell.Eval string:'Meta.restart("Restarting GNOME Shell...")' 2>/dev/null || true
fi

echo "✅ NebulaOS setup complete!"
echo "🌌 Welcome to NebulaOS - A cosmic desktop experience"

# Remove this script from autostart after first run
rm -f /etc/xdg/autostart/nebula-first-boot.desktop 2>/dev/null || true

exit 0
