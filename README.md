# 🧩 czram

A lightweight utility for zram device management on Chimera Linux inspired by [zramen](<https://github.com/atweiden/zramen>)

## 🔍 Overview

`czram` provides an elegant and efficient solution for managing compressed RAM-based swap devices (zram) on Chimera Linux. By utilizing RAM compression, zram offers significantly faster swap operations compared to traditional disk-based swap, improving system performance during high memory usage scenarios.

## ✨ Features

- **Effortless Creation** 🚀: Generate zram devices with customizable size and compression algorithms
- **Simple Management** 🔄: Remove specific devices or all active instances with a single command
- **Visibility** 👁️: List and monitor active zram devices
- **Automation** ⚙️: Seamless integration with dinit for system startup/shutdown management

### Quick Start

```bash
# Clone the repository
git clone --depth=1 https://github.com/Gur0v/czram
cd czram

# Build the project
make

# Install to system
doas make install
```

## 📚 Usage

### Creating a zram Device 💾

```
czram make [-s|--size SIZE] [-a|--algorithm ALGO]
```

Options:
- `-s`, `--size`: Device size (default: 4G)
- `-a`, `--algorithm`: Compression algorithm (default: zstd)

Example:
```
czram make -s 2G -a lzo
```

### Removing zram Devices 🗑️

```
# Remove specific device
czram toss /dev/zram0

# Remove all devices
czram toss --all
```

### Listing Active Devices 📊

```
czram list
```

## 🔌 System Integration

### dinit Service Setup

A ready-to-use dinit service is included in the repository:

1. Copy the service file:
   ```
   doas cp etc/dinit.d/czram /etc/dinit.d/
   ```

2. Enable the service:
   ```
   doas dinitctl enable czram
   ```

The default service creates a 4G zram device with zstd compression. To customize, edit `/etc/dinit.d/czram` and modify the command parameters to suit your system's needs.

## 📜 License

This project is licensed under the BSD-3-Clause License.
