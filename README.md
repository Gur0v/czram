# czram

A simple shell script to create or remove zram devices (on Chimera Linux)

## Overview

`czram` is a utility script that simplifies the creation and management of zram swap devices on Chimera Linux. ZRAM creates a compressed block device in RAM, which can be used as a swap device, providing faster swap operations compared to disk-based swap.

## Features

- Create zram devices with customizable size and compression algorithm
- Remove specific zram devices or all active devices
- Automatically formats and enables zram devices as swap
- Gracefully handles device cleanup and reset

## Installation

1. Clone this repository or download the script
2. Make the script executable:
   ```
   chmod +x czram
   ```
3. Copy the script to `/usr/bin/`:
   ```
   doas cp czram /usr/bin/
   ```

## Usage

### Creating a zram device

```
czram mk [--size <size>] [--algorithm <algorithm>]
```

Options:
- `--size`: Specify the size (default: 4G)
- `--algorithm`: Specify the compression algorithm (default: zstd)

Example:
```
czram mk --size 2G --algorithm lzo
```

### Removing zram devices

Remove a specific device:
```
czram rm /dev/zram0
```

Remove all zram devices:
```
czram rm --all
```

## dinit Service

A dinit service file is included in the repository at `etc/dinit.d/czram`. To install the service:

1. Copy the service file to the system's dinit directory:
   ```
   doas cp etc/dinit.d/czram /etc/dinit.d/
   ```

2. Enable the service to start at boot:
   ```
   doas dinitctl enable czram
   ```

By default, the service creates a 4G zram device with zstd compression. If you want to use different settings (size or compression algorithm), you'll need to modify the `/etc/dinit.d/czram` file and adjust the command parameters.

This will automatically create zram devices at system startup and remove them during shutdown.

## License

This project is licensed under the BSD-3-Clause License - see the LICENSE file for details.
