# Arch Linux Packages: BlackArch and Tools

This lesson installs the penetration testing toolchain. It adds the BlackArch
repository, installs the pentest tools the cybersecurity course uses, and
verifies the C++ toolchain with a real build.

The system is a working Arch desktop from the previous lesson.

## Install the AUR helper

Some tools are easiest to install through the AUR.

```bash
# install the AUR helper from the AUR
git clone https://aur.archlinux.org/paru.git
cd paru
makepkg -si
```

The helper compiles and installs from the AUR.

## Add the BlackArch repository

BlackArch provides attack tooling as Arch packages.

```bash
# download the bootstrap script
curl -O https://blackarch.org/strap.sh

# verify the script checksum against the published value
sha256sum strap.sh

# install the repository
chmod +x strap.sh
sudo ./strap.sh

# refresh the package databases
sudo pacman -Syy
```

The checksum must match before the script runs.

## Install the network tools

The scanning and enumeration tools.

```bash
# install the network tools
sudo pacman -S nmap masscan netcat-openbsd

# install the web testing tools
sudo pacman -S curl jq gobuster ffuf
```

The tools cover host discovery, scanning, and web fuzzing.

## Install the credential tools

The password and cracking tools.

```bash
# install the online and offline credential tools
sudo pacman -S hydra john hashcat

# install the wordlists
sudo pacman -S wordlists
```

The wordlists package provides the standard collections.

## Install the exploitation tools

The framework and the post-exploitation utilities.

```bash
# install the exploit framework
sudo pacman -S metasploit

# initialize its database
sudo msfdb init

# install the post-exploitation tools
sudo pacman -S impacket chisel
```

The framework handles payloads and sessions.

## Install the scanner tools

The vulnerability scanning tools.

```bash
# install the scanners
sudo pacman -S nuclei nikto

# update the nuclei templates
nuclei -update-templates
```

The template store must be current before scanning.

## Install the wireless tools

The wireless testing suite.

```bash
# install the wireless tools
sudo pacman -S aircrack-ng iw
```

The suite covers capture, injection, and cracking.

## Install the remaining utilities

The supporting tools used across the course.

```bash
# install the supporting utilities
sudo pacman -S proxychains-ng smbclient ripgrep fd

# install the metadata tool
sudo pacman -S exiftool
```

The utilities support pivoting, file work, and recon.

## Verify the C++ toolchain

The toolchain is proven with a real build.

```bash
# create a test project
mkdir -p ~/cpp-test
cd ~/cpp-test

# write the test program
cat > main.cpp <<'EOF'
#include <iostream>

int main() {
    std::cout << "toolchain ok\n";
    return 0;
}
EOF

# write the build definition
cat > CMakeLists.txt <<'EOF'
cmake_minimum_required(VERSION 3.20)
project(toolchain_test CXX)
add_executable(test main.cpp)
EOF

# configure with the ninja generator
cmake -B build -G Ninja

# build and run
ninja -C build
./build/test
```

The program prints toolchain ok.

## Verify the pentest tools

Every installed tool is confirmed present.

```bash
# verify the tool versions
nmap --version | head -n 1
hydra -h 2>&1 | head -n 1
nuclei -version

# verify the service tools
which msfconsole proxychains4
```

Each command prints a version or path.

## Wrap up

Package sequence: paru -> BlackArch -> network tools -> credential tools ->
exploitation tools -> scanners -> wireless -> utilities -> verify toolchain
-> verify tools. The system is ready for the cybersecurity course.
