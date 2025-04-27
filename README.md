# 🧩 czram
A lightweight utility for zram device management on Chimera Linux, inspired by [zramen](https://github.com/atweiden/zramen).

---
## 🔍 Overview
`czram` simplifies the management of compressed RAM-based swap devices (zram) on Chimera Linux. By leveraging zram's RAM compression capabilities, it provides significantly faster swap operations compared to traditional disk-based swap, enhancing system performance during high memory usage scenarios.

---
## ✨ Features
- **Effortless Creation** 🚀: Create zram devices with customizable size, compression algorithms, and priority.
- **Flexible Management** 🔄: Remove specific devices or all active instances with ease.
- **Visibility** 👁️: List and monitor active zram devices effortlessly.
- **Automation** ⚙️: Seamless integration with dinit for system startup/shutdown management.

---
## 🚀 Quick Start
### Installation
1. Clone the repository:
   ```bash
   git clone --depth=1 https://github.com/Gur0v/czram
   cd czram
   ```
2. Build the project:
   ```bash
   make
   ```
3. Install to your system:
   ```bash
   doas make install
   ```

---
## 🔌 System Integration
### dinit Service Setup
A ready-to-use dinit service is included in the repository:
1. Copy the service file:
   ```bash
   doas cp dinit.d/czram /etc/dinit.d/
   ```
2. Enable the service:
   ```bash
   doas dinitctl enable czram
   ```
The default service creates a 4G zram device with `zstd` compression. To customize, edit `/etc/dinit.d/czram` and modify the command parameters to suit your system's needs.

---
## 📚 Usage
### Creating a zram Device 💾
```bash
czram make [-s|--size SIZE] [-a|--algorithm ALGO] [-p|--priority PRIO]
```
#### Options:
- `-s`, `--size`: Device size (default: `4G`).
- `-a`, `--algorithm`: Compression algorithm (default: `zstd`).
- `-p`, `--priority`: Swap priority (default: `100`, max: `32767`).
#### Supported Compression Algorithms:
- `lzo`
- `lzo-rle`
- `lz4`
- `lz4hc`
- `zstd`
- `deflate`
- `842`
#### Example:
```bash
czram make -s 2G -a lz4 -p 500
```
---

### Removing zram Devices 🗑️
```bash
# Remove a specific device
czram toss /dev/zram0

# Remove all devices
czram toss --all
```

---
### Listing Active Devices 📊
```bash
czram list
```

---

## 📜 License

This project is licensed under the [BSD-3-Clause License](LICENSE).
