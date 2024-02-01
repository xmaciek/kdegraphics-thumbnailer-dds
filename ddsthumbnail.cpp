// MIT License
//
// Copyright (c) 2022 Maciej Latocha <latocha.maciek@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.


// Format references
// https://docs.microsoft.com/en-us/windows/win32/direct3ddds/dds-header
// https://docs.microsoft.com/en-us/windows/win32/direct3ddds/dds-pixelformat
// https://docs.microsoft.com/en-us/windows/win32/direct3d10/d3d10-graphics-programming-guide-resources-block-compression
// https://docs.microsoft.com/en-us/windows/uwp/graphics-concepts/opaque-and-1-bit-alpha-textures
// https://learn.microsoft.com/en-us/windows/win32/api/dxgiformat/ne-dxgiformat-dxgi_format

#include <KPluginFactory>
#include <KIO/ThumbnailCreator>
#include <QFile>
#include <QImage>

#include <array>
#include <algorithm>
#include <cassert>
#include <cstdint>

#ifndef NDEBUG
#include <iostream>
#define LOG( msg ) std::cerr << ( msg ) << "\n";
#else
#define LOG( msg ) {}
#endif


class DDSThumbnailCreator : public KIO::ThumbnailCreator
{
public:
    DDSThumbnailCreator(QObject *parent, const QVariantList &args)
        : KIO::ThumbnailCreator(parent, args)
    {
    }

    ~DDSThumbnailCreator() override = default;

    KIO::ThumbnailResult create(const KIO::ThumbnailRequest &request) override;
};

K_PLUGIN_CLASS_WITH_JSON(DDSThumbnailCreator, "ddsthumbnail.json")

namespace {

struct Byte3 {
    uint8_t channel[ 3 ];
    operator uint32_t () const
    {
        uint32_t ret = 0;
        ret |= channel[ 2 ]; ret <<= 8;
        ret |= channel[ 1 ]; ret <<= 8;
        ret |= channel[ 0 ];
        return ret;
    }
};

namespace colorfn {

static uint32_t makeARGB8888( uint32_t r, uint32_t g, uint32_t b, uint32_t a )
{
    return ( a << 24 ) | ( r << 16 ) | ( g << 8 ) | b;
}

static uint32_t b8g8r8( Byte3 c )
{
    return makeARGB8888( c.channel[ 2 ], c.channel[ 1 ], c.channel[ 0 ], 0xFF );
}

static uint32_t b5g5r5a1( uint16_t c )
{
    uint32_t a = ( c >> 15 ) ? 0xFF : 0;
    uint32_t r = ( c >> 10 ) & 0b11111;
    uint32_t g = ( c >> 5 ) & 0b11111;
    uint32_t b = c & 0b11111;

    r = ( r << 3 ) | ( r >> 2 );
    g = ( g << 3 ) | ( g >> 2 );
    b = ( b << 3 ) | ( b >> 2 );
    return makeARGB8888( r, g, b, a );
}

static uint32_t b5g6r5( uint16_t c )
{
    uint32_t r = c >> 11;
    uint32_t g = ( c >> 5 ) & 0b111111;
    uint32_t b = c & 0b11111;

    r = ( r << 3 ) | ( r >> 2 );
    g = ( g << 2 ) | ( g >> 4 );
    b = ( b << 3 ) | ( b >> 2 );
    return makeARGB8888( r, g, b, 0xFF );
}

static uint32_t r8( uint8_t c )
{
    return makeARGB8888( c, c, c, 0xFF );
}

}


struct PixelFormat {
    enum Flags : uint32_t {
        fAlphaPixels = 0x1,
        fAlpha = 0x2,
        fFourCC = 0x4,
        fRGB = 0x40,
        fYUV = 0x200,
        fLuminance = 0x20000,
    };

    uint32_t size = 0;
    Flags flags = {};
    uint32_t fourCC = 0;
    uint32_t rgbBitCount = 0;
    uint32_t bitmaskR = 0;
    uint32_t bitmaskG = 0;
    uint32_t bitmaskB = 0;
    uint32_t bitmaskA = 0;
};
static_assert( sizeof( PixelFormat ) == 32 );

struct DDSHeader {
    enum Flags : uint32_t {
        fCaps = 0x1,
        fHeight = 0x2,
        fWidth = 0x4,
        fPitch = 0x8,
        fPixelFormat = 0x1000,
        fMipMapCount = 0x20000,
        fLinearSize = 0x80000,
        fDepth = 0x800000,
    };
    enum Caps : uint32_t {
        fComplex = 0x8,
        fMipMap = 0x400000,
        fTexture = 0x1000,
    };

    static constexpr uint32_t MAGIC = ' SDD';
    static constexpr uint32_t SIZE = 124;

    uint32_t magic = 0;
    uint32_t size = 0;
    Flags flags = {};
    uint32_t height = 0;
    uint32_t width = 0;
    uint32_t pitchOrLinearSize = 0;
    uint32_t depth = 0;
    uint32_t mipMapCount = 0;
    uint32_t reserved[ 11 ]{};
    PixelFormat pixelFormat{};
    Caps caps{};
    uint32_t caps2 = 0;
    uint32_t caps3 = 0;
    uint32_t caps4 = 0;
    uint32_t reserved2 = 0;
};
static_assert( sizeof( DDSHeader ) == 128 );

struct DXGIHeader {
    uint32_t format = 0;
    uint32_t dimension = 0;
    uint32_t flags = 0;
    uint32_t arraySize = 0;
    uint32_t flags2 = 0;
};
static_assert( sizeof( DXGIHeader ) == 20 );

static uint32_t lerp( uint32_t a, uint32_t b, float f )
{
    const float d = static_cast<float>( b ) - static_cast<float>( a );
    return a + static_cast<uint32_t>( d * f );
}

static uint16_t lerp565( uint16_t lhs, uint16_t rhs, float f )
{
    static constexpr uint16_t MASK_R5G6B5_R = 0b1111100000000000;
    static constexpr uint16_t MASK_R5G6B5_G = 0b0000011111100000;
    static constexpr uint16_t MASK_R5G6B5_B = 0b0000000000011111;

    const float r0 = ( lhs & MASK_R5G6B5_R ) >> 11;
    const float r1 = ( rhs & MASK_R5G6B5_R ) >> 11;
    const float g0 = ( lhs & MASK_R5G6B5_G ) >> 5;
    const float g1 = ( rhs & MASK_R5G6B5_G ) >> 5;
    const float b0 = ( lhs & MASK_R5G6B5_B );
    const float b1 = ( rhs & MASK_R5G6B5_B );
    const float rd = r1 - r0;
    const float gd = g1 - g0;
    const float bd = b1 - b0;
    const uint16_t r = r0 + rd * f;
    const uint16_t g = g0 + gd * f;
    const uint16_t b = b0 + bd * f;
    return ( ( r << 11 ) & MASK_R5G6B5_R )
        | ( ( g << 5 ) & MASK_R5G6B5_G )
        | ( b & MASK_R5G6B5_B );
}

template<typename T>
struct Decompressor {
    QVector<T> blocks{};
    uint32_t width = 0;
    uint32_t index = 0;

    Decompressor( QVector<T>&& b, uint32_t w )
    : blocks( std::move( b ) )
    , width{ w }
    {}

    uint32_t operator () ()
    {
        const uint32_t x = index % width;
        const uint32_t y = index / width;
        const uint32_t blockx = x / 4;
        const uint32_t blocky = y / 4;
        const uint32_t pitch = width / 4;
        const uint32_t blockId = blocky * pitch + blockx;
        assert( static_cast<qint64>( blockId ) < blocks.size() );
        const uint32_t pixelx = x % 4;
        const uint32_t pixely = y % 4;
        const uint32_t pixelId = pixely * 4 + pixelx;
        assert( pixelId < 16 );
        index++;
        return blocks[ blockId ][ pixelId ];
    }
};

struct BC1unorm {
    uint16_t color0;
    uint16_t color1;
    uint32_t indexes;

    uint32_t operator [] ( uint32_t i ) const
    {
        assert( i < 16 );
        const uint32_t index = 0b11 & ( indexes >> ( i * 2 ) );
        return colorFromIndex( index );
    }

    uint32_t colorFromIndex( uint32_t i ) const
    {
        switch ( i ) {
        case 0: return colorfn::b5g6r5( color0 );
        case 1: return colorfn::b5g6r5( color1 );
        case 2: return color0 < color1
            ? colorfn::b5g6r5( lerp565( color0, color1, 0.333f ) )
            : colorfn::b5g6r5( lerp565( color0, color1, 0.5f ) );
        case 3: return color0 < color1
            ? 0u
            : colorfn::b5g6r5( lerp565( color0, color1, 0.667f ) );
        default: return 0;
        }
    }
};
static_assert( sizeof( BC1unorm ) == 8, "sizeof BC1unorm not equal 8" );

struct BC2unorm {
    uint16_t alphas[ 4 ];
    uint16_t color0;
    uint16_t color1;
    uint32_t indexes;

    uint32_t operator [] ( uint32_t i ) const
    {
        assert( i < 16 );
        uint32_t a = alphas[ i / 4 ];
        uint32_t alpha = 0;
        switch ( i % 4 ) {
        case 0: alpha = ( a & 0xF ); break;
        case 1: alpha = ( a & 0xF0 ) >> 4; break;
        case 2: alpha = ( a & 0xF00 ) >> 8; break;
        case 3: alpha = ( a & 0xF000 ) >> 12; break;
        }
        const uint32_t index = 0b11 & ( indexes >> ( i * 2 ) );
        return ( alpha << 28 ) | ( alpha << 24 ) | colorFromIndex( index );
    }

    uint32_t colorFromIndex( uint32_t i ) const
    {
        const uint32_t removeAlpha = 0x00FFFFFF;
        switch ( i ) {
        case 0: return colorfn::b5g6r5( color0 ) & removeAlpha;
        case 1: return colorfn::b5g6r5( color1 ) & removeAlpha;
        case 2: return colorfn::b5g6r5( lerp565( color0, color1, 0.333f ) ) & removeAlpha;
        case 3: return colorfn::b5g6r5( lerp565( color0, color1, 0.667f ) ) & removeAlpha;
        default: return 0;
        }
    }
};
static_assert( sizeof( BC2unorm ) == 16, "sizeof BC2unorm not equal 16" );

struct BC4unorm {
    uint8_t alpha0;
    uint8_t alpha1;
    uint8_t aindexes[ 6 ];

    uint8_t alphaIndice( uint32_t i ) const
    {
        uint64_t indices = 0;
        indices |= aindexes[ 5 ]; indices <<= 8;
        indices |= aindexes[ 4 ]; indices <<= 8;
        indices |= aindexes[ 3 ]; indices <<= 8;
        indices |= aindexes[ 2 ]; indices <<= 8;
        indices |= aindexes[ 1 ]; indices <<= 8;
        indices |= aindexes[ 0 ];
        indices >>= i * 3;
        return indices & 0b111;
    }

    uint32_t alpha( uint32_t i ) const
    {
        const bool b = alpha0 > alpha1;
        switch ( alphaIndice( i ) ) {
        case 0b000: return alpha0;
        case 0b001: return alpha1;
        case 0b010: return lerp( alpha0, alpha1, b ? 1.0f / 7.0f : 0.2f );
        case 0b011: return lerp( alpha0, alpha1, b ? 2.0f / 7.0f : 0.4f );
        case 0b100: return lerp( alpha0, alpha1, b ? 3.0f / 7.0f : 0.6f );
        case 0b101: return lerp( alpha0, alpha1, b ? 4.0f / 7.0f : 0.8f );
        case 0b110: return b ? lerp( alpha0, alpha1, 5.0f / 7.0f ) : 0u;
        case 0b111: return b ? lerp( alpha0, alpha1, 6.0f / 7.0f ) : 255u;
        default: return 0;
        }
    }

    uint32_t operator [] ( uint32_t i ) const
    {
        return colorfn::r8( alpha( i ) );
    }

};
static_assert( sizeof( BC4unorm ) == 8, "sizeof BC4unorm not equal 8" );

struct BC3unorm : public BC4unorm {
    uint16_t color0;
    uint16_t color1;
    uint32_t indexes;

    uint32_t operator [] ( uint32_t i ) const
    {
        assert( i < 16 );
        const uint32_t index = 0b11 & ( indexes >> ( i * 2 ) );
        return ( alpha( i ) << 24 ) | colorFromIndex( index );
    }

    uint32_t colorFromIndex( uint32_t i ) const
    {
        const uint32_t removeAlpha = 0x00FFFFFF;
        switch ( i ) {
        case 0: return colorfn::b5g6r5( color0 ) & removeAlpha;
        case 1: return colorfn::b5g6r5( color1 ) & removeAlpha;
        case 2: return colorfn::b5g6r5( lerp565( color0, color1, 0.333f ) ) & removeAlpha;
        case 3: return colorfn::b5g6r5( lerp565( color0, color1, 0.667f ) ) & removeAlpha;
        default: return 0;
        }
    }
};
static_assert( sizeof( BC3unorm ) == 16, "sizeof BC3unorm not equal 16" );

struct BC5unorm {
    BC4unorm red;
    BC4unorm green;

    uint32_t operator [] ( uint32_t i ) const
    {
        assert( i < 16 );
        return colorfn::makeARGB8888( red.alpha( i ), green.alpha( i ), 0u, 0xFFu );
    }
};
static_assert( sizeof( BC5unorm ) == 16, "sizeof BC5unorm not equal 16" );

template <typename T>
static QVector<T> readPixels( const DDSHeader& header, QFile* file )
{
    assert( file );

    const qint64 pixelCount = (qint64)header.width * (qint64)header.height;
    qint64 bytesToRead = pixelCount * (qint64)sizeof( T );
    qint64 bytesToSkip = 0;
    qint64 pixelsToAdvance = pixelCount;
    qint64 loopCount = 1;

    if ( header.flags & DDSHeader::fPitch ) {
        const qint64 bytesPerLine = (qint64)header.width * (qint64)sizeof( T );
        if ( header.pitchOrLinearSize < bytesPerLine ) {
            LOG( "Suspicious pitch value, maybe TODO" );
            return {};
        }
        if ( header.pitchOrLinearSize > bytesPerLine ) {
            if ( file->bytesAvailable() < ( header.pitchOrLinearSize * header.height ) ) {
                LOG( "File truncated or corrupted, not enough data to read" );
                return {};
            }
            bytesToRead = bytesPerLine;
            bytesToSkip = header.pitchOrLinearSize - bytesPerLine;
            loopCount = header.height;
            pixelsToAdvance = header.width;
        }
    }
    if ( file->bytesAvailable() < bytesToRead ) {
        LOG( "File truncated or corrupted, not enough data to read" );
        return {};
    }

    QVector<T> pixels{};
    pixels.resize( pixelCount );
    auto it = pixels.begin();
    assert( loopCount > 0 );
    for ( qint64 i = loopCount; i; --i ) {
        file->read( reinterpret_cast<char*>( it ), bytesToRead );
        file->skip( bytesToSkip );
        std::advance( it, pixelsToAdvance );
    }
    assert( it == pixels.end() );

    file->close();
    return pixels;
}

template<typename T>
static QVector<T> readBlocks( QFile* file, qint64 pixelCount )
{
    assert( file );
    const qint64 blocksToRead = pixelCount / 16;
    const qint64 bytesToRead = blocksToRead * sizeof( T );
    if ( file->bytesAvailable() < bytesToRead ) {
        LOG( "File truncated or corrupted, not enough data to read" );
        return {};
    }

    QVector<T> blocks{};
    blocks.resize( blocksToRead );
    file->read( reinterpret_cast<char*>( blocks.data() ), bytesToRead );
    file->close();
    return blocks;
}

struct Deswizzler {
    using Rescale = uint8_t( uint32_t );
    Rescale* rRescale = nullptr;
    Rescale* gRescale = nullptr;
    Rescale* bRescale = nullptr;
    Rescale* aRescale = nullptr;
    uint32_t rMask = 0;
    uint32_t gMask = 0;
    uint32_t bMask = 0;
    uint32_t aMask = 0;
    uint8_t rShift = 0;
    uint8_t gShift = 0;
    uint8_t bShift = 0;
    uint8_t aShift = 0;

    Deswizzler() = default;
    Deswizzler( uint32_t rm, uint32_t gm, uint32_t bm, uint32_t am )
    : rMask( rm )
    , gMask( gm )
    , bMask( bm )
    , aMask( am )
    {
        auto gibRescale = []( uint32_t mask ) -> Rescale*
        {
            switch ( __builtin_popcount( mask ) ) {
            case 0: return []( uint32_t ) -> uint8_t { return 0; };
            case 1: return []( uint32_t c ) -> uint8_t { return c ? 255 : 0; };
            case 4: return []( uint32_t c ) -> uint8_t { return static_cast<uint8_t>( ( c << 4 ) | c ); };
            case 5: return []( uint32_t c ) -> uint8_t { return static_cast<uint8_t>( ( c << 3 ) | ( c >> 2 ) ); };
            case 6: return []( uint32_t c ) -> uint8_t { return static_cast<uint8_t>( ( c << 2 ) | ( c >> 4 ) ); };
            case 8: return []( uint32_t c ) -> uint8_t { return static_cast<uint8_t>( c ); };
            case 16: return []( uint32_t c ) -> uint8_t { return static_cast<uint8_t>( c >> 8 ); };
            case 24: return []( uint32_t c ) -> uint8_t { return static_cast<uint8_t>( c >> 16 ); };
            case 32: return []( uint32_t c ) -> uint8_t { return static_cast<uint8_t>( c >> 24 ); };
            default: return nullptr;
            }
        };
        rRescale = gibRescale( rm );
        gRescale = gibRescale( gm );
        bRescale = gibRescale( bm );
        // NOTE: if alpha mask is 0, make it opaque instead
        aRescale = am ? gibRescale( am ) : []( uint32_t ) -> uint8_t { return 255; };
        rShift = rm ? __builtin_ctz( rm ) : 0;
        gShift = gm ? __builtin_ctz( gm ) : 0;
        bShift = bm ? __builtin_ctz( bm ) : 0;
        aShift = am ? __builtin_ctz( am ) : 0;
    }

    bool isValid() const
    {
        return rRescale && gRescale && bRescale && aRescale;
    }

    uint32_t operator () ( uint32_t v ) const
    {
        assert( isValid() );
        uint8_t r = rRescale( ( v & rMask ) >> rShift );
        uint8_t g = gRescale( ( v & gMask ) >> gShift );
        uint8_t b = bRescale( ( v & bMask ) >> bShift );
        uint8_t a = aRescale( ( v & aMask ) >> aShift );
        return colorfn::makeARGB8888( r, g, b, a );
    }

    uint32_t operator () ( Byte3 v ) const
    {
        return operator ()( static_cast<uint32_t>( v ) );
    }

    uint32_t operator () ( uint16_t c ) const
    {
        return operator ()( static_cast<uint32_t>( c ) );
    }

    uint32_t operator () ( uint8_t c ) const
    {
        return operator ()( static_cast<uint32_t>( c ) );
    }
};

struct ImageData {
    QVector<uint32_t> pixels;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t oWidth = 0;
    uint32_t oHeight = 0;
    bool extentNeedsResize = false;
};

template <typename TBlockType>
static ImageData blockDecompress( const DDSHeader& header, QFile* file )
{
    assert( file );

    if ( header.flags & DDSHeader::fPitch ) {
        LOG( "Suspicious BC format file with pitch flag, maybe TODO" );
        return {};
    }

    auto align4 = []( uint32_t v ) { return ( v + 3u ) & ~3u; };
    const uint32_t width = align4( header.width );
    const uint32_t height = align4( header.height );
    const qint64 pixelCount = width * height;
    QVector<TBlockType> blocks = readBlocks<TBlockType>( file, pixelCount );
    if ( blocks.empty() ) {
        return {};
    }

    ImageData ret{};
    ret.pixels.resize( pixelCount );
    ret.width = width;
    ret.height = height;
    ret.oWidth = header.width;
    ret.oHeight = header.height;
    ret.extentNeedsResize = ( header.width % 4 ) || ( header.height % 4 );
    std::generate( ret.pixels.begin(), ret.pixels.end(), Decompressor<TBlockType>( std::move( blocks ), width ) );
    return ret;
}


template <typename TSrc, uint32_t(*fn)(TSrc)>
static ImageData readAndConvert( const DDSHeader& header, QFile* file )
{
    assert( file );

    QVector<TSrc> srcPixels = readPixels<TSrc>( header, file );
    if ( srcPixels.empty() ) {
        return {};
    }

    ImageData ret{};
    ret.width = header.width;
    ret.height = header.height;
    ret.pixels.resize( srcPixels.size() );
    std::transform( srcPixels.begin(), srcPixels.end(), ret.pixels.data(), fn );
    return ret;
}

// NOTE: Happy endianness
//       DXGI_FORMAT_B8G8R8A8_UNORM == QImage::Format_ARGB32
static ImageData read_b8g8r8a8( const DDSHeader& header, QFile* file )
{
    assert( file );

    QVector<uint32_t> pixels = readPixels<uint32_t>( header, file );
    if ( pixels.empty() ) {
        return {};
    }

    ImageData ret{};
    ret.width = header.width;
    ret.height = header.height;
    ret.pixels = std::move( pixels );
    return ret;
}

static ImageData handleFourCC( const DDSHeader& header, QFile* file )
{
    assert( file );
    assert( header.pixelFormat.flags == PixelFormat::fFourCC );

    switch ( header.pixelFormat.fourCC ) {
    case '1TXD': return blockDecompress<BC1unorm>( header, file );
    case '3TXD': return blockDecompress<BC2unorm>( header, file );
    case '5TXD': return blockDecompress<BC3unorm>( header, file );
    case '01XD': break;

    // NOTE: unable to find documentation, but living specimens exists in the wild
    case 'U4CB': [[fallthrough]];
    case '1ITA': return blockDecompress<BC4unorm>( header, file );
    case 'U5CB': [[fallthrough]];
    case '2ITA': return blockDecompress<BC5unorm>( header, file );
    default:
        LOG( "Unknown fourCC value" );
        return {};
    }

    if ( file->bytesAvailable() < static_cast<qint64>( sizeof( DXGIHeader ) ) ) {
        LOG( "File truncated or corrupted, not enough data to read dxgi header" );
        return {};
    }

    DXGIHeader dxgiHeader{};
    file->read( reinterpret_cast<char*>( &dxgiHeader ), sizeof( DXGIHeader ) );
    if ( dxgiHeader.dimension != 3 /* texture 2d */ ) {
        LOG( "Unsupported dimension - expected texture 2D" );
        return {};
    }

    enum Format : uint32_t {
        DXGI_FORMAT_R8_UNORM = 61,
        DXGI_FORMAT_BC1_UNORM = 71,
        DXGI_FORMAT_BC2_UNORM = 74,
        DXGI_FORMAT_BC3_UNORM = 77,
        DXGI_FORMAT_BC4_UNORM = 80,
        DXGI_FORMAT_BC5_UNORM = 83,
        DXGI_FORMAT_B5G6R5_UNORM = 85,
        DXGI_FORMAT_B5G5R5A1_UNORM = 86,
        DXGI_FORMAT_B8G8R8A8_UNORM = 87,
    };

    switch ( dxgiHeader.format ) {
    case DXGI_FORMAT_BC1_UNORM: return blockDecompress<BC1unorm>( header, file );
    case DXGI_FORMAT_BC2_UNORM: return blockDecompress<BC2unorm>( header, file );
    case DXGI_FORMAT_BC3_UNORM: return blockDecompress<BC3unorm>( header, file );
    case DXGI_FORMAT_BC4_UNORM: return blockDecompress<BC4unorm>( header, file );
    case DXGI_FORMAT_BC5_UNORM: return blockDecompress<BC5unorm>( header, file );
    case DXGI_FORMAT_B5G5R5A1_UNORM: return readAndConvert<uint16_t, &colorfn::b5g5r5a1>( header, file );
    case DXGI_FORMAT_B5G6R5_UNORM: return readAndConvert<uint16_t, &colorfn::b5g6r5>( header, file );
    case DXGI_FORMAT_B8G8R8A8_UNORM: return read_b8g8r8a8( header, file );
    case DXGI_FORMAT_R8_UNORM: return readAndConvert<uint8_t, &colorfn::r8>( header, file );
    default:
        LOG( "Unsupported dxgi format, maybe TODO" );
        return {};
    }
}

static ImageData extractUncompressedPixels( const DDSHeader& header, QFile* file )
{
    assert( file );
    if ( header.pixelFormat.flags & PixelFormat::fYUV ) {
        LOG( "YUV images not supported, maybe TODO" );
        return {};
    }

    struct Fmt {
        uint32_t bitCount;
        std::array<uint32_t, 4> bitMasks;
        ImageData (*readAndConvert)( const DDSHeader&, QFile* );
    };

    static constexpr Fmt LUT[] = {
        Fmt{ 32, { 0x00FF0000u, 0x0000FF00u, 0x00000000FFu, 0xFF000000u }, &read_b8g8r8a8 },
        Fmt{ 24, { 0x00FF0000u, 0x0000FF00u, 0x00000000FFu, 0x00000000u }, &readAndConvert<Byte3, &colorfn::b8g8r8> },
        Fmt{ 16, { 0b1111100000000000u, 0b0000011111100000u, 0b0000000000011111u, 0u, }, &readAndConvert<uint16_t, &colorfn::b5g6r5> },
        Fmt{ 16, { 0b0111110000000000u, 0b0000001111100000u, 0b0000000000011111u, 0b1000000000000000u, }, &readAndConvert<uint16_t, &colorfn::b5g5r5a1> },
        Fmt{ 8, { 0xFFu, 0u, 0u, 0u }, &readAndConvert<uint8_t, &colorfn::r8> },
        Fmt{ 8, { 0u, 0u, 0u, 0xFFu }, &readAndConvert<uint8_t, &colorfn::r8> },
    };
    for ( auto&& fmt : LUT ) {
        assert( fmt.readAndConvert );
        if ( fmt.bitCount != header.pixelFormat.rgbBitCount ) continue;
        // TODO: expected 0 in mask if unused, clear per flags bits if found problematic
        std::array<uint32_t, 4> mask{
            header.pixelFormat.bitmaskR,
            header.pixelFormat.bitmaskG,
            header.pixelFormat.bitmaskB,
            header.pixelFormat.bitmaskA,
        };
        if ( fmt.bitMasks != mask ) continue;
        return fmt.readAndConvert( header, file );
    }

    // NOTE: if "common" format lookup not found, use slower deswizzler, no guarantees its 100% accurate for every possible permutation
    Deswizzler deswizzler{};
    const bool hasAlphaPixels = !!( header.pixelFormat.flags & PixelFormat::fAlphaPixels );

    if ( header.pixelFormat.flags & PixelFormat::fRGB ) {
        deswizzler = Deswizzler{
            header.pixelFormat.bitmaskR,
            header.pixelFormat.bitmaskG,
            header.pixelFormat.bitmaskB,
            hasAlphaPixels ? header.pixelFormat.bitmaskA : 0u
        };
    }
    else if ( header.pixelFormat.flags & PixelFormat::fLuminance ) {
        deswizzler = Deswizzler{ header.pixelFormat.bitmaskR, 0u, 0u, hasAlphaPixels ? header.pixelFormat.bitmaskA : 0u };
    }
    else if ( header.pixelFormat.flags & PixelFormat::fAlpha || hasAlphaPixels ) {
        deswizzler = Deswizzler{ 0u, 0u, 0u, header.pixelFormat.bitmaskA };
    }
    else {
        LOG( "Suspicious pixel format, maybe TODO" );
        return {};
    }

    if ( !deswizzler.isValid() ) {
        LOG( "Very sus pixel format, possibly corrupted" );
        return {};
    }

    ImageData ret{};
    ret.width = header.width;
    ret.height = header.height;

    switch ( header.pixelFormat.rgbBitCount ) {
    case 8: {
        QVector<uint8_t> tmp = readPixels<uint8_t>( header, file );
        ret.pixels.resize( tmp.size() );
        std::transform( tmp.begin(), tmp.end(), ret.pixels.begin(), deswizzler );
        return ret;
    }
    case 16: {
        QVector<uint16_t> tmp = readPixels<uint16_t>( header, file );
        ret.pixels.resize( tmp.size() );
        std::transform( tmp.begin(), tmp.end(), ret.pixels.begin(), deswizzler );
        return ret;
    }
    case 24: {
        QVector<Byte3> tmp = readPixels<Byte3>( header, file );
        ret.pixels.resize( tmp.size() );
        std::transform( tmp.begin(), tmp.end(), ret.pixels.begin(), deswizzler );
        return ret;
    }
    case 32: {
        ret.pixels = readPixels<uint32_t>( header, file );
        std::transform( ret.pixels.begin(), ret.pixels.end(), ret.pixels.begin(), deswizzler );
        return ret;
    }
    default:
        LOG( "Suspicious pixel format, maybe TODO" );
        return {};
    }
}

} // namespace

KIO::ThumbnailResult DDSThumbnailCreator::create( const KIO::ThumbnailRequest& request )
{
    QFile file{ request.url().toLocalFile() };

    if ( !file.open( QIODevice::ReadOnly ) ) {
        LOG( "File not readable" );
        return KIO::ThumbnailResult::fail();
    }

    if ( file.size() < static_cast<qint64>( sizeof( DDSHeader ) ) ) {
        LOG( "File truncated, expected at least 128 bytes" );
        return KIO::ThumbnailResult::fail();
    }

    DDSHeader header{};
    file.read( reinterpret_cast<char*>( &header ), sizeof( DDSHeader ) );
    if ( header.magic != DDSHeader::MAGIC ) {
        LOG( "Magic field not 'DDS '" );
        return KIO::ThumbnailResult::fail();
    }

    if ( header.size != DDSHeader::SIZE ) {
        LOG( "Header .size not 124" );
        return KIO::ThumbnailResult::fail();
    }

    if ( ~header.flags & ( DDSHeader::fCaps | DDSHeader::fHeight | DDSHeader::fWidth | DDSHeader::fPixelFormat ) ) {
        LOG( "Missing .flags ( caps | width | height | pixelformat )" );
        return KIO::ThumbnailResult::fail();
    }

    if ( ~header.caps & DDSHeader::Caps::fTexture ) {
        LOG( "Missing .caps flag ( texture )" );
        return KIO::ThumbnailResult::fail();
    }

    static constexpr uint64_t MAX_PIXEL_COUNT = 256u << 18; // 256MiB / sizeof( ARGB32 )
    if ( (uint64_t)header.width * (uint64_t)header.height > MAX_PIXEL_COUNT ) {
        LOG( "Intermediate thumbnail size exceeds arbitrary sane limit of 256MiB, file possibly corrupted" );
        return KIO::ThumbnailResult::fail();
    }

    const bool isFourCC = header.pixelFormat.flags == PixelFormat::fFourCC;
    ImageData data = isFourCC
        ? handleFourCC( header, &file )
        : extractUncompressedPixels( header, &file );

    if ( data.pixels.empty() ) {
        return KIO::ThumbnailResult::fail();
    }

    assert( data.width );
    assert( data.height );
    QImage image{ reinterpret_cast<const uchar*>( data.pixels.data() )
        , static_cast<int>( data.width )
        , static_cast<int>( data.height )
        , QImage::Format_ARGB32
        , nullptr // non-owning qimage
        , nullptr
    };
    if ( data.extentNeedsResize ) {
        assert( data.oWidth );
        assert( data.oHeight );
        image = image.copy( 0, 0, data.oWidth, data.oHeight );
    }
    return KIO::ThumbnailResult::pass(image.scaled(
        request.targetSize().width(),
        request.targetSize().height(),
        Qt::KeepAspectRatio
    ));
}

#include "ddsthumbnail.moc"
