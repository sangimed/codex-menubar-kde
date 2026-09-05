<p align="center">
  <img src="package/contents/images/codex-menubar-kde.svg" alt="Codex MenuBar KDE logo" width="180" />
</p>

<h1 align="center">Codex MenuBar KDE</h1>

<p align="center">
  A native KDE Plasma 6 widget for monitoring OpenAI Codex usage limits directly from the panel.
</p>

## Features

- Native Plasma 6 panel widget
- Rolling 5-hour and weekly Codex quotas
- Remaining or used percentage mode
- Reset countdowns
- Optional credit balance in the panel
- Additional Codex limit buckets when reported
- 7-day local usage history
- Low-quota desktop notifications with configurable threshold
- Manual refresh
- Persistent `codex app-server --stdio` session
- Live `account/rateLimits/updated` events
- Configurable 15–300 second fallback refresh
- Automatic app-server reconnect with exponential backoff
- No API key and no hosted backend

The default panel representation looks like:

```text
◖ 13% · W73%
```

Credits can optionally be included:

```text
◖ 13% · W73% · 240
```

The actual widget uses the Codex dual-arc logo rather than the text symbol shown above.

## Requirements

- KDE Plasma 6
- Qt 6.6 or newer
- Codex CLI installed and authenticated
- CMake 3.20 or newer for source builds
- Extra CMake Modules (ECM)
- Plasma development files for source builds

Codex must be available locally:

```bash
codex --version
```

If it is installed somewhere unusual, set `CODEX_EXECUTABLE` to the absolute path of the `codex` executable.

## Install

### Arch Linux / CachyOS via AUR

Once the package has been published to the AUR, this is the recommended installation method on Arch-based distributions:

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

### From source

On CachyOS / Arch Linux, install the build dependencies first:

```bash
sudo pacman -S --needed \
  base-devel \
  cmake \
  extra-cmake-modules \
  libplasma \
  ninja \
  qt6-base \
  qt6-declarative
```

Then build and install:

```bash
git clone https://github.com/sangimed/codex-menubar-kde.git
cd codex-menubar-kde

cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTING=ON \
  -DCMAKE_INSTALL_PREFIX=/usr

cmake --build build
ctest --test-dir build --output-on-failure
sudo cmake --install build
```

After installation, enter Plasma edit mode, choose **Add Widgets…**, search for **Codex MenuBar KDE**, and add it to your panel.

When updating a running native widget, restart Plasma so the C++ plugin is reloaded:

```bash
systemctl --user restart plasma-plasmashell.service
```

For development you can launch it directly with:

```bash
sudo pacman -S --needed plasma-sdk
plasmoidviewer -a io.github.sangimed.codexmenubarkde
```

## Packaging and releases

A release-oriented `PKGBUILD` and `.SRCINFO` live under `packaging/aur/`. Matching `v*` tags publish a GitHub release; once that release workflow succeeds, the matching AUR package is published automatically.

See [`packaging/README.md`](packaging/README.md) for the complete release and AUR flow.

## Configuration

Right-click the widget and open its configuration to change:

- remaining vs used percentages
- panel display: both quotas, 5-hour only, weekly only, or icon only
- credit balance visibility
- fallback refresh interval
- low-quota notifications
- notification threshold

Notifications use the standard freedesktop notification D-Bus service exposed by Plasma. No `notify-send` dependency is required.

## Usage history

Successful snapshots are retained locally for seven days. A new sample is stored when values change or at least every five minutes, capped at 2,500 samples.

History is stored at:

```text
~/.local/share/codex-menubar-kde/usage-history.json
```

It never leaves the machine and can be cleared from the widget popup.

## How it works

The widget starts the locally installed Codex CLI as:

```bash
codex app-server --stdio
```

The C++ backend performs the app-server initialization handshake and keeps the process alive. It requests the initial usage snapshot with:

```text
account/rateLimits/read
```

and listens for live updates through:

```text
account/rateLimits/updated
```

The backend identifies the standard windows by duration:

- `300` minutes → five-hour window
- `10080` minutes → weekly window

Current Codex protocol fields such as `windowDurationMins` and `resetsAt` may be nullable; the parser preserves usable percentage data and falls back to the historical primary/secondary ordering when duration metadata is absent.

When `rateLimitsByLimitId.codex` is present, it is preferred over the top-level `rateLimits` snapshot. Other buckets are exposed as additional limits.

## Architecture

```text
Plasma panel
    │
    ▼
QML / PlasmoidItem
    │
    ▼
CodexBackend (Qt 6 / C++)
    │
    ├── RateLimitParser
    ├── UsageHistoryStore
    ├── UsageNotificationManager ──► D-Bus notifications
    │
    ▼
QProcess
    │
    ▼
codex app-server --stdio
```

Repository layout:

- `package/` — Plasma 6/QML UI, icon and configuration
- `src/` — native Codex process, parser, history and notification integration
- `tests/` — parser and history tests
- `packaging/` — Arch/AUR packaging assets
- `.github/workflows/` — CI, tagged release and AUR publishing automation

## Troubleshooting

Follow widget logs with:

```bash
journalctl --user -f -o cat | grep --line-buffered -Ei 'codex|codexmenubarkde|plasmoid'
```

The popup also shows the exact Codex CLI executable used by the backend.

## Privacy

Codex MenuBar KDE runs locally. It uses the authenticated Codex CLI session and does not require a separate OpenAI API key or hosted service. Usage history is stored only on the local machine.

## Related project

The original macOS implementation is [CodexMenuBar](https://github.com/sangimed/codex-menubar).

## License

MIT
