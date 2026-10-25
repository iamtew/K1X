<img src="assets/logo.png" alt="K1X" width="160">

# K1X

**K1X** is a kick drum (909 default, plus 808 / 707 / Tight / Hard). Tune, click, decay, drive, filters, parallel post FX (distortion / delay / reverb), post EQ, and a length that snaps to the DAW grid.

VST3 and standalone (play + WAV export). Stupid Systems LLC. [MIT](LICENSE). Site: [docs/](docs/index.html). Manual: [docs/MANUAL.md](docs/MANUAL.md).

## Release

Windows, Linux, and macOS files come from GitHub Actions (`.github/workflows/dist.yml`). Run it from the Actions tab, or push a numeric tag such as `0.0.4`. The workflow does not publish a release. `<version>` is the `version` in `K1X.jucer`.

The `dist` artifact holds:

- `K1X-<version>-win-x64-vst3.zip`
- `K1X-<version>-win-x64-standalone.zip`
- `K1X-<version>-win-x64.msi`
- `K1X-<version>-linux-x64-vst3.zip`
- `K1X-<version>-linux-x64-standalone.zip`
- `K1X-<version>-mac-universal-vst3.zip`
- `K1X-<version>-mac-universal-standalone.zip`

Download that artifact into `dist/` and upload the files on a GitHub release:

```
gh run download <run-id> -n dist -D dist
```

The MSI installs the VST3, the standalone app, or both. Defaults are `Common Files\VST3` and `Program Files\Stupid Systems LLC\K1X`; each folder can be changed. The standalone install adds a Start menu shortcut.

The Mac zips are ad-hoc signed, not notarized. Gatekeeper blocks them until you right-click Open, or run `xattr -dr com.apple.quarantine` on the `.vst3` and the `.app`. The Linux build uses the system ALSA, FreeType, fontconfig, and X11 libraries.

`just build-dist` on Windows still writes the three Windows files only. Building the MSI needs the .NET 8 SDK.
