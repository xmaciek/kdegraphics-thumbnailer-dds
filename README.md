# kdegraphics-thumbnailer-dds
DDS texture thumbnailer for KDE

At some point I got considerably annoyed that KDE does not support DDS files since forever, so here it is, thumbnailer for them, written basicly over a weekend.

### Supported formats:
* uncompressed 24 bit & 32 bit
* BC1 (DXT1)
* BC2 (DXT3)
* BC3 (DXT5)

##### (DX10 DXGI extension)
* BC1_UNORM
* BC2_UNORM
* BC3_UNORM
* B8G8R8A8_UNORM

#### Notes:
You might want to manually adjust CMake variable `KDE_INSTALL_PLUGINDIR` because reasons.
Possible correct path is `/usr/lib/qt/plugins/`
