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

#include <KIO/ThumbCreator>
#include <QFile>
#include <QImage>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>

static constexpr uint16_t c_16rmask = 0b1111100000000000;
static constexpr uint16_t c_16gmask = 0b0000011111100000;
static constexpr uint16_t c_16bmask = 0b0000000000011111;

class DDSThumbnail : public ThumbCreator
{
public:
    DDSThumbnail() = default;
    ~DDSThumbnail() override = default;
    bool create( const QString& path, int width, int height, QImage& ) override;
    Flags flags() const override;
};

extern "C" Q_DECL_EXPORT ThumbCreator* new_creator()
{
    return new DDSThumbnail{};
}

ThumbCreator::Flags DDSThumbnail::flags() const
{
    return {};
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

    static constexpr uint32_t c_magic = ' SDD';

    uint32_t magic = c_magic;
    uint32_t size = 0;
    Flags flags = {};
    uint32_t height = 0;
    uint32_t width = 0;
    uint32_t pitchOrLinearSize = 0;
    uint32_t depth = 0;
    uint32_t mipMapCount = 0;
    uint32_t reserved[ 11 ]{};
    PixelFormat pixelFormat{};
    Caps caps = {};
    uint32_t caps2 = 0;
    uint32_t caps3 = 0;
    uint32_t caps4 = 0;
    uint32_t reserved2 = 0;
};
static_assert( sizeof( DDSHeader ) == 128 );

static uint32_t unpackRGB565( uint32_t color )
{
    const uint32_t r = ( color & c_16rmask ) >> 11;
    const uint32_t g = ( color & c_16gmask ) >> 5;
    const uint32_t b = ( color & c_16bmask );
    // main color component | substituted missing color bits
    return ( ( ( r << 3 ) | ( r & 0b111 ) ) << 16 )
         | ( ( ( g << 2 ) | ( g & 0b011 ) ) << 8 )
         | ( ( ( b << 3 ) | ( b & 0b111 ) ) );
}

static uint32_t lerp( uint32_t a, uint32_t b, float f )
{
    const float d = static_cast<float>( b ) - static_cast<float>( a );
    return a + static_cast<uint32_t>( d * f );
}

static uint16_t lerp565( uint16_t lhs, uint16_t rhs, float f )
{
    const float r0 = ( lhs & c_16rmask ) >> 11;
    const float r1 = ( rhs & c_16rmask ) >> 11;
    const float g0 = ( lhs & c_16gmask ) >> 5;
    const float g1 = ( rhs & c_16gmask ) >> 5;
    const float b0 = ( lhs & c_16bmask );
    const float b1 = ( rhs & c_16bmask );
    const float rd = r1 - r0;
    const float gd = g1 - g0;
    const float bd = b1 - b0;
    const uint16_t r = r0 + rd * f;
    const uint16_t g = g0 + gd * f;
    const uint16_t b = b0 + bd * f;
    return ( ( r << 11 ) & c_16rmask )
        | ( ( g << 5 ) & c_16gmask )
        | ( b & c_16bmask );
}

template<typename T>
struct Decompressor {
    QVector<T> blocks{};
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t index = 0;

    Decompressor( QVector<T>&& b, uint32_t w, uint32_t h )
    : blocks( std::move( b ) )
    , width{ w }
    , height{ h }
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

struct BC1 {
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
        static constexpr uint32_t alpha = 0xFF000000;
        switch ( i ) {
        case 0: return alpha | unpackRGB565( color0 );
        case 1: return alpha | unpackRGB565( color1 );
        case 2: return color0 < color1
            ? alpha | unpackRGB565( lerp565( color0, color1, 0.333f ) )
            : alpha | unpackRGB565( lerp565( color0, color1, 0.5f ) );
        case 3: return color0 < color1
            ? 0u
            : alpha | unpackRGB565( lerp565( color0, color1, 0.667f ) );
        default: return 0;
        }
    }
};
static_assert( sizeof( BC1 ) == 8, "sizeof BC1 not equal 8" );

struct BC2 {
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
        switch ( i ) {
        case 0: return unpackRGB565( color0 );
        case 1: return unpackRGB565( color1 );
        case 2: return unpackRGB565( lerp565( color0, color1, 0.333f ) );
        case 3: return unpackRGB565( lerp565( color0, color1, 0.667f ) );
        default: return 0;
        }
    }
};
static_assert( sizeof( BC2 ) == 16, "sizeof BC2 not equal 16" );

struct BC3 {
    uint8_t alpha0;
    uint8_t alpha1;
    uint8_t aindexes[ 6 ];
    uint16_t color0;
    uint16_t color1;
    uint32_t indexes;

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
        assert( i < 16 );
        const uint32_t index = 0b11 & ( indexes >> ( i * 2 ) );
        return ( alpha( i ) << 24 ) | colorFromIndex( index );
    }

    uint32_t colorFromIndex( uint32_t i ) const
    {
        switch ( i ) {
        case 0: return unpackRGB565( color0 );
        case 1: return unpackRGB565( color1 );
        case 2: return unpackRGB565( lerp565( color0, color1, 0.333f ) );
        case 3: return unpackRGB565( lerp565( color0, color1, 0.667f ) );
        default: return 0;
        }
    }
};
static_assert( sizeof( BC3 ) == 16, "sizeof BC3 not equal 16" );

template<typename T>
static QVector<T> readBlocks( QFile* file, uint32_t pixelCount )
{
    assert( file );
    QVector<T> blocks{};
    blocks.resize( pixelCount / 16 );
    assert( static_cast<qint64>( blocks.size() * sizeof( T ) ) <= file->size() );
    file->read( reinterpret_cast<char*>( blocks.data() ), sizeof( T ) * blocks.size() );
    file->close();
    return blocks;
}

static QVector<uint32_t> decompressBC( const DDSHeader& header, QFile* file )
{
    assert( file );
    assert( header.pixelFormat.flags == PixelFormat::fFourCC );

    // block compression requires texels with 4x4 extent
    if ( header.width % 4 || header.height % 4 ) {
        return {};
    }

    QVector<uint32_t> pixels{};
    pixels.resize( header.width * header.height );

    switch ( header.pixelFormat.fourCC ) {
    case '1TXD': {
        QVector<BC1> blocks = readBlocks<BC1>( file, header.width * header.height );
        std::generate( pixels.begin(), pixels.end(), Decompressor<BC1>( std::move( blocks ), header.width, header.height ) );
    } break;

    case '3TXD': {
        QVector<BC2> blocks = readBlocks<BC2>( file, header.width * header.height );
        std::generate( pixels.begin(), pixels.end(), Decompressor<BC2>( std::move( blocks ), header.width, header.height ) );
    } break;

    case '5TXD': {
        QVector<BC3> blocks = readBlocks<BC3>( file, header.width * header.height );
        std::generate( pixels.begin(), pixels.end(), Decompressor<BC3>( std::move( blocks ), header.width, header.height ) );
    } break;

    default:
        return {};
    }

    return pixels;
}

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

struct Deswizzler {
    uint32_t rMask = 0;
    uint32_t gMask = 0;
    uint32_t bMask = 0;
    uint32_t aMask = 0;
    uint8_t rShift = 0;
    uint8_t gShift = 0;
    uint8_t bShift = 0;
    uint8_t aShift = 0;
    uint8_t aFill = 0;

    Deswizzler( uint32_t rm, uint32_t gm, uint32_t bm, uint32_t am )
    : rMask( rm )
    , gMask( gm )
    , bMask( bm )
    , aMask( am )
    {
        assert( __builtin_popcount( rm ) == 8 || __builtin_popcount( rm ) == 0 );
        assert( __builtin_popcount( gm ) == 8 || __builtin_popcount( gm ) == 0 );
        assert( __builtin_popcount( bm ) == 8 || __builtin_popcount( bm ) == 0 );
        assert( __builtin_popcount( am ) == 8 || __builtin_popcount( am ) == 0 );
        rShift = rm ? __builtin_ctz( rm ) : 0;
        gShift = gm ? __builtin_ctz( gm ) : 0;
        bShift = bm ? __builtin_ctz( bm ) : 0;
        aShift = am ? __builtin_ctz( am ) : 0;
    }

    uint32_t operator () ( uint32_t v ) const
    {
        uint32_t r = ( v & rMask ) >> rShift;
        uint32_t g = ( v & gMask ) >> gShift;
        uint32_t b = ( v & bMask ) >> bShift;
        uint32_t a = ( v & aMask ) >> aShift;
        return ( ( aFill | a ) << 24 ) | ( r << 16 ) | ( g << 8 ) | b;
    }

    uint32_t operator () ( Byte3 v ) const
    {
        return operator ()( static_cast<uint32_t>( v ) );
    }
};

static QVector<uint32_t> extractUncompressedPixels( const DDSHeader& header, QFile* file )
{
    assert( file );
    if ( !( header.pixelFormat.flags & PixelFormat::fRGB ) ) {
        // non-color images not supported, maybe TODO
        return {};
    }

    Deswizzler deswizzler( header.pixelFormat.bitmaskR
        , header.pixelFormat.bitmaskG
        , header.pixelFormat.bitmaskB
        , header.pixelFormat.bitmaskA
    );

    QVector<uint32_t> pixels{};
    pixels.resize( header.width * header.height );

    switch ( header.pixelFormat.rgbBitCount ) {
    case 24: {
        QVector<Byte3> tmp{};
        tmp.resize( header.width * header.height );
        file->read( reinterpret_cast<char*>( tmp.data() ), tmp.size() * sizeof( Byte3 ) );
        deswizzler.aFill = 255;
        std::transform( tmp.begin(), tmp.end(), pixels.begin(), deswizzler );
        return pixels;
    }
    case 32:
        if ( !( header.pixelFormat.flags & ( PixelFormat::fAlpha | PixelFormat::fAlphaPixels ) ) ) {
            // TODO: suspicious pixel format
            return {};
        }
        file->read( reinterpret_cast<char*>( pixels.data() ), pixels.size() * sizeof( uint32_t ) );
        std::transform( pixels.begin(), pixels.end(), pixels.begin(), deswizzler );
        return pixels;

    default:
        // TODO: suspicious pixel format
        return {};

    }
}

bool DDSThumbnail::create( const QString& path, int width, int height, QImage& img )
{
    QFile file{ path };
    if ( file.size() < static_cast<qint32>( sizeof( DDSHeader ) ) ) {
        return false;
    }
    if ( !file.open( QIODevice::ReadOnly ) ) {
        return false;
    }

    DDSHeader header{};
    file.read( reinterpret_cast<char*>( &header ), sizeof( DDSHeader ) );
    if ( header.magic != DDSHeader::c_magic ) {
        return false;
    }

    static constexpr auto mustHaveFlags = DDSHeader::fCaps | DDSHeader::fHeight | DDSHeader::fWidth | DDSHeader::fPixelFormat;
    if ( ( header.flags & mustHaveFlags ) != mustHaveFlags ) {
        return false;
    }
    if ( ( header.caps & DDSHeader::Caps::fTexture ) != DDSHeader::Caps::fTexture ) {
        return false;
    }

    const bool isCompressed = header.pixelFormat.flags == PixelFormat::fFourCC;
    QVector<uint32_t> pixels = isCompressed
        ? decompressBC( header, &file )
        : extractUncompressedPixels( header, &file );

    if ( pixels.empty() ) {
#ifndef NDEBUG
        std::cerr << "failed to generate thumbnail for: " << path.toLocal8Bit().data() << std::endl;
#endif
        return false;
    }
    QImage image{ reinterpret_cast<const uchar*>( pixels.data() )
        , static_cast<int>( header.width )
        , static_cast<int>( header.height )
        , QImage::Format_ARGB32
        , nullptr // non-owning qimage
        , nullptr
    };
    img = image.scaled( width, height, Qt::KeepAspectRatio );
    return true;
}
