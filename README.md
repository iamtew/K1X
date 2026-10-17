<img src="assets/logo.png" alt="K1X" width="160">

# K1X

**K1X** is a kick drum (909 default, plus 808 / 707 / Tight / Hard). Tune, click, decay, drive, filters, parallel post FX (distortion / delay / reverb), post EQ, and a length that snaps to the DAW grid.

VST3 and standalone (play + WAV export). Stupid Systems LLC. [MIT](LICENSE). Site: [docs/](docs/index.html). Manual: [docs/MANUAL.md](docs/MANUAL.md).

## Release

`just build-dist` writes `dist/K1X-<version>-win-x64-vst3.zip`, `dist/K1X-<version>-win-x64-standalone.zip`, and `dist/K1X-<version>-win-x64.msi`. The MSI lets you install the VST3, the standalone app, or both. Defaults are `Common Files\VST3` and `Program Files\Stupid Systems LLC\K1X`; each folder can be changed. The standalone install adds a Start menu shortcut. Building the MSI needs the .NET 8 SDK.
