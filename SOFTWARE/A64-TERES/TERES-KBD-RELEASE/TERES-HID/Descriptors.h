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

/** \file
 *
 *  Header file for Descriptors.c.
 */

#ifndef _DESCRIPTORS_H_
#define _DESCRIPTORS_H_


	/* Includes: */
		#include <avr/pgmspace.h>

		#include <LUFA/Drivers/USB/USB.h>


		/** \hideinitializer
		 *  A list of HID report item array elements that describe a typical HID USB mouse. The resulting report descriptor
		 *  is compatible with \ref USB_MouseReport_Data_t if the \c MinAxisVal and \c MaxAxisVal values fit within a \c int8_t range
		 *  and the number of Buttons is less than 8. For other values, the report is structured according to the following layout:
		 *
		 *  \code
		 *  struct
		 *  {
		 *      uintA_t Buttons; // Pressed buttons bitmask
		 *      intB_t X; // X axis value
		 *      intB_t Y; // Y axis value
		 *  } Mouse_Report;
		 *  \endcode
		 *
		 *  Where \c intA_t is a type large enough to hold one bit per button, and \c intB_t is a type large enough to hold the
		 *  ranges of the signed \c MinAxisVal and \c MaxAxisVal values.
		 *
		 *  \param[in] MinAxisVal      Minimum X/Y logical axis value (16-bit).
		 *  \param[in] MaxAxisVal      Maximum X/Y logical axis value (16-bit).
		 *  \param[in] MinPhysicalVal  Minimum X/Y physical axis value, for movement resolution calculations (16-bit).
		 *  \param[in] MaxPhysicalVal  Maximum X/Y physical axis value, for movement resolution calculations (16-bit).
		 *  \param[in] Buttons         Total number of buttons in the device (8-bit).
		 *  \param[in] AbsoluteCoords  Boolean \c true to use absolute X/Y coordinates (e.g. touchscreen).
		 */
		#define HID_DESCRIPTOR_WHEEL_MOUSE(MinAxisVal, MaxAxisVal, MinPhysicalVal, MaxPhysicalVal, Buttons, AbsoluteCoords) \
			HID_RI_USAGE_PAGE(8, 0x01),                     \
			HID_RI_USAGE(8, 0x02),                          \
			HID_RI_COLLECTION(8, 0x01),                     \
				HID_RI_USAGE(8, 0x01),                      \
				HID_RI_COLLECTION(8, 0x00),                 \
					HID_RI_USAGE_PAGE(8, 0x09),             \
					HID_RI_USAGE_MINIMUM(8, 0x01),          \
					HID_RI_USAGE_MAXIMUM(8, Buttons),       \
					HID_RI_LOGICAL_MINIMUM(8, 0x00),        \
					HID_RI_LOGICAL_MAXIMUM(8, 0x01),        \
					HID_RI_REPORT_COUNT(8, Buttons),        \
					HID_RI_REPORT_SIZE(8, 0x01),            \
					HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE), \
					HID_RI_REPORT_COUNT(8, 0x01),           \
					HID_RI_REPORT_SIZE(8, (Buttons % 8) ? (8 - (Buttons % 8)) : 0), \
					HID_RI_INPUT(8, HID_IOF_CONSTANT),      \
					HID_RI_USAGE_PAGE(8, 0x01),             \
					HID_RI_USAGE(8, 0x30),                  \
					HID_RI_USAGE(8, 0x31),                  \
					HID_RI_LOGICAL_MINIMUM(16, MinAxisVal), \
					HID_RI_LOGICAL_MAXIMUM(16, MaxAxisVal), \
					HID_RI_PHYSICAL_MINIMUM(16, MinPhysicalVal), \
					HID_RI_PHYSICAL_MAXIMUM(16, MaxPhysicalVal), \
					HID_RI_REPORT_COUNT(8, 0x02),           \
					HID_RI_REPORT_SIZE(8, (((MinAxisVal >= -128) && (MaxAxisVal <= 127)) ? 8 : 16)), \
					HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | (AbsoluteCoords ? HID_IOF_ABSOLUTE : HID_IOF_RELATIVE)), \
					HID_RI_USAGE(8, 0x38), \
					HID_RI_LOGICAL_MINIMUM(8, -127), \
					HID_RI_LOGICAL_MAXIMUM(8, 127), \
					HID_RI_PHYSICAL_MINIMUM(8, -1), \
					HID_RI_PHYSICAL_MAXIMUM(8, 1), \
					HID_RI_REPORT_SIZE(8, 0x08), \
					HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_RELATIVE), \
				HID_RI_END_COLLECTION(0),                   \
			HID_RI_END_COLLECTION(0)



	/* Type Defines: */
		/** Type define for the device configuration descriptor structure. This must be defined in the
		 *  application code, as the configuration descriptor contains several sub-descriptors which
		 *  vary between devices, and which describe the device's usage to the host.
		 */
		typedef struct
		{
			USB_Descriptor_Configuration_Header_t Config;
		
			// Keyboard HID Interface
			USB_Descriptor_Interface_t            HID1_KeyboardInterface;
			USB_HID_Descriptor_HID_t              HID1_KeyboardHID;
			USB_Descriptor_Endpoint_t             HID1_ReportINEndpoint;

			// Mouse HID Interface
			USB_Descriptor_Interface_t            HID2_MouseInterface;
			USB_HID_Descriptor_HID_t              HID2_MouseHID;
			USB_Descriptor_Endpoint_t             HID2_ReportINEndpoint;
		} USB_Descriptor_Configuration_t;

		/** Enum for the device interface descriptor IDs within the device. Each interface descriptor
		 *  should have a unique ID index associated with it, which can be used to refer to the
		 *  interface from other descriptors.
		 */
		enum InterfaceDescriptors_t
		{

			INTERFACE_ID_Keyboard = 0, /**< Keyboard interface descriptor ID */
			INTERFACE_ID_Mouse    = 1, /**< Mouse interface descriptor ID */				
			
				

		};

		/** Enum for the device string descriptor IDs within the device. Each string descriptor should
		 *  have a unique ID index associated with it, which can be used to refer to the string from
		 *  other descriptors.
		 */
		enum StringDescriptors_t
		{
			STRING_ID_Language     = 0, /**< Supported Languages string descriptor ID (must be zero) */
			STRING_ID_Manufacturer = 1, /**< Manufacturer string ID */
			STRING_ID_Product      = 2, /**< Product string ID */
		};

	/* Macros: */
	
	
		/** Endpoint address of the Keyboard HID reporting IN endpoint. */
		#define KEYBOARD_IN_EPADDR        (ENDPOINT_DIR_IN | 5)

		/** Endpoint address of the Mouse HID reporting IN endpoint. */
		#define MOUSE_IN_EPADDR           (ENDPOINT_DIR_IN | 1)

		/** Size in bytes of each of the HID reporting IN endpoints. */
		#define HID_EPSIZE                8

	/* Function Prototypes: */
		uint16_t CALLBACK_USB_GetDescriptor(const uint16_t wValue,
		                                    const uint16_t wIndex,
		                                    const void** const DescriptorAddress)
		                                    ATTR_WARN_UNUSED_RESULT ATTR_NON_NULL_PTR_ARG(3);
      typedef struct
        {
            uint8_t Button;
            int8_t  X;
            int8_t  Y;
            int8_t Wheel;
        } ATTR_PACKED USB_WheelMouseReport_Data_t;
#endif

