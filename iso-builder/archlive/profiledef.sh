#!/usr/bin/env bash

iso_name="nebulaos"
iso_label="NEBULAOS_$(date --date="@${SOURCE_DATE_EPOCH:-$(date +%s)}" +%Y%m)"
iso_publisher="NebulaOS Project"
iso_application="NebulaOS - Liquid Glass Experience"
iso_version="$(date --date="@${SOURCE_DATE_EPOCH:-$(date +%s)}" +%Y.%m.%d)"
install_dir="arch"
buildmodes=('iso')
bootmodes=(
  'bios.syslinux'
  'uefi.grub'
)
arch="x86_64"
pacman_conf="pacman.conf"
airootfs_image_type="squashfs"
airootfs_image_tool_options=('-comp' 'xz' '-Xbcj' 'x86' '-Xdict-size' '1M' '-b' '1M')

file_permissions=(
  ["/root"]="0:0:0750"
)
