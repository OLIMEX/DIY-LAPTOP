#ifndef __DISP_DEVICE_H__
#define __DISP_DEVICE_H__

#include "disp_private.h"

s32 disp_device_set_manager(struct disp_device* dispdev, struct disp_manager *mgr);
s32 disp_device_unset_manager(struct disp_device* dispdev);
s32 disp_device_get_resolution(struct disp_device* dispdev, u32 *xres, u32 *yres);
s32 disp_device_get_timings(struct disp_device* dispdev, disp_video_timings *timings);
s32 disp_device_is_interlace(struct disp_device *dispdev);
s32 disp_device_register(struct disp_device *dispdev);
s32 disp_device_unregister(struct disp_device *dispdev);
struct disp_device* disp_device_get(int disp, disp_output_type output_type);
struct disp_device* disp_device_find(int disp, disp_output_type output_type);

#endif
