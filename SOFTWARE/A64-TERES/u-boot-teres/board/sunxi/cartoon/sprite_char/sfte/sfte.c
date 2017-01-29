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
#ifndef  __sfte_c
#define  __sfte_c

#include <common.h>
#include <linux/types.h>
#include <asm/io.h>
#include "sfte.h"
#include "SFT.h"
#include <linux/string.h>
#include <malloc.h>
#undef  _4_char_to_int32
#define _4_char_to_int32(a,b,c,d)       ( (a)<<24 | (b)<<16 | (c)<<8 | (d) )

#define SFTE_LABEL            _4_char_to_int32( 'S', 'F', 'T', 'E' )
#define INVALID_LABEL         _4_char_to_int32( 'X', 'X', 'X', 'X' )

#define INVALID_PIXEL_SIZE    0
#define INVALID_UNICODE       ((unsigned short)0xFFFF)
#define  SEEK_SET                    0

typedef struct SFTE_Bitmap_Size_
{
	size_info_t                  size_info;
	struct  SFTE_Bitmap_Size_   *next;
}SFTE_Bitmap_Size;


typedef struct _sfte_prvt_data_t
{
    int         label;                          // handle valid label
	void         *fp;                             // file pointer to the sft file
    size_info_t  *size_info_list;                 // list of all available pixel sizes
	size_info_t   cur_pixel_size;                 // current pixel size
	unsigned int         bitmap_buf_size;                // the size of the bitmap buffer
	unsigned int         unicode;                        // a cache for the current char
#ifdef SFT_USE_MUTEX
	g_mutex_t     mutex;                          // a mutex
#endif // #ifdef SFT_USE_MUTEX
}sfte_prvt_data_t;


static int  init_face( SFTE_Face face, void *fp );
static u8	  is_valid_face( SFTE_Face face );
static void   release_hdl_res( SFTE_Face face );
static int  SFTE_Get_Glyph_i( SFTE_Face face, __u16 unicode );
static unsigned int  SFTE_Get_XAdvance_i( SFTE_Face face, __u16 unicode );
static int  SFTE_Set_Pixel_Sizes_i( SFTE_Face face, unsigned int pixel_size );
static int  SFTE_Get_Pixel_Size_Count_i( SFTE_Face face );
static int  SFTE_Get_Pixel_Size_List_i( SFTE_Face face, unsigned int *list_p, unsigned int count );


SFTE_Face SFTE_New_Face( char *font_file)
{
	SFTE_Face	face = NULL;
	void		*fp   = NULL;
	char		load_addr[16] = { 0 };
	char *const ui_char_argv[6] = { "fatload", "sunxi_flash", "0:0", load_addr, font_file, NULL };

	/*request buffer to ch_lib, size is 1M*/
	fp = (void *)malloc(1024 * 1024);
	if(!fp)
	{
		printf("sunxi uichar error: unable to malloc memory for sft\n");

		goto error;
	}
	sprintf(load_addr,"%p",fp);
	/*request buffer to ch_lib, size is 1M*/
	face = (struct SFTE_FaceRec_ *) malloc( sizeof(struct SFTE_FaceRec_) );
	if( face == NULL )
	{
		printf("Error in allocating memory\n");
		goto error;
	}

	memset( face, 0, sizeof(struct SFTE_FaceRec_) );

	if(do_fat_fsload(0, 0, 5, ui_char_argv))
	{
		printf("sunxi ui_char info error : unable to open font file %s\n", ui_char_argv[4]);

		goto error;
	}

	if( init_face( face, fp ) != 0 )
	{
		printf("Error in initialising face\n");
		goto error;
	}

	return face;

error:
	if( fp != NULL )
		free( fp );
	if( face != NULL )
		free( face );

	return NULL;
}



int  SFTE_Done_Face( SFTE_Face face )
{
	if( !is_valid_face( face ) )
	{
		printf("Fail in destroying face. Because face is invalid\n");
		return -1;
	}

	release_hdl_res( face );

	free( face );

	return 0;
}




int  SFTE_Set_Pixel_Sizes( SFTE_Face face, unsigned int pixel_size )
{
	int              ret;

	debug("pixel_size = %d\n", pixel_size);
	if( !is_valid_face( face ) )
	{
		printf("Fail in setting pixel size %x. Because face is invalid\n", pixel_size);
		return 0;
	}

	ret = SFTE_Set_Pixel_Sizes_i( face, pixel_size );

	return ret;
}



int  SFTE_Get_Pixel_Sizes( SFTE_Face face, unsigned int pixel_size )
{
	if( !is_valid_face( face ) )
	{
		printf("Fail in setting pixel size %x. Because face is invalid\n", pixel_size);
		return 0;
	}

	return SFTE_Get_Pixel_Size_Count_i( face );
}



int  SFTE_Get_Pixel_Size_List( SFTE_Face face, unsigned int *list_p, unsigned int count )
{
	if( !is_valid_face( face ) || list_p == NULL )
	{
		return 0;
	}

	return SFTE_Get_Pixel_Size_List_i( face, list_p, count );
}



int  SFTE_Get_Pixel_Size_Count( SFTE_Face face )
{
	int              ret;

	if( !is_valid_face( face ) )
	{
		return 0;
	}

	ret = SFTE_Get_Pixel_Size_Count_i( face );

	return ret;
}


int  SFTE_Get_Glyph( SFTE_Face face, __u16 unicode )
{
	unsigned int              ret;

	if( !is_valid_face( face ) )
	{
		printf("Fail in getting glyph of unicode %x. Because face is invalid\n", unicode);
		return 0;
	}

	ret = SFTE_Get_Glyph_i( face, unicode );

	return ret;
}



unsigned int  SFTE_Get_XAdvance( SFTE_Face face, __u16 unicode )
{
	unsigned int              ret;

	if( !is_valid_face( face ) )
	{
		printf("Fail in getting x advance of unicode %x. Because face is invalid\n", unicode);
		return 0;
	}

	ret = SFTE_Get_XAdvance_i( face, unicode );

	return ret;
}



static int  SFTE_Get_Glyph_i( SFTE_Face face, __u16 unicode )
{
	void				*fp;
	char				*offset_fp;
	glyph_t				*glyph;
	unsigned int				pitch_abs;
	unsigned int				bitmap_size;
	sfte_prvt_data_t	*pd;
	size_info_t			*p;
	unsigned int				offset;

	pd = (sfte_prvt_data_t *)face->hidden;

	if( pd->cur_pixel_size.pixel_size == INVALID_PIXEL_SIZE )
	{
		printf("Fail in getting glyph of unicode %x. Because pixel size is NOT valid.\n", unicode);
		return -1;
	}

	/* check whether the input unicode is the current unicode */
	if( pd->unicode != INVALID_UNICODE && pd->unicode == unicode )
	{
		return 0;
	}

	fp = pd->fp;
	p  = &(pd->cur_pixel_size);
	offset_fp = fp;
/*	get glyph's offset
 *  I have no fseek or fread functions, so I only get offset with
 *  point operate
 *
*/
	offset = * (unsigned int *) (offset_fp + p->glyph_index_table_offset + unicode * sizeof(unsigned int));
	if( offset == 0 )                   // the char doesn't been contained.
	{
		return -1;
	}

	/* get glyph's information */
	glyph =(glyph_t *)(offset_fp + offset);
	face->glyph->advance.x    = glyph->advance.x  ;
	face->glyph->advance.y    = glyph->advance.y  ;
	face->glyph->bitmap_left  = glyph->bitmap_left;
	face->glyph->bitmap_top   = glyph->bitmap_top ;
	face->glyph->bitmap.rows  = glyph->rows;
	face->glyph->bitmap.width = glyph->width;
	face->glyph->bitmap.pitch = glyph->pitch;
	switch( glyph->pixel_mode )
	{
		case SFT_PIXEL_MODE_MONO:
			face->glyph->bitmap.pixel_mode = SFTE_PIXEL_MODE_MONO;
			break;
		case SFT_PIXEL_MODE_GRAY:
			face->glyph->bitmap.pixel_mode = SFTE_PIXEL_MODE_GRAY;
			break;
		default :
			printf("Error. Inlegal pixel mode %u\n", glyph->pixel_mode);
			return -1;
	}

	/* get bitmap matrix */
	if( glyph->pitch >0 )
		pitch_abs = glyph->pitch;
	else
		pitch_abs = (unsigned int)-glyph->pitch;
	bitmap_size = glyph->rows * pitch_abs;
	if( bitmap_size != 0 )
	{
		if(    face->glyph->bitmap.buffer != NULL
			&& pd->bitmap_buf_size < bitmap_size )
		{
			printf("current bitmap buffer size is %u and new bitmap size is %u.\n"
		    	  "pitch abs is %u and glyph rows is %u.\n", pd->bitmap_buf_size, bitmap_size, pitch_abs, glyph->rows);
			free( face->glyph->bitmap.buffer );
			face->glyph->bitmap.buffer = NULL;
			pd->bitmap_buf_size        = 0;
		}
		if( face->glyph->bitmap.buffer == NULL )
		{
			face->glyph->bitmap.buffer = (__u8 *) malloc( bitmap_size );
			if( face->glyph->bitmap.buffer == NULL )
			{
				printf("Error in getting glyph of unicode %x. can't allocate memory\n", unicode);
				return -1;
			}
			pd->bitmap_buf_size = bitmap_size;
		}
		memset(face->glyph->bitmap.buffer,0,bitmap_size);
		memcpy(face->glyph->bitmap.buffer,offset_fp + offset + sizeof(glyph_t),bitmap_size);
	}

	pd->unicode = unicode;

	return 0;
}



static unsigned int  SFTE_Get_XAdvance_i( SFTE_Face face, __u16 unicode )
{
	void              *fp;
	char			  *char_fp;
	sfte_prvt_data_t  *pd;
	size_info_t       *p;
	__u8               xadvance;

	pd = (sfte_prvt_data_t *)face->hidden;

	if( pd->cur_pixel_size.pixel_size == INVALID_PIXEL_SIZE )
	{
		printf("Fail in getting glyph of unicode %x. Because pixel size is NOT valid.\n", unicode);
		return 0;
	}

	/* check whether the input unicode is the current unicode */
	if( pd->unicode != INVALID_UNICODE && pd->unicode == unicode )
	{
		return face->glyph->advance.x >> 6;
	}

	fp = pd->fp;
	p  = &(pd->cur_pixel_size);
	char_fp = fp;
	/* get glyph's xadvance */
	memcpy(&xadvance, char_fp + p->glyph_xadvance_table_offset + unicode * sizeof(__u8) ,1);
	if( xadvance == 0 )                   // the char doesn't been contained.
	{
		return 0;
	}

	return xadvance;
}



static int  SFTE_Set_Pixel_Sizes_i( SFTE_Face face, unsigned int pixel_size )
{
	size_info_t        *p;
	unsigned int               border_size;
	unsigned int               buf_size;
	sfte_prvt_data_t   *pd;
	unsigned int               i;

	pd = (sfte_prvt_data_t *)face->hidden;

	/* check whether the pixel size is supported by the sft file */
	for( i = 0;  i < face->num_fixed_sizes;  i++ )
	{
		p = pd->size_info_list + i;
		if( p->pixel_size == pixel_size )
		{
			if( pd->cur_pixel_size.pixel_size == pixel_size )    // is the current size
			{
				return 0;
			}

			/* allocate bitmap buffer */
			border_size = pixel_size + ( pixel_size >> 1 );
			buf_size    = border_size * border_size;
			if(    face->glyph->bitmap.buffer != NULL
				&& pd->bitmap_buf_size < buf_size )
			{
				free( face->glyph->bitmap.buffer );
				face->glyph->bitmap.buffer = NULL;
				pd->bitmap_buf_size        = 0;
			}
			if( face->glyph->bitmap.buffer == NULL )
			{
				face->glyph->bitmap.buffer = (__u8 *) malloc( buf_size );
				printf("[%s] %d buf_size is %d\n",__func__,__LINE__,buf_size);
				if( face->glyph->bitmap.buffer == NULL )
				{
					printf("Error in setting pixel size. can't allocate memory\n");
					return -1;
				}
			}
			/* fill size information */
			face->size->metrics.ascender      = p->ascender;
			face->size->metrics.descender     = p->descender;
			face->glyph->metrics.height       = p->height       ;
	        face->glyph->metrics.horiBearingY = p->horiBearingY ;
	        memcpy( &(pd->cur_pixel_size), p, sizeof(size_info_t) );
			printf("cur_pixel_size %d pixel_size %d\n",pd->cur_pixel_size.pixel_size,p->pixel_size);
			pd->unicode = INVALID_UNICODE;

	        return 0;
		}
	}

	printf("Fail in setting pixel size. the pixel size %u is not supported by the sft file.\n", pixel_size);

	return -1;
}



static int  SFTE_Get_Pixel_Size_Count_i( SFTE_Face face )
{
	return face->num_fixed_sizes;
}


//#define min(x,y)                                 (((x)<(y))?(x):(y))
static int  SFTE_Get_Pixel_Size_List_i( SFTE_Face face, unsigned int *list_p, unsigned int count )
{
	size_info_t        *p;
	sfte_prvt_data_t   *pd;
	unsigned int               i;
	unsigned int               nr;

	pd = (sfte_prvt_data_t *)face->hidden;

	/* get size list */
	nr = min( count, face->num_fixed_sizes );
	for( i = 0;  i < nr;  i++ )
	{
		p = pd->size_info_list + i;
		list_p[i] = p->pixel_size;
    }

    return nr;
}



static u8 is_valid_face( SFTE_Face face )
{
	sfte_prvt_data_t  *pd = NULL;

	if( face == NULL )
		return 0;

	pd = (sfte_prvt_data_t *)face->hidden;
	if( pd->label != SFTE_LABEL )
		return 0;

	return 1;
}



static int init_face( SFTE_Face face, void *fp )
{
	sfte_prvt_data_t           *pd = NULL;
	sft_file_head_t             *h;
	char						*char_fp;

	h=fp;
	face->num_fixed_sizes = h->pixel_size_nr;
	face->num_glyphs      = h->char_nr;
	face->units_per_EM    = h->units_per_EM;
	face->ascender        = h->ascender;
	face->descender       = h->descender;
	face->height          = h->height;
	face->size = (SFTE_SizeRec *) malloc( sizeof(SFTE_SizeRec) );
	if( face->size == NULL )
	{
		printf("Error in allocating memory\n");
		return -1;
	}

	/* allcate a buffer to store glyph */
	face->glyph = (struct SFTE_GlyphSlotRec_ *) malloc( sizeof(struct SFTE_GlyphSlotRec_) );
	if( face->glyph == NULL )
	{
		printf("Error in allocating memory\n");
		goto error;
	}
	memset( face->glyph, 0, sizeof(struct SFTE_GlyphSlotRec_) );

	face->hidden = (void *) malloc( sizeof(sfte_prvt_data_t) );
	if( face->hidden == NULL )
	{
		printf("Error in allocating memory\n");
		return -1;
	}
	memset( face->hidden, 0, sizeof(sfte_prvt_data_t) );
	pd = (sfte_prvt_data_t *)face->hidden;

	/* get the list of all the pixel sizes */
	pd->size_info_list = (size_info_t *) malloc( sizeof(size_info_t) * face->num_fixed_sizes );
	if( pd->size_info_list == NULL )
	{
		printf("Error in allocating memory\n");
		goto error;
	}
	char_fp = fp;

	memcpy(pd->size_info_list,char_fp + h->pixel_size_tbl_offset,sizeof(size_info_t) * face->num_fixed_sizes);
	pd->label            = SFTE_LABEL;
	pd->fp               = fp;
	pd->cur_pixel_size.pixel_size = INVALID_PIXEL_SIZE;
	pd->bitmap_buf_size  = 0;
	pd->unicode          = INVALID_UNICODE;

#ifdef SFT_USE_MUTEX
	pd->mutex = g_create_mutex( );
	if( pd->mutex == NULL )
	{
		printf("Error in creating a mutex.\n");
		goto error;
	}
#endif // #ifdef SFT_USE_MUTEX

	return 0;

error:
	release_hdl_res( face );
	return -1;
}



static void   release_hdl_res( SFTE_Face face )
{
	if( face->hidden != NULL )
	{
		sfte_prvt_data_t  *pd = NULL;

		pd = (sfte_prvt_data_t *)face->hidden;
		if( pd->size_info_list != NULL )
		{
			free( pd->size_info_list );
			pd->size_info_list = NULL;
		}
		if( pd->fp != NULL )
		{
			free( pd->fp );
			pd->fp = NULL;
		}
#ifdef SFT_USE_MUTEX
		if( pd->mutex != NULL )
		{
			__u8  err;

			g_delete_mutex( pd->mutex, &err );
			pd->mutex = NULL;
		}
#endif // #ifdef SFT_USE_MUTEX
		pd->label = INVALID_LABEL;
		free( face->hidden );
		face->hidden = NULL;
	}
	if( face->glyph != NULL )
	{
		if( face->glyph->bitmap.buffer != NULL )
		{
			free( face->glyph->bitmap.buffer );
			face->glyph->bitmap.buffer = NULL;
		}
		free( face->glyph );
		face->glyph = NULL;
	}
	if( face->size != NULL )
	{
		free( face->size );
		face->size = NULL;
	}
}



#endif     //  ifndef __sfte_c

/* end of sfte.c */
