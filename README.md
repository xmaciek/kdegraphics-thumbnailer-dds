# kdegraphics-thumbnailer-dds
DDS texture thumbnailer for KDE

At some point I got considerably annoyed that KDE does not support DDS files since forever, so here it is.

### Supported formats:
* uncompressed 8, 16, 24, 32 bit
* BC1 (DXT1, DXT2)
* BC2 (DXT3, DXT4)
* BC3 (DXT5)
* BC4U (ATI1)
* BC4S
* BC5U (ATI2)
* BC5S

##### (DX10 DXGI extension)
* BC1 (unorm, srgb, typeless)
* BC2 (unorm, srgb, typeless)
* BC3 (unorm, srgb, typeless)
* BC4 (unorm, srgb, typeless)
* BC5 (unorm, srgb, typeless)
* BC7 (unorm, srgb, typeless)
* B4G4R4A4_UNORM
* B5G6R5_UNORM
* B5G5R5A1_UNORM
* B8G8R8A8_UNORM
* R8_UNORM

#### Building
`cmake path/to/kdegraphics-thumbnailer-dds --preset release`
`make`
`make install`

#### Notes:
Verify you have enabled thumbnailer in Dolphin settings, goto
* `Configure`->`Configure Dolphin`->`Interface`->`Previews`->`Microsoft DirectDraw Surface (DDS)`

Thumbnailer doesn't work for formats listed here
* Probably you are running into thumbnailer collision and have other dds thumbnail handler enabled.
`kio-extras` has a hardcoded list of mime types for optional `.so` files, even if `.../qt6/plugins/imageformats/kimg_dds.so` is missing the `imagethumbnail.so` will try and fail to process it.
`kio` is choosing thumbnail handlers by last-mime-thumbnail-plugin from whatever plugin factory gives. You have 2 options, either:
  * disable `Images (GIF, PNG, BMP, ...)`
  * hexedit `/usr/lib/qt6/plugins/kf6/thumbcreator/imagethumbnail.so` and overwrite json metadata `x-dds` value with `x-xxx` to avoid mime selection collision.
  * wait until KDE implements thumbnailer prioritization (extremely unlikely).

Typeless colorspace is treated as unorm.

Clearing thumbnail directory via any of:
* `rm -r $HOME/.cache/thumbnails/*`
* `make nuke` (custom target for the above)
