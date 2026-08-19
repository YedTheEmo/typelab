# Arch Linux Install: Disk

This lesson installs Arch Linux from the live environment booted from a USB
drive written with Rufus. It covers connecting to the network, partitioning
the disk, and mounting the new filesystem.

This is the first of four setup lessons. The rest of this course assumes a
working Arch system, so every command here is typed on the live environment
that the USB booted.

## Boot from the USB

The USB drive was written with Rufus in DD or ISO image mode.

```text
insert the USB drive
enter the boot menu at power on
select the USB drive
choose the first entry of the Arch installer
```

The installer boots into a root shell on a live system.

## Verify the boot mode

The install path depends on UEFI or BIOS.

```bash
# confirm the system booted in UEFI mode
ls /sys/firmware/efi/efivars
```

The directory exists when the system booted through UEFI.

## Check the network

The installer needs a connection to the package mirrors.

```bash
# show the network interfaces
ip link

# bring up a wired interface, if present
ip link set enp0s3 up

# test the connection
ping -c 3 archlinux.org
```

A working connection is required before continuing.

## Connect to wireless

Wireless uses the interactive iwctl client.

```bash
# start the wireless daemon
iwctl

# inside iwctl: list the devices
device list

# inside iwctl: scan for networks
station wlan0 scan
station wlan0 get-networks

# inside iwctl: connect to the network
station wlan0 connect YourNetworkName
```

Enter the passphrase when prompted, then exit iwctl.

```bash
# exit the wireless client
exit

# confirm the connection
ping -c 3 archlinux.org
```

Wireless is ready when the ping succeeds.

## Verify the clock

The installer date affects signatures and timestamps.

```bash
# confirm the system clock
timedatectl

# sync the clock over the network
timedatectl set-ntp true
```

The network time protocol keeps the clock accurate.

## Identify the target disk

The wrong disk is an unrecoverable mistake.

```bash
# list the block devices
lsblk

# show the disks with their sizes
lsblk -o NAME,SIZE,TYPE,MODEL
```

The USB drive is the installer media and is never a target.

## Partition the disk

The disk is partitioned with fdisk.

```bash
# open fdisk on the target disk
fdisk /dev/sda
```

Inside fdisk, create the partitions in order.

```bash
# clear the partition table
g

# create the EFI system partition
n
1
[enter]
+512M
t
1

# create the swap partition
n
2
[enter]
+4G

# create the root partition
n
3
[enter]
[enter]

# write the table and exit
w
```

The root partition fills the remaining space.

## Format the partitions

Each partition gets its filesystem.

```bash
# format the EFI partition
mkfs.fat -F 32 /dev/sda1

# format the swap partition
mkswap /dev/sda2

# format the root partition
mkfs.ext4 /dev/sda3
```

The filesystems are ready for mounting.

## Mount the new system

The root partition becomes the new system's root.

```bash
# mount the root partition
mount /dev/sda3 /mnt

# enable swap
swapon /dev/sda2

# create the EFI mount point
mount --mkdir /dev/sda1 /mnt/boot
```

Every partition is mounted under /mnt.

## Verify the mounts

Confirm the layout before installing packages.

```bash
# show the mounted filesystems
findmnt

# show the disk usage
lsblk
```

The mounted layout is correct when all partitions appear.

## Wrap up

Disk sequence: boot -> network -> clock -> partition -> format -> mount.
The next lesson installs the system packages into /mnt.
