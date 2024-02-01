# kdegraphics-thumbnailer-dds
DDS texture thumbnailer for KDE

At some point I got considerably annoyed that KDE does not support DDS files since forever, so here it is.

### Supported formats:
* uncompressed 8, 16, 24, 32 bit
* BC1 (DXT1)
* BC2 (DXT3)
* BC3 (DXT5)
* BC4U (ATI1)
* BC5U (ATI2)

##### (DX10 DXGI extension)
* BC1_UNORM
* BC2_UNORM
* BC3_UNORM
* BC4_UNORM
* BC5_UNORM
* B5G6R5_UNORM
* B5G5R5A1_UNORM
* B8G8R8A8_UNORM
* R8_UNORM

#### Notes:
You might want to manually adjust CMake variable `KDE_INSTALL_PLUGINDIR` because reasons.
Possible correct path is `/usr/lib/qt/plugins/`

Clearing thumbnail directory via `rm -r $HOME/.cache/thumbnails/*`
