# Arch Linux Install: Disk - concepts

Installing Arch Linux is a manual process, which is exactly why it suits a
typing course: every step is a command the installer would otherwise hide
behind a graphical wizard. By typing the commands, the student learns what an
installation actually does.

The live environment booted from the Rufus USB drive is a complete Linux
system running from memory. It has a root shell, the package manager, and the
install tools, but nothing is saved until the target disk is prepared.

## UEFI versus BIOS

Modern systems boot through UEFI. The boot mode decides which partition
layout and which bootloader commands the install uses. This course targets
UEFI, the default for systems from the last decade.

The check is simple: the directory /sys/firmware/efi exists when the system
booted in UEFI mode. Its presence or absence shapes everything that follows.

## The network first

The installer downloads the base system from Arch mirrors, so a working
network is the first requirement. Wired connections usually need nothing more
than bringing the interface up. Wireless uses the iwctl client, which is a
small interactive tool already present on the live image.

## Partitioning

A disk must be divided before it can hold a filesystem. This install uses
three partitions:

```text
EFI system partition: the bootloader and firmware files
swap: overflow and suspend storage
root: the operating system itself
```

fdisk is an interactive tool. Its prompts are typed, and the partition table
is only written when the w command is given. Before w, nothing is changed on
the disk.

## Filesystems

Each partition receives a filesystem:

```text
EFI: a FAT filesystem, required by the firmware
swap: swap space, not a filesystem in the usual sense
root: ext4, the default Arch root filesystem
```

The commands mkfs.fat, mkswap, and mkfs.ext4 are destructive: they overwrite
whatever was on the partition. The disk was chosen carefully in the previous
step for exactly this reason.

## Mounting

A filesystem only becomes usable when it is mounted. The root partition is
mounted at /mnt, and the installer's chroot later uses that mount point as
the new system's root. The EFI partition mounts at /mnt/boot so the firmware
files land on the FAT partition.

## The divide

The install is deliberately split into two lessons at this point. This lesson
ends with a mounted, formatted disk. The next lesson installs the packages
into that disk, configures it, and makes it bootable.
