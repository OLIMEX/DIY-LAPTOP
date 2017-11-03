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

#include <common.h>
#include <asm/io.h>
#include <asm/arch/ccmu.h>
#include <asm/arch/platform.h>

typedef struct core_pll_freq_tbl {
    int FactorN;
    int FactorK;
    int FactorM;
    int FactorP;
    int pading;
}PLL_TABLE;


static PLL_TABLE pll1_table[] =
{  //N      K   M   P
    {9 ,    0,  0,  2},    //Theory Freq1 = 0   , Actual Freq2 = 60  , Index = 0
    {9 ,    0,  0,  2},    //Theory Freq1 = 6   , Actual Freq2 = 60  , Index = 1
    {9 ,    0,  0,  2},    //Theory Freq1 = 12  , Actual Freq2 = 60  , Index = 2
    {9 ,    0,  0,  2},    //Theory Freq1 = 18  , Actual Freq2 = 60  , Index = 3
	{9 , 	0,	0,	2},    //Theory Freq1 = 24  , Actual Freq2 = 60  , Index = 4
	{9 , 	0,	0,	2},    //Theory Freq1 = 30  , Actual Freq2 = 60  , Index = 5
	{9 , 	0,	0,	2},    //Theory Freq1 = 36  , Actual Freq2 = 60  , Index = 6
	{9 , 	0,	0,	2},    //Theory Freq1 = 42  , Actual Freq2 = 60  , Index = 7
	{9 , 	0,	0,	2},    //Theory Freq1 = 48  , Actual Freq2 = 60  , Index = 8
	{9 , 	0,	0,	2},    //Theory Freq1 = 54  , Actual Freq2 = 60  , Index = 9
	{9 , 	0,	0,	2},    //Theory Freq1 = 60  , Actual Freq2 = 60  , Index = 10
	{10,	0,	0,	2},    //Theory Freq1 = 66  , Actual Freq2 = 66  , Index = 11
	{11,	0,	0,	2},    //Theory Freq1 = 72  , Actual Freq2 = 72  , Index = 12
	{12,	0,	0,	2},    //Theory Freq1 = 78  , Actual Freq2 = 78  , Index = 13
	{13,	0,	0,	2},    //Theory Freq1 = 84  , Actual Freq2 = 84  , Index = 14
	{14,	0,	0,	2},    //Theory Freq1 = 90  , Actual Freq2 = 90  , Index = 15
	{15,	0,	0,	2},    //Theory Freq1 = 96  , Actual Freq2 = 96  , Index = 16
	{16,	0,	0,	2},    //Theory Freq1 = 102 , Actual Freq2 = 102 , Index = 17
	{17,	0,	0,	2},    //Theory Freq1 = 108 , Actual Freq2 = 108 , Index = 18
	{18,	0,	0,	2},    //Theory Freq1 = 114 , Actual Freq2 = 114 , Index = 19
	{9 ,	0,	0,	1},    //Theory Freq1 = 120 , Actual Freq2 = 120 , Index = 20
	{10,    0,	0,	1},    //Theory Freq1 = 126 , Actual Freq2 = 132 , Index = 21
	{10,    0,	0,	1},    //Theory Freq1 = 132 , Actual Freq2 = 132 , Index = 22
	{11,    0,	0,	1},    //Theory Freq1 = 138 , Actual Freq2 = 144 , Index = 23
	{11,    0,	0,	1},    //Theory Freq1 = 144 , Actual Freq2 = 144 , Index = 24
	{12,    0,	0,	1},    //Theory Freq1 = 150 , Actual Freq2 = 156 , Index = 25
	{12,    0,	0,	1},    //Theory Freq1 = 156 , Actual Freq2 = 156 , Index = 26
	{13,    0,	0,	1},    //Theory Freq1 = 162 , Actual Freq2 = 168 , Index = 27
	{13,    0,	0,	1},    //Theory Freq1 = 168 , Actual Freq2 = 168 , Index = 28
	{14,    0,	0,	1},    //Theory Freq1 = 174 , Actual Freq2 = 180 , Index = 29
	{14,    0,	0,	1},    //Theory Freq1 = 180 , Actual Freq2 = 180 , Index = 30
	{15,    0,	0,	1},    //Theory Freq1 = 186 , Actual Freq2 = 192 , Index = 31
	{15,    0,	0,	1},    //Theory Freq1 = 192 , Actual Freq2 = 192 , Index = 32
	{16,    0,	0,	1},    //Theory Freq1 = 198 , Actual Freq2 = 204 , Index = 33
	{16,    0,	0,	1},    //Theory Freq1 = 204 , Actual Freq2 = 204 , Index = 34
	{17,    0,	0,	1},    //Theory Freq1 = 210 , Actual Freq2 = 216 , Index = 35
	{17,    0,	0,	1},    //Theory Freq1 = 216 , Actual Freq2 = 216 , Index = 36
	{18,    0,	0,	1},    //Theory Freq1 = 222 , Actual Freq2 = 228 , Index = 37
	{18,    0,	0,	1},    //Theory Freq1 = 228 , Actual Freq2 = 228 , Index = 38
	{9 ,	0,	0,	0},    //Theory Freq1 = 234 , Actual Freq2 = 240 , Index = 39
	{9 ,	0,	0,	0},    //Theory Freq1 = 240 , Actual Freq2 = 240 , Index = 40
	{10,	0,	0,	0},    //Theory Freq1 = 246 , Actual Freq2 = 264 , Index = 41
	{10,	0,	0,	0},    //Theory Freq1 = 252 , Actual Freq2 = 264 , Index = 42
	{10,	0,	0,	0},    //Theory Freq1 = 258 , Actual Freq2 = 264 , Index = 43
	{10,	0,	0,	0},    //Theory Freq1 = 264 , Actual Freq2 = 264 , Index = 44
	{11,	0,	0,	0},    //Theory Freq1 = 270 , Actual Freq2 = 288 , Index = 45
	{11,	0,	0,	0},    //Theory Freq1 = 276 , Actual Freq2 = 288 , Index = 46
	{11,	0,	0,	0},    //Theory Freq1 = 282 , Actual Freq2 = 288 , Index = 47
	{11,	0,	0,	0},    //Theory Freq1 = 288 , Actual Freq2 = 288 , Index = 48
	{12,	0,	0,	0},    //Theory Freq1 = 294 , Actual Freq2 = 312 , Index = 49
	{12,	0,	0,	0},    //Theory Freq1 = 300 , Actual Freq2 = 312 , Index = 50
	{12,	0,	0,	0},    //Theory Freq1 = 306 , Actual Freq2 = 312 , Index = 51
	{12,	0,	0,	0},    //Theory Freq1 = 312 , Actual Freq2 = 312 , Index = 52
	{13,	0,	0,	0},    //Theory Freq1 = 318 , Actual Freq2 = 336 , Index = 53
	{13,	0,	0,	0},    //Theory Freq1 = 324 , Actual Freq2 = 336 , Index = 54
	{13,	0,	0,	0},    //Theory Freq1 = 330 , Actual Freq2 = 336 , Index = 55
	{13,	0,	0,	0},    //Theory Freq1 = 336 , Actual Freq2 = 336 , Index = 56
	{14,	0,	0,	0},    //Theory Freq1 = 342 , Actual Freq2 = 360 , Index = 57
	{14,	0,	0,	0},    //Theory Freq1 = 348 , Actual Freq2 = 360 , Index = 58
	{14,	0,	0,	0},    //Theory Freq1 = 354 , Actual Freq2 = 360 , Index = 59
	{14,	0,	0,	0},    //Theory Freq1 = 360 , Actual Freq2 = 360 , Index = 60
	{15,	0,	0,	0},    //Theory Freq1 = 366 , Actual Freq2 = 384 , Index = 61
	{15,	0,	0,	0},    //Theory Freq1 = 372 , Actual Freq2 = 384 , Index = 62
	{15,	0,	0,	0},    //Theory Freq1 = 378 , Actual Freq2 = 384 , Index = 63
	{15,    0,	0,	0},    //Theory Freq1 = 384 , Actual Freq2 = 384 , Index = 64
	{16,	0,	0,	0},    //Theory Freq1 = 390 , Actual Freq2 = 408 , Index = 65
	{16,	0,	0,	0},    //Theory Freq1 = 396 , Actual Freq2 = 408 , Index = 66
	{16,	0,	0,	0},    //Theory Freq1 = 402 , Actual Freq2 = 408 , Index = 67
	{16,	0,	0,	0},    //Theory Freq1 = 408 , Actual Freq2 = 408 , Index = 68
	{17,	0,	0,	0},    //Theory Freq1 = 414 , Actual Freq2 = 432 , Index = 69
	{17,	0,	0,	0},    //Theory Freq1 = 420 , Actual Freq2 = 432 , Index = 70
	{17,	0,	0,	0},    //Theory Freq1 = 426 , Actual Freq2 = 432 , Index = 71
	{17,	0,	0,	0},    //Theory Freq1 = 432 , Actual Freq2 = 432 , Index = 72
	{18,	0,	0,	0},    //Theory Freq1 = 438 , Actual Freq2 = 456 , Index = 73
	{18,	0,	0,	0},    //Theory Freq1 = 444 , Actual Freq2 = 456 , Index = 74
	{18,	0,	0,	0},    //Theory Freq1 = 450 , Actual Freq2 = 456 , Index = 75
	{18,	0,	0,	0},    //Theory Freq1 = 456 , Actual Freq2 = 456 , Index = 76
	{19,	0,	0,	0},    //Theory Freq1 = 462 , Actual Freq2 = 480 , Index = 77
	{19,	0,	0,	0},    //Theory Freq1 = 468 , Actual Freq2 = 480 , Index = 78
	{19,	0,	0,	0},    //Theory Freq1 = 474 , Actual Freq2 = 480 , Index = 79
	{19,	0,	0,	0},    //Theory Freq1 = 480 , Actual Freq2 = 480 , Index = 80
	{20,	0,	0,	0},    //Theory Freq1 = 486 , Actual Freq2 = 504 , Index = 81
	{20,	0,	0,	0},    //Theory Freq1 = 492 , Actual Freq2 = 504 , Index = 82
	{20,	0,	0,	0},    //Theory Freq1 = 498 , Actual Freq2 = 504 , Index = 83
	{20,	0,	0,	0},    //Theory Freq1 = 504 , Actual Freq2 = 504 , Index = 84
	{21,	0,	0,	0},    //Theory Freq1 = 510 , Actual Freq2 = 528 , Index = 85
	{21,	0,	0,	0},    //Theory Freq1 = 516 , Actual Freq2 = 528 , Index = 86
	{21,	0,	0,	0},    //Theory Freq1 = 522 , Actual Freq2 = 528 , Index = 87
	{21,	0,	0,	0},    //Theory Freq1 = 528 , Actual Freq2 = 528 , Index = 88
	{22,	0,	0,	0},    //Theory Freq1 = 534 , Actual Freq2 = 552 , Index = 89
	{22,	0,	0,	0},    //Theory Freq1 = 540 , Actual Freq2 = 552 , Index = 90
	{22,	0,	0,	0},    //Theory Freq1 = 546 , Actual Freq2 = 552 , Index = 91
	{22,	0,	0,	0},    //Theory Freq1 = 552 , Actual Freq2 = 552 , Index = 92
	{23,	0,	0,	0},    //Theory Freq1 = 558 , Actual Freq2 = 576 , Index = 93
	{23,	0,	0,	0},    //Theory Freq1 = 564 , Actual Freq2 = 576 , Index = 94
	{23,	0,	0,	0},    //Theory Freq1 = 570 , Actual Freq2 = 576 , Index = 95
	{23,	0,	0,	0},    //Theory Freq1 = 576 , Actual Freq2 = 576 , Index = 96
	{24,	0,	0,	0},    //Theory Freq1 = 582 , Actual Freq2 = 600 , Index = 97
	{24,	0,	0,	0},    //Theory Freq1 = 588 , Actual Freq2 = 600 , Index = 98
	{24,	0,	0,	0},    //Theory Freq1 = 594 , Actual Freq2 = 600 , Index = 99
	{24,	0,	0,	0},    //Theory Freq1 = 600 , Actual Freq2 = 600 , Index = 100
	{25,	0,	0,	0},    //Theory Freq1 = 606 , Actual Freq2 = 624 , Index = 101
	{25,	0,	0,	0},    //Theory Freq1 = 612 , Actual Freq2 = 624 , Index = 102
	{25,	0,	0,	0},    //Theory Freq1 = 618 , Actual Freq2 = 624 , Index = 103
	{25,	0,	0,	0},    //Theory Freq1 = 624 , Actual Freq2 = 624 , Index = 104
	{26,	0,	0,	0},    //Theory Freq1 = 630 , Actual Freq2 = 648 , Index = 105
	{26,	0,	0,	0},    //Theory Freq1 = 636 , Actual Freq2 = 648 , Index = 106
	{26,	0,	0,	0},    //Theory Freq1 = 642 , Actual Freq2 = 648 , Index = 107
	{26,	0,	0,	0},    //Theory Freq1 = 648 , Actual Freq2 = 648 , Index = 108
	{27,	0,	0,	0},    //Theory Freq1 = 654 , Actual Freq2 = 672 , Index = 109
	{27,	0,	0,	0},    //Theory Freq1 = 660 , Actual Freq2 = 672 , Index = 110
	{27,	0,	0,	0},    //Theory Freq1 = 666 , Actual Freq2 = 672 , Index = 111
	{27,	0,	0,	0},    //Theory Freq1 = 672 , Actual Freq2 = 672 , Index = 112
	{28,	0,	0,	0},    //Theory Freq1 = 678 , Actual Freq2 = 696 , Index = 113
	{28,	0,	0,	0},    //Theory Freq1 = 684 , Actual Freq2 = 696 , Index = 114
	{28,	0,	0,	0},    //Theory Freq1 = 690 , Actual Freq2 = 696 , Index = 115
	{28,	0,	0,	0},    //Theory Freq1 = 696 , Actual Freq2 = 696 , Index = 116
	{29,	0,	0,	0},    //Theory Freq1 = 702 , Actual Freq2 = 720 , Index = 117
	{29,	0,	0,	0},    //Theory Freq1 = 708 , Actual Freq2 = 720 , Index = 118
	{29,	0,	0,	0},    //Theory Freq1 = 714 , Actual Freq2 = 720 , Index = 119
	{29,	0,	0,	0},    //Theory Freq1 = 720 , Actual Freq2 = 720 , Index = 120
	{15,	1,	0,	0},    //Theory Freq1 = 726 , Actual Freq2 = 768 , Index = 121
	{15,	1,	0,	0},    //Theory Freq1 = 732 , Actual Freq2 = 768 , Index = 122
	{15,	1,	0,	0},    //Theory Freq1 = 738 , Actual Freq2 = 768 , Index = 123
	{15,	1,	0,	0},    //Theory Freq1 = 744 , Actual Freq2 = 768 , Index = 124
	{15,	1,	0,	0},    //Theory Freq1 = 750 , Actual Freq2 = 768 , Index = 125
	{15,	1,	0,	0},    //Theory Freq1 = 756 , Actual Freq2 = 768 , Index = 126
	{15,	1,	0,	0},    //Theory Freq1 = 762 , Actual Freq2 = 768 , Index = 127
	{15,	1,	0,	0},    //Theory Freq1 = 768 , Actual Freq2 = 768 , Index = 128
	{10,	2,	0,	0},    //Theory Freq1 = 774 , Actual Freq2 = 792 , Index = 129
	{10,	2,	0,	0},    //Theory Freq1 = 780 , Actual Freq2 = 792 , Index = 130
	{10,	2,	0,	0},    //Theory Freq1 = 786 , Actual Freq2 = 792 , Index = 131
	{10,	2,	0,	0},    //Theory Freq1 = 792 , Actual Freq2 = 792 , Index = 132
	{16,	1,	0,	0},    //Theory Freq1 = 798 , Actual Freq2 = 816 , Index = 133
	{16,	1,	0,	0},    //Theory Freq1 = 804 , Actual Freq2 = 816 , Index = 134
	{16,	1,	0,	0},    //Theory Freq1 = 810 , Actual Freq2 = 816 , Index = 135
	{16,	1,	0,	0},    //Theory Freq1 = 816 , Actual Freq2 = 816 , Index = 136
	{17,	1,	0,	0},    //Theory Freq1 = 822 , Actual Freq2 = 864 , Index = 137
	{17,	1,	0,	0},    //Theory Freq1 = 828 , Actual Freq2 = 864 , Index = 138
	{17,	1,	0,	0},    //Theory Freq1 = 834 , Actual Freq2 = 864 , Index = 139
	{17,	1,	0,	0},    //Theory Freq1 = 840 , Actual Freq2 = 864 , Index = 140
	{17,	1,	0,	0},    //Theory Freq1 = 846 , Actual Freq2 = 864 , Index = 141
	{17,	1,	0,	0},    //Theory Freq1 = 852 , Actual Freq2 = 864 , Index = 142
	{17,	1,	0,	0},    //Theory Freq1 = 858 , Actual Freq2 = 864 , Index = 143
	{17,	1,	0,	0},    //Theory Freq1 = 864 , Actual Freq2 = 864 , Index = 144
	{18,	1,	0,	0},    //Theory Freq1 = 870 , Actual Freq2 = 912 , Index = 145
	{18,	1,	0,	0},    //Theory Freq1 = 876 , Actual Freq2 = 912 , Index = 146
	{18,	1,	0,	0},    //Theory Freq1 = 882 , Actual Freq2 = 912 , Index = 147
	{18,	1,	0,	0},    //Theory Freq1 = 888 , Actual Freq2 = 912 , Index = 148
	{18,	1,	0,	0},    //Theory Freq1 = 894 , Actual Freq2 = 912 , Index = 149
	{18,	1,	0,	0},    //Theory Freq1 = 900 , Actual Freq2 = 912 , Index = 150
	{18,	1,	0,	0},    //Theory Freq1 = 906 , Actual Freq2 = 912 , Index = 151
	{18,	1,	0,	0},    //Theory Freq1 = 912 , Actual Freq2 = 912 , Index = 152
	{12,	2,	0,	0},    //Theory Freq1 = 918 , Actual Freq2 = 936 , Index = 153
	{12,	2,	0,	0},    //Theory Freq1 = 924 , Actual Freq2 = 936 , Index = 154
	{12,	2,	0,	0},    //Theory Freq1 = 930 , Actual Freq2 = 936 , Index = 155
	{12,	2,	0,	0},    //Theory Freq1 = 936 , Actual Freq2 = 936 , Index = 156
	{19,	1,	0,	0},    //Theory Freq1 = 942 , Actual Freq2 = 960 , Index = 157
	{19,	1,	0,	0},    //Theory Freq1 = 948 , Actual Freq2 = 960 , Index = 158
	{19,	1,	0,	0},    //Theory Freq1 = 954 , Actual Freq2 = 960 , Index = 159
	{19,	1,	0,	0},    //Theory Freq1 = 960 , Actual Freq2 = 960 , Index = 160
	{20,	1,	0,	0},    //Theory Freq1 = 966 , Actual Freq2 = 1008, Index = 161
	{20,	1,	0,	0},    //Theory Freq1 = 972 , Actual Freq2 = 1008, Index = 162
	{20,	1,	0,	0},    //Theory Freq1 = 978 , Actual Freq2 = 1008, Index = 163
	{20,	1,	0,	0},    //Theory Freq1 = 984 , Actual Freq2 = 1008, Index = 164
	{20,	1,	0,	0},    //Theory Freq1 = 990 , Actual Freq2 = 1008, Index = 165
	{20,	1,	0,	0},    //Theory Freq1 = 996 , Actual Freq2 = 1008, Index = 166
	{20,	1,	0,	0},    //Theory Freq1 = 1002, Actual Freq2 = 1008, Index = 167
	{20,	1,	0,	0},    //Theory Freq1 = 1008, Actual Freq2 = 1008, Index = 168
	{21,	1,	0,	0},    //Theory Freq1 = 1014, Actual Freq2 = 1056, Index = 169
	{21,	1,	0,	0},    //Theory Freq1 = 1020, Actual Freq2 = 1056, Index = 170
	{21,	1,	0,	0},    //Theory Freq1 = 1026, Actual Freq2 = 1056, Index = 171
	{21,	1,	0,	0},    //Theory Freq1 = 1032, Actual Freq2 = 1056, Index = 172
	{21,	1,	0,	0},    //Theory Freq1 = 1038, Actual Freq2 = 1056, Index = 173
	{21,	1,	0,	0},    //Theory Freq1 = 1044, Actual Freq2 = 1056, Index = 174
	{21,	1,	0,	0},    //Theory Freq1 = 1050, Actual Freq2 = 1056, Index = 175
	{21,	1,	0,	0},    //Theory Freq1 = 1056, Actual Freq2 = 1056, Index = 176
	{14,	2,	0,	0},    //Theory Freq1 = 1062, Actual Freq2 = 1080, Index = 177
	{14,	2,	0,	0},    //Theory Freq1 = 1068, Actual Freq2 = 1080, Index = 178
	{14,	2,	0,	0},    //Theory Freq1 = 1074, Actual Freq2 = 1080, Index = 179
	{14,	2,	0,	0},    //Theory Freq1 = 1080, Actual Freq2 = 1080, Index = 180
	{22,	1,	0,	0},    //Theory Freq1 = 1086, Actual Freq2 = 1104, Index = 181
	{22,	1,	0,	0},    //Theory Freq1 = 1092, Actual Freq2 = 1104, Index = 182
	{22,	1,	0,	0},    //Theory Freq1 = 1098, Actual Freq2 = 1104, Index = 183
	{22,	1,	0,	0},    //Theory Freq1 = 1104, Actual Freq2 = 1104, Index = 184
	{23,	1,	0,	0},    //Theory Freq1 = 1110, Actual Freq2 = 1152, Index = 185
	{23,	1,	0,	0},    //Theory Freq1 = 1116, Actual Freq2 = 1152, Index = 186
	{23,	1,	0,	0},    //Theory Freq1 = 1122, Actual Freq2 = 1152, Index = 187
	{23,	1,	0,	0},    //Theory Freq1 = 1128, Actual Freq2 = 1152, Index = 188
	{23,	1,	0,	0},    //Theory Freq1 = 1134, Actual Freq2 = 1152, Index = 189
	{23,	1,	0,	0},    //Theory Freq1 = 1140, Actual Freq2 = 1152, Index = 190
	{23,	1,	0,	0},    //Theory Freq1 = 1146, Actual Freq2 = 1152, Index = 191
	{23,	1,	0,	0},    //Theory Freq1 = 1152, Actual Freq2 = 1152, Index = 192
	{24,	1,	0,	0},    //Theory Freq1 = 1158, Actual Freq2 = 1200, Index = 193
	{24,	1,	0,	0},    //Theory Freq1 = 1164, Actual Freq2 = 1200, Index = 194
	{24,	1,	0,	0},    //Theory Freq1 = 1170, Actual Freq2 = 1200, Index = 195
	{24,	1,	0,	0},    //Theory Freq1 = 1176, Actual Freq2 = 1200, Index = 196
	{24,	1,	0,	0},    //Theory Freq1 = 1182, Actual Freq2 = 1200, Index = 197
	{24,	1,	0,	0},    //Theory Freq1 = 1188, Actual Freq2 = 1200, Index = 198
	{24,	1,	0,	0},    //Theory Freq1 = 1194, Actual Freq2 = 1200, Index = 199
	{24,	1,	0,	0},    //Theory Freq1 = 1200, Actual Freq2 = 1200, Index = 200
	{16,	2,	0,	0},    //Theory Freq1 = 1206, Actual Freq2 = 1224, Index = 201
	{16,	2,	0,	0},    //Theory Freq1 = 1212, Actual Freq2 = 1224, Index = 202
	{16,	2,	0,	0},    //Theory Freq1 = 1218, Actual Freq2 = 1224, Index = 203
	{16,	2,	0,	0},    //Theory Freq1 = 1224, Actual Freq2 = 1224, Index = 204
	{25,	1,	0,	0},    //Theory Freq1 = 1230, Actual Freq2 = 1248, Index = 205
	{25,	1,	0,	0},    //Theory Freq1 = 1236, Actual Freq2 = 1248, Index = 206
	{25,	1,	0,	0},    //Theory Freq1 = 1242, Actual Freq2 = 1248, Index = 207
	{25,	1,	0,	0},    //Theory Freq1 = 1248, Actual Freq2 = 1248, Index = 208
	{26,	1,	0,	0},    //Theory Freq1 = 1254, Actual Freq2 = 1296, Index = 209
	{26,	1,	0,	0},    //Theory Freq1 = 1260, Actual Freq2 = 1296, Index = 210
	{26,	1,	0,	0},    //Theory Freq1 = 1266, Actual Freq2 = 1296, Index = 211
	{26,	1,	0,	0},    //Theory Freq1 = 1272, Actual Freq2 = 1296, Index = 212
	{26,	1,	0,	0},    //Theory Freq1 = 1278, Actual Freq2 = 1296, Index = 213
	{26,	1,	0,	0},    //Theory Freq1 = 1284, Actual Freq2 = 1296, Index = 214
	{26,	1,	0,	0},    //Theory Freq1 = 1290, Actual Freq2 = 1296, Index = 215
	{26,	1,	0,	0},    //Theory Freq1 = 1296, Actual Freq2 = 1296, Index = 216
	{27,	1,	0,	0},    //Theory Freq1 = 1302, Actual Freq2 = 1344, Index = 217
	{27,	1,	0,	0},    //Theory Freq1 = 1308, Actual Freq2 = 1344, Index = 218
	{27,	1,	0,	0},    //Theory Freq1 = 1314, Actual Freq2 = 1344, Index = 219
	{27,	1,	0,	0},    //Theory Freq1 = 1320, Actual Freq2 = 1344, Index = 220
	{27,	1,	0,	0},    //Theory Freq1 = 1326, Actual Freq2 = 1344, Index = 221
	{27,	1,	0,	0},    //Theory Freq1 = 1332, Actual Freq2 = 1344, Index = 222
	{27,	1,	0,	0},    //Theory Freq1 = 1338, Actual Freq2 = 1344, Index = 223
	{27,	1,	0,	0},    //Theory Freq1 = 1344, Actual Freq2 = 1344, Index = 224
	{18,	2,	0,	0},    //Theory Freq1 = 1350, Actual Freq2 = 1368, Index = 225
	{18,	2,	0,	0},    //Theory Freq1 = 1356, Actual Freq2 = 1368, Index = 226
	{18,	2,	0,	0},    //Theory Freq1 = 1362, Actual Freq2 = 1368, Index = 227
	{18,	2,	0,	0},    //Theory Freq1 = 1368, Actual Freq2 = 1368, Index = 228
	{19,	2,	0,	0},    //Theory Freq1 = 1374, Actual Freq2 = 1440, Index = 229
	{19,	2,	0,	0},    //Theory Freq1 = 1380, Actual Freq2 = 1440, Index = 230
	{19,	2,	0,	0},    //Theory Freq1 = 1386, Actual Freq2 = 1440, Index = 231
	{19,	2,	0,	0},    //Theory Freq1 = 1392, Actual Freq2 = 1440, Index = 232
	{19,	2,	0,	0},    //Theory Freq1 = 1398, Actual Freq2 = 1440, Index = 233
	{19,	2,	0,	0},    //Theory Freq1 = 1404, Actual Freq2 = 1440, Index = 234
	{19,	2,	0,	0},    //Theory Freq1 = 1410, Actual Freq2 = 1440, Index = 235
	{19,	2,	0,	0},    //Theory Freq1 = 1416, Actual Freq2 = 1440, Index = 236
	{19,	2,	0,	0},    //Theory Freq1 = 1422, Actual Freq2 = 1440, Index = 237
	{19,	2,	0,	0},    //Theory Freq1 = 1428, Actual Freq2 = 1440, Index = 238
	{19,	2,	0,	0},    //Theory Freq1 = 1434, Actual Freq2 = 1440, Index = 239
	{19,	2,	0,	0},    //Theory Freq1 = 1440, Actual Freq2 = 1440, Index = 240
	{20,	2,	0,	0},    //Theory Freq1 = 1446, Actual Freq2 = 1512, Index = 241
	{20,	2,	0,	0},    //Theory Freq1 = 1452, Actual Freq2 = 1512, Index = 242
	{20,	2,	0,	0},    //Theory Freq1 = 1458, Actual Freq2 = 1512, Index = 243
	{20,	2,	0,	0},    //Theory Freq1 = 1464, Actual Freq2 = 1512, Index = 244
	{20,	2,	0,	0},    //Theory Freq1 = 1470, Actual Freq2 = 1512, Index = 245
	{20,	2,	0,	0},    //Theory Freq1 = 1476, Actual Freq2 = 1512, Index = 246
	{20,	2,	0,	0},    //Theory Freq1 = 1482, Actual Freq2 = 1512, Index = 247
	{20,	2,	0,	0},    //Theory Freq1 = 1488, Actual Freq2 = 1512, Index = 248
	{20,	2,	0,	0},    //Theory Freq1 = 1494, Actual Freq2 = 1512, Index = 249
	{20,	2,	0,	0},    //Theory Freq1 = 1500, Actual Freq2 = 1512, Index = 250
	{20,	2,	0,	0},    //Theory Freq1 = 1506, Actual Freq2 = 1512, Index = 251
	{20,	2,	0,	0},    //Theory Freq1 = 1512, Actual Freq2 = 1512, Index = 252
	{15,	3,	0,	0},    //Theory Freq1 = 1518, Actual Freq2 = 1536, Index = 253
	{15,	3,	0,	0},    //Theory Freq1 = 1524, Actual Freq2 = 1536, Index = 254
	{15,	3,	0,	0},    //Theory Freq1 = 1530, Actual Freq2 = 1536, Index = 255
	{15,	3,	0,	0},    //Theory Freq1 = 1536, Actual Freq2 = 1536, Index = 256
	{21,	2,	0,	0},    //Theory Freq1 = 1542, Actual Freq2 = 1584, Index = 257
	{21,	2,	0,	0},    //Theory Freq1 = 1548, Actual Freq2 = 1584, Index = 258
	{21,	2,	0,	0},    //Theory Freq1 = 1554, Actual Freq2 = 1584, Index = 259
	{21,	2,	0,	0},    //Theory Freq1 = 1560, Actual Freq2 = 1584, Index = 260
	{21,	2,	0,	0},    //Theory Freq1 = 1566, Actual Freq2 = 1584, Index = 261
	{21,	2,	0,	0},    //Theory Freq1 = 1572, Actual Freq2 = 1584, Index = 262
	{21,	2,	0,	0},    //Theory Freq1 = 1578, Actual Freq2 = 1584, Index = 263
	{21,	2,	0,	0},    //Theory Freq1 = 1584, Actual Freq2 = 1584, Index = 264
	{16,	3,	0,	0},    //Theory Freq1 = 1590, Actual Freq2 = 1632, Index = 265
	{16,	3,	0,	0},    //Theory Freq1 = 1596, Actual Freq2 = 1632, Index = 266
	{16,	3,	0,	0},    //Theory Freq1 = 1602, Actual Freq2 = 1632, Index = 267
	{16,	3,	0,	0},    //Theory Freq1 = 1608, Actual Freq2 = 1632, Index = 268
	{16,	3,	0,	0},    //Theory Freq1 = 1614, Actual Freq2 = 1632, Index = 269
	{16,	3,	0,	0},    //Theory Freq1 = 1620, Actual Freq2 = 1632, Index = 270
	{16,	3,	0,	0},    //Theory Freq1 = 1626, Actual Freq2 = 1632, Index = 271
	{16,	3,	0,	0},    //Theory Freq1 = 1632, Actual Freq2 = 1632, Index = 272
	{22,	2,	0,	0},    //Theory Freq1 = 1638, Actual Freq2 = 1656, Index = 273
	{22,	2,	0,	0},    //Theory Freq1 = 1644, Actual Freq2 = 1656, Index = 274
	{22,	2,	0,	0},    //Theory Freq1 = 1650, Actual Freq2 = 1656, Index = 275
	{22,	2,	0,	0},    //Theory Freq1 = 1656, Actual Freq2 = 1656, Index = 276
	{23,	2,	0,	0},    //Theory Freq1 = 1662, Actual Freq2 = 1728, Index = 277
	{23,	2,	0,	0},    //Theory Freq1 = 1668, Actual Freq2 = 1728, Index = 278
	{23,	2,	0,	0},    //Theory Freq1 = 1674, Actual Freq2 = 1728, Index = 279
	{23,	2,	0,	0},    //Theory Freq1 = 1680, Actual Freq2 = 1728, Index = 280
	{23,	2,	0,	0},    //Theory Freq1 = 1686, Actual Freq2 = 1728, Index = 281
	{23,	2,	0,	0},    //Theory Freq1 = 1692, Actual Freq2 = 1728, Index = 282
	{23,	2,	0,	0},    //Theory Freq1 = 1698, Actual Freq2 = 1728, Index = 283
	{23,	2,	0,	0},    //Theory Freq1 = 1704, Actual Freq2 = 1728, Index = 284
	{23,	2,	0,	0},    //Theory Freq1 = 1710, Actual Freq2 = 1728, Index = 285
	{23,	2,	0,	0},    //Theory Freq1 = 1716, Actual Freq2 = 1728, Index = 286
	{23,	2,	0,	0},    //Theory Freq1 = 1722, Actual Freq2 = 1728, Index = 287
	{23,	2,	0,	0},    //Theory Freq1 = 1728, Actual Freq2 = 1728, Index = 288
	{24,	2,	0,	0},    //Theory Freq1 = 1734, Actual Freq2 = 1800, Index = 289
	{24,	2,	0,	0},    //Theory Freq1 = 1740, Actual Freq2 = 1800, Index = 290
	{24,	2,	0,	0},    //Theory Freq1 = 1746, Actual Freq2 = 1800, Index = 291
	{24,	2,	0,	0},    //Theory Freq1 = 1752, Actual Freq2 = 1800, Index = 292
	{24,	2,	0,	0},    //Theory Freq1 = 1758, Actual Freq2 = 1800, Index = 293
	{24,	2,	0,	0},    //Theory Freq1 = 1764, Actual Freq2 = 1800, Index = 294
	{24,	2,	0,	0},    //Theory Freq1 = 1770, Actual Freq2 = 1800, Index = 295
	{24,	2,	0,	0},    //Theory Freq1 = 1776, Actual Freq2 = 1800, Index = 296
	{24,	2,	0,	0},    //Theory Freq1 = 1782, Actual Freq2 = 1800, Index = 297
	{24,	2,	0,	0},    //Theory Freq1 = 1788, Actual Freq2 = 1800, Index = 298
	{24,	2,	0,	0},    //Theory Freq1 = 1794, Actual Freq2 = 1800, Index = 299
	{24,	2,	0,	0},    //Theory Freq1 = 1800, Actual Freq2 = 1800, Index = 300
	{25,	2,	0,	0},    //Theory Freq1 = 1806, Actual Freq2 = 1872, Index = 301
	{25,	2,	0,	0},    //Theory Freq1 = 1812, Actual Freq2 = 1872, Index = 302
	{25,	2,	0,	0},    //Theory Freq1 = 1818, Actual Freq2 = 1872, Index = 303
	{25,	2,	0,	0},    //Theory Freq1 = 1824, Actual Freq2 = 1872, Index = 304
	{25,	2,	0,	0},    //Theory Freq1 = 1830, Actual Freq2 = 1872, Index = 305
	{25,	2,	0,	0},    //Theory Freq1 = 1836, Actual Freq2 = 1872, Index = 306
	{25,	2,	0,	0},    //Theory Freq1 = 1842, Actual Freq2 = 1872, Index = 307
	{25,	2,	0,	0},    //Theory Freq1 = 1848, Actual Freq2 = 1872, Index = 308
	{25,	2,	0,	0},    //Theory Freq1 = 1854, Actual Freq2 = 1872, Index = 309
	{25,	2,	0,	0},    //Theory Freq1 = 1860, Actual Freq2 = 1872, Index = 310
	{25,	2,	0,	0},    //Theory Freq1 = 1866, Actual Freq2 = 1872, Index = 311
	{25,	2,	0,	0},    //Theory Freq1 = 1872, Actual Freq2 = 1872, Index = 312
	{26,	2,	0,	0},    //Theory Freq1 = 1878, Actual Freq2 = 1944, Index = 313
	{26,	2,	0,	0},    //Theory Freq1 = 1884, Actual Freq2 = 1944, Index = 314
	{26,	2,	0,	0},    //Theory Freq1 = 1890, Actual Freq2 = 1944, Index = 315
	{26,	2,	0,	0},    //Theory Freq1 = 1896, Actual Freq2 = 1944, Index = 316
	{26,	2,	0,	0},    //Theory Freq1 = 1902, Actual Freq2 = 1944, Index = 317
	{26,	2,	0,	0},    //Theory Freq1 = 1908, Actual Freq2 = 1944, Index = 318
	{26,	2,	0,	0},    //Theory Freq1 = 1914, Actual Freq2 = 1944, Index = 319
	{26,	2,	0,	0},    //Theory Freq1 = 1920, Actual Freq2 = 1944, Index = 320
	{26,	2,	0,	0},    //Theory Freq1 = 1926, Actual Freq2 = 1944, Index = 321
	{26,	2,	0,	0},    //Theory Freq1 = 1932, Actual Freq2 = 1944, Index = 322
	{26,	2,	0,	0},    //Theory Freq1 = 1938, Actual Freq2 = 1944, Index = 323
	{26,	2,	0,	0},    //Theory Freq1 = 1944, Actual Freq2 = 1944, Index = 324
	{27,	2,	0,	0},    //Theory Freq1 = 1950, Actual Freq2 = 2016, Index = 325
	{27,	2,	0,	0},    //Theory Freq1 = 1956, Actual Freq2 = 2016, Index = 326
	{27,	2,	0,	0},    //Theory Freq1 = 1962, Actual Freq2 = 2016, Index = 327
	{27,	2,	0,	0},    //Theory Freq1 = 1968, Actual Freq2 = 2016, Index = 328
	{27,	2,	0,	0},    //Theory Freq1 = 1974, Actual Freq2 = 2016, Index = 329
	{27,	2,	0,	0},    //Theory Freq1 = 1980, Actual Freq2 = 2016, Index = 330
	{27,	2,	0,	0},    //Theory Freq1 = 1986, Actual Freq2 = 2016, Index = 331
	{27,	2,	0,	0},    //Theory Freq1 = 1992, Actual Freq2 = 2016, Index = 332
	{27,	2,	0,	0},    //Theory Freq1 = 1998, Actual Freq2 = 2016, Index = 333
	{27,	2,	0,	0},    //Theory Freq1 = 2004, Actual Freq2 = 2016, Index = 334
	{27,	2,	0,	0},    //Theory Freq1 = 2010, Actual Freq2 = 2016, Index = 335
	{27,	2,	0,	0},    //Theory Freq1 = 2016, Actual Freq2 = 2016, Index = 336
};


static int clk_get_pll_para(PLL_TABLE *factor, int pll_clk)
{
    int index;
	PLL_TABLE *target_factor;

    index = pll_clk / 6;
	target_factor = &pll1_table[index];

    factor->FactorN = target_factor->FactorN;
    factor->FactorK = target_factor->FactorK;
    factor->FactorM = target_factor->FactorM;
    factor->FactorP = target_factor->FactorP;

    return 0;
}

int sunxi_clock_get_pll6(void)
{
	unsigned int reg_val;
	int factor_n, factor_k, pll6;

	reg_val = readl(CCMU_PLL_PERIPH0_CTRL_REG);
	factor_n = ((reg_val >> 8) & 0x1f) + 1;
	factor_k = ((reg_val >> 4) & 0x03) + 1;
	pll6 = 24 * factor_n * factor_k/2;
	return pll6;
}

int sunxi_clock_get_corepll(void)
{
    unsigned int reg_val;
    int 	div_m, div_p;
    int 	factor_k, factor_n;
    int 	clock,clock_src;

    reg_val   = readl(CCMU_CPUX_AXI_CFG_REG);
    clock_src = (reg_val >> 16) & 0x03;

    switch(clock_src)
    {
        case 0:
            clock = 0; //LOSC
            break;
        case 1:
            clock = 24;//OSC24M
            break;
        case 2:
        case 3:
            reg_val  = readl(CCMU_PLL_CPUX_CTRL_REG);
            div_p    = ((reg_val >>16) & 0x3);
            factor_n = ((reg_val >> 8) & 0x1f) + 1;
            factor_k = ((reg_val >> 4) & 0x3) + 1;
            div_m    = ((reg_val >> 0) & 0x3) + 1;

            clock = 24 * factor_n * factor_k/div_m/(1<<div_p);
        	break;
        default:
        	return 0;
    }
	return clock;
}

int sunxi_clock_get_axi(void)
{
    unsigned int reg_val;
    int clock_cpux, factor;

    reg_val   = readl(CCMU_CPUX_AXI_CFG_REG);
    factor    = ((reg_val >> 0) & 0x03) + 1;
    clock_cpux = sunxi_clock_get_corepll();

    return clock_cpux/factor;
}

int sunxi_clock_get_ahb(void)
{
	unsigned int reg_val;
	int factor;
	int clock;
    int src = 0;

	reg_val = readl(CCMU_AHB1_APB1_CFG_REG);

    src = (reg_val >> 12)&0x3;
    clock = 0;
    switch(src)
    {
        case 0://src is LOSC
            break;
        case 1://src is OSC24M
            clock = 24;
            break;
        case 2://src is axi
            factor  = (reg_val >> 4) & 0x03;
            clock   = sunxi_clock_get_axi()>>factor;
            break;
        case 3://src is pll6(1x)/AHB1_PRE_DIV
            factor  = (reg_val >> 6) & 0x03;
            clock   = sunxi_clock_get_pll6()/(factor+1);
        break;
    }

	return clock;
}

int sunxi_clock_get_apb(void)
{
	unsigned int reg_val;
	int          clock, factor;

	reg_val = readl(CCMU_AHB1_APB1_CFG_REG);
	factor  = (reg_val >> 8) & 0x03;
	clock   = sunxi_clock_get_ahb();

	if(factor)
	{
		clock >>= factor;
	}
	else
	{
		clock >>= 1;
	}

	return clock;
}

void sunxi_clock_set_ahb2(void)
{
	unsigned int reg_val;
	reg_val = readl(CCMU_AHB2_CFG_GREG);
	reg_val &= (~0x3);
	//0: clk src is ahb1 1: clk src is pll_periph0(1x)/2
	reg_val |= (0x1<<0);
	writel(reg_val, CCMU_AHB2_CFG_GREG);
}

int sunxi_clock_get_ahb2(void)
{
	unsigned int reg_val;
	reg_val = readl(CCMU_AHB2_CFG_GREG);
	reg_val &= 0x3;
	if(reg_val == 0)
	{
		return sunxi_clock_get_ahb();
	}
	else
	{
		return sunxi_clock_get_pll6()/2;
	}
}

int sunxi_clock_get_mbus(void)
{
	unsigned int reg_val;
	unsigned int src = 0,clock=0, div = 0;
	reg_val = readl(CCMU_MBUS_CLK_REG);

	//get src
	src = (reg_val >> 24)&0x3;
	//get div M, the divided clock is divided by M+1
	div = (reg_val&0x3) + 1;

	switch(src)
	{
		case 0://src is OSC24M
			clock = 24;
			break;
		case 1://src is   pll_periph0(1x)/2
			clock = sunxi_clock_get_pll6()*2;
			break;
		case 2://src is pll_ddr0  --not set in boot
			clock   = 0;
			break;
		case 3://src is pll_ddr1 --not set in boot
			clock   = 0;
			break;
	}

	clock = clock/div;

	return clock;
}

int pll_lock_is_new_mode(void)
{
	__u32 reg_val;
	reg_val = readl(CCMU_PLL_LOCK_CTRL_REG);
	reg_val &= LOCK_EN_NEW_MODE;
	return reg_val > 0 ? 1:0;
}

void enable_pll_lock_bit(__u32 lock_bit)
{
	__u32 reg_val;
	reg_val = readl(CCMU_PLL_LOCK_CTRL_REG);
	reg_val |= lock_bit;
	writel(reg_val, CCMU_PLL_LOCK_CTRL_REG);
}

void disable_pll_lock_bit(__u32 lock_bit)
{
	__u32 reg_val;
	reg_val = readl(CCMU_PLL_LOCK_CTRL_REG);
	reg_val &= (~lock_bit);
	writel(reg_val, CCMU_PLL_LOCK_CTRL_REG);
}


int sunxi_clock_set_corepll(int frequency)
{
	unsigned int reg_val;
	PLL_TABLE  pll_factor;
	int pll_new_mode = 0;

	if(!frequency)
	{
		frequency = 408;
	}
	else if(frequency < 24)
	{
		frequency = 24;
	}
	pll_new_mode = pll_lock_is_new_mode();

	//scp maybe has been enabled this bit already
	if(!pll_new_mode)
	{
		enable_pll_lock_bit(LOCK_EN_NEW_MODE);
	}
	//switch to 24M
	reg_val = readl(CCMU_CPUX_AXI_CFG_REG);
	reg_val &= ~(0x03 << 16);
	reg_val |=  (0x01 << 16);
	writel(reg_val,CCMU_CPUX_AXI_CFG_REG);
	__udelay(20);

	//get config para form freq table
	clk_get_pll_para(&pll_factor, frequency);

	disable_pll_lock_bit(LOCK_EN_PLL_CPUX);
	reg_val = readl(CCMU_PLL_CPUX_CTRL_REG);
	reg_val &= ~((1<<31) | (0x03 << 16) | (0x1f << 8) | (0x03 << 4) | (0x03 << 0));
	reg_val |=  (pll_factor.FactorP << 16) | (pll_factor.FactorN<<8) | (pll_factor.FactorK<<4) | (pll_factor.FactorM << 0) ;
	writel(reg_val, CCMU_PLL_CPUX_CTRL_REG);
	enable_pll_lock_bit(LOCK_EN_PLL_CPUX);
	writel((1<<31) | readl(CCMU_PLL_CPUX_CTRL_REG), CCMU_PLL_CPUX_CTRL_REG);

	//wait  stable
#ifndef FPGA_PLATFORM
	do
	{
		reg_val = readl(CCMU_PLL_CPUX_CTRL_REG);
	}
	while(!(reg_val & (0x1 << 28)));
	__udelay(20);
#endif
	disable_pll_lock_bit(LOCK_EN_PLL_CPUX);


	//switch clk src to COREPLL
	reg_val = readl(CCMU_CPUX_AXI_CFG_REG);
	reg_val &= ~(0x03 << 16);
	reg_val |=  (0x02 << 16);
	writel(reg_val, CCMU_CPUX_AXI_CFG_REG);
	__udelay(20);

	//set ahb2 clk src
	sunxi_clock_set_ahb2();
	if(!pll_new_mode)
	{
		disable_pll_lock_bit(LOCK_EN_NEW_MODE);
	}
	return  0;
}


