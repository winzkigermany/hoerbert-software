#!/bin/sh

export PATH="$HOME/Qt/5.13.1/clang_64/bin:$PATH"

/Users/rainer/Qt/5.13.2/clang_64/bin/macdeployqt ../Build/hoerbert.app -always-overwrite
./macdeployqtfix.py ../Build/hoerbert.app ~/Qt/5.13.1/
