# Arch Linux Desktop: Wayland - concepts

A bare Arch system boots to a text login. This lesson turns it into a
graphical desktop without sacrificing the Arch philosophy: the pieces are
chosen and installed one at a time, and the student knows what each piece is
for.

## The user account

Root is for administration. Daily use happens as a regular user, granted
administrative power only through sudo. The wheel group is the standard
sudo-allowed group, and visudo edits the sudoers file with syntax checking so
a mistake cannot lock the system out of administration.

## NetworkManager

The installer's network setup was temporary. NetworkManager is the persistent
service that manages wired and wireless connections after boot. Enabling it
with systemctl --now starts it immediately and registers it to start at boot.

## The graphics stack

Wayland needs two layers working together:

```text
the GPU driver, which talks to the hardware
mesa, which implements the graphics API the desktop uses
```

The exact video driver depends on the GPU vendor: amdgpu for AMD, the nvidia
driver for NVIDIA, and the built-in modesetting for most others. This course
assumes AMD hardware, which works cleanly on Wayland.

## The compositor

On Wayland, the compositor is the display server. There is no separate X
server: the compositor draws windows, handles input, and manages the screen
directly. Hyprland is a modern tiling compositor that works well for a typing
and development workflow.

## The display manager

The display manager provides the graphical login screen and starts the user
session. SDDM runs at boot, presents the greeter, and launches the chosen
session. The configuration file forces the greeter to run on Wayland.

## PipeWire

PipeWire is the modern Linux audio server. The pipewire-pulse compatibility
layer lets PulseAudio-only applications use it without change, and
wireplumber manages its device routing. The services run per-user, which is
why they are enabled with systemctl --user.

## The toolchain

The C++ toolchain is installed in this lesson because the desktop session
verifies it. gcc and gdb are the compiler and debugger, cmake and ninja are
the build system, and clang is the alternative compiler. The vulkan-devel
package provides the headers the graphics course and shader course need.

## The verification

The desktop is only finished when it proves itself: the session type reports
wayland, the compositor reports its protocol, and the toolchain reports its
version. Each verification is the evidence that the setup worked.
