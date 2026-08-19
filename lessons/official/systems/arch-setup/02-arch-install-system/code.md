# Arch Linux Install: System

This lesson installs the base system into the mounted disk, configures the
system inside a chroot, installs the bootloader, and reboots into the new
install.

The disk from the previous lesson is mounted at /mnt. Every command in this
lesson builds the system that lives there.

## Install the base packages

The base system is installed with pacstrap.

```bash
# install the base system into /mnt
pacstrap -K /mnt base linux linux-firmware

# install the basic utilities used by the rest of the course
pacstrap -K /mnt base-devel networkmanager sudo vim git
```

The packages download from the mirrors and extract into /mnt.

## Generate the fstab

The mount table is generated from the current layout.

```bash
# generate the fstab entries
genfstab -U /mnt >> /mnt/etc/fstab

# verify the generated table
cat /mnt/etc/fstab
```

The fstab remounts every partition at boot.

## Enter the chroot

The new system becomes the working environment.

```bash
# enter the installed system
arch-chroot /mnt
```

The chroot makes the installed root the current root.

## Set the time zone

The system needs a time zone and clock.

```bash
# link the time zone
ln -sf /usr/share/zoneinfo/Region/City /etc/localtime

# generate the hardware clock setting
hwclock --systohc
```

Replace Region/City with the actual location.

## Configure the locale

The system language is generated from the locale files.

```bash
# uncomment the desired locale in the file
vim /etc/locale.gen

# generate the locales
locale-gen

# set the system language
echo "LANG=en_US.UTF-8" > /etc/locale.conf
```

The locale is active on the next login.

## Set the hostname

The system needs a name on the network.

```bash
# write the hostname
echo "archbox" > /etc/hostname

# map the hostname in the hosts file
echo "127.0.1.1 archbox.localdomain archbox" >> /etc/hosts
```

The hostname identifies the system on the network.

## Set the root password

Root access is protected with a password.

```bash
# set the root password
passwd
```

Enter a password twice when prompted.

## Install the bootloader

The system is not bootable until a bootloader is installed.

```bash
# install the GRUB package and the EFI tool
pacman -S grub efibootmgr

# install GRUB to the EFI system partition
grub-install --target=x86_64-efi --efi-directory=/boot --bootloader-id=GRUB

# generate the GRUB configuration
grub-mkconfig -o /boot/grub/grub.cfg
```

GRUB writes its files to the mounted EFI partition.

## Exit and unmount

Leave the chroot and finish the disk work.

```bash
# exit the chroot
exit

# unmount the new system
umount -R /mnt

# disable swap
swapoff /dev/sda2
```

The disk is now ready for the reboot.

## Reboot

Boot into the new system.

```bash
# restart the machine
reboot
```

Remove the USB drive when the machine shuts down.

## Verify the boot

The new system should boot to a login prompt.

```bash
# the login prompt appears as root
login: root

# confirm the system identity
hostname

# confirm the disk mounted correctly
findmnt /
```

A root login at the prompt confirms the install.

## Wrap up

System sequence: pacstrap -> fstab -> chroot -> clock -> locale -> hostname
-> password -> GRUB -> reboot. The next lesson sets up the user and desktop.
