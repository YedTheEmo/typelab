# Arch Linux Desktop: Wayland

This lesson turns the bare Arch system into a usable desktop: a regular user,
network management, graphics drivers, a Wayland compositor, and audio.

The system boots to a root login prompt from the previous lesson. Log in as
root to begin.

## Create the user

Daily work happens as a regular user, not root.

```bash
# create the user account
useradd -m -G wheel user

# set the user password
passwd user

# add sudo access for the wheel group
EDITOR=vim visudo
```

Uncomment the wheel group line in visudo, then save.

## Verify the sudo access

The user must be able to run administrative commands.

```bash
# switch to the new user
su - user

# confirm sudo works
sudo whoami
```

The command prints root when sudo is configured correctly.

## Enable the network service

NetworkManager manages wired and wireless connections.

```bash
# exit to root, then enable the service
exit
systemctl enable --now NetworkManager

# confirm the service is active
systemctl status NetworkManager
```

The network comes up automatically from now on.

## Install the graphics stack

The GPU drivers and the Wayland foundation.

```bash
# install the graphics and mesa stack
pacman -S mesa xf86-video-amdgpu libva-mesa-driver mesa-vdpau

# install the Wayland foundation libraries
pacman -S wayland wayland-utils wl-clipboard
```

Adjust the video driver to match the hardware.

## Install a Wayland compositor

The compositor draws the desktop.

```bash
# install Hyprland and its helpers
pacman -S hyprland hyprpaper waybar

# install the terminal and the display manager
pacman -S kitty sddm
```

Hyprland is the Wayland compositor for this course.

## Configure the display manager

SDDM presents the login screen on Wayland.

```bash
# enable the display manager at boot
systemctl enable sddm

# set the display manager to use Wayland
vim /etc/sddm.conf.d/10-wayland.conf
```

Write the session type into the file:

```text
[General]
DisplayServer=wayland
GreeterEnvironment=QT_QUICK_BACKEND=software
```

The greeter is ready at the next boot.

## Install audio

PipeWire provides sound and audio routing.

```bash
# install the audio server and the helper
pacman -S pipewire pipewire-pulse wireplumber

# enable the audio services
systemctl --user enable pipewire pipewire-pulse wireplumber
```

Audio starts with the user session.

## Install fonts and utilities

The desktop needs fonts, an editor, and basic tools.

```bash
# install fonts
pacman -S noto-fonts noto-fonts-cjk ttf-jetbrains-mono

# install the working utilities
pacman -S firefox ripgrep fd unzip
```

The system is now usable for real work.

## Install the C++ toolchain

The compiler and build tools for the C++ work.

```bash
# install the core toolchain
pacman -S gcc gdb make

# install the build systems and the graphics libraries
pacman -S cmake ninja clang vulkan-devel

# verify the compiler
g++ --version
```

The toolchain is verified before the desktop is tested.

## Reboot into the desktop

The display manager greeter appears after the reboot.

```bash
# restart the system
reboot
```

Log in as the user at the SDDM greeter.

## Verify the desktop

Confirm the compositor and the toolchain work.

```bash
# check the session type
echo $XDG_SESSION_TYPE

# confirm the display server is Wayland
wayland-info | grep -m1 'name ='

# confirm the terminal runs
kitty --version
```

The session type prints wayland.

## Wrap up

Desktop sequence: user -> network -> graphics -> compositor -> display manager
-> audio -> fonts -> toolchain -> reboot -> verify.
