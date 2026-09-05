# Packaging

## Arch Linux / AUR

The `aur/` directory contains the files needed to publish `codex-menubar-kde` to the Arch User Repository.

The package is tied to release tags (`v<version>`). Before publishing a new version:

1. Update the project version in `CMakeLists.txt`.
2. Update `package/metadata.json`.
3. Update `aur/PKGBUILD` and regenerate `aur/.SRCINFO` with `makepkg --printsrcinfo > .SRCINFO`.
4. Push and verify CI.
5. Create and push the matching Git tag, for example `v0.2.0`.

To test the package locally after the matching tag exists:

```bash
cd packaging/aur
makepkg -si
```

Runtime dependencies are intentionally limited to Plasma and Qt. Notifications use the standard freedesktop notification D-Bus interface, so `libnotify`/`notify-send` is not required.

### First-time AUR setup

AUR write access uses SSH. Create a dedicated key:

```bash
ssh-keygen -t ed25519 -f ~/.ssh/aur -C "codex-menubar-kde AUR"
```

Add the contents of `~/.ssh/aur.pub` to the SSH public keys in your AUR account, then test access:

```bash
ssh -i ~/.ssh/aur aur@aur.archlinux.org help
```

Store the private key as the `AUR_SSH_PRIVATE_KEY` GitHub Actions secret for this repository:

```bash
gh secret set AUR_SSH_PRIVATE_KEY < ~/.ssh/aur
```

Do not commit the private key to Git.

### Automated publishing

`.github/workflows/aur.yml` runs on `v*` tag pushes and can also be started manually. It verifies that CMake, Plasma metadata, `PKGBUILD`, and `.SRCINFO` all use the same version, then pushes `PKGBUILD` and `.SRCINFO` to:

```text
ssh://aur@aur.archlinux.org/codex-menubar-kde.git
```

The AUR only accepts package commits on its `master` branch; the workflow handles this automatically.

For the first publication, the AUR repository may be empty. That is expected. Once the first push succeeds, users can install the package with an AUR helper such as:

```bash
yay -S codex-menubar-kde
```

or:

```bash
paru -S codex-menubar-kde
```

Without an AUR helper:

```bash
git clone https://aur.archlinux.org/codex-menubar-kde.git
cd codex-menubar-kde
makepkg -si
```

## GitHub releases

`.github/workflows/release.yml` runs for `v*` tags. It verifies that the tag matches CMake, Plasma metadata, and the AUR package version, builds and tests on Arch Linux, stages the installation tree, creates an `arch-x86_64.tar.zst`, generates SHA-256 checksums, and publishes a GitHub release.

A tag therefore starts two independent workflows:

- `Release` publishes the GitHub release and release assets.
- `Publish AUR` publishes the matching AUR package metadata.

The AUR package is the preferred installation method on Arch-based distributions because upgrades can be tracked by AUR helpers and the resulting package is managed by pacman.
