#!/bin/sh

export PATH="$HOME/Qt5.15.2/5.15.2/clang_64/bin:$PATH"

~/Qt5.15.2/5.15.2/clang_64/bin/macdeployqt ../Build/hoerbert.app -always-overwrite
./macdeployqtfix.py ../Build/hoerbert.app ~/Qt5.15.2/5.15.2/
