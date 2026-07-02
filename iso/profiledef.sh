#!/usr/bin/env bash
# shellcheck disable=SC2034

iso_name="nebulaos"
iso_label="NEBULAOS_$(date --date="@${SOURCE_DATE_EPOCH:-$(date +%s)}" +%Y%m)"
iso_publisher="NebulaOS Project"
iso_application="NebulaOS"
iso_version="$(date --date="@${SOURCE_DATE_EPOCH:-$(date +%s)}" +%Y.%m.%d)"
install_dir="arch"
buildmodes=('iso')

bootmodes=(
    'bios.syslinux.mbr'
    'bios.syslinux.eltorito'
    'uefi.systemd-boot'
)

arch="x86_64"
pacman_conf="pacman.conf"
airootfs_image_type="squashfs"

file_permissions=(
    ["/etc/shadow"]="0:0:0400"
    ["/etc/gshadow"]="0:0:0400"
    ["/root"]="0:0:0750"
)
