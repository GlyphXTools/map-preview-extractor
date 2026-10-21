# map-preview-extractor
[![build](https://github.com/GlyphXTools/map-preview-extractor/actions/workflows/build.yml/badge.svg)](https://github.com/GlyphXTools/map-preview-extractor/actions/workflows/build.yml?query=branch%3Amaster)

Extracts the preview image from GlyphX's maps

## Usage

Use the "Browse" button to select a Map file where you want to extract the preview from. You can also drag a map file onto the application window or executable. Then hit "Extract Preview" and select a location to save the TGA file to.

**Important**: After extracting, the preview image is REMOVED from the original map file.

## Base Layouts

This tool will make it possible to create fully functional Base Layouts in Galactic Conquest maps in Empire at War: Forces of Corruption. By removing the preview image from the map, the markers are enabled on the Base Layout screen in the game.

Note that the extracted preview image can be used as background for the Base Layout: resize the extracted image to 512x512 and save it in the Textures directory as a DX3 DDS file, with the same filename as the map.

## 64-bit update

Petroglyph [released an updated 64-bit-compatible version of the map editor](https://www.petroglyphgames.com/eawmodtool/). This version does **NOT** save embedded texture previews in the map anymore. At all. So the map preview extractor doesn't work on map created with this updated version of the editor.

However, there is good news. The editor has a menu option "Tools > Generate Map Preview Image" which generates & saves the preview image to the location "..\Art\Textures\MapPreview\". This path is relative to the editor executable, not relative to the opened map.

So for instance, for the Steam version, you just have to ensure the folder "...\steamapps\common\Star Wars Empire at War\Art\Textures\MapPreview" exists, and whenever you execute that menu option, the previews will be saved there.
