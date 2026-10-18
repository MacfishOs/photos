# Aevis Photos

A sleek, modern photo viewer and media organizer for the Aevis Desktop Environment.

Part of the **[Project Aevis](https://github.com/project-Aevis)** suite.

## Features

- Fast image rendering with zoom and pan
- Album organization and favorite marking
- EXIF metadata viewer
- Fullscreen slideshow mode
- FishUI light and dark theme integration

## Dependencies

### Debian / Ubuntu

```shell
sudo apt install cmake gcc qt6-base-dev qt6-declarative-dev
```

### Arch Linux

```shell
sudo pacman -S cmake gcc qt6-base qt6-declarative
```

## Build and Install

```shell
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr ..
make
sudo make install
```

## License

This project is licensed under the GNU General Public License v3.0 (GPLv3).
