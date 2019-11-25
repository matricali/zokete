[![license](https://img.shields.io/github/license/matricali/zokete.svg)](https://matricali.mit-license.org/2014) [![GitHub contributors](https://img.shields.io/github/contributors/matricali/zokete.svg)](https://github.com/matricali/zokete/graphs/contributors) [![Build Status](https://travis-ci.org/matricali/zokete.svg?branch=master)](https://travis-ci.org/matricali/zokete) [![Latest stable release](https://img.shields.io/badge/dynamic/json.svg?label=stable&url=https%3A%2F%2Fapi.github.com%2Frepos%2Fmatricali%2Fzokete%2Freleases%2Flatest&query=%24.name&colorB=blue)](https://github.com/matricali/zokete/releases/latest)

# zokete
Simple SOCKS5 Server. Written in _C_.

## Requeriments
* `gcc` compiler

## Build
```bash
git clone --depth=1 https://github.com/matricali/zokete.git
cd zokete
make
make install
```
Then you can do
```bash
$ zoketed -v
zoketed v0.1 - Simple SOCKS5 Server (https://github.com/matricali/zokete)
```
