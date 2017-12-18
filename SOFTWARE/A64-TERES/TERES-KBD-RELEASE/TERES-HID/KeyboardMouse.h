/*
  This software is based on the LUFA library. Modifications of the 
  software are released under GPL but LUFA library itself is copyrigthed
  by its creator Dean Camera. Refer to the license below on the usage of
  LUFA library.

	Chris Boudacoff @ Olimex Ltd
	chris <at> protonic <dot> co <dot> uk
*/

/*
             LUFA Library
     Copyright (C) Dean Camera, 2015.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2015  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/


#ifndef _KEYBOARD_MOUSE_H_
#define _KEYBOARD_MOUSE_H_


	/* Includes: */
		#include <avr/io.h>
		#include <avr/wdt.h>
		#include <avr/power.h>
		#include <avr/interrupt.h>
		#include <stdbool.h>
		#include <string.h>
		#include <stdio.h>
		#include <util/delay.h>
		#include "Descriptors.h"
		
		#include <LUFA/Common/Common.h>
		#include <LUFA/Drivers/USB/USB.h>
		#include <LUFA/Platform/Platform.h>
		#include <LUFA/Drivers/Peripheral/TWI.h>
		#include <LUFA/Drivers/Peripheral/ADC.h>
		
		/** Magic bootloader key to unlock forced application start mode. */
		#define MAGIC_BOOT_KEY               0xDC42	

#define	HID_KEY_WLAN				246
#define HID_KEY_RFKILL				247 
#define HID_KEY_LOCK_TOUCHPAD		199
#define HID_KEY_BRIGTHNESS_DOWN		0x6f 
#define HID_KEY_BRIGTHNESS_UP		0x70
#define HID_KEYBOARD_SC_SLEEP		0x71
#define HID_KEY_DISPLAY_SWITCH		0xe3
#define HID_KEYBOARD_SC_LEFTMETA	0x7d 
#define HID_KEYBOARD_SC_COMPOSE		0x65

//#define HID_KEYBOARD_MODIFIER_LEFTSHIFT 0x02
//#define HID_KEYBOARD_MODIFIER_RIGHTSHIFT  0x20


/** Bootloader special address to start the user application */
#define COMMAND_STARTAPPLICATION   0xFFFF
		
	/* Function Prototypes: */
		void SetupHardware(void);
		uint8_t M_Buttons(void);
		void EVENT_USB_Device_Connect(void);
		void EVENT_USB_Device_Disconnect(void);
		void EVENT_USB_Device_ConfigurationChanged(void);
		void EVENT_USB_Device_ControlRequest(void);
		void EVENT_USB_Device_StartOfFrame(void);
		void Read_TP(void);
		bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
		                                         uint8_t* const ReportID,
		                                         const uint8_t ReportType,
		                                         void* ReportData,
		                                         uint16_t* const ReportSize);
		void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
		                                          const uint8_t ReportID,
		                                          const uint8_t ReportType,
		                                          const void* ReportData,
		                                          const uint16_t ReportSize);
	
const int keys[224] = {HID_KEYBOARD_SC_LEFT_CONTROL,HID_KEYBOARD_SC_GRAVE_ACCENT_AND_TILDE,HID_KEYBOARD_SC_F1,HID_KEYBOARD_SC_F2,HID_KEYBOARD_SC_5_AND_PERCENTAGE,HID_KEYBOARD_SC_6_AND_CARET,HID_KEYBOARD_SC_EQUAL_AND_PLUS,HID_KEYBOARD_SC_F8,HID_KEYBOARD_SC_MINUS_AND_UNDERSCORE,0,HID_KEYBOARD_SC_F9,HID_KEYBOARD_SC_DELETE,0,0,
  0,HID_KEYBOARD_SC_TAB,HID_KEYBOARD_SC_CAPS_LOCK,HID_KEYBOARD_SC_F3,HID_KEYBOARD_SC_T,HID_KEYBOARD_SC_Y,HID_KEYBOARD_SC_CLOSING_BRACKET_AND_CLOSING_BRACE,HID_KEYBOARD_SC_F7,HID_KEYBOARD_SC_OPENING_BRACKET_AND_OPENING_BRACE,0,HID_KEYBOARD_SC_BACKSPACE,0,0,HID_KEYBOARD_SC_LEFT_SHIFT,
  0,HID_KEYBOARD_SC_A,HID_KEYBOARD_SC_S,HID_KEYBOARD_SC_D,HID_KEYBOARD_SC_F,HID_KEYBOARD_SC_J,HID_KEYBOARD_SC_K,HID_KEYBOARD_SC_L,HID_KEYBOARD_SC_SEMICOLON_AND_COLON,0,HID_KEYBOARD_SC_BACKSLASH_AND_PIPE,0,0,HID_KEYBOARD_SC_RIGHT_SHIFT,
  0,HID_KEYBOARD_SC_ESCAPE,0,HID_KEYBOARD_SC_F4,HID_KEYBOARD_SC_G,HID_KEYBOARD_SC_H,HID_KEYBOARD_SC_F6,0,HID_KEYBOARD_SC_APOSTROPHE_AND_QUOTE,HID_KEYBOARD_SC_LEFT_ALT,HID_KEYBOARD_SC_F11,HID_KEYBOARD_SC_SPACE,0,HID_KEYBOARD_SC_UP_ARROW,
  HID_KEYBOARD_SC_RIGHT_CONTROL,HID_KEYBOARD_SC_Z,HID_KEYBOARD_SC_X,HID_KEYBOARD_SC_C,HID_KEYBOARD_SC_V,HID_KEYBOARD_SC_M,HID_KEYBOARD_SC_COMMA_AND_LESS_THAN_SIGN,HID_KEYBOARD_SC_DOT_AND_GREATER_THAN_SIGN,0,0,HID_KEYBOARD_SC_ENTER,0,0,0,
  0,0,0,0,HID_KEYBOARD_SC_B,HID_KEYBOARD_SC_N,0,HID_KEYBOARD_SC_COMPOSE,HID_KEYBOARD_SC_SLASH_AND_QUESTION_MARK,HID_KEYBOARD_SC_RIGHT_ALT,HID_KEYBOARD_SC_F12,HID_KEYBOARD_SC_DOWN_ARROW,HID_KEYBOARD_SC_RIGHT_ARROW,HID_KEYBOARD_SC_LEFT_ARROW,
 0,HID_KEYBOARD_SC_Q,HID_KEYBOARD_SC_W,HID_KEYBOARD_SC_E,HID_KEYBOARD_SC_R,HID_KEYBOARD_SC_U,HID_KEYBOARD_SC_I,HID_KEYBOARD_SC_O,HID_KEYBOARD_SC_P,0,0,0,0,0,
  HID_KEYBOARD_SC_F5,HID_KEYBOARD_SC_1_AND_EXCLAMATION,HID_KEYBOARD_SC_2_AND_AT,HID_KEYBOARD_SC_3_AND_HASHMARK,HID_KEYBOARD_SC_4_AND_DOLLAR,HID_KEYBOARD_SC_7_AND_AMPERSAND,HID_KEYBOARD_SC_8_AND_ASTERISK,HID_KEYBOARD_SC_9_AND_OPENING_PARENTHESIS,HID_KEYBOARD_SC_0_AND_CLOSING_PARENTHESIS,HID_KEYBOARD_SC_PRINT_SCREEN,HID_KEYBOARD_SC_F10,0,0,0,

	HID_KEYBOARD_SC_LEFT_CONTROL,HID_KEYBOARD_SC_GRAVE_ACCENT_AND_TILDE,HID_KEYBOARD_SC_SLEEP,HID_KEY_WLAN,HID_KEYBOARD_SC_5_AND_PERCENTAGE,HID_KEYBOARD_SC_6_AND_CARET,HID_KEYBOARD_SC_EQUAL_AND_PLUS,HID_KEY_BRIGTHNESS_UP,HID_KEYBOARD_SC_MINUS_AND_UNDERSCORE,0,HID_KEY_DISPLAY_SWITCH,HID_KEYBOARD_SC_DELETE,0,0,
  0,HID_KEYBOARD_SC_TAB,HID_KEYBOARD_SC_CAPS_LOCK,HID_KEY_LOCK_TOUCHPAD,HID_KEYBOARD_SC_T,HID_KEYBOARD_SC_Y,HID_KEYBOARD_SC_CLOSING_BRACKET_AND_CLOSING_BRACE,HID_KEY_BRIGTHNESS_DOWN,HID_KEYBOARD_SC_OPENING_BRACKET_AND_OPENING_BRACE,0,HID_KEYBOARD_SC_BACKSPACE,0,0,HID_KEYBOARD_SC_LEFT_SHIFT,
  0,HID_KEYBOARD_SC_A,HID_KEYBOARD_SC_S,HID_KEYBOARD_SC_D,HID_KEYBOARD_SC_F,HID_KEYBOARD_SC_KEYPAD_1_AND_END,HID_KEYBOARD_SC_KEYPAD_2_AND_DOWN_ARROW,HID_KEYBOARD_SC_KEYPAD_3_AND_PAGE_DOWN,HID_KEYBOARD_SC_KEYPAD_MINUS,0,HID_KEYBOARD_SC_BACKSLASH_AND_PIPE,0,0,HID_KEYBOARD_SC_RIGHT_SHIFT,
  0,HID_KEYBOARD_SC_ESCAPE,0, HID_KEYBOARD_SC_VOLUME_DOWN,HID_KEYBOARD_SC_G,HID_KEYBOARD_SC_H,HID_KEYBOARD_SC_MUTE,0,HID_KEYBOARD_SC_APOSTROPHE_AND_QUOTE,HID_KEYBOARD_SC_LEFT_ALT,HID_KEYBOARD_SC_INSERT,HID_KEYBOARD_SC_SPACE,0,HID_KEYBOARD_SC_PAGE_UP,
  HID_KEYBOARD_SC_RIGHT_CONTROL,HID_KEYBOARD_SC_Z,HID_KEYBOARD_SC_X,HID_KEYBOARD_SC_C,HID_KEYBOARD_SC_V,HID_KEYBOARD_SC_KEYPAD_0_AND_INSERT ,HID_KEYBOARD_SC_COMMA_AND_LESS_THAN_SIGN,HID_KEYBOARD_SC_DOT_AND_GREATER_THAN_SIGN,0,0,HID_KEYBOARD_SC_ENTER,0,0,0,
  0,0,0,0,HID_KEYBOARD_SC_B,HID_KEYBOARD_SC_N,0,HID_KEYBOARD_SC_COMPOSE,HID_KEYBOARD_SC_KEYPAD_PLUS,HID_KEYBOARD_SC_RIGHT_ALT,HID_KEYBOARD_SC_F12,HID_KEYBOARD_SC_PAGE_DOWN,HID_KEYBOARD_SC_END,HID_KEYBOARD_SC_HOME,
 0,HID_KEYBOARD_SC_Q,HID_KEYBOARD_SC_W,HID_KEYBOARD_SC_E,HID_KEYBOARD_SC_R,HID_KEYBOARD_SC_KEYPAD_4_AND_LEFT_ARROW,HID_KEYBOARD_SC_KEYPAD_5 ,HID_KEYBOARD_SC_KEYPAD_6_AND_RIGHT_ARROW,HID_KEYBOARD_SC_KEYPAD_ASTERISK ,0,0,0,0,0,
  HID_KEYBOARD_SC_VOLUME_UP ,HID_KEYBOARD_SC_1_AND_EXCLAMATION,HID_KEYBOARD_SC_2_AND_AT,HID_KEYBOARD_SC_3_AND_HASHMARK,HID_KEYBOARD_SC_4_AND_DOLLAR,HID_KEYBOARD_SC_7_AND_AMPERSAND,HID_KEYBOARD_SC_8_AND_ASTERISK,HID_KEYBOARD_SC_9_AND_OPENING_PARENTHESIS,HID_KEYBOARD_SC_KEYPAD_SLASH,HID_KEYBOARD_SC_NUM_LOCK ,HID_KEYBOARD_SC_PAUSE,0,0,0

};


const int keym[112] = {HID_KEYBOARD_MODIFIER_LEFTCTRL,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,HID_KEYBOARD_MODIFIER_LEFTGUI,HID_KEYBOARD_MODIFIER_LEFTSHIFT,
  0,0,0,0,0,0,0,0,0,0,0,0,0,HID_KEYBOARD_MODIFIER_RIGHTSHIFT,
  0,0,0,0,0,0,0,0,0,HID_KEYBOARD_MODIFIER_LEFTALT,0,0,0,0,
  HID_KEYBOARD_MODIFIER_RIGHTCTRL ,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,HID_KEYBOARD_MODIFIER_RIGHTALT,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0};


//const int rowX[14]={12,13,5,6,1,A4,A1,7,A3,30,A2,A0,31,4};
const uint8_t rowY[8]={0xbf,0xfd,0x7f,0xfe,0xef,0xdf,0xfb,0xf7};
const uint8_t rowYN[8]={0x40,0x02,0x80,0x01,0x10,0x20,0x04,0x08};



#define INT_PIN PIN2

bool key_fn = false;
bool ActionSend = false;
bool tap_enabled = true;

int oldbutton;	                                          
uint8_t tpdata[0x20];
uint16_t lastposX=0xffff;
uint16_t lastposY=0xffff;

uint8_t button = 0;
uint8_t keyc;
uint8_t colmn;
uint8_t lastfingers;
uint8_t tpguard;
uint8_t touch_to;
bool drag = false;
bool TouchPadLocked = false;
bool tp_guard = false; 
bool ReadTP = false;
bool safeclick = true;
#define MOUSE_LEFT (1<<0)
#define MOUSE_RIGTH (1<<1)
#define MOUSE_MIDDLE (1<<2)
#define DELTAX 6
#define DELTAY 5
#define WHEEL 1
#define HYST 1
#define DRAG_HYST 5
uint8_t ticks;
uint8_t mouse = 0;
uint16_t MR_X;
uint16_t MR_Y;
uint8_t MR_B;
uint8_t MR_W;
uint8_t dragtime;
uint8_t speedlimit;
#define DRAGTIME 8

#define MINPRESS 3
#define min_CLICK_MS CLICK_MS-MINPRESS
uint8_t presstime;
#define DRAG_ENABLED 1
#define TP_LOCK	20
#define TAP_ENABLED 1

uint8_t time_zero;
uint8_t time_one;
uint8_t time_two;
uint8_t time_pressed;
#define PRETAP	3
#define CLICK_MS 8
uint8_t MR_B_REQ = 0;
#define TOUCH_TO	8
uint16_t startposX=0xffff;
uint16_t startposY=0xffff;
bool itsDone = false;
#endif



