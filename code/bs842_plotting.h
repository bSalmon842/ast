/*
File: bs842_plotting.h
Author: Brock Salmon
Notice: Public Domain

// This software is dual-licensed to the public domain and under the following
// license: you are granted a perpetual, irrevocable license to copy, modify,
// publish, and distribute this file as you see fit.
*/

// TODO(bSalmon): Example docs
// TODO(bSalmon): Plot Data naming
// TODO(bSalmon): Data output in top right of plot
// TODO(bSalmon): Check stb stuff for paths that aren't used, minimise code footprint

#ifndef BS842_INCLUDE_PLOTTING_H

#ifdef __cplusplus
extern "C"
{
#endif
    
#ifndef BS842_FMOD
#include <math.h>
#define BS842_FMOD(val, modVal) fmod(val, modVal)
#endif
    
#ifndef BS842_STRLEN
#include <string.h>
#define BS842_STRLEN(string) strlen(string)
#endif
    
#ifndef BSDEF
#ifdef BS842_PLOTTING_STATIC
#define BSDEF static
#else
#define BSDEF extern
#endif
#endif
    
#ifndef BS842_ASSERT
#define BS842_ASSERT(check) if(!(check)) {*(int *)0 = 0;}
#endif
    
#ifndef BS842_ARRAY_COUNT
#define BS842_ARRAY_COUNT(array) (sizeof(array) / sizeof((array)[0]))
#endif
    
#ifndef BS842_MEMALLOC
#define BS842_MEMALLOC(size) calloc(1, size)
#define BS842_MEMFREE(mem) free(mem)
#endif
    
#ifndef BS842_MAX
#define BS842_MAX(a, b) (a >= b ? a : b)
#define BS842_MIN(a, b) (a <= b ? a : b)
#endif
    
#ifndef BS842_BPP
#define BS842_BPP 4
#endif
    
    // NOTE(bSalmon): stb_truetype structs
#ifndef STB_TRUETYPE_IMPLEMENTATION
#define STBTT_assert(x)    BS842_ASSERT(x)
    
    typedef unsigned char   stbtt_uint8;
    typedef signed   char   stbtt_int8;
    typedef unsigned short  stbtt_uint16;
    typedef signed   short  stbtt_int16;
    typedef unsigned int    stbtt_uint32;
    typedef signed   int    stbtt_int32;
    
    typedef struct
    {
        unsigned char *data;
        int cursor;
        int size;
    } stbtt__buf;
    
    struct stbtt_fontinfo
    {
        void           * userdata;
        unsigned char  * data;              // pointer to .ttf file
        int              fontstart;         // offset of start of font
        
        int numGlyphs;                     // number of glyphs, needed for range checking
        
        int loca,head,glyf,hhea,hmtx,kern,gpos,svg; // table locations as offset from start of .ttf
        int index_map;                     // a cmap mapping for our chosen character encoding
        int indexToLocFormat;              // format needed to map from glyph index to glyph
        
        stbtt__buf cff;                    // cff font data
        stbtt__buf charstrings;            // the charstring index
        stbtt__buf gsubrs;                 // global charstring subroutines index
        stbtt__buf subrs;                  // private charstring subroutines index
        stbtt__buf fontdicts;              // array of font dicts
        stbtt__buf fdselect;               // map from glyph to fontdict
    };
    
    enum { // platformID
        STBTT_PLATFORM_ID_UNICODE   =0,
        STBTT_PLATFORM_ID_MAC       =1,
        STBTT_PLATFORM_ID_ISO       =2,
        STBTT_PLATFORM_ID_MICROSOFT =3
    };
    
    enum { // encodingID for STBTT_PLATFORM_ID_MICROSOFT
        STBTT_MS_EID_SYMBOL        =0,
        STBTT_MS_EID_UNICODE_BMP   =1,
        STBTT_MS_EID_SHIFTJIS      =2,
        STBTT_MS_EID_UNICODE_FULL  =10
    };
    
#ifdef _MSC_VER
#define STBTT__NOTUSED(v)  (void)(v)
#else
#define STBTT__NOTUSED(v)  (void)sizeof(v)
#endif
    
#define stbtt_vertex_type short // can't use stbtt_int16 because that's not visible in the header file
    typedef struct
    {
        stbtt_vertex_type x,y,cx,cy,cx1,cy1;
        unsigned char type,padding;
    } stbtt_vertex;
    
    typedef struct
    {
        int bounds;
        int started;
        float first_x, first_y;
        float x, y;
        stbtt_int32 min_x, max_x, min_y, max_y;
        
        stbtt_vertex *pvertices;
        int num_vertices;
    } stbtt__csctx;
#define STBTT__CSCTX_INIT(bounds) {bounds,0, 0,0, 0,0, 0,0,0,0, NULL, 0}
    
#define STBTT_ifloor(x)   ((int) floor(x))
#define STBTT_iceil(x)    ((int) ceil(x))
    
    enum {
        STBTT_vmove=1,
        STBTT_vline,
        STBTT_vcurve,
    };
    
#define STBTT_sqrt(x)      sqrt(x)
#define STBTT_fabs(x)      fabs(x)
    
#define STBTT_malloc(x,u)  ((void)(u),malloc(x))
#define STBTT_free(x,u)    ((void)(u),free(x))
#define STBTT_memcpy       memcpy
#define STBTT_memset       memset
    
    typedef struct
    {
        int w,h,stride;
        unsigned char *pixels;
    } stbtt__bitmap;
    
    typedef struct
    {
        float x,y;
    } stbtt__point;
    
    typedef struct stbtt__edge {
        float x0,y0, x1,y1;
        int invert;
    } stbtt__edge;
    
#define STBTT__COMPARE(a,b)  ((a)->y0 < (b)->y0)
    
    typedef struct stbtt__hheap_chunk
    {
        struct stbtt__hheap_chunk *next;
    } stbtt__hheap_chunk;
    
    typedef struct stbtt__hheap
    {
        struct stbtt__hheap_chunk *head;
        void   *first_free;
        int    num_remaining_in_head_chunk;
    } stbtt__hheap;
    
    typedef struct stbtt__active_edge
    {
        struct stbtt__active_edge *next;
        float fx,fdx,fdy;
        float direction;
        float sy;
        float ey;
        
    } stbtt__active_edge;
    
#define ttBYTE(p)     (* (stbtt_uint8 *) (p))
#define ttCHAR(p)     (* (stbtt_int8 *) (p))
    
    static stbtt_uint16 ttUSHORT(stbtt_uint8 *p) { return p[0]*256 + p[1]; }
    static stbtt_int16 ttSHORT(stbtt_uint8 *p)   { return p[0]*256 + p[1]; }
    static stbtt_uint32 ttULONG(stbtt_uint8 *p)  { return (p[0]<<24) + (p[1]<<16) + (p[2]<<8) + p[3]; }
#endif
    
    struct BS842_Internal_V2F
    {
        union
        {
            float e[2];
            struct {float x; float y;};
        };
    };
    
    struct BS842_Internal_V2I
    {
        union
        {
            int e[2];
            struct {int x; int y;};
        };
    };
    
    enum BS842_Internal_PlotAxis
    {
        PlotAxis_X,
        PlotAxis_Y,
    };
    
    enum BS842_PlotOption
    {
        PlotOpt_MinValX,
        PlotOpt_MaxValX,
        PlotOpt_MinValY,
        PlotOpt_MaxValY,
        
        PlotOpt_MinorX,
        PlotOpt_MajorX,
        PlotOpt_MinorY,
        PlotOpt_MajorY,
        
        PlotOpt_Colour1,
        PlotOpt_Colour2,
        PlotOpt_ColourBack,
        PlotOpt_ColourSub,
        PlotOpt_ColourMargin,
        PlotOpt_ColourBorders,
        PlotOpt_ColourCursor,
        PlotOpt_ColourText,
        
        PlotOpt_MarginLeft,
        PlotOpt_MarginRight,
        PlotOpt_MarginBottom,
        PlotOpt_MarginTop,
        
        PlotOpt_Count,
    };
    
    typedef struct
    {
        int width;
        int height;
        int pitch;
        void *memory;
    } BS842_Plotting_Internal_BackBuffer;
    
    typedef struct
    {
        float *xData;
        float *yData;
        int datumCount;
    } BS842_DataSet;
    
    typedef struct
    {
        bool enabled;
        
        char title[32];
        
        BS842_Plotting_Internal_BackBuffer backBuffer;
        BS842_Internal_V2I pos;
        
        void *optionValues; // 32bit values
        
        stbtt_fontinfo fontInfo;
        
        BS842_DataSet data[2];
        
        int cursor1Pos;
        int cursor2Pos;
        bool highlightActive;
        
        BS842_Internal_V2I mousePosRel;
    } BS842_Plot;
    
    typedef struct
    {
        bool initialised;
        BS842_Plot plots[8];
        BS842_Internal_V2I mousePos;
        bool mouseLeftDown;
        bool mouseRightDown;
    } BS842_Plotting_Internal_Info;
    
    static BS842_Plotting_Internal_Info bs842_plIntInfo = {};
    
    BSDEF void BS842_Plotting_Init(int plotCount, int plotWidth, int plotHeight, char *fontPath);
    
    BSDEF void BS842_Plotting_ChangePlotOption(int plotIndex, BS842_PlotOption option);
    BSDEF void BS842_Plotting_GetPlotOption(void *result, int plotIndex, BS842_PlotOption option);
    
    BSDEF void BS842_Plotting_PlotData(int plotIndex, s32 dataSetIndex, float *xData, float *yData, int datumCount);
    BSDEF void BS842_Plotting_PlotTitle(int plotIndex, char *title);
    
    BSDEF void BS842_Plotting_UpdatePlot(int plotIndex);
    BSDEF void BS842_Plotting_ResizePlot(int plotIndex, int width, int height, int posX, int posY);
    BSDEF void BS842_Plotting_DrawPlotMemory(int plotIndex, void *destBuffer, int destPitch);
    
    BSDEF void BS842_Plotting_MouseInfo(int mouseX, int mouseY, bool mouseLeftDown);
    
#ifdef __cplusplus
}
#endif

#define BS842_INCLUDE_PLOTTING_H
#endif

#ifdef BS842_PLOTTING_IMPLEMENTATION

#ifndef STB_TRUETYPE_IMPLEMENTATION

static stbtt_uint8 stbtt__buf_get8(stbtt__buf *b)
{
    if (b->cursor >= b->size)
        return 0;
    return b->data[b->cursor++];
}

static stbtt_uint8 stbtt__buf_peek8(stbtt__buf *b)
{
    if (b->cursor >= b->size)
        return 0;
    return b->data[b->cursor];
}

static void stbtt__buf_seek(stbtt__buf *b, int o)
{
    STBTT_assert(!(o > b->size || o < 0));
    b->cursor = (o > b->size || o < 0) ? b->size : o;
}

static stbtt_uint32 stbtt__buf_get(stbtt__buf *b, int n)
{
    stbtt_uint32 v = 0;
    int i;
    STBTT_assert(n >= 1 && n <= 4);
    for (i = 0; i < n; i++)
        v = (v << 8) | stbtt__buf_get8(b);
    return v;
}

static stbtt__buf stbtt__new_buf(const void *p, size_t size)
{
    stbtt__buf r;
    STBTT_assert(size < 0x40000000);
    r.data = (stbtt_uint8*) p;
    r.size = (int) size;
    r.cursor = 0;
    return r;
}

#define stbtt__buf_get16(b)  stbtt__buf_get((b), 2)
#define stbtt__buf_get32(b)  stbtt__buf_get((b), 4)

static stbtt__buf stbtt__buf_range(const stbtt__buf *b, int o, int s)
{
    stbtt__buf r = stbtt__new_buf(NULL, 0);
    if (o < 0 || s < 0 || o > b->size || s > b->size - o) return r;
    r.data = b->data + o;
    r.size = s;
    return r;
}

static stbtt__buf stbtt__cff_get_index(stbtt__buf *b)
{
    int count, start, offsize;
    start = b->cursor;
    count = stbtt__buf_get16(b);
    if (count) {
        offsize = stbtt__buf_get8(b);
        STBTT_assert(offsize >= 1 && offsize <= 4);
        stbtt__buf_seek(b, b->cursor + (offsize * count));
        stbtt__buf_seek(b, b->cursor + (stbtt__buf_get(b, offsize) - 1));
    }
    return stbtt__buf_range(b, start, b->cursor - start);
}

static stbtt_uint32 stbtt__cff_int(stbtt__buf *b)
{
    int b0 = stbtt__buf_get8(b);
    if (b0 >= 32 && b0 <= 246)       return b0 - 139;
    else if (b0 >= 247 && b0 <= 250) return (b0 - 247)*256 + stbtt__buf_get8(b) + 108;
    else if (b0 >= 251 && b0 <= 254) return -(b0 - 251)*256 - stbtt__buf_get8(b) - 108;
    else if (b0 == 28)               return stbtt__buf_get16(b);
    else if (b0 == 29)               return stbtt__buf_get32(b);
    STBTT_assert(0);
    return 0;
}

static void stbtt__dict_get_ints(stbtt__buf *b, int key, int outcount, stbtt_uint32 *out)
{
    int i;
    stbtt__buf operands = {};
    stbtt__buf_seek(b, 0);
    while (b->cursor < b->size) {
        int start = b->cursor, end, op;
        while (stbtt__buf_peek8(b) >= 28)
        {
            int v, b0 = stbtt__buf_peek8(b);
            STBTT_assert(b0 >= 28);
            if (b0 == 30) {
                stbtt__buf_seek(b, b->cursor + 1);
                while (b->cursor < b->size) {
                    v = stbtt__buf_get8(b);
                    if ((v & 0xF) == 0xF || (v >> 4) == 0xF)
                        break;
                }
            } else {
                stbtt__cff_int(b);
            }
        }
        end = b->cursor;
        op = stbtt__buf_get8(b);
        if (op == 12)  op = stbtt__buf_get8(b) | 0x100;
        if (op == key) operands = stbtt__buf_range(b, start, end-start);
    }
    if (operands.data == 0)
    {
        operands = stbtt__buf_range(b, 0, 0);
    }
    for (i = 0; i < outcount && operands.cursor < operands.size; i++)
        out[i] = stbtt__cff_int(&operands);
}

static stbtt__buf stbtt__cff_index_get(stbtt__buf b, int i)
{
    int count, offsize, start, end;
    stbtt__buf_seek(&b, 0);
    count = stbtt__buf_get16(&b);
    offsize = stbtt__buf_get8(&b);
    STBTT_assert(i >= 0 && i < count);
    STBTT_assert(offsize >= 1 && offsize <= 4);
    stbtt__buf_seek(&b, b.cursor + (i*offsize));
    start = stbtt__buf_get(&b, offsize);
    end = stbtt__buf_get(&b, offsize);
    return stbtt__buf_range(&b, 2+(count+1)*offsize+start, end - start);
}

static stbtt_uint32 stbtt__find_table(stbtt_uint8 *data, stbtt_uint32 fontstart, const char *tag)
{
    stbtt_int32 num_tables = ttUSHORT(data+fontstart+4);
    stbtt_uint32 tabledir = fontstart + 12;
    stbtt_int32 i;
    for (i=0; i < num_tables; ++i) {
        stbtt_uint32 loc = tabledir + 16*i;
        if ((data+loc)[0] == tag[0] && (data+loc)[1] == tag[1] && (data+loc)[2] == tag[2] && (data+loc)[3] == tag[3])
            return ttULONG(data+loc+8);
    }
    return 0;
}

static stbtt__buf stbtt__get_subrs(stbtt__buf cff, stbtt__buf fontdict)
{
    stbtt_uint32 subrsoff = 0, private_loc[2] = { 0, 0 };
    stbtt__buf pdict;
    stbtt__dict_get_ints(&fontdict, 18, 2, private_loc);
    if (!private_loc[1] || !private_loc[0]) return stbtt__new_buf(NULL, 0);
    pdict = stbtt__buf_range(&cff, private_loc[1], private_loc[0]);
    stbtt__dict_get_ints(&pdict, 19, 1, &subrsoff);
    if (!subrsoff) return stbtt__new_buf(NULL, 0);
    stbtt__buf_seek(&cff, private_loc[1]+subrsoff);
    return stbtt__cff_get_index(&cff);
}

static int stbtt_InitFont(stbtt_fontinfo *info, unsigned char *data, int fontstart)
{
    stbtt_uint32 cmap, t;
    stbtt_int32 i,numTables;
    
    info->data = data;
    info->fontstart = fontstart;
    info->cff = stbtt__new_buf(NULL, 0);
    
    cmap = stbtt__find_table(data, fontstart, "cmap");       // required
    info->loca = stbtt__find_table(data, fontstart, "loca"); // required
    info->head = stbtt__find_table(data, fontstart, "head"); // required
    info->glyf = stbtt__find_table(data, fontstart, "glyf"); // required
    info->hhea = stbtt__find_table(data, fontstart, "hhea"); // required
    info->hmtx = stbtt__find_table(data, fontstart, "hmtx"); // required
    info->kern = stbtt__find_table(data, fontstart, "kern"); // not required
    info->gpos = stbtt__find_table(data, fontstart, "GPOS"); // not required
    
    if (!cmap || !info->head || !info->hhea || !info->hmtx)
        return 0;
    if (info->glyf) {
        // required for truetype
        if (!info->loca) return 0;
    } else {
        // initialization for CFF / Type2 fonts (OTF)
        stbtt__buf b, topdict, topdictidx;
        stbtt_uint32 cstype = 2, charstrings = 0, fdarrayoff = 0, fdselectoff = 0;
        stbtt_uint32 cff;
        
        cff = stbtt__find_table(data, fontstart, "CFF ");
        if (!cff) return 0;
        
        info->fontdicts = stbtt__new_buf(NULL, 0);
        info->fdselect = stbtt__new_buf(NULL, 0);
        
        // @TODO this should use size from table (not 512MB)
        info->cff = stbtt__new_buf(data+cff, 512*1024*1024);
        b = info->cff;
        
        // read the header
        stbtt__buf_seek(&b, b.cursor + 2);
        stbtt__buf_seek(&b, stbtt__buf_get8(&b)); // hdrsize
        
        // @TODO the name INDEX could list multiple fonts,
        // but we just use the first one.
        stbtt__cff_get_index(&b);  // name INDEX
        topdictidx = stbtt__cff_get_index(&b);
        topdict = stbtt__cff_index_get(topdictidx, 0);
        stbtt__cff_get_index(&b);  // string INDEX
        info->gsubrs = stbtt__cff_get_index(&b);
        
        stbtt__dict_get_ints(&topdict, 17, 1, &charstrings);
        stbtt__dict_get_ints(&topdict, 0x100 | 6, 1, &cstype);
        stbtt__dict_get_ints(&topdict, 0x100 | 36, 1, &fdarrayoff);
        stbtt__dict_get_ints(&topdict, 0x100 | 37, 1, &fdselectoff);
        info->subrs = stbtt__get_subrs(b, topdict);
        
        // we only support Type 2 charstrings
        if (cstype != 2) return 0;
        if (charstrings == 0) return 0;
        
        if (fdarrayoff) {
            // looks like a CID font
            if (!fdselectoff) return 0;
            stbtt__buf_seek(&b, fdarrayoff);
            info->fontdicts = stbtt__cff_get_index(&b);
            info->fdselect = stbtt__buf_range(&b, fdselectoff, b.size-fdselectoff);
        }
        
        stbtt__buf_seek(&b, charstrings);
        info->charstrings = stbtt__cff_get_index(&b);
    }
    
    t = stbtt__find_table(data, fontstart, "maxp");
    if (t)
        info->numGlyphs = ttUSHORT(data+t+4);
    else
        info->numGlyphs = 0xffff;
    
    info->svg = -1;
    
    // find a cmap encoding table we understand *now* to avoid searching
    // later. (todo: could make this installable)
    // the same regardless of glyph.
    numTables = ttUSHORT(data + cmap + 2);
    info->index_map = 0;
    for (i=0; i < numTables; ++i) {
        stbtt_uint32 encoding_record = cmap + 4 + 8 * i;
        // find an encoding we understand:
        switch(ttUSHORT(data+encoding_record)) {
            case STBTT_PLATFORM_ID_MICROSOFT:
            switch (ttUSHORT(data+encoding_record+2)) {
                case STBTT_MS_EID_UNICODE_BMP:
                case STBTT_MS_EID_UNICODE_FULL:
                // MS/Unicode
                info->index_map = cmap + ttULONG(data+encoding_record+4);
                break;
            }
            break;
            case STBTT_PLATFORM_ID_UNICODE:
            // Mac/iOS has these
            // all the encodingIDs are unicode, so we don't bother to check it
            info->index_map = cmap + ttULONG(data+encoding_record+4);
            break;
        }
    }
    if (info->index_map == 0)
        return 0;
    
    info->indexToLocFormat = ttUSHORT(data+info->head + 50);
    return 1;
}

static int stbtt_FindGlyphIndex(const stbtt_fontinfo *info, int unicode_codepoint)
{
    stbtt_uint8 *data = info->data;
    stbtt_uint32 index_map = info->index_map;
    
    stbtt_uint16 format = ttUSHORT(data + index_map + 0);
    if (format == 0) { // apple byte encoding
        stbtt_int32 bytes = ttUSHORT(data + index_map + 2);
        if (unicode_codepoint < bytes-6)
            return ttBYTE(data + index_map + 6 + unicode_codepoint);
        return 0;
    } else if (format == 6) {
        stbtt_uint32 first = ttUSHORT(data + index_map + 6);
        stbtt_uint32 count = ttUSHORT(data + index_map + 8);
        if ((stbtt_uint32) unicode_codepoint >= first && (stbtt_uint32) unicode_codepoint < first+count)
            return ttUSHORT(data + index_map + 10 + (unicode_codepoint - first)*2);
        return 0;
    } else if (format == 2) {
        STBTT_assert(0); // @TODO: high-byte mapping for japanese/chinese/korean
        return 0;
    } else if (format == 4) { // standard mapping for windows fonts: binary search collection of ranges
        stbtt_uint16 segcount = ttUSHORT(data+index_map+6) >> 1;
        stbtt_uint16 searchRange = ttUSHORT(data+index_map+8) >> 1;
        stbtt_uint16 entrySelector = ttUSHORT(data+index_map+10);
        stbtt_uint16 rangeShift = ttUSHORT(data+index_map+12) >> 1;
        
        // do a binary search of the segments
        stbtt_uint32 endCount = index_map + 14;
        stbtt_uint32 search = endCount;
        
        if (unicode_codepoint > 0xffff)
            return 0;
        
        // they lie from endCount .. endCount + segCount
        // but searchRange is the nearest power of two, so...
        if (unicode_codepoint >= ttUSHORT(data + search + rangeShift*2))
            search += rangeShift*2;
        
        // now decrement to bias correctly to find smallest
        search -= 2;
        while (entrySelector) {
            stbtt_uint16 end;
            searchRange >>= 1;
            end = ttUSHORT(data + search + searchRange*2);
            if (unicode_codepoint > end)
                search += searchRange*2;
            --entrySelector;
        }
        search += 2;
        
        {
            stbtt_uint16 offset, start;
            stbtt_uint16 item = (stbtt_uint16) ((search - endCount) >> 1);
            
            STBTT_assert(unicode_codepoint <= ttUSHORT(data + endCount + 2*item));
            start = ttUSHORT(data + index_map + 14 + segcount*2 + 2 + 2*item);
            if (unicode_codepoint < start)
                return 0;
            
            offset = ttUSHORT(data + index_map + 14 + segcount*6 + 2 + 2*item);
            if (offset == 0)
                return (stbtt_uint16) (unicode_codepoint + ttSHORT(data + index_map + 14 + segcount*4 + 2 + 2*item));
            
            return ttUSHORT(data + offset + (unicode_codepoint-start)*2 + index_map + 14 + segcount*6 + 2 + 2*item);
        }
    } else if (format == 12 || format == 13) {
        stbtt_uint32 ngroups = ttULONG(data+index_map+12);
        stbtt_int32 low,high;
        low = 0; high = (stbtt_int32)ngroups;
        // Binary search the right group.
        while (low < high) {
            stbtt_int32 mid = low + ((high-low) >> 1); // rounds down, so low <= mid < high
            stbtt_uint32 start_char = ttULONG(data+index_map+16+mid*12);
            stbtt_uint32 end_char = ttULONG(data+index_map+16+mid*12+4);
            if ((stbtt_uint32) unicode_codepoint < start_char)
                high = mid;
            else if ((stbtt_uint32) unicode_codepoint > end_char)
                low = mid+1;
            else {
                stbtt_uint32 start_glyph = ttULONG(data+index_map+16+mid*12+8);
                if (format == 12)
                    return start_glyph + unicode_codepoint-start_char;
                else // format == 13
                    return start_glyph;
            }
        }
        return 0; // not found
    }
    // @TODO
    STBTT_assert(0);
    return 0;
}

static stbtt_int32 stbtt__GetGlyphClass(stbtt_uint8 *classDefTable, int glyph)
{
    stbtt_uint16 classDefFormat = ttUSHORT(classDefTable);
    switch(classDefFormat)
    {
        case 1: {
            stbtt_uint16 startGlyphID = ttUSHORT(classDefTable + 2);
            stbtt_uint16 glyphCount = ttUSHORT(classDefTable + 4);
            stbtt_uint8 *classDef1ValueArray = classDefTable + 6;
            
            if (glyph >= startGlyphID && glyph < startGlyphID + glyphCount)
                return (stbtt_int32)ttUSHORT(classDef1ValueArray + 2 * (glyph - startGlyphID));
            
            classDefTable = classDef1ValueArray + 2 * glyphCount;
        } break;
        
        case 2: {
            stbtt_uint16 classRangeCount = ttUSHORT(classDefTable + 2);
            stbtt_uint8 *classRangeRecords = classDefTable + 4;
            
            // Binary search.
            stbtt_int32 l=0, r=classRangeCount-1, m;
            int strawStart, strawEnd, needle=glyph;
            while (l <= r) {
                stbtt_uint8 *classRangeRecord;
                m = (l + r) >> 1;
                classRangeRecord = classRangeRecords + 6 * m;
                strawStart = ttUSHORT(classRangeRecord);
                strawEnd = ttUSHORT(classRangeRecord + 2);
                if (needle < strawStart)
                    r = m - 1;
                else if (needle > strawEnd)
                    l = m + 1;
                else
                    return (stbtt_int32)ttUSHORT(classRangeRecord + 4);
            }
            
            classDefTable = classRangeRecords + 6 * classRangeCount;
        } break;
        
        default: {
            // There are no other cases.
            STBTT_assert(0);
        } break;
    }
    
    return -1;
}

static int stbtt_GetCodepointKernAdvance(const stbtt_fontinfo *info, int ch1, int ch2)
{
    int result = 0;
    if (info->kern || info->gpos) // if no kerning table, don't waste time looking up both codepoint->glyphs
    {
        int xAdvance = 0;
        
        int g1 = stbtt_FindGlyphIndex(info,ch1);
        int g2 = stbtt_FindGlyphIndex(info,ch2);
        
        if (info->gpos)
        {
            int advanceAmount = 0;
            
            stbtt_uint16 lookupListOffset;
            stbtt_uint8 *lookupList;
            stbtt_uint16 lookupCount;
            stbtt_uint8 *data;
            stbtt_int32 i;
            
            if (info->gpos)
            {
                data = info->data + info->gpos;
                
                bool exitOut = false;
                
                if (ttUSHORT(data+0) != 1)
                {
                    exitOut = true;
                }
                if (ttUSHORT(data+2) != 0)
                {
                    exitOut = true;
                }
                
                lookupListOffset = ttUSHORT(data+8);
                lookupList = data + lookupListOffset;
                lookupCount = ttUSHORT(lookupList);
                
                for (i=0; i<lookupCount && !exitOut; ++i) {
                    stbtt_uint16 lookupOffset = ttUSHORT(lookupList + 2 + 2 * i);
                    stbtt_uint8 *lookupTable = lookupList + lookupOffset;
                    
                    stbtt_uint16 lookupType = ttUSHORT(lookupTable);
                    stbtt_uint16 subTableCount = ttUSHORT(lookupTable + 4);
                    stbtt_uint8 *subTableOffsets = lookupTable + 6;
                    switch(lookupType) {
                        case 2: { // Pair Adjustment Positioning Subtable
                            stbtt_int32 sti;
                            for (sti=0; sti<subTableCount && !exitOut; sti++) {
                                stbtt_uint16 subtableOffset = ttUSHORT(subTableOffsets + 2 * sti);
                                stbtt_uint8 *table = lookupTable + subtableOffset;
                                stbtt_uint16 posFormat = ttUSHORT(table);
                                stbtt_uint16 coverageOffset = ttUSHORT(table + 2);
                                
                                stbtt_uint8 *coverageTable = table + coverageOffset;
                                stbtt_int32 coverageIndex = -1;
                                stbtt_uint16 coverageFormat = ttUSHORT(coverageTable);
                                switch(coverageFormat) {
                                    case 1: {
                                        stbtt_uint16 glyphCount = ttUSHORT(coverageTable + 2);
                                        
                                        // Binary search.
                                        stbtt_int32 l=0, r=glyphCount-1, m;
                                        int straw, needle=g1;
                                        while (l <= r) {
                                            stbtt_uint8 *glyphArray = coverageTable + 4;
                                            stbtt_uint16 glyphID;
                                            m = (l + r) >> 1;
                                            glyphID = ttUSHORT(glyphArray + 2 * m);
                                            straw = glyphID;
                                            if (needle < straw)
                                                r = m - 1;
                                            else if (needle > straw)
                                                l = m + 1;
                                            else {
                                                coverageIndex = m;
                                                break;
                                            }
                                        }
                                    } break;
                                    
                                    case 2: {
                                        stbtt_uint16 rangeCount = ttUSHORT(coverageTable + 2);
                                        stbtt_uint8 *rangeArray = coverageTable + 4;
                                        
                                        // Binary search.
                                        stbtt_int32 l=0, r=rangeCount-1, m;
                                        int strawStart, strawEnd, needle=g1;
                                        while (l <= r) {
                                            stbtt_uint8 *rangeRecord;
                                            m = (l + r) >> 1;
                                            rangeRecord = rangeArray + 6 * m;
                                            strawStart = ttUSHORT(rangeRecord);
                                            strawEnd = ttUSHORT(rangeRecord + 2);
                                            if (needle < strawStart)
                                                r = m - 1;
                                            else if (needle > strawEnd)
                                                l = m + 1;
                                            else {
                                                stbtt_uint16 startCoverageIndex = ttUSHORT(rangeRecord + 4);
                                                coverageIndex = startCoverageIndex + g1 - strawStart;
                                            }
                                        }
                                    } break;
                                    
                                    default: {
                                        // There are no other cases.
                                        STBTT_assert(0);
                                    } break;
                                }
                                
                                if (coverageIndex == -1) continue;
                                
                                switch (posFormat) {
                                    case 1: {
                                        stbtt_int32 l, r, m;
                                        int straw, needle;
                                        stbtt_uint16 valueFormat1 = ttUSHORT(table + 4);
                                        stbtt_uint16 valueFormat2 = ttUSHORT(table + 6);
                                        stbtt_int32 valueRecordPairSizeInBytes = 2;
                                        stbtt_uint16 pairSetCount = ttUSHORT(table + 8);
                                        stbtt_uint16 pairPosOffset = ttUSHORT(table + 10 + 2 * coverageIndex);
                                        stbtt_uint8 *pairValueTable = table + pairPosOffset;
                                        stbtt_uint16 pairValueCount = ttUSHORT(pairValueTable);
                                        stbtt_uint8 *pairValueArray = pairValueTable + 2;
                                        // TODO: Support more formats.
                                        if (valueFormat1 != 4)
                                        {
                                            exitOut = true;
                                            break;
                                        }
                                        if (valueFormat2 != 0)
                                        {
                                            exitOut = true;
                                            break;
                                        }
                                        
                                        STBTT_assert(coverageIndex < pairSetCount);
                                        STBTT__NOTUSED(pairSetCount);
                                        
                                        needle=g2;
                                        r=pairValueCount-1;
                                        l=0;
                                        
                                        // Binary search.
                                        while (l <= r) {
                                            stbtt_uint16 secondGlyph;
                                            stbtt_uint8 *pairValue;
                                            m = (l + r) >> 1;
                                            pairValue = pairValueArray + (2 + valueRecordPairSizeInBytes) * m;
                                            secondGlyph = ttUSHORT(pairValue);
                                            straw = secondGlyph;
                                            if (needle < straw)
                                                r = m - 1;
                                            else if (needle > straw)
                                                l = m + 1;
                                            else {
                                                advanceAmount = ttSHORT(pairValue + 2);
                                                exitOut = true;
                                            }
                                        }
                                    } break;
                                    
                                    case 2: {
                                        stbtt_uint16 valueFormat1 = ttUSHORT(table + 4);
                                        stbtt_uint16 valueFormat2 = ttUSHORT(table + 6);
                                        
                                        stbtt_uint16 classDef1Offset = ttUSHORT(table + 8);
                                        stbtt_uint16 classDef2Offset = ttUSHORT(table + 10);
                                        int glyph1class = stbtt__GetGlyphClass(table + classDef1Offset, g1);
                                        int glyph2class = stbtt__GetGlyphClass(table + classDef2Offset, g2);
                                        
                                        stbtt_uint16 class1Count = ttUSHORT(table + 12);
                                        stbtt_uint16 class2Count = ttUSHORT(table + 14);
                                        STBTT_assert(glyph1class < class1Count);
                                        STBTT_assert(glyph2class < class2Count);
                                        
                                        // TODO: Support more formats.
                                        if (valueFormat1 != 4)
                                        {
                                            exitOut = true;
                                            break;
                                        }
                                        if (valueFormat2 != 0)
                                        {
                                            exitOut = true;
                                            break;
                                        }
                                        
                                        if (glyph1class >= 0 && glyph1class < class1Count && glyph2class >= 0 && glyph2class < class2Count) {
                                            stbtt_uint8 *class1Records = table + 16;
                                            stbtt_uint8 *class2Records = class1Records + 2 * (glyph1class * class2Count);
                                            advanceAmount = ttSHORT(class2Records + 2 * glyph2class);
                                            exitOut = true;
                                        }
                                    } break;
                                    
                                    default: {
                                        // There are no other cases.
                                        STBTT_assert(0);
                                        break;
                                    };
                                }
                            }
                            break;
                        };
                        
                        default:
                        // TODO: Implement other stuff.
                        break;
                    }
                }
            }
            
            xAdvance += advanceAmount;
        }
        else if (info->kern)
        {
            int advanceAmount = 0;
            
            stbtt_uint8 *data = info->data + info->kern;
            stbtt_uint32 needle, straw;
            int l, r, m;
            
            // we only look at the first table. it must be 'horizontal' and format 0.
            bool earlyOut = false;
            if (!info->kern)
                earlyOut = true;
            if (ttUSHORT(data+2) < 1) // number of tables, need at least 1
                earlyOut = true;
            if (ttUSHORT(data+8) != 1) // horizontal flag must be set in format
                earlyOut = true;
            
            if (!earlyOut)
            {
                l = 0;
                r = ttUSHORT(data+10) - 1;
                needle = g1 << 16 | g2;
                while (l <= r) {
                    m = (l + r) >> 1;
                    straw = ttULONG(data+18+(m*6)); // note: unaligned read
                    if (needle < straw)
                        r = m - 1;
                    else if (needle > straw)
                        l = m + 1;
                    else
                    {
                        advanceAmount = ttSHORT(data+22+(m*6));
                        break;
                    }
                }
            }
            
            xAdvance += advanceAmount;
        }
        
        result = xAdvance;
    }
    
    return result;
}

static void stbtt_GetCodepointHMetrics(const stbtt_fontinfo *info, int codepoint, int *advanceWidth, int *leftSideBearing)
{
    int glyph_index = stbtt_FindGlyphIndex(info,codepoint);
    stbtt_uint16 numOfLongHorMetrics = ttUSHORT(info->data+info->hhea + 34);
    if (glyph_index < numOfLongHorMetrics) {
        if (advanceWidth)     *advanceWidth    = ttSHORT(info->data + info->hmtx + 4*glyph_index);
        if (leftSideBearing)  *leftSideBearing = ttSHORT(info->data + info->hmtx + 4*glyph_index + 2);
    } else {
        if (advanceWidth)     *advanceWidth    = ttSHORT(info->data + info->hmtx + 4*(numOfLongHorMetrics-1));
        if (leftSideBearing)  *leftSideBearing = ttSHORT(info->data + info->hmtx + 4*numOfLongHorMetrics + 2*(glyph_index - numOfLongHorMetrics));
    }
}

static void stbtt_setvertex(stbtt_vertex *v, stbtt_uint8 type, stbtt_int32 x, stbtt_int32 y, stbtt_int32 cx, stbtt_int32 cy)
{
    v->type = type;
    v->x = (stbtt_int16) x;
    v->y = (stbtt_int16) y;
    v->cx = (stbtt_int16) cx;
    v->cy = (stbtt_int16) cy;
}

static int stbtt__GetGlyfOffset(const stbtt_fontinfo *info, int glyph_index)
{
    int g1,g2;
    
    STBTT_assert(!info->cff.size);
    
    if (glyph_index >= info->numGlyphs) return -1; // glyph index out of range
    if (info->indexToLocFormat >= 2)    return -1; // unknown index->glyph map format
    
    if (info->indexToLocFormat == 0) {
        g1 = info->glyf + ttUSHORT(info->data + info->loca + glyph_index * 2) * 2;
        g2 = info->glyf + ttUSHORT(info->data + info->loca + glyph_index * 2 + 2) * 2;
    } else {
        g1 = info->glyf + ttULONG (info->data + info->loca + glyph_index * 4);
        g2 = info->glyf + ttULONG (info->data + info->loca + glyph_index * 4 + 4);
    }
    
    return g1==g2 ? -1 : g1; // if length is 0, return -1
}

static void stbtt__track_vertex(stbtt__csctx *c, stbtt_int32 x, stbtt_int32 y)
{
    if (x > c->max_x || !c->started) c->max_x = x;
    if (y > c->max_y || !c->started) c->max_y = y;
    if (x < c->min_x || !c->started) c->min_x = x;
    if (y < c->min_y || !c->started) c->min_y = y;
    c->started = 1;
}

static void stbtt__csctx_v(stbtt__csctx *c, stbtt_uint8 type, stbtt_int32 x, stbtt_int32 y, stbtt_int32 cx, stbtt_int32 cy, stbtt_int32 cx1, stbtt_int32 cy1)
{
    if (c->bounds) {
        stbtt__track_vertex(c, x, y);
    } else {
        stbtt_setvertex(&c->pvertices[c->num_vertices], type, x, y, cx, cy);
        c->pvertices[c->num_vertices].cx1 = (stbtt_int16) cx1;
        c->pvertices[c->num_vertices].cy1 = (stbtt_int16) cy1;
    }
    c->num_vertices++;
}

static void stbtt__csctx_close_shape(stbtt__csctx *ctx)
{
    if (ctx->first_x != ctx->x || ctx->first_y != ctx->y)
        stbtt__csctx_v(ctx, STBTT_vline, (int)ctx->first_x, (int)ctx->first_y, 0, 0, 0, 0);
}

static void stbtt__csctx_rmove_to(stbtt__csctx *ctx, float dx, float dy)
{
    stbtt__csctx_close_shape(ctx);
    ctx->first_x = ctx->x = ctx->x + dx;
    ctx->first_y = ctx->y = ctx->y + dy;
    stbtt__csctx_v(ctx, STBTT_vmove, (int)ctx->x, (int)ctx->y, 0, 0, 0, 0);
}

static void stbtt__csctx_rline_to(stbtt__csctx *ctx, float dx, float dy)
{
    ctx->x += dx;
    ctx->y += dy;
    stbtt__csctx_v(ctx, STBTT_vline, (int)ctx->x, (int)ctx->y, 0, 0, 0, 0);
}

static int stbtt__run_charstring(const stbtt_fontinfo *info, int glyph_index, stbtt__csctx *c)
{
    int in_header = 1, maskbits = 0, subr_stack_height = 0, sp = 0, v, i, b0;
    int has_subrs = 0, clear_stack;
    float s[48];
    stbtt__buf subr_stack[10], subrs = info->subrs, b;
    float f;
    
#define STBTT__CSERR(s) (0)
    
    // this currently ignores the initial width value, which isn't needed if we have hmtx
    b = stbtt__cff_index_get(info->charstrings, glyph_index);
    while (b.cursor < b.size) {
        i = 0;
        clear_stack = 1;
        b0 = stbtt__buf_get8(&b);
        switch (b0) {
            // @TODO implement hinting
            case 0x13: // hintmask
            case 0x14: // cntrmask
            if (in_header)
                maskbits += (sp / 2); // implicit "vstem"
            in_header = 0;
            stbtt__buf_seek(&b, b.cursor + ((maskbits + 7) / 8));
            break;
            
            case 0x01: // hstem
            case 0x03: // vstem
            case 0x12: // hstemhm
            case 0x17: // vstemhm
            maskbits += (sp / 2);
            break;
            
            case 0x15: // rmoveto
            in_header = 0;
            if (sp < 2) return STBTT__CSERR("rmoveto stack");
            stbtt__csctx_rmove_to(c, s[sp-2], s[sp-1]);
            break;
            case 0x04: // vmoveto
            in_header = 0;
            if (sp < 1) return STBTT__CSERR("vmoveto stack");
            stbtt__csctx_rmove_to(c, 0, s[sp-1]);
            break;
            case 0x16: // hmoveto
            in_header = 0;
            if (sp < 1) return STBTT__CSERR("hmoveto stack");
            stbtt__csctx_rmove_to(c, s[sp-1], 0);
            break;
            
            case 0x05: // rlineto
            if (sp < 2) return STBTT__CSERR("rlineto stack");
            for (; i + 1 < sp; i += 2)
                stbtt__csctx_rline_to(c, s[i], s[i+1]);
            break;
            
            // hlineto/vlineto and vhcurveto/hvcurveto alternate horizontal and vertical
            // starting from a different place.
            
            case 0x07: // vlineto
            if (sp < 1) return STBTT__CSERR("vlineto stack");
            goto vlineto;
            case 0x06: // hlineto
            if (sp < 1) return STBTT__CSERR("hlineto stack");
            for (;;) {
                if (i >= sp) break;
                stbtt__csctx_rline_to(c, s[i], 0);
                i++;
                vlineto:
                if (i >= sp) break;
                stbtt__csctx_rline_to(c, 0, s[i]);
                i++;
            }
            break;
            
            case 0x0A: // callsubr
            if (!has_subrs) {
                if (info->fdselect.size)
                {
                    stbtt__buf fdselect = info->fdselect;
                    int nranges, start, end, vTemp, fmt, fdselector = -1;
                    
                    stbtt__buf_seek(&fdselect, 0);
                    fmt = stbtt__buf_get8(&fdselect);
                    if (fmt == 0) {
                        // untested
                        stbtt__buf_seek(&fdselect, fdselect.cursor + glyph_index);
                        fdselector = stbtt__buf_get8(&fdselect);
                    } else if (fmt == 3) {
                        nranges = stbtt__buf_get16(&fdselect);
                        start = stbtt__buf_get16(&fdselect);
                        for (int index = 0; index < nranges; index++) {
                            vTemp = stbtt__buf_get8(&fdselect);
                            end = stbtt__buf_get16(&fdselect);
                            if (glyph_index >= start && glyph_index < end) {
                                fdselector = vTemp;
                                break;
                            }
                            start = end;
                        }
                    }
                    if (fdselector == -1) stbtt__new_buf(NULL, 0);
                    subrs = stbtt__get_subrs(info->cff, stbtt__cff_index_get(info->fontdicts, fdselector));
                }
                has_subrs = 1;
            }
            // fallthrough
            case 0x1D: // callgsubr
            {
                if (sp < 1) return STBTT__CSERR("call(g|)subr stack");
                v = (int) s[--sp];
                if (subr_stack_height >= 10) return STBTT__CSERR("recursion limit");
                subr_stack[subr_stack_height++] = b;
                
                stbtt__buf idx = (b0 == 0x0A) ? subrs : info->gsubrs;
                int vTemp = v;
                stbtt__buf_seek(&idx, 0);
                int count = stbtt__buf_get16(&idx);
                int bias = 107;
                if (count >= 33900)
                    bias = 32768;
                else if (count >= 1240)
                    bias = 1131;
                vTemp += bias;
                if (vTemp < 0 || vTemp >= count)
                {
                    b = stbtt__new_buf(NULL, 0);
                }
                else
                {
                    b = stbtt__cff_index_get(idx, vTemp);
                }
                
                if (b.size == 0) return STBTT__CSERR("subr not found");
                b.cursor = 0;
                clear_stack = 0;
            }
            break;
            
            case 0x0B: // return
            if (subr_stack_height <= 0) return STBTT__CSERR("return outside subr");
            b = subr_stack[--subr_stack_height];
            clear_stack = 0;
            
            break;
            
            case 0x0E: // endchar
            stbtt__csctx_close_shape(c);
            return 1;
            
            default:
            if (b0 != 255 && b0 != 28 && (b0 < 32 || b0 > 254))
                return STBTT__CSERR("reserved operator");
            
            // push immediate
            if (b0 == 255) {
                f = (float)(stbtt_int32)stbtt__buf_get32(&b) / 0x10000;
            } else {
                stbtt__buf_seek(&b, b.cursor - 1);
                f = (float)(stbtt_int16)stbtt__cff_int(&b);
            }
            if (sp >= 48) return STBTT__CSERR("push stack overflow");
            s[sp++] = f;
            clear_stack = 0;
            break;
        }
        if (clear_stack) sp = 0;
    }
    return STBTT__CSERR("no endchar");
    
#undef STBTT__CSERR
}

static void stbtt_GetGlyphBitmapBoxSubpixel(const stbtt_fontinfo *font, int glyph, float scale_x, float scale_y,float shift_x, float shift_y, int *ix0, int *iy0, int *ix1, int *iy1)
{
    int x0=0,y0=0,x1=0,y1=0; // =0 suppresses compiler warning
    
    int glyphBoxCheck = 1;
    if (font->cff.size) {
        stbtt__csctx c = STBTT__CSCTX_INIT(1);
        int r = stbtt__run_charstring(font, glyph, &c);
        x0 = r ? c.min_x : 0;
        y0 = r ? c.min_y : 0;
        x1 = r ? c.max_x : 0;
        y1 = r ? c.max_y : 0;
    } else {
        
        int g = stbtt__GetGlyfOffset(font, glyph);
        if (g < 0)
        {
            glyphBoxCheck = 0;
        }
        else
        {
            x0 = ttSHORT(font->data + g + 2);
            y0 = ttSHORT(font->data + g + 4);
            x1 = ttSHORT(font->data + g + 6);
            y1 = ttSHORT(font->data + g + 8);
        }
    }
    
    if (!glyphBoxCheck) {
        // e.g. space character
        if (ix0) *ix0 = 0;
        if (iy0) *iy0 = 0;
        if (ix1) *ix1 = 0;
        if (iy1) *iy1 = 0;
    } else {
        // move to integral bboxes (treating pixels as little squares, what pixels get touched)?
        if (ix0) *ix0 = STBTT_ifloor( x0 * scale_x + shift_x);
        if (iy0) *iy0 = STBTT_ifloor(-y1 * scale_y + shift_y);
        if (ix1) *ix1 = STBTT_iceil ( x1 * scale_x + shift_x);
        if (iy1) *iy1 = STBTT_iceil (-y0 * scale_y + shift_y);
    }
}

static int stbtt__close_shape(stbtt_vertex *vertices, int num_vertices, int was_off, int start_off,
                              stbtt_int32 sx, stbtt_int32 sy, stbtt_int32 scx, stbtt_int32 scy, stbtt_int32 cx, stbtt_int32 cy)
{
    if (start_off) {
        if (was_off)
            stbtt_setvertex(&vertices[num_vertices++], STBTT_vcurve, (cx+scx)>>1, (cy+scy)>>1, cx,cy);
        stbtt_setvertex(&vertices[num_vertices++], STBTT_vcurve, sx,sy,scx,scy);
    } else {
        if (was_off)
            stbtt_setvertex(&vertices[num_vertices++], STBTT_vcurve,sx,sy,cx,cy);
        else
            stbtt_setvertex(&vertices[num_vertices++], STBTT_vline,sx,sy,0,0);
    }
    return num_vertices;
}

static int stbtt__GetGlyphShapeT2(const stbtt_fontinfo *info, int glyph_index, stbtt_vertex **pvertices)
{
    // runs the charstring twice, once to count and once to output (to avoid realloc)
    stbtt__csctx count_ctx = STBTT__CSCTX_INIT(1);
    stbtt__csctx output_ctx = STBTT__CSCTX_INIT(0);
    if (stbtt__run_charstring(info, glyph_index, &count_ctx)) {
        *pvertices = (stbtt_vertex*)STBTT_malloc(count_ctx.num_vertices*sizeof(stbtt_vertex), info->userdata);
        output_ctx.pvertices = *pvertices;
        if (stbtt__run_charstring(info, glyph_index, &output_ctx)) {
            STBTT_assert(output_ctx.num_vertices == count_ctx.num_vertices);
            return output_ctx.num_vertices;
        }
    }
    *pvertices = NULL;
    return 0;
}

static int stbtt__GetGlyphShapeTT(const stbtt_fontinfo *info, int glyph_index, stbtt_vertex **pvertices)
{
    stbtt_int16 numberOfContours;
    stbtt_uint8 *endPtsOfContours;
    stbtt_uint8 *data = info->data;
    stbtt_vertex *vertices=0;
    int num_vertices=0;
    int g = stbtt__GetGlyfOffset(info, glyph_index);
    
    *pvertices = NULL;
    
    if (g < 0) return 0;
    
    numberOfContours = ttSHORT(data + g);
    
    if (numberOfContours > 0) {
        stbtt_uint8 flags=0,flagcount;
        stbtt_int32 ins, i,j=0,m,n, next_move, was_off=0, off, start_off=0;
        stbtt_int32 x,y,cx,cy,sx,sy, scx,scy;
        stbtt_uint8 *points;
        endPtsOfContours = (data + g + 10);
        ins = ttUSHORT(data + g + 10 + numberOfContours * 2);
        points = data + g + 10 + numberOfContours * 2 + 2 + ins;
        
        n = 1+ttUSHORT(endPtsOfContours + numberOfContours*2-2);
        
        m = n + 2*numberOfContours;  // a loose bound on how many vertices we might need
        vertices = (stbtt_vertex *) STBTT_malloc(m * sizeof(vertices[0]), info->userdata);
        if (vertices == 0)
            return 0;
        
        next_move = 0;
        flagcount=0;
        
        // in first pass, we load uninterpreted data into the allocated array
        // above, shifted to the end of the array so we won't overwrite it when
        // we create our final data starting from the front
        
        off = m - n; // starting offset for uninterpreted data, regardless of how m ends up being calculated
        
        // first load flags
        
        for (i=0; i < n; ++i) {
            if (flagcount == 0) {
                flags = *points++;
                if (flags & 8)
                    flagcount = *points++;
            } else
                --flagcount;
            vertices[off+i].type = flags;
        }
        
        // now load x coordinates
        x=0;
        for (i=0; i < n; ++i) {
            flags = vertices[off+i].type;
            if (flags & 2) {
                stbtt_int16 dx = *points++;
                x += (flags & 16) ? dx : -dx; // ???
            } else {
                if (!(flags & 16)) {
                    x = x + (stbtt_int16) (points[0]*256 + points[1]);
                    points += 2;
                }
            }
            vertices[off+i].x = (stbtt_int16) x;
        }
        
        // now load y coordinates
        y=0;
        for (i=0; i < n; ++i) {
            flags = vertices[off+i].type;
            if (flags & 4) {
                stbtt_int16 dy = *points++;
                y += (flags & 32) ? dy : -dy; // ???
            } else {
                if (!(flags & 32)) {
                    y = y + (stbtt_int16) (points[0]*256 + points[1]);
                    points += 2;
                }
            }
            vertices[off+i].y = (stbtt_int16) y;
        }
        
        // now convert them to our format
        num_vertices=0;
        sx = sy = cx = cy = scx = scy = 0;
        for (i=0; i < n; ++i) {
            flags = vertices[off+i].type;
            x     = (stbtt_int16) vertices[off+i].x;
            y     = (stbtt_int16) vertices[off+i].y;
            
            if (next_move == i) {
                if (i != 0)
                    num_vertices = stbtt__close_shape(vertices, num_vertices, was_off, start_off, sx,sy,scx,scy,cx,cy);
                
                // now start the new one
                start_off = !(flags & 1);
                if (start_off) {
                    // if we start off with an off-curve point, then when we need to find a point on the curve
                    // where we can start, and we need to save some state for when we wraparound.
                    scx = x;
                    scy = y;
                    if (!(vertices[off+i+1].type & 1)) {
                        // next point is also a curve point, so interpolate an on-point curve
                        sx = (x + (stbtt_int32) vertices[off+i+1].x) >> 1;
                        sy = (y + (stbtt_int32) vertices[off+i+1].y) >> 1;
                    } else {
                        // otherwise just use the next point as our start point
                        sx = (stbtt_int32) vertices[off+i+1].x;
                        sy = (stbtt_int32) vertices[off+i+1].y;
                        ++i; // we're using point i+1 as the starting point, so skip it
                    }
                } else {
                    sx = x;
                    sy = y;
                }
                stbtt_setvertex(&vertices[num_vertices++], STBTT_vmove,sx,sy,0,0);
                was_off = 0;
                next_move = 1 + ttUSHORT(endPtsOfContours+j*2);
                ++j;
            } else {
                if (!(flags & 1)) { // if it's a curve
                    if (was_off) // two off-curve control points in a row means interpolate an on-curve midpoint
                        stbtt_setvertex(&vertices[num_vertices++], STBTT_vcurve, (cx+x)>>1, (cy+y)>>1, cx, cy);
                    cx = x;
                    cy = y;
                    was_off = 1;
                } else {
                    if (was_off)
                        stbtt_setvertex(&vertices[num_vertices++], STBTT_vcurve, x,y, cx, cy);
                    else
                        stbtt_setvertex(&vertices[num_vertices++], STBTT_vline, x,y,0,0);
                    was_off = 0;
                }
            }
        }
        num_vertices = stbtt__close_shape(vertices, num_vertices, was_off, start_off, sx,sy,scx,scy,cx,cy);
    } else if (numberOfContours < 0) {
        // Compound shapes.
        int more = 1;
        stbtt_uint8 *comp = data + g + 10;
        num_vertices = 0;
        vertices = 0;
        while (more) {
            stbtt_uint16 flags, gidx;
            int comp_num_verts = 0, i;
            stbtt_vertex *comp_verts = 0, *tmp = 0;
            float mtx[6] = {1,0,0,1,0,0}, m, n;
            
            flags = ttSHORT(comp); comp+=2;
            gidx = ttSHORT(comp); comp+=2;
            
            if (flags & 2) { // XY values
                if (flags & 1) { // shorts
                    mtx[4] = ttSHORT(comp); comp+=2;
                    mtx[5] = ttSHORT(comp); comp+=2;
                } else {
                    mtx[4] = ttCHAR(comp); comp+=1;
                    mtx[5] = ttCHAR(comp); comp+=1;
                }
            }
            else {
                // @TODO handle matching point
                STBTT_assert(0);
            }
            if (flags & (1<<3)) { // WE_HAVE_A_SCALE
                mtx[0] = mtx[3] = ttSHORT(comp)/16384.0f; comp+=2;
                mtx[1] = mtx[2] = 0;
            } else if (flags & (1<<6)) { // WE_HAVE_AN_X_AND_YSCALE
                mtx[0] = ttSHORT(comp)/16384.0f; comp+=2;
                mtx[1] = mtx[2] = 0;
                mtx[3] = ttSHORT(comp)/16384.0f; comp+=2;
            } else if (flags & (1<<7)) { // WE_HAVE_A_TWO_BY_TWO
                mtx[0] = ttSHORT(comp)/16384.0f; comp+=2;
                mtx[1] = ttSHORT(comp)/16384.0f; comp+=2;
                mtx[2] = ttSHORT(comp)/16384.0f; comp+=2;
                mtx[3] = ttSHORT(comp)/16384.0f; comp+=2;
            }
            
            // Find transformation scales.
            m = (float) STBTT_sqrt(mtx[0]*mtx[0] + mtx[1]*mtx[1]);
            n = (float) STBTT_sqrt(mtx[2]*mtx[2] + mtx[3]*mtx[3]);
            
            // Get indexed glyph.
            comp_num_verts = (!info->cff.size) ? stbtt__GetGlyphShapeTT(info, gidx, &comp_verts) : stbtt__GetGlyphShapeT2(info, gidx, &comp_verts);
            if (comp_num_verts > 0) {
                // Transform vertices.
                for (i = 0; i < comp_num_verts; ++i) {
                    stbtt_vertex* v = &comp_verts[i];
                    stbtt_vertex_type x,y;
                    x=v->x; y=v->y;
                    v->x = (stbtt_vertex_type)(m * (mtx[0]*x + mtx[2]*y + mtx[4]));
                    v->y = (stbtt_vertex_type)(n * (mtx[1]*x + mtx[3]*y + mtx[5]));
                    x=v->cx; y=v->cy;
                    v->cx = (stbtt_vertex_type)(m * (mtx[0]*x + mtx[2]*y + mtx[4]));
                    v->cy = (stbtt_vertex_type)(n * (mtx[1]*x + mtx[3]*y + mtx[5]));
                }
                // Append vertices.
                tmp = (stbtt_vertex*)STBTT_malloc((num_vertices+comp_num_verts)*sizeof(stbtt_vertex), info->userdata);
                if (!tmp) {
                    if (vertices) STBTT_free(vertices, info->userdata);
                    if (comp_verts) STBTT_free(comp_verts, info->userdata);
                    return 0;
                }
                if (num_vertices > 0) STBTT_memcpy(tmp, vertices, num_vertices*sizeof(stbtt_vertex));
                STBTT_memcpy(tmp+num_vertices, comp_verts, comp_num_verts*sizeof(stbtt_vertex));
                if (vertices) STBTT_free(vertices, info->userdata);
                vertices = tmp;
                STBTT_free(comp_verts, info->userdata);
                num_vertices += comp_num_verts;
            }
            // More components ?
            more = flags & (1<<5);
        }
    } else {
        // numberOfCounters == 0, do nothing
    }
    
    *pvertices = vertices;
    return num_vertices;
}

static void stbtt__sort_edges_quicksort(stbtt__edge *p, int n)
{
    /* threshold for transitioning to insertion sort */
    while (n > 12) {
        stbtt__edge t;
        int c01,c12,c,m,i,j;
        
        /* compute median of three */
        m = n >> 1;
        c01 = STBTT__COMPARE(&p[0],&p[m]);
        c12 = STBTT__COMPARE(&p[m],&p[n-1]);
        /* if 0 >= mid >= end, or 0 < mid < end, then use mid */
        if (c01 != c12) {
            /* otherwise, we'll need to swap something else to middle */
            int z;
            c = STBTT__COMPARE(&p[0],&p[n-1]);
            /* 0>mid && mid<n:  0>n => n; 0<n => 0 */
            /* 0<mid && mid>n:  0>n => 0; 0<n => n */
            z = (c == c12) ? 0 : n-1;
            t = p[z];
            p[z] = p[m];
            p[m] = t;
        }
        /* now p[m] is the median-of-three */
        /* swap it to the beginning so it won't move around */
        t = p[0];
        p[0] = p[m];
        p[m] = t;
        
        /* partition loop */
        i=1;
        j=n-1;
        for(;;) {
            /* handling of equality is crucial here */
            /* for sentinels & efficiency with duplicates */
            for (;;++i) {
                if (!STBTT__COMPARE(&p[i], &p[0])) break;
            }
            for (;;--j) {
                if (!STBTT__COMPARE(&p[0], &p[j])) break;
            }
            /* make sure we haven't crossed */
            if (i >= j) break;
            t = p[i];
            p[i] = p[j];
            p[j] = t;
            
            ++i;
            --j;
        }
        /* recurse on smaller side, iterate on larger */
        if (j < (n-i)) {
            stbtt__sort_edges_quicksort(p,j);
            p = p+i;
            n = n-i;
        } else {
            stbtt__sort_edges_quicksort(p+i, n-i);
            n = j;
        }
    }
}

static void stbtt__handle_clipped_edge(float *scanline, int x, stbtt__active_edge *e, float x0, float y0, float x1, float y1)
{
    if (y0 == y1) return;
    STBTT_assert(y0 < y1);
    STBTT_assert(e->sy <= e->ey);
    if (y0 > e->ey) return;
    if (y1 < e->sy) return;
    if (y0 < e->sy) {
        x0 += (x1-x0) * (e->sy - y0) / (y1-y0);
        y0 = e->sy;
    }
    if (y1 > e->ey) {
        x1 += (x1-x0) * (e->ey - y1) / (y1-y0);
        y1 = e->ey;
    }
    
    if (x0 == x)
    {
        STBTT_assert(x1 <= x+1);
    }
    else if (x0 == x+1)
    {
        STBTT_assert(x1 >= x);
    }
    else if (x0 <= x)
    {
        STBTT_assert(x1 <= x);
    }
    else if (x0 >= x+1)
    {
        STBTT_assert(x1 >= x+1);
    }
    else
    {
        STBTT_assert(x1 >= x && x1 <= x+1);
    }
    
    if (x0 <= x && x1 <= x)
        scanline[x] += e->direction * (y1-y0);
    else if (x0 >= x+1 && x1 >= x+1)
        ;
    else {
        STBTT_assert(x0 >= x && x0 <= x+1 && x1 >= x && x1 <= x+1);
        scanline[x] += e->direction * (y1-y0) * (1-((x0-x)+(x1-x))/2); // coverage = 1 - average x position
    }
}

static void stbtt__add_point(stbtt__point *points, int n, float x, float y)
{
    if (!points) return; // during first pass, it's unallocated
    points[n].x = x;
    points[n].y = y;
}

// tessellate until threshold p is happy... @TODO warped to compensate for non-linear stretching
static int stbtt__tesselate_curve(stbtt__point *points, int *num_points, float x0, float y0, float x1, float y1, float x2, float y2, float objspace_flatness_squared, int n)
{
    // midpoint
    float mx = (x0 + 2*x1 + x2)/4;
    float my = (y0 + 2*y1 + y2)/4;
    // versus directly drawn line
    float dx = (x0+x2)/2 - mx;
    float dy = (y0+y2)/2 - my;
    if (n > 16) // 65536 segments on one curve better be enough!
        return 1;
    if (dx*dx+dy*dy > objspace_flatness_squared) { // half-pixel error allowed... need to be smaller if AA
        stbtt__tesselate_curve(points, num_points, x0,y0, (x0+x1)/2.0f,(y0+y1)/2.0f, mx,my, objspace_flatness_squared,n+1);
        stbtt__tesselate_curve(points, num_points, mx,my, (x1+x2)/2.0f,(y1+y2)/2.0f, x2,y2, objspace_flatness_squared,n+1);
    } else {
        stbtt__add_point(points, *num_points,x2,y2);
        *num_points = *num_points+1;
    }
    return 1;
}

static void stbtt_MakeCodepointBitmap(const stbtt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale, int codepoint)
{
    int glyph = stbtt_FindGlyphIndex(info,codepoint);
    int ix0,iy0;
    stbtt_vertex *vertices;
    int num_verts = (!info->cff.size) ? stbtt__GetGlyphShapeTT(info, glyph, &vertices) : stbtt__GetGlyphShapeT2(info, glyph, &vertices);
    stbtt__bitmap gbm;
    
    stbtt_GetGlyphBitmapBoxSubpixel(info, glyph, scale, scale, 0.0f, 0.0f, &ix0,&iy0,0,0);
    gbm.pixels = output;
    gbm.w = out_w;
    gbm.h = out_h;
    gbm.stride = out_stride;
    
    if (gbm.w && gbm.h)
    {
        int winding_count      = 0;
        int *winding_lengths   = NULL;
        stbtt__point *windings = 0;
        float objspace_flatness = 0.35f / scale;
        stbtt__point *points=0;
        int num_points=0;
        
        float objspace_flatness_squared = objspace_flatness * objspace_flatness;
        int num=0,start=0, pass;
        
        // count how many "moves" there are to get the contour count
        for (int i=0; i < num_verts; ++i)
            if (vertices[i].type == STBTT_vmove)
            ++num;
        
        winding_count = num;
        if (num == 0)
        {
            windings = 0;
        }
        else
        {
            winding_lengths = (int *) STBTT_malloc(sizeof(*winding_lengths) * num, info->userdata);
            
            if (winding_lengths == 0) {
                winding_count = 0;
                windings = 0;
            }
            else
            {
                // make two passes through the points so we don't need to realloc
                bool failed = false;
                for (pass=0; pass < 2; ++pass) {
                    float x=0,y=0;
                    if (pass == 1) {
                        points = (stbtt__point *) STBTT_malloc(num_points * sizeof(points[0]), info->userdata);
                        if (points == NULL)
                        {
                            failed = true;
                            break;
                        }
                    }
                    num_points = 0;
                    num = -1;
                    for (int i=0; i < num_verts; ++i) {
                        switch (vertices[i].type) {
                            case STBTT_vmove:
                            // start the next contour
                            if (num >= 0)
                                winding_lengths[num] = num_points - start;
                            ++num;
                            start = num_points;
                            
                            x = vertices[i].x, y = vertices[i].y;
                            stbtt__add_point(points, num_points++, x,y);
                            break;
                            case STBTT_vline:
                            x = vertices[i].x, y = vertices[i].y;
                            stbtt__add_point(points, num_points++, x, y);
                            break;
                            case STBTT_vcurve:
                            stbtt__tesselate_curve(points, &num_points, x,y,
                                                   vertices[i].cx, vertices[i].cy,
                                                   vertices[i].x,  vertices[i].y,
                                                   objspace_flatness_squared, 0);
                            x = vertices[i].x, y = vertices[i].y;
                            break;
                        }
                    }
                    winding_lengths[num] = num_points - start;
                }
                
                if (!failed)
                {
                    windings = points;
                }
                else
                {
                    STBTT_free(points, info->userdata);
                    STBTT_free(winding_lengths, info->userdata);
                    winding_lengths = 0;
                    winding_count = 0;
                    windings = NULL;
                }
            }
        }
        if (windings) {
            float y_scale_inv = -scale;
            int n,i,j,k,m;
            
            // now we have to blow out the windings into explicit edge lists
            n = 0;
            for (i=0; i < winding_count; ++i)
                n += winding_lengths[i];
            
            stbtt__edge *e = (stbtt__edge *) STBTT_malloc(sizeof(*e) * (n+1), info->userdata); // add an extra one as a sentinel
            if (e == 0) return;
            n = 0;
            
            m=0;
            for (i=0; i < winding_count; ++i) {
                stbtt__point *p = windings + m;
                m += winding_lengths[i];
                j = winding_lengths[i]-1;
                for (k=0; k < winding_lengths[i]; j=k++) {
                    int a=k,b=j;
                    // skip the edge if horizontal
                    if (p[j].y == p[k].y)
                        continue;
                    // add edge from j to k to the list
                    e[n].invert = 0;
                    if (p[j].y > p[k].y) {
                        e[n].invert = 1;
                        a=j,b=k;
                    }
                    e[n].x0 = p[a].x * scale;
                    e[n].y0 = (p[a].y * y_scale_inv);
                    e[n].x1 = p[b].x * scale;
                    e[n].y1 = (p[b].y * y_scale_inv);
                    ++n;
                }
            }
            
            // now sort the edges by their highest point (should snap to integer, and then by x)
            stbtt__sort_edges_quicksort(e, n);
            
            int i2,j2;
            for (i2=1; i2 < n; ++i2) {
                stbtt__edge t = e[i2], *a = &t;
                j2 = i2;
                while (j2 > 0) {
                    stbtt__edge *b = &e[j2-1];
                    int c = STBTT__COMPARE(a,b);
                    if (!c) break;
                    e[j2] = e[j2-1];
                    --j2;
                }
                if (i2 != j2)
                    e[j2] = t;
            }
            
            stbtt__hheap hh = { 0, 0, 0 };
            stbtt__edge *eTemp = e;
            stbtt__active_edge *active = NULL;
            int y,j3=0, i3;
            float scanline_data[129], *scanline, *scanline2;
            
            if (gbm.w > 64)
                scanline = (float *) STBTT_malloc((gbm.w*2+1) * sizeof(float), info->userdata);
            else
                scanline = scanline_data;
            
            scanline2 = scanline + gbm.w;
            
            y = iy0;
            eTemp[n].y0 = (float) (iy0 + gbm.h) + 1;
            
            while (j3 < gbm.h) {
                // find center of pixel for this scanline
                float scan_y_top    = y + 0.0f;
                float scan_y_bottom = y + 1.0f;
                stbtt__active_edge **step = &active;
                
                STBTT_memset(scanline , 0, gbm.w*sizeof(scanline[0]));
                STBTT_memset(scanline2, 0, (gbm.w+1)*sizeof(scanline[0]));
                
                // update all active edges;
                // remove all active edges that terminate before the top of this scanline
                while (*step) {
                    stbtt__active_edge * z = *step;
                    if (z->ey <= scan_y_top) {
                        *step = z->next; // delete from list
                        STBTT_assert(z->direction);
                        z->direction = 0;
                        *(void **) z = hh.first_free;
                        hh.first_free = z;
                    } else {
                        step = &((*step)->next); // advance through list
                    }
                }
                
                // insert all edges that start before the bottom of this scanline
                while (eTemp->y0 <= scan_y_bottom) {
                    if (eTemp->y0 != eTemp->y1) {
                        stbtt__active_edge *z = 0;
                        int size = sizeof(*z);
                        bool skipOut = false;
                        if (hh.first_free) {
                            void *p = hh.first_free;
                            hh.first_free = * (void **) p;
                            z = (stbtt__active_edge *)p;
                        } else {
                            
                            if (hh.num_remaining_in_head_chunk == 0) {
                                int count = (size < 32 ? 2000 : size < 128 ? 800 : 100);
                                stbtt__hheap_chunk *c = (stbtt__hheap_chunk *) STBTT_malloc(sizeof(stbtt__hheap_chunk) + size * count, info->userdata);
                                if (c == NULL)
                                {
                                    z = 0;
                                    skipOut = true;
                                }
                                else
                                {
                                    c->next = hh.head;
                                    hh.head = c;
                                    hh.num_remaining_in_head_chunk = count;
                                }
                            }
                            if (!skipOut)
                            {
                                --hh.num_remaining_in_head_chunk;
                                z = (stbtt__active_edge *)((char *) (hh.head) + sizeof(stbtt__hheap_chunk) + size * hh.num_remaining_in_head_chunk);
                            }
                        }
                        
                        if (!skipOut)
                        {
                            float dxdy = (eTemp->x1 - eTemp->x0) / (eTemp->y1 - eTemp->y0);
                            STBTT_assert(z != NULL);
                            if (z)
                            {
                                z->fdx = dxdy;
                                z->fdy = dxdy != 0.0f ? (1.0f/dxdy) : 0.0f;
                                z->fx = eTemp->x0 + dxdy * (scan_y_top - eTemp->y0);
                                z->fx -= ix0;
                                z->direction = eTemp->invert ? 1.0f : -1.0f;
                                z->sy = eTemp->y0;
                                z->ey = eTemp->y1;
                                z->next = 0;
                            }
                        }
                        if (z != NULL) {
                            if (j3 == 0 && iy0 != 0) {
                                if (z->ey < scan_y_top) {
                                    // this can happen due to subpixel positioning and some kind of fp rounding error i think
                                    z->ey = scan_y_top;
                                }
                            }
                            STBTT_assert(z->ey >= scan_y_top); // if we get really unlucky a tiny bit of an edge can be out of bounds
                            // insert at front
                            z->next = active;
                            active = z;
                        }
                    }
                    ++eTemp;
                }
                
                // now process all active edges
                if (active)
                {
                    stbtt__active_edge *activeTemp = active;
                    float *scanlineTemp = scanline;
                    float *scanline_fill = scanline2+1;
                    
                    float y_bottom = scan_y_top+1;
                    
                    while (activeTemp) {
                        // brute force every pixel
                        
                        // compute intersection points with top & bottom
                        STBTT_assert(activeTemp->ey >= scan_y_top);
                        
                        if (activeTemp->fdx == 0) {
                            float x0 = activeTemp->fx;
                            if (x0 < gbm.w) {
                                if (x0 >= 0) {
                                    stbtt__handle_clipped_edge(scanlineTemp,(int) x0,activeTemp, x0,scan_y_top, x0,y_bottom);
                                    stbtt__handle_clipped_edge(scanline_fill-1,(int) x0+1,activeTemp, x0,scan_y_top, x0,y_bottom);
                                } else {
                                    stbtt__handle_clipped_edge(scanline_fill-1,0,activeTemp, x0,scan_y_top, x0,y_bottom);
                                }
                            }
                        } else {
                            float x0 = activeTemp->fx;
                            float dx = activeTemp->fdx;
                            float xb = x0 + dx;
                            float x_top, x_bottom;
                            float sy0,sy1;
                            float dy = activeTemp->fdy;
                            STBTT_assert(activeTemp->sy <= y_bottom && activeTemp->ey >= scan_y_top);
                            
                            // compute endpoints of line segment clipped to this scanline (if the
                            // line segment starts on this scanline. x0 is the intersection of the
                            // line with scan_y_top, but that may be off the line segment.
                            if (activeTemp->sy > scan_y_top) {
                                x_top = x0 + dx * (activeTemp->sy - scan_y_top);
                                sy0 = activeTemp->sy;
                            } else {
                                x_top = x0;
                                sy0 = scan_y_top;
                            }
                            if (activeTemp->ey < y_bottom) {
                                x_bottom = x0 + dx * (activeTemp->ey - scan_y_top);
                                sy1 = activeTemp->ey;
                            } else {
                                x_bottom = xb;
                                sy1 = y_bottom;
                            }
                            
                            if (x_top >= 0 && x_bottom >= 0 && x_top < gbm.w && x_bottom < gbm.w) {
                                // from here on, we don't have to range check x values
                                
                                if ((int) x_top == (int) x_bottom) {
                                    float height;
                                    // simple case, only spans one pixel
                                    int x = (int) x_top;
                                    height = sy1 - sy0;
                                    STBTT_assert(x >= 0 && x < gbm.w);
                                    scanlineTemp[x] += activeTemp->direction * (1-((x_top - x) + (x_bottom-x))/2)  * height;
                                    scanline_fill[x] += activeTemp->direction * height; // everything right of this pixel is filled
                                } else {
                                    int x,x1,x2;
                                    float y_crossing, step2, sign, area;
                                    // covers 2+ pixels
                                    if (x_top > x_bottom) {
                                        // flip scanline vertically; signed area is the same
                                        float t;
                                        sy0 = y_bottom - (sy0 - scan_y_top);
                                        sy1 = y_bottom - (sy1 - scan_y_top);
                                        t = sy0, sy0 = sy1, sy1 = t;
                                        t = x_bottom, x_bottom = x_top, x_top = t;
                                        dx = -dx;
                                        dy = -dy;
                                        t = x0, x0 = xb, xb = t;
                                    }
                                    
                                    x1 = (int) x_top;
                                    x2 = (int) x_bottom;
                                    // compute intersection with y axis at x1+1
                                    y_crossing = (x1+1 - x0) * dy + scan_y_top;
                                    
                                    sign = activeTemp->direction;
                                    // area of the rectangle covered from y0..y_crossing
                                    area = sign * (y_crossing-sy0);
                                    // area of the triangle (x_top,y0), (x+1,y0), (x+1,y_crossing)
                                    scanlineTemp[x1] += area * (1-((x_top - x1)+(x1+1-x1))/2);
                                    
                                    step2 = sign * dy;
                                    for (x = x1+1; x < x2; ++x) {
                                        scanlineTemp[x] += area + step2/2;
                                        area += step2;
                                    }
                                    y_crossing += dy * (x2 - (x1+1));
                                    
                                    STBTT_assert(STBTT_fabs(area) <= 1.01f);
                                    
                                    scanlineTemp[x2] += area + sign * (1-((x2-x2)+(x_bottom-x2))/2) * (sy1-y_crossing);
                                    
                                    scanline_fill[x2] += sign * (sy1-sy0);
                                }
                            } else {
                                int x;
                                for (x=0; x < gbm.w; ++x) {
                                    // rename variables to clearly-defined pairs
                                    float y0 = scan_y_top;
                                    float x1 = (float) (x);
                                    float x2 = (float) (x+1);
                                    float x3 = xb;
                                    float y3 = y_bottom;
                                    
                                    // x = e->x + e->dx * (y-scan_y_top)
                                    // (y-scan_y_top) = (x - e->x) / e->dx
                                    // y = (x - e->x) / e->dx + scan_y_top
                                    float y1 = (x - x0) / dx + scan_y_top;
                                    float y2 = (x+1 - x0) / dx + scan_y_top;
                                    
                                    if (x0 < x1 && x3 > x2) {         // three segments descending down-right
                                        stbtt__handle_clipped_edge(scanlineTemp,x,activeTemp, x0,y0, x1,y1);
                                        stbtt__handle_clipped_edge(scanlineTemp,x,activeTemp, x1,y1, x2,y2);
                                        stbtt__handle_clipped_edge(scanlineTemp,x,activeTemp, x2,y2, x3,y3);
                                    } else if (x3 < x1 && x0 > x2) {  // three segments descending down-left
                                        stbtt__handle_clipped_edge(scanlineTemp,x,activeTemp, x0,y0, x2,y2);
                                        stbtt__handle_clipped_edge(scanlineTemp,x,activeTemp, x2,y2, x1,y1);
                                        stbtt__handle_clipped_edge(scanlineTemp,x,activeTemp, x1,y1, x3,y3);
                                    } else if (x0 < x1 && x3 > x1) {  // two segments across x, down-right
                                        stbtt__handle_clipped_edge(scanlineTemp,x,activeTemp, x0,y0, x1,y1);
                                        stbtt__handle_clipped_edge(scanlineTemp,x,activeTemp, x1,y1, x3,y3);
                                    } else if (x3 < x1 && x0 > x1) {  // two segments across x, down-left
                                        stbtt__handle_clipped_edge(scanlineTemp,x,activeTemp, x0,y0, x1,y1);
                                        stbtt__handle_clipped_edge(scanlineTemp,x,activeTemp, x1,y1, x3,y3);
                                    } else if (x0 < x2 && x3 > x2) {  // two segments across x+1, down-right
                                        stbtt__handle_clipped_edge(scanlineTemp,x,activeTemp, x0,y0, x2,y2);
                                        stbtt__handle_clipped_edge(scanlineTemp,x,activeTemp, x2,y2, x3,y3);
                                    } else if (x3 < x2 && x0 > x2) {  // two segments across x+1, down-left
                                        stbtt__handle_clipped_edge(scanlineTemp,x,activeTemp, x0,y0, x2,y2);
                                        stbtt__handle_clipped_edge(scanlineTemp,x,activeTemp, x2,y2, x3,y3);
                                    } else {  // one segment
                                        stbtt__handle_clipped_edge(scanlineTemp,x,activeTemp, x0,y0, x3,y3);
                                    }
                                }
                            }
                        }
                        activeTemp = activeTemp->next;
                    }
                }
                
                {
                    float sum = 0;
                    for (i3=0; i3 < gbm.w; ++i3) {
                        float k2;
                        int m2;
                        sum += scanline2[i3];
                        k2 = scanline[i3] + sum;
                        k2 = (float) STBTT_fabs(k2)*255 + 0.5f;
                        m2 = (int) k2;
                        if (m2 > 255) m2 = 255;
                        gbm.pixels[j3*gbm.stride + i3] = (unsigned char) m2;
                    }
                }
                
                // advance all the edges
                step = &active;
                while (*step) {
                    stbtt__active_edge *z = *step;
                    z->fx += z->fdx; // advance to position for current scanline
                    step = &((*step)->next); // advance through list
                }
                
                ++y;
                ++j3;
            }
            
            stbtt__hheap_chunk *c = hh.head;
            while (c) {
                stbtt__hheap_chunk *next = c->next;
                STBTT_free(c, info->userdata);
                c = next;
            }
            
            if (scanline != scanline_data)
                STBTT_free(scanline, info->userdata);
            eTemp = 0;
            
            STBTT_free(e, info->userdata);
            STBTT_free(winding_lengths, info->userdata);
            STBTT_free(windings, info->userdata);
        }
    }
    
    STBTT_free(vertices, info->userdata);
}
#endif

BSDEF unsigned char BS842_Internal_Lerp(unsigned char a, unsigned char b, float t)
{
    unsigned char result = (unsigned char)((1.0f - t) * a + t * b);
    return result;
}

BSDEF void BS842_Internal_DrawText(BS842_Plotting_Internal_BackBuffer *backBuffer, stbtt_fontinfo fontInfo, char *text, BS842_Internal_V2I pos, float lineHeight, int colour)
{
    pos.y -= (int)(lineHeight / 2.0f + 0.5f);
    
    int bitmapWidth = 0;
    int bitmapHeight = (int)(lineHeight + 0.5f);
    unsigned char *textBitmap = 0;
    
    float scale = lineHeight / (ttSHORT(fontInfo.data + fontInfo.hhea + 4) - ttSHORT(fontInfo.data + fontInfo.hhea + 6));
    int ascent = ttSHORT(fontInfo.data + fontInfo.hhea + 4);
    
    ascent = (int)((float)ascent * scale + 0.5f);
    
    int textX = 0;
    int pass = 0;
    while (pass < 2)
    {
        textX = 0;
        for (int i = 0; i < BS842_STRLEN(text); ++i)
        {
            int charWidth;
            int lsb;
            stbtt_GetCodepointHMetrics(&fontInfo, text[i], &charWidth, &lsb);
            /* (Note that each Codepoint call has an alternative Glyph version which caches the work required to lookup the character word[i].) */
            
            if (pass > 0)
            {
                /* get bounding box for character (may be offset to account for chars that dip above or below the line */
                int c_x1, c_y1, c_x2, c_y2;
                stbtt_GetGlyphBitmapBoxSubpixel(&fontInfo, stbtt_FindGlyphIndex(&fontInfo, text[i]), scale, scale, 0.0f, 0.0f, &c_x1, &c_y1, &c_x2, &c_y2);
                
                /* compute y (different characters have different heights */
                int y = ascent + c_y1;
                
                /* render character (stride and offset is important here) */
                int byteOffset = textX + int(lsb * scale + 0.5f) + (y * bitmapWidth);
                stbtt_MakeCodepointBitmap(&fontInfo, textBitmap + byteOffset, c_x2 - c_x1, c_y2 - c_y1, bitmapWidth, scale, text[i]);
            }
            
            /* advance x */
            textX += (int)(charWidth * scale + 0.5f);
            
            /* add kerning */
            int kern;
            kern = stbtt_GetCodepointKernAdvance(&fontInfo, text[i], text[i + 1]);
            textX += (int)(kern * scale + 0.5f);
        }
        
        if (pass == 0)
        {
            bitmapWidth = textX;
            textBitmap = (unsigned char *)BS842_MEMALLOC(bitmapWidth * bitmapHeight);
        }
        
        pass++;
    }
    
    pos.x -= (textX / 2);
    
    unsigned char *row = (unsigned char *)backBuffer->memory + (pos.y * backBuffer->pitch);
    for (s32 y = 0; y < bitmapHeight; ++y)
    {
        int *pixel = (int *)row + pos.x;
        for (s32 x = 0; x < bitmapWidth; ++x)
        {
            int textBitmapOffset = (y * bitmapWidth) + x;
            if (textBitmap[textBitmapOffset])
            {
                unsigned char pixelR = (*pixel >> 16) & 0xFF;
                unsigned char pixelG = (*pixel >> 8) & 0xFF;
                unsigned char pixelB = *pixel & 0xFF;
                
                unsigned char colourR = (colour >> 16) & 0xFF;
                unsigned char colourG = (colour >> 8) & 0xFF;
                unsigned char colourB = colour & 0xFF;
                
                unsigned char newR = BS842_Internal_Lerp(pixelR, colourR, ((float)textBitmap[textBitmapOffset] / 255.0f));
                unsigned char newG = BS842_Internal_Lerp(pixelG, colourG, ((float)textBitmap[textBitmapOffset] / 255.0f));
                unsigned char newB = BS842_Internal_Lerp(pixelB, colourB, ((float)textBitmap[textBitmapOffset] / 255.0f));
                
                *pixel++ = (0xFF << 24) | (newR << 16) | (newG << 8) | newB;
            }
            else
            {
                pixel++;
            }
        }
        
        row += backBuffer->pitch;
    }
    
    BS842_MEMFREE(textBitmap);
}

BSDEF float *BS842_Internal_GetMarks(int *marksCount, float minVal, float maxVal, float modVal)
{
    float *result = 0;
    
    for (float val = minVal; val <= maxVal; val += 1.0f)
    {
        double check = BS842_FMOD(val, modVal);
        if (check == 0.000)
        {
            (*marksCount)++;
        }
    }
    
    if (*marksCount > 0)
    {
        result = (float *)BS842_MEMALLOC(sizeof(float) * *marksCount);
    }
    
    int markIndex = 0;
    for (float val = minVal; val <= maxVal; val += 1.0f)
    {
        double check = BS842_FMOD(val, modVal);
        if (check == 0.000)
        {
            result[markIndex++] = val;
        }
    }
    
    return result;
}

BSDEF int BS842_Internal_GetValueInMappedRanges(float inValue, float inMin, float inMax, int mapMin, int mapMax)
{
    int result = 0;
    
    double slope = 1.0 * ((double)mapMax - (double)mapMin) / (inMax - inMin);
    result = (int)(((double)mapMin + slope * (inValue - inMin)) + 0.5);
    
    return result;
}

BSDEF void BS842_Internal_DrawDottedHorizontalLine(BS842_Plotting_Internal_BackBuffer *backBuffer, int xLeft, int xRight, int y, int colour)
{
    if (y >= 0 && y < backBuffer->height)
    {
        unsigned char *row = (unsigned char *)backBuffer->memory + (y * backBuffer->pitch);
        int *pixel = (int *)row + xLeft;
        for (int x = xLeft; x < xRight; ++x)
        {
            if ((x % 10) < 6)
            {
                *pixel++ = colour;
            }
            else
            {
                pixel++;
            }
        }
    }
}

BSDEF void BS842_Internal_DrawDottedVerticalLine(BS842_Plotting_Internal_BackBuffer *backBuffer, int yTop, int yBottom, int x, int colour)
{
    if (x >= 0 && x < backBuffer->width)
    {
        unsigned char *row = (unsigned char *)backBuffer->memory + (yTop * backBuffer->pitch);
        for (int y = yTop; y < yBottom; ++y)
        {
            if ((y % 10) < 6)
            {
                int *pixel = (int *)row + x;
                *pixel = colour;
            }
            
            row += backBuffer->pitch;
        }
    }
}

BSDEF void BS842_Internal_DrawBresenhamLine(BS842_Plotting_Internal_BackBuffer *backBuffer, BS842_Internal_V2I start, BS842_Internal_V2I end, BS842_Internal_V2I minBounds, BS842_Internal_V2I maxBounds, float width, int colour)
{
    // NOTE(bSalmon): http://members.chello.at/~easyfilter/bresenham.html
    int dx = abs(end.x - start.x), sx = start.x < end.x ? 1 : -1; 
    int dy = abs(end.y - start.y), sy = start.y < end.y ? 1 : -1; 
    int err = dx - dy, e2, x2, y2;                          /* error value e_xy */
    float ed = dx + dy == 0 ? 1 : (float)sqrt((float)dx*dx+(float)dy*dy);
    
    int *pixel = (int *)backBuffer->memory;
    int x = 0, y = 0;
    for (width = (width + 1) / 2; ; )
    {
        x = start.x;
        y = start.y;
        if (x >= minBounds.x && y >= minBounds.y && x <= maxBounds.x && y <= maxBounds.y)
        {
            pixel = (int *)backBuffer->memory + (y * backBuffer->width) + x;
            *pixel = colour;
        }
        
        e2 = err; x2 = start.x;
        if (2 * e2 >= -dx)
        {
            for (e2 += dy, y2 = start.y; e2 < ed * width && (end.y != y2 || dx > dy); e2 += dx)
            {
                x = start.x;
                y = (y2 += sy);
                if (x >= minBounds.x && y >= minBounds.y && x <= maxBounds.x && y <= maxBounds.y)
                {
                    pixel = (int *)backBuffer->memory + (y * backBuffer->width) + x;
                    *pixel = colour;
                }
            }
            if (start.x == end.x) break;
            e2 = err; err -= dy; start.x += sx; 
        }
        if (2 * e2 <= dy)
        {
            for (e2 = dx - e2; e2 < ed * width && (end.x != x2 || dx < dy); e2 += dy)
            {
                x = (x2 += sx);
                y = start.y;
                if (x >= minBounds.x && y >= minBounds.y && x <= maxBounds.x && y <= maxBounds.y)
                {
                    pixel = (int *)backBuffer->memory + (y * backBuffer->width) + x;
                    *pixel = colour;
                }
            }
            if (start.y == end.y) break;
            err += dx; start.y += sy; 
        }
    }
}

BSDEF void BS842_Plotting_Init(int plotCount, int plotWidth, int plotHeight, char *fontPath)
{
    BS842_ASSERT(!bs842_plIntInfo.initialised);
    
    for (int plotIndex = 0; plotIndex < plotCount; ++plotIndex)
    {
        BS842_Plot *currPlot = &bs842_plIntInfo.plots[plotIndex];
        
        currPlot->enabled = true;
        
        sprintf(currPlot->title, "Plot %d", plotIndex);
        
        currPlot->backBuffer.width = plotWidth;
        currPlot->backBuffer.height = plotHeight;
        currPlot->backBuffer.pitch = plotWidth * BS842_BPP;
        currPlot->backBuffer.memory = BS842_MEMALLOC(currPlot->backBuffer.height * currPlot->backBuffer.pitch);
        
        currPlot->pos = {0, 0};
        
        unsigned char* fontBuffer;
        int fontSize = 0;
        
        FILE* fontFile = fopen(fontPath, "rb");
        fseek(fontFile, 0, SEEK_END);
        fontSize = ftell(fontFile); /* how long is the file ? */
        fseek(fontFile, 0, SEEK_SET); /* reset */
        
        fontBuffer = (unsigned char *)BS842_MEMALLOC(fontSize);
        fread(fontBuffer, fontSize, 1, fontFile);
        fclose(fontFile);
        
        /* prepare font */
        bool fontInitResult = false;
        fontInitResult = stbtt_InitFont(&currPlot->fontInfo, fontBuffer, 0);
        BS842_ASSERT(fontInitResult);
        
        currPlot->optionValues = BS842_MEMALLOC(sizeof(int) * PlotOpt_Count);
        int *intOptions = (int *)currPlot->optionValues;
        float *floatOptions = (float *)currPlot->optionValues;
        floatOptions[PlotOpt_MinValX] = 0.0f;
        floatOptions[PlotOpt_MaxValX] = 100.0f;
        floatOptions[PlotOpt_MinValY] = 0.0f;
        floatOptions[PlotOpt_MaxValY] = 100.0f;
        
        intOptions[PlotOpt_Colour1] = 0xFFFF0000;
        intOptions[PlotOpt_Colour2] = 0xFF00FF00;
        intOptions[PlotOpt_ColourBack] = 0xFFDDDDDD;
        intOptions[PlotOpt_ColourSub] = 0xFF888888;
        intOptions[PlotOpt_ColourMargin] = 0xFFBBBBBB;
        intOptions[PlotOpt_ColourBorders] = 0xFF000000;
        intOptions[PlotOpt_ColourCursor] = 0xFF0000FF;
        intOptions[PlotOpt_ColourText] = 0xFF000000;
        
        intOptions[PlotOpt_MarginLeft] = 75;
        intOptions[PlotOpt_MarginRight] = 15;
        intOptions[PlotOpt_MarginBottom] = 30;
        intOptions[PlotOpt_MarginTop] = 24;
        
        // TODO(bSalmon): Should this be percentage or value based? (Value based for now)
        floatOptions[PlotOpt_MinorX] = 5.0f;
        floatOptions[PlotOpt_MajorX] = 10.0f;
        floatOptions[PlotOpt_MinorY] = 5.0f;
        floatOptions[PlotOpt_MajorY] = 25.0f;
        
    }
    
    bs842_plIntInfo.initialised = true;
}

BSDEF void BS842_Plotting_ChangePlotOption(int plotIndex, BS842_PlotOption option, int intVal, float fltVal)
{
    BS842_Plot *plot = &bs842_plIntInfo.plots[plotIndex];
    BS842_ASSERT(bs842_plIntInfo.initialised && plot->enabled);
    
    switch (option)
    {
        case PlotOpt_MinValX:
        case PlotOpt_MaxValX:
        case PlotOpt_MinValY:
        case PlotOpt_MaxValY:
        case PlotOpt_MinorX:
        case PlotOpt_MajorX:
        case PlotOpt_MinorY:
        case PlotOpt_MajorY:
        {
            float *floatOptions = (float *)plot->optionValues;
            floatOptions[option] = fltVal;
        } break;
        
        default:
        {
            int *intOptions = (int *)plot->optionValues;
            intOptions[option] = intVal;
        } break;
    }
}

BSDEF void BS842_Plotting_GetPlotOption(void *result, int plotIndex, BS842_PlotOption option)
{
    BS842_Plot *plot = &bs842_plIntInfo.plots[plotIndex];
    BS842_ASSERT(bs842_plIntInfo.initialised && plot->enabled);
    
    switch (option)
    {
        case PlotOpt_MinValX:
        case PlotOpt_MaxValX:
        case PlotOpt_MinValY:
        case PlotOpt_MaxValY:
        case PlotOpt_MinorX:
        case PlotOpt_MajorX:
        case PlotOpt_MinorY:
        case PlotOpt_MajorY:
        {
            float *floatOptions = (float *)plot->optionValues;
            *(float *)result = floatOptions[option];
        } break;
        
        default:
        {
            int *intOptions = (int *)plot->optionValues;
            *(int *)result = intOptions[option];
        } break;
    }
    
}

BSDEF void BS842_Plotting_PlotData(int plotIndex, s32 dataSetIndex, float *xData, float *yData, int datumCount)
{
    BS842_Plot *plot = &bs842_plIntInfo.plots[plotIndex];
    BS842_ASSERT(bs842_plIntInfo.initialised && plot->enabled);
    
    if (plot->data[dataSetIndex].xData) { BS842_MEMFREE(plot->data[dataSetIndex].xData); }
    if (plot->data[dataSetIndex].yData) { BS842_MEMFREE(plot->data[dataSetIndex].yData); }
    
    plot->data[dataSetIndex].xData = (float *)BS842_MEMALLOC(datumCount * sizeof(float));
    plot->data[dataSetIndex].yData = (float *)BS842_MEMALLOC(datumCount * sizeof(float));
    plot->data[dataSetIndex].datumCount = datumCount;
    
    float maxValY = ((float *)plot->optionValues)[PlotOpt_MaxValY];
    for (int datumIndex = 0; datumIndex < datumCount; ++datumIndex)
    {
        plot->data[dataSetIndex].xData[datumIndex] = xData[datumIndex];
        plot->data[dataSetIndex].yData[datumIndex] = maxValY - yData[datumIndex];
    }
}

BSDEF void BS842_Plotting_PlotTitle(int plotIndex, char *title)
{
    BS842_Plot *plot = &bs842_plIntInfo.plots[plotIndex];
    BS842_ASSERT(bs842_plIntInfo.initialised && plot->enabled);
    
    char *c = title;
    char *dest = plot->title;
    while (*c)
    {
        *dest++ = *c++;
    }
}

BSDEF void BS842_Plotting_UpdatePlot(int plotIndex)
{
    // TODO(bSalmon): SIMD or just general optimisation
    BS842_Plot *plot = &bs842_plIntInfo.plots[plotIndex];
    BS842_ASSERT(bs842_plIntInfo.initialised && plot->enabled);
    
    int rightEdge = plot->backBuffer.width - 1;
    int bottomEdge = plot->backBuffer.height - 1;
    
    int borderColour = ((int *)plot->optionValues)[PlotOpt_ColourBorders];
    int backgroundColour = ((int *)plot->optionValues)[PlotOpt_ColourBack];
    int marginColour = ((int *)plot->optionValues)[PlotOpt_ColourMargin];
    int subColour = ((int *)plot->optionValues)[PlotOpt_ColourSub];
    int cursorColour = ((int *)plot->optionValues)[PlotOpt_ColourCursor];
    int textColour = ((int *)plot->optionValues)[PlotOpt_ColourText];
    
    int colour[2] = {((int *)plot->optionValues)[PlotOpt_Colour1], ((int *)plot->optionValues)[PlotOpt_Colour2]};
    
    int marginLeft = ((int *)plot->optionValues)[PlotOpt_MarginLeft];
    int marginRight = ((int *)plot->optionValues)[PlotOpt_MarginRight];
    int marginTop = ((int *)plot->optionValues)[PlotOpt_MarginTop];
    int marginBottom = ((int *)plot->optionValues)[PlotOpt_MarginBottom];
    
    unsigned char *row = (unsigned char *)plot->backBuffer.memory;
    for (int y = 0; y < plot->backBuffer.height; ++y)
    {
        int *pixel = (int *)row;
        for (int x = 0; x < plot->backBuffer.width; ++x)
        {
            if (x == 0 || y == 0 ||
                x == rightEdge || y == bottomEdge ||
                (x == marginLeft && y >= marginTop && y <= bottomEdge - marginBottom) ||
                (x == rightEdge - marginRight && y >= marginTop && y <= bottomEdge - marginBottom) ||
                (y == marginTop && x >= marginLeft && x <= rightEdge - marginRight) ||
                (y == bottomEdge - marginBottom && x >= marginLeft && x <= rightEdge - marginRight))
            {
                *pixel++ = borderColour;
            }
            else if (x < marginLeft || y < marginTop ||
                     x > rightEdge - marginRight || y > bottomEdge - marginBottom)
            {
                *pixel++ = marginColour;
            }
            else
            {
                *pixel++ = backgroundColour;
            }
            
        }
        
        row += plot->backBuffer.pitch;
    }
    
    BS842_Internal_V2I plotMin = {marginLeft + 1, marginTop + 1};
    BS842_Internal_V2I plotMax = {(rightEdge - marginRight) - 1, (bottomEdge - marginBottom) - 1};
    
    BS842_Internal_V2I plotDims = {(plotMax.x - plotMin.x), (plotMax.y - plotMin.y)};
    
    BS842_Internal_V2F minVal = {((float *)plot->optionValues)[PlotOpt_MinValX], ((float *)plot->optionValues)[PlotOpt_MinValY]};
    BS842_Internal_V2F maxVal = {((float *)plot->optionValues)[PlotOpt_MaxValX], ((float *)plot->optionValues)[PlotOpt_MaxValY]};
    
    BS842_Internal_V2F minor = {((float *)plot->optionValues)[PlotOpt_MinorX], ((float *)plot->optionValues)[PlotOpt_MinorY]};
    BS842_Internal_V2F major = {((float *)plot->optionValues)[PlotOpt_MajorX], ((float *)plot->optionValues)[PlotOpt_MajorY]};
    
    BS842_Internal_V2I minorMarksCount = {0, 0};
    BS842_Internal_V2I majorMarksCount = {0, 0};
    
    float *minorMarksX = BS842_Internal_GetMarks(&minorMarksCount.x, minVal.x, maxVal.x, minor.x);
    float *majorMarksX = BS842_Internal_GetMarks(&majorMarksCount.x, minVal.x, maxVal.x, major.x);
    float *minorMarksY = BS842_Internal_GetMarks(&minorMarksCount.y, minVal.y, maxVal.y, minor.y);
    float *majorMarksY = BS842_Internal_GetMarks(&majorMarksCount.y, minVal.y, maxVal.y, major.y);
    
    for (int y = plotMin.y; y <= plotMax.y; ++y)
    {
        for (int minorIndex = 0; minorIndex < minorMarksCount.y; ++minorIndex)
        {
            if (BS842_Internal_GetValueInMappedRanges(minorMarksY[minorIndex], minVal.y, maxVal.y, plotMin.y, plotMax.y) == y)
            {
                BS842_Internal_DrawDottedHorizontalLine(&plot->backBuffer, plotMin.x, plotMax.x, y, subColour);
            }
        }
        
        for (int majorIndex = 0; majorIndex < majorMarksCount.y; ++majorIndex)
        {
            if (BS842_Internal_GetValueInMappedRanges(majorMarksY[majorIndex], minVal.y, maxVal.y, plotMin.y, plotMax.y) == y)
            {
                BS842_Internal_DrawBresenhamLine(&plot->backBuffer, {plotMin.x, y}, {plotMax.x, y}, plotMin, plotMax, 1.0f, subColour);
                
                // TODO(bSalmon): Calc x gap from line using string length
                float textLineHeight = 13.0f;
                char majorText[16] = {};
                sprintf(majorText, "%.01f\0", majorMarksY[(majorMarksCount.y - 1) - majorIndex]);
                BS842_Internal_DrawText(&plot->backBuffer, plot->fontInfo, majorText, {plotMin.x - (int)textLineHeight * 2, y},
                                        textLineHeight, textColour);
            }
        }
    }
    
    for (int x = plotMin.x; x <= plotMax.x; ++x)
    {
        for (int minorIndex = 0; minorIndex < minorMarksCount.x; ++minorIndex)
        {
            if (BS842_Internal_GetValueInMappedRanges(minorMarksX[minorIndex], minVal.x, maxVal.x, plotMin.x, plotMax.x) == x)
            {
                BS842_Internal_DrawDottedVerticalLine(&plot->backBuffer, plotMin.y, plotMax.y, x, subColour);
            }
        }
        
        for (int majorIndex = 0; majorIndex < majorMarksCount.x; ++majorIndex)
        {
            if (BS842_Internal_GetValueInMappedRanges(majorMarksX[majorIndex], minVal.x, maxVal.x, plotMin.x, plotMax.x) == x)
            {
                BS842_Internal_DrawBresenhamLine(&plot->backBuffer, {x, plotMin.y}, {x, plotMax.y}, plotMin, plotMax, 1.0f, subColour);
                
                float textLineHeight = 13.0f;
                char majorText[16] = {};
                sprintf(majorText, "%.01f\0", majorMarksX[majorIndex]);
                BS842_Internal_DrawText(&plot->backBuffer, plot->fontInfo, majorText, {x, plotMax.y + (int)textLineHeight},
                                        textLineHeight, textColour);
            }
        }
    }
    
    if (minorMarksX) { BS842_MEMFREE(minorMarksX); }
    if (majorMarksX) { BS842_MEMFREE(majorMarksX); }
    if (minorMarksY) { BS842_MEMFREE(minorMarksY); }
    if (majorMarksY) { BS842_MEMFREE(majorMarksY); }
    
    float titleLineHeight = (float)(marginTop - 4);
    BS842_Internal_DrawText(&plot->backBuffer, plot->fontInfo, plot->title, {plotMin.x + (plotDims.x / 2), (int)((marginTop / 2.0f) + 0.5f)},
                            titleLineHeight, textColour);
    
    for (int dataSetIndex = 0; dataSetIndex < BS842_ARRAY_COUNT(plot->data); ++dataSetIndex)
    {
        BS842_Internal_V2I prevMapped = {0, 0};
        for (int point = 0; point < plot->data[dataSetIndex].datumCount; ++point)
        {
            BS842_Internal_V2I mapped =
            {BS842_Internal_GetValueInMappedRanges(plot->data[dataSetIndex].xData[point], minVal.x, maxVal.x, plotMin.x, plotMax.x),
                BS842_Internal_GetValueInMappedRanges(plot->data[dataSetIndex].yData[point], minVal.y, maxVal.y, plotMin.y, plotMax.y)};
            
            if (point > 0)
            {
                BS842_Internal_DrawBresenhamLine(&plot->backBuffer, prevMapped, mapped, plotMin, plotMax, 1.0f, colour[dataSetIndex]);
            }
            
            prevMapped = mapped;
        }
    }
    
    if (plot->mousePosRel.x >= 0 && plot->mousePosRel.y >= 0 &&
        plot->mousePosRel.x <= plotDims.x && plot->mousePosRel.y <= plotDims.y)
    {
        if (!plot->highlightActive)
        {
            int tempCursor = plot->mousePosRel.x + plotMin.x;
            BS842_Internal_DrawBresenhamLine(&plot->backBuffer, {tempCursor, plotMin.y}, {tempCursor, plotMax.y}, plotMin, plotMax, 1.0f, cursorColour);
            
            if (bs842_plIntInfo.mouseLeftDown)
            {
                plot->highlightActive = true;
                plot->cursor1Pos = plot->mousePosRel.x + plotMin.x;
                plot->cursor2Pos = plot->cursor1Pos;
            }
        }
        else
        {
            plot->cursor2Pos = plot->mousePosRel.x + plotMin.x;
            BS842_Internal_DrawBresenhamLine(&plot->backBuffer, {plot->cursor1Pos, plotMin.y}, {plot->cursor1Pos, plotMax.y}, plotMin, plotMax, 1.0f, cursorColour);
            BS842_Internal_DrawBresenhamLine(&plot->backBuffer, {plot->cursor2Pos, plotMin.y}, {plot->cursor2Pos, plotMax.y}, plotMin, plotMax, 1.0f, cursorColour);
            
            
            if (!bs842_plIntInfo.mouseLeftDown)
            {
                plot->highlightActive = false;
            }
        }
        
        if (bs842_plIntInfo.mouseRightDown)
        {
            plot->cursor1Pos = plot->cursor2Pos = -1;
        }
    }
    
    int start = BS842_MIN(plot->cursor1Pos, plot->cursor2Pos);
    int end = BS842_MAX(plot->cursor1Pos, plot->cursor2Pos);
    if ((start - end) != 0)
    {
        for (int y = plotMin.y; y < plotMax.y; ++y)
        {
            for (int x = start + 1; x < end; ++x)
            {
                int *pixel = (int *)plot->backBuffer.memory + (y * plot->backBuffer.width) + x;
                if (*pixel == backgroundColour)
                {
                    *pixel = (~(*pixel)) | 0xFF000000;
                }
            }
        }
    }
    
}

BSDEF void BS842_Plotting_ResizePlot(int plotIndex, int width, int height, int posX = -1, int posY = -1)
{
    BS842_Plot *plot = &bs842_plIntInfo.plots[plotIndex];
    BS842_ASSERT(bs842_plIntInfo.initialised && plot->enabled);
    
    BS842_MEMFREE(plot->backBuffer.memory);
    
    plot->backBuffer.width = width;
    plot->backBuffer.height = height;
    plot->backBuffer.pitch = width * BS842_BPP;
    plot->backBuffer.memory = BS842_MEMALLOC(plot->backBuffer.height * plot->backBuffer.pitch);
    
    if (posX >= 0)
    {
        plot->pos.x = posX;
    }
    if (posY >= 0)
    {
        plot->pos.y = posY;
    }
}

BSDEF void BS842_Plotting_DrawPlotMemory(int plotIndex, void *destBuffer, int destPitch)
{
    // TODO(bSalmon): SIMD
    BS842_Plot *plot = &bs842_plIntInfo.plots[plotIndex];
    BS842_ASSERT(bs842_plIntInfo.initialised && plot->enabled);
    
    unsigned char *srcRow = (unsigned char *)plot->backBuffer.memory;
    unsigned char *destRow = (unsigned char *)destBuffer + (plot->pos.y * destPitch) + (plot->pos.x * BS842_BPP);
    for (int y = 0; y < plot->backBuffer.height; ++y)
    {
        int *srcPixel = (int *)srcRow;
        int *destPixel = (int *)destRow;
        for (int x = 0; x < plot->backBuffer.width; ++x)
        {
            *destPixel++ = *srcPixel++;
        }
        
        srcRow += plot->backBuffer.pitch;
        destRow += destPitch;
    }
}

BSDEF void BS842_Plotting_MouseInfo(int mouseX, int mouseY, bool mouseLeftDown, bool mouseRightDown)
{
    BS842_ASSERT(bs842_plIntInfo.initialised);
    
    bs842_plIntInfo.mousePos = {mouseX, mouseY};
    bs842_plIntInfo.mouseLeftDown = mouseLeftDown;
    bs842_plIntInfo.mouseRightDown = mouseRightDown;
    
    for (int plotIndex = 0; plotIndex < BS842_ARRAY_COUNT(bs842_plIntInfo.plots); ++plotIndex)
    {
        BS842_Plot *currPlot = &bs842_plIntInfo.plots[plotIndex];
        if (currPlot->enabled)
        {
            int marginLeft = ((int *)currPlot->optionValues)[PlotOpt_MarginLeft];
            int marginTop = ((int *)currPlot->optionValues)[PlotOpt_MarginTop];
            BS842_Internal_V2I plotMin = {marginLeft + 1, marginTop + 1};
            
            currPlot->mousePosRel = {mouseX - (currPlot->pos.x + plotMin.x), mouseY - (currPlot->pos.y + plotMin.y)};
        }
    }
}

#endif