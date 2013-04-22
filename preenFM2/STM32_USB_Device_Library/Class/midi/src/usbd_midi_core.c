
#include "usbd_midi_core.h"
#include "usb_dcd.h"
#include "PreenFM.h"


/*********************************************
 midi Device library callbacks
 *********************************************/
static uint8_t usbd_midi_Init(void *pdev, uint8_t cfgidx);
static uint8_t usbd_midi_DeInit(void *pdev, uint8_t cfgidx);
static uint8_t usbd_midi_Setup(void *pdev, USB_SETUP_REQ *req);
static uint8_t usbd_midi_EP0_RxReady(void *pdev);
static uint8_t usbd_midi_DataIn(void *pdev, uint8_t epnum);
static uint8_t usbd_midi_DataOut(void *pdev, uint8_t epnum);
static uint8_t usbd_midi_SOF(void *pdev);
static uint8_t usbd_midi_OUT_Incplt(void *pdev);
static uint8_t *usbd_midi_GetCfgDesc(uint8_t speed, uint16_t *length);

USBD_Class_cb_TypeDef midiCallback = {
		usbd_midi_Init,
		usbd_midi_DeInit,
		usbd_midi_Setup,
		(void*) 0, /* EP0_TxSent */
		usbd_midi_EP0_RxReady,
		usbd_midi_DataIn,
		usbd_midi_DataOut,
		usbd_midi_SOF,
		(void*) 0,
		usbd_midi_OUT_Incplt,
		usbd_midi_GetCfgDesc,
		usbd_midi_GetCfgDesc
};



#define MIDI_CONFIG_DESC_SIZE 					103
#define USB_CONFIGURATION_DESCRIPTOR_TYPE       0x02
#define USB_INTERFACE_DESCRIPTOR_TYPE           0x04
#define USB_DEVICE_CLASS_AUDIO                  0x01
#define USB_DEVICE_SUBCLASS_MIDISTREAMING       0x03

/* USB AUDIO device Configuration Descriptor */
static uint8_t usbd_audio_CfgDesc[MIDI_CONFIG_DESC_SIZE] =
{
	/* Configuration 1 */
	0x09,                                 /* bLength */
	USB_CONFIGURATION_DESCRIPTOR_TYPE,    /* bDescriptorType */
	LOBYTE(MIDI_CONFIG_DESC_SIZE),        /* wTotalLength  86 bytes*/
	HIBYTE(MIDI_CONFIG_DESC_SIZE),
	0x02,                                 /* bNumInterfaces */
	0x01,                                 /* bConfigurationValue */
	0x00,                                 /* iConfiguration */
	0xC0,                                 /* bmAttributes  = b6 + b7 : self powered */
	0x32,                                 /* bMaxPower = 100 mA*/
	/* 09 byte*/

	// B.3.1 Standard AC Interface Descriptor
	/* Audio Control standard */
	0x9,         /* sizeof(usbDescrInterface): length of descriptor in bytes */
	0x04,   /* descriptor type */
	0x00,         /* index of this interface */
	0x00,         /* alternate setting for this interface */
	0x00,         /* endpoints excl 0: number of endpoint descriptors to follow */
	0x01,         /* AUDIO */
	0x01,         /* AUDIO_Control*/
	0x00,         /* */
	0x00,         /* string index for interface */

	// B.3.2 Class-specific AC Interface Descriptor
	/* AudioControl   Class-Specific descriptor */
	0x09,         /* sizeof(usbDescrCDC_HeaderFn): length of descriptor in bytes */
	0x24,         /* descriptor type */
	0x01,         /* header functional descriptor */
	0x0, 0x01,      /* bcdADC */
	0x09, 0x00,         /* wTotalLength */
	0x01,         /* */
	0x01,         /* */


	// B.4 MIDIStreaming Interface Descriptors

	// B.4.1 Standard MS Interface Descriptor
	/* PreenFM Standard interface descriptor */
	0x09,            /* bLength */
	0x04,        /* bDescriptorType */
	0x01,                                 /* bInterfaceNumber */
	0x00,                                 /* bAlternateSetting */
	0x02,                                 /* bNumEndpoints */
	USB_DEVICE_CLASS_AUDIO,               /* bInterfaceClass */
	USB_DEVICE_SUBCLASS_MIDISTREAMING,	/* bInterfaceSubClass : MIDIStreaming*/
	0x00,             					/* InterfaceProtocol: NOT USED */
	0x00,                                 /* iInterface : NOT USED*/
	/* 09 byte*/

	// B.4.2 Class-specific MS Interface Descriptor
	/* MS Class-Specific descriptor */
	0x07,         /* length of descriptor in bytes */
	0x24,         /* descriptor type */
	0x01,         /* header functional descriptor */
	0x0, 0x01,      /* bcdADC */
	37, 0,         /* wTotalLength : Why 0x41 ?? XH : don't know... */

	// B.4.3 MIDI IN Jack Descriptor
	// Midi in Jack Descriptor (Embedded)
	0x06,         /* bLength */
	0x24,         /* descriptor type */
	0x02,         /* MIDI_IN_JACK desc subtype */
	0x01,         /* EMBEDDED bJackType */
	0x01,         /* bJackID */
	0x00,         /* UNUSEED */

	// Midi in Jack Descriptor (External)
	0x06,         /* bLength */
	0x24,         /* descriptor type */
	0x02,         /* MIDI_IN_JACK desc subtype */
	0x02,         /* EXTERNAL bJackType */
	0x02,         /* bJackID */
	0x00,         /* UNUSEED */

	// Table B4.4
	// Midi Out Jack Descriptor (Embedded)
	0x09,         /* length of descriptor in bytes */
	0x24,         /* descriptor type */
	0x03,         /* MIDI_OUT_JACK descriptor */
	0x01,         /* EMBEDDED bJackType */
	0x03,         /* bJackID */
	0x01,         /* No of input pins */
	0x02,         /* BaSourceID  */
	0x01,         /* BaSourcePin  */
	0X00,         /* iJack : UNUSED */

	// Midi Out Jack Descriptor (External)
	0x09,         /* length of descriptor in bytes */
	0x24,         /* descriptor type */
	0x03,         /* MIDI_OUT_JACK descriptor */
	0x02,         /* EXTERNAL bJackType */
	0x04,         /* bJackID */
	0x01,         /* No of input pins */
	0x01,         /* BaSourceID  */
	0x01,         /* BaSourcePin  */
	0X00,         /* iJack : UNUSED */


	// ===== B.5 Bulk OUT Endpoint Descriptors
	//B.5.1 Standard Bulk OUT Endpoint Descriptor
	0x09,      /* bLenght */
	0x05,   	/* bDescriptorType = endpoint */
	0x01,      /* bEndpointAddress OUT endpoint number 1 */
	0x02,         /* bmAttributes: 2:Bulk, 3:Interrupt endpoint */
	0x40, 0X00,         /* wMaxPacketSize */
	0x00,         /* bIntervall in ms */
	0x00,         /* bRefresh */
	0x00,         /* bSyncAddress */

	// B.5.2 Class-specific MS Bulk OUT Endpoint Descriptor
	0x06,         /* bLength of descriptor in bytes */
	0x25,         /* bDescriptorType */
	0x01,         /* bDescriptorSubtype : GENERAL */
	0x02,         /* bNumEmbMIDIJack  */
	0x01,         /* baAssocJackID (0) */
	0x02,         /* baAssocJackID (0) */


	//B.6 Bulk IN Endpoint Descriptors
	//B.6.1 Standard Bulk IN Endpoint Descriptor
	0x09,         /* bLenght */
	0x05,   		/* bDescriptorType = endpoint */
	0x81,         /* bEndpointAddress IN endpoint number 1 */
	0X02,         /* bmAttributes: 2: Bulk, 3: Interrupt endpoint */
	0x40, 0X00,         /* wMaxPacketSize */
	0X00,         /* bIntervall in ms */
	0X00,         /* bRefresh */
	0X00,         /* bSyncAddress */

	// B.6.2 Class-specific MS Bulk IN Endpoint Descriptor
	0X06,         /* bLength of descriptor in bytes */
	0X25,         /* bDescriptorType */
	0x01,         /* bDescriptorSubtype */
	0X02,         /* bNumEmbMIDIJack (0) */
	0X03,         /* baAssocJackID (0) */
	0X04,         /* baAssocJackID (0) */

};



static uint8_t usbd_midi_Init(void *pdev, uint8_t cfgidx) {
	/* Open EP IN */
	DCD_EP_Open((USB_OTG_CORE_HANDLE *)pdev,
			  0x81,
			  0x40,
			  0x02);

	/* Open EP OUT */
	DCD_EP_Open((USB_OTG_CORE_HANDLE *)pdev,
			  0x1,
			  0x40,
			  0x02);

	// Prepare for next midi information
	DCD_EP_PrepareRx((USB_OTG_CORE_HANDLE *)pdev,
			0x1,
			(uint8_t*)midiBuff,
			4);


	return USBD_OK;
}

static uint8_t usbd_midi_DeInit(void *pdev, uint8_t cfgidx) {
	  /* Close MIDI EPs */
	  DCD_EP_Close ((USB_OTG_CORE_HANDLE *)pdev , 0x81);
	  DCD_EP_Close ((USB_OTG_CORE_HANDLE *)pdev , 0x1);

	  return USBD_OK;
}

static uint8_t usbd_midi_Setup(void *pdev, USB_SETUP_REQ *req) {
	return 0;
}
static uint8_t usbd_midi_EP0_RxReady(void *pdev) {
	return 0;
}
static uint8_t usbd_midi_DataIn(void *pdev, uint8_t epnum) {
	return 0;
}
static uint8_t usbd_midi_DataOut(void *pdev, uint8_t epnum) {

	if ((midiBuff[0] & 0xf) == 0x9 || (midiBuff[0] & 0xf) == 0x8) {
		midiNote = (midiBuff[0] >> 4)+1;
	}

	// Prepare for next midi information
	DCD_EP_PrepareRx((USB_OTG_CORE_HANDLE *)pdev,
			0x1,
			(uint8_t*)midiBuff,
			4);

	return USBD_OK;
}
static uint8_t usbd_midi_SOF(void *pdev) {
	return 0;
}
static uint8_t usbd_midi_OUT_Incplt(void *pdev) {
	return 0;
}
static uint8_t *usbd_midi_GetCfgDesc(uint8_t speed, uint16_t *length) {
	*length = sizeof (usbd_audio_CfgDesc);
	return usbd_audio_CfgDesc;
}


