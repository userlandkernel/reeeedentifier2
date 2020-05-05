# reeeedentifier2
ABN AMRO e.dentifier2 can do internet banking, but can it also play pong? lets find out

## Writeup

### The device
The ABN AMRO e.dentifier2 is a device used for internet banking.  
It is a USB smartcard reader device but can be used without USB as well.  

### Under the skin
To open the device you need a Phillips screwdriver.  
The two Phillips screws on the bottom of the back are hidden beneath the black stickers.  
First notable thing on the board are the two 3V Panasconic batteries for powering the device when not on USB.  
It appears the device is always turned on and has an interrupt listener for the smartcard reader.  
The interrupt handler fires when the reader (chip in the middle with the 6 golden pins) detects a smartcard.
It will turn on the display and show the preprogrammed main menu.

### Sniffing glue
USB is by default not a secure protocol for communication.  
To securely communicate over USB there must be some kind of TLS implementation in a way that the device is the authority and the host is the client.  
Client should exchange a pairing request (like a certificate signing request) that is signed by ABN AMRO and the device should then require the user to either accept or deny the request.  
After that relatively secure communication would be possible.
However this is not the case with the ABN e.dentifier2.

The e.dentifier2 has been mentioned as being vulnerable to man-in-the middle attacks in other research.  
Considering the user is in a trusted environment and on a trusted personal non-shared machine this would not be a major issue.
Malware can still benefit from this issue so it would be better if the bank revises their future devices.  

From the customer provided documentation of ABN AMRO for the device could be learned that the bank provides a driver working on both macOS and Windows platforms.  
Drivers implement code to control devices and therefore I decided to download and reverse engineer the software in the hope to discover the protocol used for communication over USB.  


### Unpacking goods
I decided to go for the driver for macOS because the USB API's are pretty well designed on macOS and very easy to read.  
Chances were higher that macOS code would use ANSI C api's instead of C++ making it easier to reverse engineer.  
The macOS driver came in an installer, a .pkg file.  
To unpack these files on linux you need to use the XAR (eXtensible archiver) and cpio.  

(WIP)
