#
# Makefile for SUNXI USB2.0 Dual Role Controller Driver.
#

sunxi_usbc-objs := usbc/usbc.o \
				usbc/usbc_dev.o \
				usbc/usbc_phy.o \
				misc/sunxi_usb_debug.o \
				manager/usb_manager.o \
				manager/usbc0_platform.o \
				manager/usb_hcd_servers.o \
				manager/usb_hw_scan.o \
				manager/usb_msg_center.o

obj-$(CONFIG_USB_SUNXI_USB_MANAGER) += sunxi_usbc.o

sunxi_usb_udc-objs :=  udc/sunxi_udc.o \
                    udc/sunxi_udc_board.o \
                    udc/sunxi_udc_debug.o \
                    udc/sunxi_udc_dma.o

obj-$(CONFIG_USB_SUNXI_UDC0) += sunxi_usb_udc.o

