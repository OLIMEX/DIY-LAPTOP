#ifndef	_LINUX_AXP81X_FILENODE_H_
#define	_LINUX_AXP81X_FILENODE_H_

#define AXP_POK_SET                     AXP81X_POK_SET
#define AXP_BUFFERC                     AXP81X_BUFFERC
#define AXP_CHARGE_CONTROL1             AXP81X_CHARGE_CONTROL1
#define AXP_CHARGE_CONTROL2             AXP81X_CHARGE_CONTROL2
#define AXP_CHARGE_CONTROL3             AXP81X_CHARGE_CONTROL3
#define AXP_ADC_CONTROL3                AXP81X_ADC_CONTROL3
#define AXP_ADC_CONTROL4                AXP81X_ADC_CONTROL4
#define AXP_CHARGE_VBUS                 AXP81X_CHARGE_VBUS
#define AXP_CHARGE_AC                   AXP81X_CHARGE_AC
#define AXP_BC_SET                      AXP81X_BC_SET
#define AXP_BC_DET_STATUS               AXP81X_BC_DET_STATUS

#define	AXP_IRQ_USBRE                   AXP81X_IRQ_USBRE
#define	AXP_IRQ_USBIN                   AXP81X_IRQ_USBIN
#define	AXP_IRQ_USBOV                   AXP81X_IRQ_USBOV
#define	AXP_IRQ_ACRE                    AXP81X_IRQ_ACRE
#define	AXP_IRQ_ACIN                    AXP81X_IRQ_ACIN
#define	AXP_IRQ_ACOV                    AXP81X_IRQ_ACOV
#define	AXP_IRQ_CHAOV                   AXP81X_IRQ_CHAOV
#define	AXP_IRQ_CHAST                   AXP81X_IRQ_CHAST
#define	AXP_IRQ_BATATOU                 AXP81X_IRQ_BATATOU
#define	AXP_IRQ_BATATIN                 AXP81X_IRQ_BATATIN
#define AXP_IRQ_BATRE                   AXP81X_IRQ_BATRE
#define AXP_IRQ_BATIN                   AXP81X_IRQ_BATIN
#define AXP_IRQ_QBATINWORK              AXP81X_IRQ_QBATINWORK
#define AXP_IRQ_BATINWORK               AXP81X_IRQ_BATINWORK
#define AXP_IRQ_QBATOVWORK              AXP81X_IRQ_QBATOVWORK
#define AXP_IRQ_BATOVWORK               AXP81X_IRQ_BATOVWORK
#define AXP_IRQ_QBATINCHG               AXP81X_IRQ_QBATINCHG
#define AXP_IRQ_BATINCHG                AXP81X_IRQ_BATINCHG
#define AXP_IRQ_QBATOVCHG               AXP81X_IRQ_QBATOVCHG
#define AXP_IRQ_BATOVCHG                AXP81X_IRQ_BATOVCHG
#define AXP_IRQ_EXTLOWARN2              AXP81X_IRQ_EXTLOWARN2
#define AXP_IRQ_EXTLOWARN1              AXP81X_IRQ_EXTLOWARN1
#define AXP_IRQ_ADCFINISHED             AXP81X_IRQ_ADCFINISHED
#define AXP_IRQ_PMICTEMP_OV_LEVEL2      AXP81X_IRQ_PMICTEMP_OV_LEVEL2
#define AXP_IRQ_GPIO0TG                 AXP81X_IRQ_GPIO0TG
#define AXP_IRQ_GPIO1TG                 AXP81X_IRQ_GPIO1TG
#define AXP_IRQ_PEK_OFFTIME             AXP81X_IRQ_PEK_OFFTIME
#define AXP_IRQ_PEK_LONGTIME            AXP81X_IRQ_PEK_LONGTIME
#define AXP_IRQ_PEK_SHORTTIME           AXP81X_IRQ_PEK_SHORTTIME
#define AXP_IRQ_PEK_NEDGE               AXP81X_IRQ_PEK_NEDGE
#define AXP_IRQ_PEK_PEDGE               AXP81X_IRQ_PEK_PEDGE
#define AXP_IRQ_TIMER                   AXP81X_IRQ_TIMER
#define AXP_IRQ_VBUS_CHNG               AXP81X_IRQ_VBUS_CHNG
#define AXP_IRQ_MV_CHNG                 AXP81X_IRQ_MV_CHNG
#define AXP_IRQ_BC_USB_CHNG             AXP81X_IRQ_BC_USB_CHNG
#define AXP_INTEN1                      AXP81X_INTEN1
#define AXP_INTEN2                      AXP81X_INTEN2
#define AXP_INTEN3                      AXP81X_INTEN3
#define AXP_INTEN4                      AXP81X_INTEN4
#define AXP_INTEN5                      AXP81X_INTEN5
#define AXP_INTEN6                      AXP81X_INTEN6
#define AXP_INTSTS1                     AXP81X_INTSTS1
#define AXP_INTSTS2                     AXP81X_INTSTS2
#define AXP_INTSTS3                     AXP81X_INTSTS3
#define AXP_INTSTS4                     AXP81X_INTSTS4
#define AXP_INTSTS5                     AXP81X_INTSTS5
#define AXP_INTSTS6                     AXP81X_INTSTS6

#define AXP_STATUS_SOURCE               AXP81X_STATUS_SOURCE
#define AXP_STATUS_ACUSBSH              AXP81X_STATUS_ACUSBSH
#define AXP_STATUS_BATCURDIR            AXP81X_STATUS_BATCURDIR
#define AXP_STATUS_USBLAVHO             AXP81X_STATUS_USBLAVHO
#define AXP_STATUS_USBVA                AXP81X_STATUS_USBVA
#define AXP_STATUS_USBEN                AXP81X_STATUS_USBEN
#define AXP_STATUS_ACVA                 AXP81X_STATUS_ACVA
#define AXP_STATUS_ACEN                 AXP81X_STATUS_ACEN
#define AXP_STATUS_BATINACT             AXP81X_STATUS_BATINACT
#define AXP_STATUS_BATEN                AXP81X_STATUS_BATEN
#define AXP_STATUS_INCHAR               AXP81X_STATUS_INCHAR
#define AXP_STATUS_ICTEMOV              AXP81X_STATUS_ICTEMOV

#define AXP_VBATH_RES                   AXP81X_VBATH_RES
#define AXP_OCVBATH_RES                 AXP81X_OCVBATH_RES
#define AXP_VTS_RES                     AXP81X_VTS_RES
#define AXP_CHARGE_STATUS               AXP81X_CHARGE_STATUS
#define AXP_IN_CHARGE                   AXP81X_IN_CHARGE
#define AXP_INTTEMP                     AXP81X_INTTEMP

#define AXP_VOL_MAX                     AXP81X_VOL_MAX
#define AXP_CAP                         AXP81X_CAP

#define CHARGE_VOLTAGE_LEVEL0           AXP81X_CHARGE_VOLTAGE_LEVEL0
#define CHARGE_VOLTAGE_LEVEL1           AXP81X_CHARGE_VOLTAGE_LEVEL1
#define CHARGE_VOLTAGE_LEVEL2           AXP81X_CHARGE_VOLTAGE_LEVEL2
#define CHARGE_VOLTAGE_LEVEL3           AXP81X_CHARGE_VOLTAGE_LEVEL3
#define CHARGE_CURRENT_MIN              AXP81X_CHARGE_CURRENT_MIN
#define CHARGE_CURRENT_MAX              AXP81X_CHARGE_CURRENT_MAX
#define CHARGE_CURRENT_STEP             AXP81X_CHARGE_CURRENT_STEP
#define CHARGE_END_LEVEL0               AXP81X_CHARGE_END_LEVEL0
#define CHARGE_END_LEVEL1               AXP81X_CHARGE_END_LEVEL1
#define CHARGE_PRETIME_MIN              AXP81X_CHARGE_PRETIME_MIN
#define CHARGE_PRETIME_MAX              AXP81X_CHARGE_PRETIME_MAX
#define CHARGE_PRETIME_STEP             AXP81X_CHARGE_PRETIME_STEP
#define CHARGE_FASTTIME_MIN             AXP81X_CHARGE_FASTTIME_MIN
#define CHARGE_FASTTIME_MAX             AXP81X_CHARGE_FASTTIME_MAX
#define CHARGE_FASTTIME_STEP            AXP81X_CHARGE_FASTTIME_STEP

#define AXP81X_NOTIFIER_ON          (AXP_IRQ_USBIN | AXP_IRQ_USBRE | \
                                    AXP_IRQ_ACIN  | AXP_IRQ_ACRE | \
                                    AXP_IRQ_BATIN | AXP_IRQ_BATRE | \
                                    AXP_IRQ_BATINWORK | AXP_IRQ_BATOVWORK | \
                                    AXP_IRQ_QBATINCHG | AXP_IRQ_BATINCHG | \
                                    AXP_IRQ_QBATOVCHG | AXP_IRQ_BATOVCHG | \
                                    AXP_IRQ_CHAST |  AXP_IRQ_CHAOV | \
                                    (u64)AXP_IRQ_PEK_NEDGE | \
                                    (u64)AXP_IRQ_PEK_PEDGE)
#endif

