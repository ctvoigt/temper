grabbyTEMPer
============    

A command line sensor logger for TEMPer1 devices

Uses pcsensor.c by Michitaka Ohno, Juan Carlos Perez and Robert Kavaler

Install intructions (Debian / Ubuntu)
-------------------------------------

Install libusb-0.1.4 and dev package, plus build-essential 

    sudo apt-get install build-essential libusb-0.1.4 libusb-dev

Compile

    make

Install

    sudo make install

Uninstall

    sudo make uninstall


Usage instructions
------------------

Run log.sh

    ./log.sh

This will log the temperature every 5 seconds to stdout as a CSV stream which 
you can pipe to a text file and open in your favourite spreadsheet package 
later. 


To allow non-root users access
------------------------------
*automatically*

    sudo make group
    sudo make rules-install
    sudo usermod -aG temper <USER>

Reboot to apply udev rules and group settings.
 
*manually*

1. Add the udev rule set in /etc/udev/rules.d/ using the 99-temper.rules 
2. Add a 'temper' group (using groupadd or edit the /etc/group file)
3. Add users to the 'temper' group
4. Reload the udev rules 
    * udevcontrol reload_rules or sudo udevadm trigger
5. Unplug and replug the TEMPer device
6. If groups are not updated or udev still cases trouble (Does work with root permission, but not with user permissions), reboot.

*alternativilly*

Use commented out lines from 99-temper.rules to allow all users reading the devices.


