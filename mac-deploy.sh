#!/bin/sh

export PATH="$HOME/Qt/5.15.2/clang_64/bin:$PATH"

~/Qt/5.15.2/clang_64/bin/macdeployqt ../Build/hoerbert.app -always-overwrite
./macdeployqtfix.py ../Build/hoerbert.app ~/Qt/5.15.2/
