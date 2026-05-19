# eamio-proxy
A simple implementation of the eamio.dll defined by the [Bemanitools](https://github.com/djhackersdev/bemanitools) bt5 api to forward calls to multiple eamio.dll implementations at the same time. This is useful if you want to use multiple card reader emulation methods at the same time. For example if you want to use the keypad of [this cool project](https://github.com/cloud11665/iidx_btools_subscreen) while still using your e-amusement card readers with bemanitools eamio-icca.dll, you can use both at the same time with eamio-proxy.

## Usage
- Alongside your bemanitools installation, place all the eamio dlls you want to use and name them `eamio-something.dll`
- Get the zip from the [latest release](https://github.com/sharuruka/eamio-proxy/releases) of this project.
- Extract the zip and choose either the 64 bit or 32 bit eamio dll matching the game you are setting up.
- Place the dll in the same directory as your bemanitools installation.
- Create a file called `eamio-proxy.conf` and list the names of all the eamio dlls you want to use on different lines.
- Make sure card reader emulation is enabled in the bemanitools config file
