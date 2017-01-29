/*
 * codec-utils.h
 *
 * Copyright (c) 2015 Allwinner.
 *
 * Author: Liu shaohua <liushaohua@allwinnertech.com>
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 */
#ifndef CODEC_UTILS_H_
#define CODEC_UTILS_H_

extern int codec_utils_probe(struct platform_device *pdev);
extern int codec_utils_remove(struct platform_device *pdev);
#endif