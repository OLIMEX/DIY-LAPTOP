#ifndef __HDMI_BSP_H_
#define __HDMI_BSP_H_

#define LINUX_OS

#if defined(__LIB__)
#if defined(__ARM64__)
#define uintptr_t unsigned long
#else
#define uintptr_t unsigned int
#endif

#define __iomem
#endif
typedef struct
{
	void (*delay_us) (unsigned long us);
	void (*delay_ms) (unsigned long ms);
} hdmi_bsp_func;

#ifndef NULL
#define NULL 0
#endif

enum color_space
{
	BT601 = 1,
	BT709,
	EXT_CSC,
};

struct video_para
{
	unsigned int 			vic;
	enum color_space	csc;
	unsigned char			is_hdmi;
	unsigned char			is_yuv;
	unsigned char			is_hcts;
};

enum audio_type
{
	PCM = 1,
	AC3,
	MPEG1,
	MP3,
	MPEG2,
	AAC,
	DTS,
	ATRAC,
	OBA,
	DDP,
	DTS_HD,
	MAT,
	DST,
	WMA_PRO,
};

struct audio_para
{
	enum	audio_type	type;
	unsigned char			ca;
	unsigned int			sample_rate;
	unsigned int			sample_bit;
	unsigned int			ch_num;
	unsigned int 			vic;
};

int bsp_hdmi_set_func(hdmi_bsp_func *func);
void bsp_hdmi_set_addr(uintptr_t base_addr);
void bsp_hdmi_init(void);
void bsp_hdmi_set_video_en(unsigned char enable);
int bsp_hdmi_video(struct video_para *video);
int bsp_hdmi_audio(struct audio_para *audio);
int bsp_hdmi_ddc_read(char cmd,char pointer,char offset,int nbyte,char * pbuf);
unsigned int bsp_hdmi_get_hpd(void);
void bsp_hdmi_standby(void);
void bsp_hdmi_hrst(void);
void bsp_hdmi_hdl(void);
//@version: 0:A, 1:B, 2:C, 3:D
void bsp_hdmi_set_version(unsigned int version);
int bsp_hdmi_hdcp_err_check(void);
int bsp_hdmi_cec_get_simple_msg(unsigned char *msg);

#endif
