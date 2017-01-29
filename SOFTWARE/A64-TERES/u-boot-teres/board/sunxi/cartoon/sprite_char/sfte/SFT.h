/*
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Jerry Wang <wangflord@allwinnertech.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#ifndef  __SFT_h
#define  __SFT_h

#include <common.h>

#ifdef SFT_OS_WIN32
	#define __PACKED__
#else
	#define __PACKED__        __packed
#endif


#define SFT_MAGIC_SIZE            4
#define SFT_VERSION_SIZE          4

#define SFT_MAGIC             "SFTT"
#define VERSION               "2.OO"


  /*************************************************************************/
  /*                                                                       */
  /* <Enum>                                                                */
  /*    sft_pixel_mode_e                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    An enumeration type used to describe the format of pixels in a     */
  /*    given bitmap.                                                      */
  /*                                                                       */
  /* <Values>                                                              */
  /*    SFT_PIXEL_MODE_MONO ::                                             */
  /*      A monochrome bitmap, using 1 bit per pixel.  Note that pixels    */
  /*      are stored in most-significant order (MSB), which means that     */
  /*      the left-most pixel in a byte has value 128.                     */
  /*                                                                       */
  /*    SFT_PIXEL_MODE_GRAY ::                                             */
  /*      An 8-bit bitmap, generally used to represent anti-aliased glyph  */
  /*      images.  Each pixel is stored in one byte.  Note that the number */
  /*      of value `gray' levels is stored in the `num_bytes' field of     */
  /*      the @FT_Bitmap structure (it generally is 256).                  */
  /*************************************************************************/
typedef enum _sft_pixel_mode_e
{
	SFT_PIXEL_MODE_MONO = 1,
    SFT_PIXEL_MODE_GRAY = 2,

    SFT_PIXEL_MODE_
}sft_pixel_mode_e;


//#ifdef SFT_OS_WIN32
//	#pragma pack(push, 1)
//#endif // #ifdef SFT_OS_WIN32
//
//typedef __PACKED__ struct _SFT_Vector
//{
//  __u16  x;
//  __u16  y;
//} SFT_Vector;
//
//#ifdef SFT_OS_WIN32
//	#pragma pack(pop)
//#endif // #ifdef SFT_OS_WIN32


typedef struct
{
  unsigned short  x;
  unsigned short  y;
}
__attribute__ ((packed)) SFT_Vector;
  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    glyph_t                                                            */
  /*                                                                       */
  /* <Fields>                                                              */
  /*                                                                       */
  /*    advance      :: This is the transformed advance width for the      */
  /*                    glyph.                                             */
  /*                                                                       */
  /*    rows         :: The number of bitmap rows.                         */
  /*                                                                       */
  /*    width        :: The number of pixels in bitmap row.                */
  /*                                                                       */
  /*    pitch        :: The pitch's absolute value is the number of bytes  */
  /*                    taken by one bitmap row, including padding.        */
  /*                    However, the pitch is positive when the bitmap has */
  /*                    a `down' flow, and negative when it has an `up'    */
  /*                    flow.  In all cases, the pitch is an offset to add */
  /*                    to a bitmap pointer in order to go down one row.   */
  /*                                                                       */
  /*    pixel_mode   :: The pixel mode, i.e., how pixel bits are stored.   */
  /*                    See @FT_Pixel_Mode for possible values.            */
  /*                                                                       */
  /*    bitmap_left  :: This is the bitmap's left bearing expressed        */
  /*                    in integer pixels.  Of course, this is only        */
  /*                    valid if the format is                             */
  /*                    @FT_GLYPH_FORMAT_BITMAP.                           */
  /*                                                                       */
  /*    bitmap_top   :: This is the bitmap's top bearing expressed in      */
  /*                    integer pixels.  Remember that this is the         */
  /*                    distance from the baseline to the top-most         */
  /*                    glyph scanline, upwards y-coordinates being        */
  /*                    *positive*.                                        */
  /*                                                                       */
  /*************************************************************************/
//#ifdef SFT_OS_WIN32
//	#pragma pack(push, 1)
//#endif // #ifdef SFT_OS_WIN32

typedef struct
{
    SFT_Vector  		 advance;
    short       		 bitmap_left;
    short       		 bitmap_top;
    unsigned short       rows;
    unsigned short       width;
    short       		 pitch;
    short       		 pixel_mode;
}
__attribute__ ((packed)) glyph_t;

//#ifdef SFT_OS_WIN32
//	#pragma pack(pop)
//#endif // #ifdef SFT_OS_WIN32



typedef struct _size_info_t
{
	unsigned int  pixel_size;
    unsigned int  ascender;    /* ascender in 26.6 frac. pixels               */
    unsigned int  descender;   /* descender in 26.6 frac. pixels              */
    unsigned int  height;
    unsigned int  horiBearingY;
    unsigned int  glyph_index_table_offset;
    unsigned int  glyph_xadvance_table_offset;
}size_info_t;


typedef struct _sft_file_head_t
{
	char   		  magic[SFT_MAGIC_SIZE];          // magic
	char   		  version[SFT_VERSION_SIZE];      // version
	unsigned int  file_head_size;             // the size of the SFT file head
	unsigned int  char_nr;                    // the number of all characters contained in the file
	unsigned int  pixel_size_nr;              // the number of all pixel sizes supported by the SFT file
	unsigned int  pixel_size_tbl_offset;      // the offset of pixel size table off the start of the file.
    unsigned int  units_per_EM;
    int           ascender;
    int  		  descender;
    unsigned int  height;
	unsigned int  reserved[4];                // reserved
}sft_file_head_t;



#endif     //  ifndef __SFT_h

/* end of SFT.h */
