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
 *  Main source file for the KeyboardMouse demo. This file contains the main tasks of
 *  the demo and is responsible for the initial application hardware configuration.
 */

#include "KeyboardMouse.h"
/** Magic lock for forced application start. If the HWBE fuse is programmed and BOOTRST is unprogrammed, the bootloader
 *  will start if the /HWB line of the AVR is held low and the system is reset. However, if the /HWB line is still held
 *  low when the application attempts to start via a watchdog reset, the bootloader will re-start. If set to the value
 *  \ref MAGIC_BOOT_KEY the special init function \ref Application_Jump_Check() will force the application to start.
 */
uint16_t MagicBootKey ATTR_NO_INIT;

/** Buffer to hold the previously generated Keyboard HID report, for comparison purposes inside the HID class driver. */
static uint8_t PrevKeyboardHIDReportBuffer[sizeof(USB_KeyboardReport_Data_t)];

/** Buffer to hold the previously generated Mouse HID report, for comparison purposes inside the HID class driver. */
static uint8_t PrevMouseHIDReportBuffer[sizeof(USB_WheelMouseReport_Data_t)];



int limited(int value)
{
if (value < 0)
{
	if (abs(value) > speedlimit)
				return -speedlimit;

}
else
{
	if (value > speedlimit)
				return speedlimit;
	
	
}		

return value;
	
	
}	

/** LUFA HID Class driver interface configuration and state information. This structure is
 *  passed to all HID Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another. This is for the keyboard HID
 *  interface within the device.
 */
USB_ClassInfo_HID_Device_t Keyboard_HID_Interface =
	{
		.Config =
			{
				.InterfaceNumber              = INTERFACE_ID_Keyboard,
				.ReportINEndpoint             =
					{
						.Address              = KEYBOARD_IN_EPADDR,
						.Size                 = HID_EPSIZE,
						.Banks                = 1,
					},
				.PrevReportINBuffer           = PrevKeyboardHIDReportBuffer,
				.PrevReportINBufferSize       = sizeof(PrevKeyboardHIDReportBuffer),
			},
	};

/** LUFA HID Class driver interface configuration and state information. This structure is
 *  passed to all HID Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another. This is for the mouse HID
 *  interface within the device.
 */
USB_ClassInfo_HID_Device_t Mouse_HID_Interface =
	{
		.Config =
			{
				.InterfaceNumber              = INTERFACE_ID_Mouse,
				.ReportINEndpoint             =
					{
						.Address              = MOUSE_IN_EPADDR,
						.Size                 = HID_EPSIZE,
						.Banks                = 1,
					},
				.PrevReportINBuffer           = PrevMouseHIDReportBuffer,
				.PrevReportINBufferSize       = sizeof(PrevMouseHIDReportBuffer),
			},
	};

  
  void Jump_To_Bootloader(void)
  {
      // If USB is used, detach from the bus and reset it
      USB_Disable();

      // Disable all interrupts
      cli();

      // Wait one seconds for the USB detachment to register on the host
      Delay_MS(1000);

      // Set the bootloader key to the magic value and force a reset
      wdt_enable(WDTO_250MS);
      for (;;);
  }

void tp_guarder(void)
{
	tp_guard=true;
	tpguard=TP_LOCK;
}
/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{


	SetupHardware();
	GlobalInterruptEnable();

	for (;;)
	{
		
	
		HID_Device_USBTask(&Keyboard_HID_Interface);
		DDRE=0x00;
		PORTE=0xff;	
		PORTB=0xfd;
		DDRB=0xff;
		HID_Device_USBTask(&Mouse_HID_Interface);
		USB_USBTask();

		if  ((PINE & (1<<2)) == 0) 
		{
			DDRB=0xff;	
			PORTB=0xfe;
			 Delay_MS(10);
			if ((PINC & (1<<7)) == 0) 
			{	
			
		
				PORTB=0xbf;	
				 Delay_MS(10);
				if ((PIND & (1<<4)) == 0)
							Jump_To_Bootloader();
			}
		}
	}
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware()
{
#if (ARCH == ARCH_AVR8)
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);
#elif (ARCH == ARCH_XMEGA)
	/* Start the PLL to multiply the 2MHz RC oscillator to 32MHz and switch the CPU core to run from it */
	XMEGACLK_StartPLL(CLOCK_SRC_INT_RC2MHZ, 2000000, F_CPU);
	XMEGACLK_SetCPUClockSource(CLOCK_SRC_PLL);

	/* Start the 32MHz internal RC oscillator and start the DFLL to increase it to 48MHz using the USB SOF as a reference */
	XMEGACLK_StartInternalOscillator(CLOCK_SRC_INT_RC32MHZ);
	XMEGACLK_StartDFLL(CLOCK_SRC_INT_RC32MHZ, DFLL_REF_INT_USBSOF, F_USB);

	PMIC.CTRL = PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;
#endif

	/* Hardware Initialization */

	//Keyboard init
	DDRB = 0xff;
	PORTB = 0xff;

	DDRD = 0;
	PORTD = 0xfc;

	DDRC = 0;
	PORTC = 0xc0;

	DDRE = 0;
    PORTE = 0x44;

    DDRF = 0;
    PORTF = 0xf2;



	TWI_Init(TWI_BIT_PRESCALE_4 , TWI_BITLENGTH_FROM_FREQ(TWI_BIT_PRESCALE_4 , 100000));
	ADC_Init(ADC_FREE_RUNNING  | ADC_PRESCALE_128);
	ADCSRA |= 1 << ADIE;
	MCUCR = 0;
	EIMSK |= 1 << INT2;

	ADC_SetupChannel(0);
	ADC_StartReading(ADC_REFERENCE_AVCC | ADC_LEFT_ADJUSTED | ADC_CHANNEL0);
	
	// set timer0 counter initial value to 0
	TCNT0=0x00;
	// start timer0 with /1024 prescaler
	TCCR0B = (1<<CS02) | (1<<CS00);
	TIMSK0=1<<TOIE0;

	USB_Init();
	button = 0xff;
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{

}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{

}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	ConfigSuccess &= HID_Device_ConfigureEndpoints(&Keyboard_HID_Interface);
	ConfigSuccess &= HID_Device_ConfigureEndpoints(&Mouse_HID_Interface);
	USB_Device_EnableSOFEvents();

}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	HID_Device_ProcessControlRequest(&Keyboard_HID_Interface);
	HID_Device_ProcessControlRequest(&Mouse_HID_Interface);
	
	
}

/** Event handler for the USB device Start Of Frame event. */
void EVENT_USB_Device_StartOfFrame(void)
{
	HID_Device_MillisecondElapsed(&Keyboard_HID_Interface);
	HID_Device_MillisecondElapsed(&Mouse_HID_Interface);
}

/** HID class driver callback function for the creation of HID reports to the host.
 *
 *  \param[in]     HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in,out] ReportID    Report ID requested by the host if non-zero, otherwise callback should set to the generated report ID
 *  \param[in]     ReportType  Type of the report to create, either HID_REPORT_ITEM_In or HID_REPORT_ITEM_Feature
 *  \param[out]    ReportData  Pointer to a buffer where the created report should be stored
 *  \param[out]    ReportSize  Number of bytes written in the report (or zero if no report is to be sent)
 *
 *  \return Boolean \c true to force the sending of the report, \c false to let the library determine if it needs to be sent
 */
bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                         uint8_t* const ReportID,
                                         const uint8_t ReportType,
                                         void* ReportData,
                                         uint16_t* const ReportSize)
{


	/* Determine which interface must have its report generated */
	if (HIDInterfaceInfo == &Keyboard_HID_Interface)
	{
		USB_KeyboardReport_Data_t* KeyboardReport = (USB_KeyboardReport_Data_t*)ReportData;

uint8_t x=0;
uint8_t fn = 0;

keyc = 0; //reset keys
KeyboardReport->Modifier = 0;

for (x=0;x<6;x++)
KeyboardReport->KeyCode[x]=0;


if (itsDone)
{
//	
	
itsDone = false;	
#if 0

switch (tpdata[2])
{
case 0:
KeyboardReport->KeyCode[0]=HID_KEYBOARD_SC_0_AND_CLOSING_PARENTHESIS;	
break;
case 1:
KeyboardReport->KeyCode[0]=HID_KEYBOARD_SC_1_AND_EXCLAMATION;	
break;
case 2:
KeyboardReport->KeyCode[0]=HID_KEYBOARD_SC_2_AND_AT;
break;
default:
KeyboardReport->KeyCode[0]=HID_KEYBOARD_SC_DOT_AND_GREATER_THAN_SIGN;
break;
}

keyc = 1;

#endif
}
DDRE=0;
DDRF=0;
DDRC=0;
	DDRD = 0;
	PORTD = 0xfc;
PORTE=0xFF;
PORTF=0xFF;
PORTC=0xFF;


DDRB = 0xff;

for (colmn = 0; colmn<8; colmn++)
{
	
uint8_t keynow=0;

if (colmn == 0)
{
cli();
DDRB=0x00; 
PORTB=0x00;
DDRD = (1<<4);
PORTD = (1<<4);

Delay_MS(2);

if (PINB & 0x40)
{
	fn = 112;
	//tp_guarder();
	if (PINB & 0x01)
	{
	KeyboardReport->KeyCode[keyc]=HID_KEYBOARD_SC_PAGE_UP;
	if (keyc<5) keyc++;
	}	
	if (PINB & 0x02)
	{
	KeyboardReport->Modifier |= HID_KEYBOARD_MODIFIER_LEFTSHIFT;
	KeyboardReport->KeyCode[keyc]=HID_KEYBOARD_SC_LEFT_SHIFT;
	if (keyc<5) keyc++;
	
	}		
	if (PINB & 0x20)
	{
	KeyboardReport->KeyCode[keyc]=HID_KEYBOARD_SC_HOME;
	if (keyc<5) keyc++;
	}
	
	if (PINB & 0x80)
	{
	KeyboardReport->Modifier |= HID_KEYBOARD_MODIFIER_RIGHTSHIFT;
	KeyboardReport->KeyCode[keyc]=HID_KEYBOARD_SC_RIGHT_SHIFT;
	if (keyc<5) keyc++;
	}		
	
	
	
}	
else
{
		
	if (PINB & 0x01)
	{
	KeyboardReport->KeyCode[keyc]=HID_KEYBOARD_SC_UP_ARROW;
	if (keyc<5) keyc++;
	//tp_guarder();
	}	
	if (PINB & 0x02)
	{
	KeyboardReport->Modifier |= HID_KEYBOARD_MODIFIER_LEFTSHIFT;
	KeyboardReport->KeyCode[keyc]=HID_KEYBOARD_SC_LEFT_SHIFT;
	if (keyc<5) keyc++;
	}		
	if (PINB & 0x20)
	{
	KeyboardReport->KeyCode[keyc]=HID_KEYBOARD_SC_LEFT_ARROW;
	if (keyc<5) keyc++;
	//tp_guarder();
	}
	
	if (PINB & 0x80)
	{
	KeyboardReport->Modifier |= HID_KEYBOARD_MODIFIER_RIGHTSHIFT;
	KeyboardReport->KeyCode[keyc]=HID_KEYBOARD_SC_RIGHT_SHIFT;
	if (keyc<5) keyc++;
	}		
	
	
	
}	

//////x0
DDRD = (1<<6);
PORTD = (1<<6);
DDRB=0x00; 
PORTB=0x00;
Delay_MS(2);

if (PINB & 0x10)
{
	KeyboardReport->Modifier |= HID_KEYBOARD_MODIFIER_RIGHTCTRL;
	KeyboardReport->KeyCode[keyc]=HID_KEYBOARD_SC_RIGHT_CONTROL;
	if (keyc<5) keyc++;
	
}	
if (PINB & 0x40)
{
	KeyboardReport->Modifier |= HID_KEYBOARD_MODIFIER_LEFTCTRL;
	KeyboardReport->KeyCode[keyc]=HID_KEYBOARD_SC_LEFT_CONTROL;
	if (keyc<5) keyc++;
	
}	
if (PINB & 0x08)
{
	if (fn==112)
	KeyboardReport->KeyCode[keyc]=HID_KEYBOARD_SC_VOLUME_UP;
	else
	KeyboardReport->KeyCode[keyc]=HID_KEYBOARD_SC_F5;
	if (keyc<5) keyc++;
	
}	


PORTB = rowY[colmn];
DDRB = 0xff;
	DDRD = 0;
	PORTD = 0xfc;
Delay_MS(3);
sei();
} 

PORTB = rowY[colmn];
while (PINB != rowY[colmn]);

x=1;

if  ((PINC & (1<<7)) == 0) 
{
	//tp_guarder();
keynow=	keys[colmn*14+x+fn];
KeyboardReport->Modifier |= keym[colmn*14+x];
if (keynow!=0)
{
KeyboardReport->KeyCode[keyc]=keys[colmn*14+x+fn];
if (keyc<5) keyc++;
}
}
x++;//2
if  ((PINC & (1<<6)) == 0) 
{
//	tp_guarder();
keynow=	keys[colmn*14+x+fn];
KeyboardReport->Modifier |= keym[colmn*14+x];
if (keynow!=0)
{
KeyboardReport->KeyCode[keyc]=keynow;
if (keyc<5) keyc++;
}
}
x++;//3
//Delay_MS(5);
if  ((PIND & (1<<7))== 0) 
{
//	tp_guarder();
keynow=	keys[colmn*14+x+fn];
KeyboardReport->Modifier |= keym[colmn*14+x];


if (keynow == HID_KEY_LOCK_TOUCHPAD)
    {
		TouchPadLocked=!TouchPadLocked;
	while ((PIND & (1<<7))== 0);	
	}
if (keynow!=0)
{
KeyboardReport->KeyCode[keyc]=keynow;
if (keyc<5) keyc++;
}
}
x++;//4
//Delay_MS(5);
if  ((PIND & (1<<3)) == 0) 
{
//	tp_guarder();
keynow=	keys[colmn*14+x+fn];
KeyboardReport->Modifier |= keym[colmn*14+x];
if (keynow!=0)
{
KeyboardReport->KeyCode[keyc]=keynow;
if (keyc<5) keyc++;
}
}
x++;//5
if  ((PINF & (1<<1)) == 0) 
{
//	tp_guarder();
keynow=	keys[colmn*14+x+fn];
KeyboardReport->Modifier |= keym[colmn*14+x];
if (keynow!=0)
{
KeyboardReport->KeyCode[keyc]=keynow;
if (keyc<5) keyc++;
}
}
x++;//6
if  ((PINF & (1<<6)) == 0) 
{
//	tp_guarder();
keynow=	keys[colmn*14+x+fn];
KeyboardReport->Modifier |= keym[colmn*14+x];
if (keynow!=0)
{
KeyboardReport->KeyCode[keyc]=keynow;
if (keyc<5) keyc++;
}
}
x++;//7
if  ((PINE & (1<<6)) == 0) 
{
//	tp_guarder();
keynow=	keys[colmn*14+x+fn];
KeyboardReport->Modifier |= keym[colmn*14+x];
if (keynow!=0)
{
KeyboardReport->KeyCode[keyc]=keynow;
if (keyc<5) keyc++;
}
}
x++;//8
if  ((PINF & (1<<4)) == 0) 
{
//	tp_guarder();
keynow=	keys[colmn*14+x+fn];
KeyboardReport->Modifier |= keym[colmn*14+x];
if (keynow!=0)
{
KeyboardReport->KeyCode[keyc]=keynow;
if (keyc<5) keyc++;
}
}
x++;//9
if  ((PIND & (1<<5)) == 0) 
{
//	tp_guarder();
keynow=	keys[colmn*14+x+fn];
KeyboardReport->Modifier |= keym[colmn*14+x];
if (keynow!=0)
{
KeyboardReport->KeyCode[keyc]=keynow;
if (keyc<5) keyc++;
}
}
x++;//10
if  ((PINF & (1<<5)) == 0) 
{
//	tp_guarder();
keynow=	keys[colmn*14+x+fn];
KeyboardReport->Modifier |= keym[colmn*14+x];
if (keynow!=0)
{
KeyboardReport->KeyCode[keyc]=keynow;
if (keyc<5) keyc++;
}
}


x++;//11
if  ((PINF & (1<<7)) == 0) 
{
//	tp_guarder();
keynow=	keys[colmn*14+x+fn];
KeyboardReport->Modifier |= keym[colmn*14+x];
if (keynow!=0)
{
KeyboardReport->KeyCode[keyc]=keynow;
if (keyc<5) keyc++;
}
}


x++;//12
if  ((PINE & (1<<2)) == 0) 
{
//	tp_guarder();
keynow=	keys[colmn*14+x+fn];
KeyboardReport->Modifier |= keym[colmn*14+x];
if (keynow!=0)
{
KeyboardReport->KeyCode[keyc]=keynow;
if (keyc<5) keyc++;

}
}
	DDRD = 0;
	PORTD = 0xfc;

	
}

if ((KeyboardReport->Modifier == 0) && (keyc!=0))  tp_guarder();


		*ReportSize = sizeof(USB_KeyboardReport_Data_t);
		return true;//return false;
	}
if (HIDInterfaceInfo == &Mouse_HID_Interface)
	{
		USB_WheelMouseReport_Data_t* MouseReport = (USB_WheelMouseReport_Data_t*)ReportData;

	
			MR_Y = 0;
			MR_X = 0;
			MR_W = 0;
			MR_B |= mouse;// (1 << 0);
		
//cli();		
if (ActionSend)
{	
uint16_t posx1 = tpdata[3]<<8 | tpdata[4];
uint16_t posy1 = tpdata[5]<<8 | tpdata[6];	
switch (tpdata[2])
{
case 0:
	if (lastfingers != 0)
	{
		if (time_zero>PRETAP)
		{
			
			if (time_pressed<CLICK_MS)
			{
				
			if  (abs(startposX+startposY-posx1-posy1)<10)
			{
				if (time_two != 0) 
					MR_B_REQ = 	MOUSE_RIGTH;
				else
					MR_B_REQ = MOUSE_LEFT;
			}	
			}	
			
			

		}
		
		 time_zero = 0;	
	}	
		speedlimit=0;
		lastposX = 0;
		lastposY = 0;

break;
	
case 1:


	
			touch_to=TOUCH_TO;
	
				switch (lastfingers)
				{	
				case 0:
				
						presstime=CLICK_MS;
						speedlimit=0;
				break;
				case 1:
					
			if (lastposX != 0)
			{
				MR_X = limited(posx1 - lastposX);
				MR_Y = limited(posy1 - lastposY);
				
					
			}
				break;
					
				case 2:


				speedlimit=0;
				break;
			default:
			
						break;
			}
	if (lastfingers != 1)
	{
	time_one = 0;
	startposX=posx1;
	startposY=posy1;
	lastfingers = 1;
	}	
			
			
			
		lastposX = posx1;
		lastposY = posy1;
		if (speedlimit != 0x80)
		speedlimit+=0x04;

break;
				
	
case 2:


	
	
	if (lastfingers == 0)
				touch_to=TOUCH_TO;

			if (lastfingers == 2)
			{
				if (lastposY > posy1 + DRAG_HYST)
							MR_W = WHEEL;
							else if (lastposY + DRAG_HYST < posy1)
							MR_W = -WHEEL;
							
			if (MR_W !=0)  time_zero = 0;	 				
			presstime=0;
			}
			else
			{

			time_two = 0;
			startposX=posx1;
			startposY=posy1;
			lastfingers = 2;
	
			}	
		lastposX = posx1;
		lastposY = posy1;	
				
	break;


default:
		lastposX = 0;
		lastposY = 0;

		break;

	
}			
lastfingers = tpdata[2];
ActionSend = false;	
itsDone = true;
} 	
	MouseReport->Y = MR_Y;
	MouseReport->X = MR_X;
	MouseReport->Button = MR_B;
	MouseReport->Wheel = MR_W;
	
	MR_B=0;

		*ReportSize = sizeof(USB_WheelMouseReport_Data_t);
		return true;
	}

}




/** HID class driver callback function for the processing of HID reports from the host.
 *
 *  \param[in] HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in] ReportID    Report ID of the received report from the host
 *  \param[in] ReportType  The type of report that the host has sent, either HID_REPORT_ITEM_Out or HID_REPORT_ITEM_Feature
 *  \param[in] ReportData  Pointer to a buffer where the received report has been stored
 *  \param[in] ReportSize  Size in bytes of the received HID report
 */
void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                          const uint8_t ReportID,
                                          const uint8_t ReportType,
                                          const void* ReportData,
                                          const uint16_t ReportSize)
{
	if (HIDInterfaceInfo == &Keyboard_HID_Interface)
	{

	}
}

ISR(ADC_vect)
{

				ADCSRA |= (1 << ADIF);
				button= ADCH;
					
						
			if (button == oldbutton)
				{
				if (button < 0xe8)
						{
   
							if (button>0xa0) mouse = MOUSE_RIGTH;
        
								else if (button>0x80)  mouse = MOUSE_LEFT;

									else mouse = MOUSE_MIDDLE;
					
						}	
				else mouse = 0;
				}
		  else oldbutton = button;

}

	
	
	
   	

ISR(INT2_vect)
{
	#if 1
 if (TWI_StartTransmission(0x48 | TWI_ADDRESS_WRITE, 10) == TWI_ERROR_NoError)
	       {
	           TWI_SendByte(0x00);
	           TWI_StopTransmission();
			
	           if (TWI_StartTransmission(0x48 | TWI_ADDRESS_READ, 10) == TWI_ERROR_NoError)
	           {


	               // Read some bytes, acknowledge after the last byte is received????
	             for (int s=0;s<0x06;s++)
	               TWI_ReceiveByte(&tpdata[s], false);
	               
	               
	               TWI_ReceiveByte(&tpdata[0x06], true);

	           }
	       }
	       // Must stop transmission afterwards to release the bus
	       TWI_StopTransmission();
	       
if ((!TouchPadLocked) && (!tp_guard))       
							ActionSend = true;
#endif
}

// timer0 overflow ~60hz
ISR(TIMER0_OVF_vect) {
	
#if 1	

		
		if (TWI_StartTransmission(0x48 | TWI_ADDRESS_WRITE, 10) == TWI_ERROR_NoError)
	       {
	           TWI_SendByte(0x00);
	           TWI_StopTransmission();
			
	           if (TWI_StartTransmission(0x48 | TWI_ADDRESS_READ, 10) == TWI_ERROR_NoError)
	           {


	               // Read some bytes, acknowledge after the last byte is received????
	             for (int s=0;s<0x06;s++)
	               TWI_ReceiveByte(&tpdata[s], false);
	               
	               
	               TWI_ReceiveByte(&tpdata[0x06], true);

	           }
	       }
	       // Must stop transmission afterwards to release the bus
	       TWI_StopTransmission();
	  if ((!TouchPadLocked) && (!tp_guard) && (tpdata[0]!=0))       
							ActionSend = true;     
		

	
	
	
#endif	
	
	
						if ((time_zero != 0xff) && (tpdata[2]==0)) time_zero++;
						
						if (time_zero==TOUCH_TO)
						{
#if TAP_ENABLED							
							 MR_B=MR_B_REQ;
							 MR_B_REQ=0;
#endif
							 time_one=0;
							 time_two=0;
							 time_pressed=0;
						}
						if ((time_one != 0xff) && (tpdata[2]==1)) time_one++;
						if ((time_two != 0xff) && (tpdata[2]==2)) time_two++;
						
						
						if (ticks!=0) ticks--;
						if (dragtime!=0) dragtime--;
						if (presstime != 0) presstime--;
						if (tpguard !=0) tpguard--;
						if (tpguard==1) tp_guard = false;
						
						
						
						if (touch_to!=0) touch_to--;
						if (touch_to==1)
						{
							tap_enabled=true;
							lastposX=0;
							lastposY=0;
							drag=false;
							if (presstime!=0)
								ActionSend = true;
							else
							lastfingers=0;
						
						}
}
