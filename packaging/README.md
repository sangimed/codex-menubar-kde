# Packaging

## Arch Linux / AUR

The `aur/` directory contains the files needed to publish `codex-menubar-kde` to the Arch User Repository.

The package is intentionally tied to release tags (`v<version>`). Before publishing a new AUR revision:

1. Update the project version in `CMakeLists.txt`.
2. Update `package/metadata.json`.
3. Update `aur/PKGBUILD` and regenerate `aur/.SRCINFO` with `makepkg --printsrcinfo > .SRCINFO`.
4. Push and verify CI.
5. Create the matching Git tag, for example `v0.2.0`.
6. Copy `PKGBUILD` and `.SRCINFO` to the AUR package repository and push them there.

To test the package locally after the matching tag exists:

```bash
cd packaging/aur
makepkg -si
```

Runtime dependencies are intentionally limited to Plasma and Qt. Notifications use the standard freedesktop notification D-Bus interface, so `libnotify`/`notify-send` is not required.

## GitHub releases

`.github/workflows/release.yml` runs for `v*` tags. It verifies that the tag matches both CMake and Plasma metadata, builds and tests on Arch Linux, stages the installation tree, creates an `arch-x86_64.tar.zst`, generates SHA-256 checksums, and publishes a GitHub release.

The AUR package remains the preferred installation method on Arch-based distributions because it is tracked by the package manager.
