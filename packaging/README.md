# Packaging

## Release overview

Release tags are the source of truth. A tag such as `v0.2.0` must match the version in:

- `CMakeLists.txt`
- `package/metadata.json`
- `aur/PKGBUILD`
- `aur/.SRCINFO`

The GitHub release and AUR publication are intentionally separate. This allows a version to be released on GitHub first and published to the AUR later without creating a new tag.

## GitHub releases

`.github/workflows/release.yml` runs for `v*` tags. It:

1. verifies version consistency;
2. builds and tests on Arch Linux;
3. smoke-tests the installed Plasma layout;
4. builds a native Arch package with the release `PKGBUILD`;
5. generates `SHA256SUMS`;
6. publishes the package and checksum as GitHub release assets.

The resulting package is a normal pacman package, for example:

```text
codex-menubar-kde-0.2.0-1-x86_64.pkg.tar.zst
```

It can be installed directly with:

```bash
sudo pacman -U ./codex-menubar-kde-0.2.0-1-x86_64.pkg.tar.zst
```

To create a release after CI on `main` is green:

```bash
git tag -a v0.2.0 -m "Codex MenuBar KDE v0.2.0"
git push origin v0.2.0
```

## Arch Linux / AUR

The `aur/` directory contains the files needed to publish `codex-menubar-kde` to the Arch User Repository.

Before publishing a new version:

1. Update the project version in `CMakeLists.txt`.
2. Update `package/metadata.json`.
3. Update `aur/PKGBUILD`.
4. Regenerate `aur/.SRCINFO` with `makepkg --printsrcinfo > .SRCINFO`.
5. Push and verify CI.
6. Create and push the matching Git tag.
7. Verify the GitHub release succeeded.

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

### Publish to AUR

`.github/workflows/aur.yml` is started manually. Supply the already released tag to publish, for example:

```text
v0.2.0
```

The workflow checks out that exact tag, verifies version consistency, and pushes `PKGBUILD` and `.SRCINFO` to:

```text
ssh://aur@aur.archlinux.org/codex-menubar-kde.git
```

Keeping AUR publication manual means a temporary AUR account or service issue cannot make an otherwise valid GitHub release fail.

The AUR only accepts package commits on its `master` branch; the workflow handles this automatically.

Once the first AUR push succeeds, users can install the package with:

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
