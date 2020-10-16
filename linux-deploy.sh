#!/bin/sh

export PATH="$HOME/Qt/5.14.1/gcc_64/bin/:$PATH"
export VERSION="2.0.6"	#linuxdeployqt uses this for the app name

cp ./hoerbert/hoerbert.png ../Build
cp ./hoerbert/hoerbert.desktop ../Build
/usr/local/bin/linuxdeployqt-6-x86_64.AppImage ../Build/hoerbert -appimage -always-overwrite -executable=../Build/ffmpeg/ffmpeg -executable=../Build/ffmpeg/ffplay -executable=../Build/ffmpeg/ffprobe -executable=../Build/freac/freaccmd -executable=../Build/ffmpeg/ffmpeg

gpg2 --output hörbert-$VERSION-x86_64.AppImage.sig --detach-sig hörbert-$VERSION-x86_64.AppImage
