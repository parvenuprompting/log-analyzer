#!/bin/bash
set -e
echo "Creating iconset directory..."
mkdir -p resources/MyIcon.iconset

echo "Resizing images..."
sips -s format png -z 16 16     resources/logo.png --out resources/MyIcon.iconset/icon_16x16.png
sips -s format png -z 32 32     resources/logo.png --out resources/MyIcon.iconset/icon_16x16@2x.png
sips -s format png -z 32 32     resources/logo.png --out resources/MyIcon.iconset/icon_32x32.png
sips -s format png -z 64 64     resources/logo.png --out resources/MyIcon.iconset/icon_32x32@2x.png
sips -s format png -z 128 128   resources/logo.png --out resources/MyIcon.iconset/icon_128x128.png
sips -s format png -z 256 256   resources/logo.png --out resources/MyIcon.iconset/icon_128x128@2x.png
sips -s format png -z 256 256   resources/logo.png --out resources/MyIcon.iconset/icon_256x256.png
sips -s format png -z 512 512   resources/logo.png --out resources/MyIcon.iconset/icon_256x256@2x.png
sips -s format png -z 512 512   resources/logo.png --out resources/MyIcon.iconset/icon_512x512.png
sips -s format png -z 1024 1024 resources/logo.png --out resources/MyIcon.iconset/icon_512x512@2x.png

echo "Converting to ICNS..."
iconutil -c icns resources/MyIcon.iconset -o resources/AppIcon.icns

echo "Cleaning up..."
rm -rf resources/MyIcon.iconset

echo "Done. Created resources/AppIcon.icns"
