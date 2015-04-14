/*
 * pcsensor.c by Christian Voigt (c) 2014 (ctvoigt@verpeil.de) 
 * based on pcsensor.c by Michitaka Ohno (c) 2011 (elpeo@mars.dti.ne.jp)
 * based on pcsensor.c by Juan Carlos Perez (c) 2011 (cray@isp-sl.com)
 * based on Temper.c by Robert Kavaler (c) 2009 (relavak.com)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 * 
 * THIS SOFTWARE IS PROVIDED BY Juan Carlos Perez ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Robert kavaler BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

#include "pcsensor.h"
 
#define INTERFACE1 (0x00)
#define INTERFACE2 (0x01)
#define SUPPORTED_DEVICES (2)
#define MAX_DEV 8
#define MAX_PASSES_PER_SENSOR 4

/* Calibration adjustments */
/* See http://www.pitt-pladdy.com/blog/_20110824-191017_0100_TEMPer_under_Linux_perl_with_Cacti/ */
const static float scale = 1.0287;
const static float offset = -0.85;

/* further consts */
const static int reqIntLen=8;
const static int reqBulkLen=8;
const static int timeout=5000; /* timeout in ms */
const static unsigned short vendor_id[] = { 
	0x1130,
	0x0c45
};
const static unsigned short product_id[] = { 
	0x660c,
	0x7401
};

const static char uTemperatura[] = { 0x01, 0x80, 0x33, 0x01, 0x00, 0x00, 0x00, 0x00 };
const static char uIni1[] = { 0x01, 0x82, 0x77, 0x01, 0x00, 0x00, 0x00, 0x00 };
const static char uIni2[] = { 0x01, 0x86, 0xff, 0x01, 0x00, 0x00, 0x00, 0x00 };
const static char uCmd0[] = {    0,    0,    0,    0,    0,    0,    0,    0 };
const static char uCmd1[] = {   10,   11,   12,   13,    0,    0,    2,    0 };
const static char uCmd2[] = {   10,   11,   12,   13,    0,    0,    1,    0 };
const static char uCmd3[] = { 0x52,    0,    0,    0,    0,    0,    0,    0 };
const static char uCmd4[] = { 0x54,    0,    0,    0,    0,    0,    0,    0 };

/* global vars */
int temperDebugMode = 0;
int countHandles = 0;
static usb_dev_handle *handles[MAX_DEV];

static int device_type(usb_dev_handle *lvr_winusb){
	struct usb_device *dev;
	int i;
	dev = usb_device(lvr_winusb);
	for(i =0;i < SUPPORTED_DEVICES;i++){
		if (dev->descriptor.idVendor == vendor_id[i] && 
			dev->descriptor.idProduct == product_id[i] ) {
			return i;
		}
	}
	return -1;
}

static int usb_detach(usb_dev_handle *lvr_winusb, int iInterface) {
	int ret;
 
	ret = usb_detach_kernel_driver_np(lvr_winusb, iInterface);
	if(ret) {
		if(errno == ENODATA) {
			if(temperDebugMode) {
				printf("Device already detached\n");
			}
		} else {
			if(temperDebugMode) {
				printf("Detach failed: %s[%d]\n",
					strerror(errno), errno);
				printf("Continuing anyway\n");
			}
		}
	} else {
		if(temperDebugMode) {
			printf("detach successful\n");
		}
	}
	return ret;
} 

static int find_lvr_winusb() {
 
	struct usb_bus *bus;
	struct usb_device *dev;
	int i;

	memset(handles, 0, sizeof(handles)); 
	countHandles = 0;
	for (bus = usb_busses; bus; bus = bus->next) {
		for (dev = bus->devices; dev; dev = dev->next) {
			for(i =0;i < SUPPORTED_DEVICES;i++){
				if (dev->descriptor.idVendor == vendor_id[i] && 
					dev->descriptor.idProduct == product_id[i] ) {
					usb_dev_handle *handle;
					if(temperDebugMode) {
						printf("lvr_winusb with Vendor Id: %x and Product Id: %x found.\n", vendor_id[i], product_id[i]);
					}

					if (!(handle = usb_open(dev))) {
						if(temperDebugMode){
							printf("Could not open USB device\n");
						}
						break;
					}
					if (temperDebugMode)
						printf("\nDevice Filename: %s\n",dev->filename);
                                	handles[countHandles++] = handle;
					if (i == MAX_DEV)
						break;

				}
			}
		}
	}
	return countHandles;
}

static int setup_libusb_access() {
     if(!find_lvr_winusb()) {

		if(temperDebugMode){
			printf("Count of found USB Devices: %lu.\n", sizeof(handles));
		}
                printf("Couldn't find the USB device, Exiting\n");
                return 0;
        }
	return 1;
}
 
static int ini_control_transfer(usb_dev_handle *dev) {
	int r,i;

	char question[] = { 0x01,0x01 };

	r = usb_control_msg(dev, 0x21, 0x09, 0x0201, 0x00, (char *) question, 2, timeout);
	if( r < 0 )
	{
		if(temperDebugMode){
			printf("USB control write"); 
		}
		return -1;
	}


	if(temperDebugMode) {
		for (i=0;i<reqIntLen; i++) printf("%02x ",question[i] & 0xFF);
		printf("\n");
	}
	return 0;
}
 
static int control_transfer(usb_dev_handle *dev, const char *pquestion) {
	int r,i;

	char question[reqIntLen];
    
	memcpy(question, pquestion, sizeof question);

	r = usb_control_msg(dev, 0x21, 0x09, 0x0200, 0x01, (char *) question, reqIntLen, timeout);
	if( r < 0 )
	{
		if(temperDebugMode){
			printf("USB control write");
		}
		return -1;
	}

	if(temperDebugMode) {
		for (i=0;i<reqIntLen; i++) printf("%02x ",question[i]  & 0xFF);
		printf("\n");
	}
	return 0;
}

static int interrupt_read(usb_dev_handle *dev) {
 
	int r,i;
	char answer[reqIntLen];
	memset (answer,'0',reqIntLen);
	r = usb_interrupt_read(dev, 0x82, answer, reqIntLen, timeout);
	if( r != reqIntLen )
	{
		if(temperDebugMode){
			printf("USB interrupt read");
		}
		return -1;
	}

	if(temperDebugMode) {
		for (i=0;i<reqIntLen; i++) printf("%02x ",answer[i]  & 0xFF);
    
		printf("\n");
	}
	return 0;
}

static int interrupt_read_temperatura(usb_dev_handle *dev, float *tempC) {
 
	int r,i, temperature;
	char answer[reqIntLen];
	memset (answer,'0',reqIntLen);
	r = usb_interrupt_read(dev, 0x82, answer, reqIntLen, timeout);
	if( r != reqIntLen )
	{
		if(temperDebugMode){
			printf("USB interrupt read");
		}
		return -1;
	}


	if(temperDebugMode) {
		for (i=0;i<reqIntLen; i++) printf("%02x ",answer[i]  & 0xFF);
    
		printf("\n");
	}
    
	temperature = (answer[3] & 0xFF) + (answer[2] << 8);
	/* msb means the temperature is negative -- less than 0 Celsius -- and in 2'complement form.
	* We can't be sure that the host uses 2's complement to store negative numbers
	* so if the temperature is negative, we 'manually' get its magnitude
	* by explicity getting it's 2's complement and then we return the negative of that.
	*/

	 if ((answer[2] & 0x80)!=0) {
		 /* return the negative of magnitude of the temperature */
		 temperature = -((temperature ^ 0xffff)+1);
	 }
	*tempC = temperature * (125.0 / 32000.0);
	return 0;
}

static int get_data(usb_dev_handle *dev, char *buf, int len){
	return usb_control_msg(dev, 0xa1, 1, 0x300, 0x01, (char *)buf, len, timeout);
}

static int get_temperature(usb_dev_handle *dev, float *tempC){
	char buf[256];
	int ret, temperature, i;

	control_transfer(dev, uCmd1 );
	control_transfer(dev, uCmd4 );
	for(i = 0; i < 7; i++) {
		control_transfer(dev, uCmd0 );
	}
	control_transfer(dev, uCmd2 );
	ret = get_data(dev, buf, 256);
	if(ret < 2) {
		return -1;
	}

	temperature = (buf[1] & 0xFF) + (buf[0] << 8);	
	*tempC = temperature * (125.0 / 32000.0);
	return 0;
}

int pcsensor_open(usb_dev_handle* lvr_winusb){
	char buf[256];
	int i, ret;

	usb_detach(lvr_winusb, INTERFACE1);
        

	usb_detach(lvr_winusb, INTERFACE2);
        
 
	if (usb_set_configuration(lvr_winusb, 0x01) < 0) {
		if(temperDebugMode){
			printf("Could not set configuration 1\n");
		}
		return 0;
	}
 

	// Microdia tiene 2 interfaces
	if (usb_claim_interface(lvr_winusb, INTERFACE1) < 0) {
		if(temperDebugMode){
			printf("Could not claim interface\n");
		}
		return 0;
	}
 
	if (usb_claim_interface(lvr_winusb, INTERFACE2) < 0) {
		if(temperDebugMode){
			printf("Could not claim interface\n");
		}
		return 0;
	}
 




	switch(device_type(lvr_winusb)){
	case 0:
		control_transfer(lvr_winusb, uCmd1 );
		control_transfer(lvr_winusb, uCmd3 );
		control_transfer(lvr_winusb, uCmd2 );
		ret = get_data(lvr_winusb, buf, 256);
		if(temperDebugMode){	
			printf("Other Stuff (%d bytes):\n", ret);
			for(i = 0; i < ret; i++) {
				printf(" %02x", buf[i] & 0xFF);
				if(i % 16 == 15) {
					printf("\n");
				}
			}
			printf("\n");
		}
		break;
	case 1:
		if (ini_control_transfer(lvr_winusb) < 0) {
			fprintf(stderr, "Failed to ini_control_transfer (device_type 1)");
			return 0;
		}
      
		control_transfer(lvr_winusb, uTemperatura );
		interrupt_read(lvr_winusb);
 
		control_transfer(lvr_winusb, uIni1 );
		interrupt_read(lvr_winusb);
 
		control_transfer(lvr_winusb, uIni2 );
		interrupt_read(lvr_winusb);
		interrupt_read(lvr_winusb);
		break;
	}

	if(temperDebugMode){
		printf("device_type=%d\n", device_type(lvr_winusb));
	}
	return 1;
}

void pcsensor_close(usb_dev_handle* lvr_winusb){
	usb_release_interface(lvr_winusb, INTERFACE1);
	usb_release_interface(lvr_winusb, INTERFACE2);

	usb_close(lvr_winusb);
}

float pcsensor_get_temperature(usb_dev_handle* lvr_winusb){
	float tempc;
	int ret;
	switch(device_type(lvr_winusb)){
	case 0:
		ret = get_temperature(lvr_winusb, &tempc);
		break;
	case 1:
		control_transfer(lvr_winusb, uTemperatura );
		ret = interrupt_read_temperatura(lvr_winusb, &tempc);
		break;
	}
	if(ret < 0){
		return FLT_MIN;
	}
	return tempc;
}

float correct(float tempc) {
	return (tempc * scale) + offset;
}

void print_temp(float tempc, int sensorNumber) {
	struct tm *utc;
	time_t t;
	t = time(NULL);
	utc = gmtime(&t);
	
	char dt[80];
	strftime(dt, 80, "%d-%b-%Y %H:%M", utc);

	printf("Sensor%d,%s,%f\n", sensorNumber, dt, tempc);
	fflush(stdout);
}


int read_temper() {
	int passes,i,errorCode = 0;
	float tempc = 0.0000;

	if(temperDebugMode) {
		usb_set_debug(255);
	} else {
		usb_set_debug(0);
	}
	usb_init();
	usb_find_busses();
	usb_find_devices();


	if (!setup_libusb_access()) {
		if(temperDebugMode) {
			printf("No Device found, exit program with error.");
		}
		return 1; // No Device found.
	} 

	for (i=0;i<countHandles;i++) {
		do {
			usb_dev_handle* lvr_winusb = handles[i];
			pcsensor_open(lvr_winusb);

			if (!lvr_winusb) {
				// Open fails sometime, sleep and try again 
				sleep(3);
				pcsensor_close(lvr_winusb);
			}
			else {
				tempc = pcsensor_get_temperature(lvr_winusb);
				//pcsensor_close(lvr_winusb);
			}
			++passes;
		}
		/* Read fails silently with a 0.0 return, so repeat until not zero
		   or until we have read the same zero value 3 times (just in case
		   temp is really dead on zero */
		while ((tempc > -0.0001 && tempc < 0.0001) || passes >= MAX_PASSES_PER_SENSOR);

			if (!((tempc > -0.0001 && tempc < 0.0001) || passes >= MAX_PASSES_PER_SENSOR)) {
				print_temp(correct(tempc), i+1);			
			}
			else {
				if(temperDebugMode) {
					printf("Zero Read Error during reading device %d, tried MAX_PASSES_PER_SENSOR times", i);
				}
				errorCode = 2; // Zero Read Error.
			}
	}
	return errorCode;
}


