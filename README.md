# Codex MenuBar KDE

A native KDE Plasma 6 widget for monitoring your OpenAI Codex usage limits at a glance.

> **Status:** early development. The first implementation is intentionally focused on a small, native Plasma 6 core.

## Features

- Plasma 6 panel widget with a compact quota summary
- Rolling 5-hour Codex usage
- Weekly Codex usage
- Used or remaining percentage mode
- Reset countdowns
- Codex plan and credit balance when reported
- Manual refresh
- Persistent local `codex app-server --stdio` session
- Live updates from `account/rateLimits/updated`
- Configurable 15–300 second fallback refresh
- Automatic app-server reconnect with exponential backoff
- No API key and no hosted backend

The default panel representation looks like:

```text
13% · W73%
```

Credits can optionally be included:

```text
13% · W73% · 240
```

## Requirements

- KDE Plasma 6
- Qt 6.6 or newer
- Codex CLI installed and authenticated
- CMake 3.20 or newer
- Extra CMake Modules (ECM)
- Plasma development files

### CachyOS / Arch Linux

Install the development dependencies with:

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

Codex must also be available:

```bash
codex --version
```

If it is installed somewhere unusual, set `CODEX_EXECUTABLE` to the absolute path of the `codex` executable.

## Build

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

For development you can also launch the widget directly:

```bash
plasmoidviewer -a io.github.sangimed.codexmenubarkde
```

`plasmoidviewer` is provided by `plasma-sdk` on Arch-based distributions.

## How it works

The widget starts the locally installed Codex CLI as:

```bash
codex app-server --stdio
```

The C++ backend performs the app-server initialization handshake and keeps the process alive. It requests the initial usage snapshot with:

```text
account/rateLimits/read
```

It then listens for:

```text
account/rateLimits/updated
```

The backend identifies the rolling five-hour and weekly windows by their duration:

- `300` minutes → five-hour window
- `10080` minutes → weekly window

When `rateLimitsByLimitId.codex` is present, it is preferred over the top-level `rateLimits` snapshot, matching CodexMenuBar on macOS.

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
    │
    ▼
QProcess
    │
    ▼
codex app-server --stdio
```

The widget package and the native backend are deliberately separated:

- `package/` — Plasma 6/QML UI and configuration
- `src/` — native Codex process and JSON-RPC integration
- `tests/` — protocol/model parsing tests

## Current scope

This first version implements the core monitoring path. Feature parity with the macOS project will be expanded incrementally, including richer additional-limit presentation, threshold notifications, local history, packaging, and release automation.

## Privacy

Codex MenuBar KDE runs locally. It uses the authenticated Codex CLI session and does not require a separate OpenAI API key or hosted service.

## Related project

The original macOS implementation is [CodexMenuBar](https://github.com/sangimed/codex-menubar).

## License

MIT
