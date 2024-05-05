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
* B5G6R5_UNORM
* B5G5R5A1_UNORM
* B8G8R8A8_UNORM
* R8_UNORM

#### Notes:
Typeless colorspace is treated as unorm.

Clearing thumbnail directory via `rm -r $HOME/.cache/thumbnails/*`
