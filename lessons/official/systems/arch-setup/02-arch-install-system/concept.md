# Arch Linux Install: System - concepts

The mounted disk from the previous lesson is empty. This lesson fills it:
the base packages are downloaded and extracted, the system configuration is
written, and a bootloader makes the result bootable.

## pacstrap and the chroot

pacstrap installs packages into a directory instead of the running system. It
builds the skeleton of the new install at /mnt while the live environment
keeps running.

The chroot then changes the apparent root directory to /mnt. Commands typed
inside arch-chroot run against the installed system's files: its /etc, its
/bin, and its package manager. Configuration written here belongs to the new
system, not the live environment.

## The base packages

The base install is deliberately minimal.

```text
base: the core system
linux: the kernel
linux-firmware: hardware drivers
base-devel: the build toolchain
networkmanager: network management
sudo, vim, git: the working essentials
```

Everything else in this course is installed on demand, which is the Arch
philosophy: a system is the sum of the packages the owner chose.

## The fstab

The filesystem table tells the kernel which partitions to mount at boot and
where. genfstab reads the current mount layout and writes the matching table.
It is generated from the running system, which is why it is done immediately
after mounting.

## Configuration files

Inside the chroot, the system configuration is written by hand:

```text
/etc/localtime: the time zone
/etc/locale.gen and /etc/locale.conf: the language
/etc/hostname and /etc/hosts: the network identity
root password: stored in /etc/shadow
```

Each file is small and human-readable. Typing them directly teaches what a
graphical installer would write invisibly.

## The bootloader

A UEFI system needs software that the firmware can launch. GRUB is installed
to the EFI system partition, and grub-mkconfig generates the menu that
appears at boot.

The bootloader is the boundary between the firmware and the operating system.
Without it, the disk is full of files but nothing boots.

## Reboot and verify

The reboot is the first real test of the install. Removing the USB drive
forces the machine to boot from the new disk. A successful root login proves
the bootloader, the kernel, and the base system are all working together.
