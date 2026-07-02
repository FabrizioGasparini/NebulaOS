#!/bin/bash
set -e -u

# Set timezone
ln -sf /usr/share/zoneinfo/UTC /etc/localtime

# Set root shell
usermod -s /usr/bin/bash root

# Enable services
systemctl enable systemd-resolved
systemctl enable systemd-networkd

# Clean up
rm -f /root/customize_airootfs.sh
