# czram
Lightweight zram manager for [Chimera Linux](https://chimera-linux.org), inspired by [zramen](https://github.com/atweiden/zramen).

## Overview
`czram` manages compressed RAM-based swap devices using the Linux kernel zram module.  
Provides faster swap performance than disk with efficient memory compression.
Written in C with zero dependencies beyond core system utilities.

## Features
- Simple configuration via `/etc/default/czram`
- Fast provisioning of zram swap devices with automatic sizing
- Device management: create and remove zram devices as needed
- Status reporting of active zram devices
- Dinit integration for system startup

## Configuration
Configure via `/etc/default/czram`:
```
# Size: absolute (2G, 512M) or RAM percentage (25%RAM)
SIZE=25%RAM
# Compression algorithm
ALGO=zstd
# Swap priority (higher values preferred)
PRIO=32767
```

## Installation
```
git clone https://github.com/Gur0v/czram
cd czram
doas make clean install
```

Installs:
- `/usr/bin/czram` (binary)
- `/etc/default/czram` (config)
- `/etc/dinit.d/czram` (service)

## Dinit Integration
Enable zram on boot:
```
doas dinitctl enable czram
```

## Usage
Create zram device:
```
czram make
```

Remove zram devices:
```
czram toss /dev/zram0    # Remove specific device
czram toss --all         # Remove all devices
```

List active devices:
```
czram list
```

## License
[BSD-3-Clause License](LICENSE)
