# Purpose

Voxel editor for large scenes.

This editor was written to have a linux voxel editor with animation support for
my own engine and evolved into something that others might find useful, too.

# Features

* Large scene support
* Load vox, qbt, qb
* Save to vox, qbt, qb
* Exporting to a lot of formats (dae, obj, fbx, gltf, ...)
* Auto-saving
* Console to script your modeling (CTRL+TAB cmdlist)
* Key bindings via cfg file
* Four viewport mode
* Grid resolution
* Mirror mode
* Auto cropping volumes
* Auto generate content like trees or noise volumes
* Import heightmaps
* Undo/Redo
* Custom color palettes
* Reference positions to rotate around or create auto-generated structures at
* Viewport screenshots
* Import and voxelize dae, obj, fbx and a lot more formats
* Modelling without the mouse - just via shortcuts and keyboard

# Controls

* Placing voxels via left mouse button
* Deleting voxels via right mouse button
* Rotating viewport via middle mouse button
* Holding the left or right mouse buttons spans an AABB to perform the action in

# Planned

* Animation support
* Copy/paste
* Selections
* Improve voxelizer
* Support multiple RawVolumes (layers) (e.g. MagicaVoxel VOX format and Qubicle support this)
* Improve vox extension import
* Windows and MacOSX support
* Color selection via keyboard only
* Extend palette widget to show the used colors