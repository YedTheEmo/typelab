# Arch Linux Packages: BlackArch and Tools - concepts

The desktop is set up, but the course work needs tools. This lesson installs
the two tool families the course relies on: the penetration testing stack and
the verified C++ toolchain.

## The AUR helper

The Arch User Repository holds packages that are not in the official
repositories. paru is an AUR helper: it downloads a package's build files,
runs the build, and installs the result. Building from the AUR is the normal
way to get software on Arch, and a helper makes it routine.

## The BlackArch repository

BlackArch is a repository of security tools packaged for Arch. The strap
script adds its package database to the system, after which security tools
install with pacman like any other package.

The checksum check matters. The script configures a package source that runs
with root privileges during installs. Installing a tampered script would
compromise the system, so the published checksum is verified before execution.

## Why these tools

The package list matches the penetration testing course exactly:

```text
network tools: scanning and enumeration
credential tools: online and offline password attacks
exploitation tools: payloads, sessions, post-exploitation
scanners: template-based vulnerability checking
wireless tools: monitor mode and handshake attacks
supporting utilities: pivoting, file work, recon
```

The tools are installed as a batch here so the course never pauses to
install a missing dependency. Each one is verified after installation.

## The toolchain build

The C++ toolchain was installed with the desktop, but a toolchain is only
proven by compiling. The verification builds a small program with the
project's own build system: CMake configures the build from a CMakeLists
file, Ninja executes it, and the resulting binary runs. A program that
compiles and prints its output proves the compiler, the linker, and the
build system all work together.

## The end state

The system is now a complete Arch installation:

```text
a working Wayland desktop
a buildable C++ toolchain
the full penetration testing tool stack
```

Everything the course types from here on runs on a machine that was itself
installed by typing. The install is finished, and the work can begin.
