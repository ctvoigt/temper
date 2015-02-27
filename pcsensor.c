/*
 * pcsensor.c by Christian Voigt (c) 2014 (ctvoigt@verpeil.de) 
 * pcsensor.c by Michitaka Ohno (c) 2011 (elpeo@mars.dti.ne.jp)
 * based oc pcsensor.c by Juan Carlos Perez (c) 2011 (cray@isp-sl.com)
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

#define MAX_DEV 4

const static unsigned short vendor_id[] = { 
	0x1130,
	0x0c45
};
const static unsigned short product_id[] = { 
	0x660c,
	0x7401
};

#include <usb.h>
#include <stdio.h>
#include <time.h>

#include <string.h>
#include <errno.h>
#include <signal.h> 
 
 
#define VERSION "1.0.0"
 
#define VENDOR_ID  0x0c45
#define PRODUCT_ID 0x7401
 
#define INTERFACE1 0x00
#define INTERFACE2 0x01


 
const static int reqIntLen=8;
const static int reqBulkLen=8;
const static int endpoint_Int_in=0x82; /* endpoint 0x81 address for IN */
const static int endpoint_Int_out=0x00; /* endpoint 1 address for OUT */
const static int endpoint_Bulk_in=0x82; /* endpoint 0x81 address for IN */
const static int endpoint_Bulk_out=0x00; /* endpoint 1 address for OUT */
const static int timeout=5000; /* timeout in ms */
 
const static char uTemperatura[] = { 0x01, 0x80, 0x33, 0x01, 0x00, 0x00, 0x00, 0x00 };
const static char uIni1[] = { 0x01, 0x82, 0x77, 0x01, 0x00, 0x00, 0x00, 0x00 };
const static char uIni2[] = { 0x01, 0x86, 0xff, 0x01, 0x00, 0x00, 0x00, 0x00 };

static int bsalir=1;
static int debug=0;
static int seconds=5;
static int formato=0;
static int mrtg=0;
static int calibration=0;

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

void bad(const char *why) {
        fprintf(stderr,"Fatal error> %s\n",why);
        exit(17);
}

static int usb_detach(usb_dev_handle *lvr_winusb, int iInterface) {
	int ret;
 
	ret = usb_detach_kernel_driver_np(lvr_winusb, iInterface);
	if(ret) {
		if(errno == ENODATA) {
			if(debug) {
				printf("Device already detached\n");
			}
		} else {
			if(debug) {
				printf("Detach failed: %s[%d]\n",
					strerror(errno), errno);
				printf("Continuing anyway\n");
			}
		}
	} else {
		if(debug) {
			printf("detach successful\n");
		}
	}
	return ret;
}

static usb_dev_handle* setup_libusb_access() {
     usb_dev_handle *lvr_winusb[MAX_DEV];
     int i = 0;

	if(debug) {
		usb_set_debug(255);
	} else {
		usb_set_debug(0);
	}
	usb_init();
	usb_find_busses();
	usb_find_devices();

 
	if(!find_lvr_winusb()) {
		if(debug){
			printf("Couldn't find the USB device, Exiting\n");
		}
		return NULL;
	}
        
       
     for (i = 0; handles[i] != NULL && i < MAX_DEV; i++) {
            usb_detach(handles[i], INTERFACE1);
        

            usb_detach(handles[i], INTERFACE2);
        
 
	if (usb_set_configuration(handles[i], 0x01) < 0) {
		if(debug){
			printf("Could not set configuration 1\n");
		}
		return NULL;
	}
 

	// Microdia tiene 2 interfaces
	if (usb_claim_interface(handles[i], INTERFACE1) < 0) {
		if(debug){
			printf("Could not claim interface\n");
		}
		return NULL;
	}
 
	if (usb_claim_interface(handles[i], INTERFACE2) < 0) {
		if(debug){
			printf("Could not claim interface\n");
		}
		return NULL;
	}
 	}
	return handles[i];
}
 
int find_lvr_winusb() {
 
    struct usb_bus *bus;
    struct usb_device *dev;
	int i;

	memset(handles, 0, sizeof(handles)); 
	i = 0;
        for (bus = usb_busses; bus; bus = bus->next) {
        for (dev = bus->devices; dev; dev = dev->next) {
                        if (dev->descriptor.idVendor == VENDOR_ID && 
                                dev->descriptor.idProduct == PRODUCT_ID ) {
                                usb_dev_handle *handle;
                                if(debug) {
                                  printf("lvr_winusb with Vendor Id: %x and Product Id: %x found.\n", VENDOR_ID, PRODUCT_ID);
                                }
								
                                if (!(handle = usb_open(dev))) {
                                        printf("Could not open USB device\n");
                                        continue;
                                }

								printf("",printf("\nDevice Filename: \n",dev->filename));
                                handles[i++] = handle;
				if (i == MAX_DEV)
					break;
                        }
                }
        }
        return i;
}


static int ini_control_transfer(usb_dev_handle *dev) {
	int r,i;

	memset(handles, 0, sizeof(handles)); 
	i = 0;
        for (bus = usb_busses; bus; bus = bus->next) {
        for (dev = bus->devices; dev; dev = dev->next) {
                        if (dev->descriptor.idVendor == VENDOR_ID && 
                                dev->descriptor.idProduct == PRODUCT_ID ) {
                                usb_dev_handle *handle;
                                if(debug) {
                                  printf("lvr_winusb with Vendor Id: %x and Product Id: %x found.\n", VENDOR_ID, PRODUCT_ID);
                                }
								
                                if (!(handle = usb_open(dev))) {
                                        printf("Could not open USB device\n");
                                        continue;
                                }

	r = usb_control_msg(dev, 0x21, 0x09, 0x0201, 0x00, (char *) question, 2, timeout);
	if( r < 0 )
	{
		if(debug){
			printf("USB control write"); 
		}
		return -1;
	}


	if(debug) {
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
		if(debug){
			printf("USB control write");
		}
		return -1;
	}

	if(debug) {
		for (i=0;i<reqIntLen; i++) printf("%02x ",question[i]  & 0xFF);
		printf("\n");
	}
	return 0;
}

static int interrupt_read(usb_dev_handle *dev) {
 
    int r,i;
    char answer[reqIntLen];
    char question[reqIntLen];
    for (i=0;i<reqIntLen; i++) question[i]=i;
    r = usb_interrupt_write(dev, endpoint_Int_out, question, reqIntLen, timeout);
    if( r < 0 )
    {
          perror("USB interrupt write"); bad("USB write failed"); 
    }
    r = usb_interrupt_read(dev, endpoint_Int_in, answer, reqIntLen, timeout);
    if( r != reqIntLen )
    {
          perror("USB interrupt read"); bad("USB read failed"); 
    }

    if(debug) {
       for (i=0;i<reqIntLen; i++) printf("%i, %i, \n",question[i],answer[i]);
    }
 
    usb_release_interface(dev, 0);
}

void interrupt_read(usb_dev_handle *dev) {
 
    int r,i;
    unsigned char answer[reqIntLen];
    bzero(answer, reqIntLen);
    
	r = usb_interrupt_read(dev, 0x82, answer, reqIntLen, timeout);
	if( r != reqIntLen )
	{
		if(debug){
			printf("USB interrupt read");
		}
		return -1;
	}

	if(debug) {
		for (i=0;i<reqIntLen; i++) printf("%02x ",answer[i]  & 0xFF);
    
		printf("\n");
	}
	return 0;
}

static int interrupt_read_temperatura(usb_dev_handle *dev, float *tempC) {
 
	int r,i, temperature;
	char answer[reqIntLen];
	bzero(answer, reqIntLen);
    
	r = usb_interrupt_read(dev, 0x82, answer, reqIntLen, timeout);
	if( r != reqIntLen )
	{
		if(debug){
			printf("USB interrupt read");
		}
		return -1;
	}


	if(debug) {
		for (i=0;i<reqIntLen; i++) printf("%02x ",answer[i]  & 0xFF);
    
		printf("\n");
	}
    
	temperature = (answer[3] & 0xFF) + (answer[2] << 8);
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

void ex_program(int sig) {
      bsalir=1;
 
      (void) signal(SIGINT, SIG_DFL);
}
 
int main( int argc, char **argv) {
 
     float tempc;
     int c, i;
     struct tm *local;
     time_t t;

     memset(handles, 0, sizeof(handles));
     while ((c = getopt (argc, argv, "mfcwvhl::a:")) != -1)
     switch (c)
       {
       case 'v':
         debug = 1;
         break;
       case 'c':
         formato=1; //Celsius
         break;
       case 'f':
         formato=2; //Fahrenheit
         break;
       case 'w':
         formato=3; //Web in Celsius
         break;
       case 'm':
         mrtg=1;
         break;
       case 'l':
         if (optarg!=NULL){
           if (!sscanf(optarg,"%i",&seconds)==1) {
             fprintf (stderr, "Error: '%s' is not numeric.\n", optarg);
             exit(EXIT_FAILURE);
           } else {           
              bsalir = 0;
              break;
           }
         } else {
           bsalir = 0;
           seconds = 5;
           break;
         }
       case 'a':
         if (!sscanf(optarg,"%i",&calibration)==1) {
             fprintf (stderr, "Error: '%s' is not numeric.\n", optarg);
             exit(EXIT_FAILURE);
         } else {           
              break;
         }
       case '?':
       case 'h':
         printf("pcsensor version %s\n",VERSION);
	 printf("      Aviable options:\n");
	 printf("          -h help\n");
	 printf("          -v verbose\n");
	 printf("          -l[n] loop every 'n' seconds, default value is 5s\n");
	 printf("          -c output only in Celsius\n");
	 printf("          -f output only in Fahrenheit\n");
	 printf("          -a[n] increase or decrease temperature in 'n' degrees for device calibration\n");
	 printf("          -m output for mrtg integration\n");
	 printf("          -w output for cgi integration\n");
  
	 exit(EXIT_FAILURE);
       default:
         if (isprint (optopt))
           fprintf (stderr, "Unknown option `-%c'.\n", optopt);
         else
           fprintf (stderr,
                    "Unknown option character `\\x%x'.\n",
                    optopt);
         exit(EXIT_FAILURE);
       }

     if (optind < argc) {
        fprintf(stderr, "Non-option ARGV-elements, try -h for help.\n");
        exit(EXIT_FAILURE);
     }
 
     if (setup_libusb_access() == 0) {
         exit(EXIT_FAILURE);
     } 

     (void) signal(SIGINT, ex_program);

     for (i = 0; handles[i] != NULL && i < MAX_DEV; i++) {
	     ini_control_transfer(handles[i]);
      
	     control_transfer(handles[i], uTemperatura );
	     interrupt_read(handles[i]);
 
	     control_transfer(handles[i], uIni1 );
	     interrupt_read(handles[i]);
 
	     control_transfer(handles[i], uIni2 );
	     interrupt_read(handles[i]);
	     interrupt_read(handles[i]);
	}
 
     do {
     	for (i = 0; handles[i] != NULL && i < MAX_DEV; i++) {
           control_transfer(handles[i], uTemperatura );
           interrupt_read_temperatura(handles[i], &tempc);

           t = time(NULL);
           local = localtime(&t);
	   
	   if (formato==3) {
		printf("\n%.2f\n", tempc);
		exit(0);
	   }

           if (mrtg) {
              if (formato==2) {
                  printf("%.2f\n", (9.0 / 5.0 * tempc + 32.0));
                  printf("%.2f\n", (9.0 / 5.0 * tempc + 32.0));
              } else {
                  printf("%.2f\n", tempc);
                  printf("%.2f\n", tempc);
              }
              
              printf("%02d:%02d\n", 
                          local->tm_hour,
                          local->tm_min);

              printf("pcsensor\n");
           } else {
              printf("%04d/%02d/%02d %02d:%02d:%02d ", 
                          local->tm_year +1900, 
                          local->tm_mon + 1, 
                          local->tm_mday,
                          local->tm_hour,
                          local->tm_min,
                          local->tm_sec);

              if (formato==2) {
                  printf("Temperature %.2fF\n", (9.0 / 5.0 * tempc + 32.0));
              } else if (formato==1) {
                  printf("Temperature %.2fC\n", tempc);
              } else {
                  printf("Temperature %.2fF %.2fC\n", (9.0 / 5.0 * tempc + 32.0), tempc);
              }
           }
           
           if (!bsalir)
              sleep(seconds);
	}
     } while (!bsalir);
                                       
    for (i = 0; handles[i] != NULL && i < MAX_DEV; i++) {
     usb_release_interface(handles[i], INTERFACE1);
     usb_release_interface(handles[i], INTERFACE2);
     
     usb_close(handles[i]); 
    }
      
     return 0; 
}

