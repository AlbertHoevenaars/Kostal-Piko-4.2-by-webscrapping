# Kostal-Piko-4.2-by-webscrapping
Values read from the Kostal Piko inverter are read from the website and available as CSV, Json and mqtt values.

Functions list:
*	Created for and only tested on a KostaPiko 4.2 Inverter
*	Create at first start an Access Point for Wifi connection
*	Let user connect device to their own home network. With static or DHCP IP number
*	Read time from NTP server.
*	Holds webserver with webpage to control and view status of the KostalPiko Reader
*	Connect to MQTT to publish values
*	Access to KostalPiko Reader is protected by a username and password.
*	Reads KostalPiko values from website of KostalPiko inverter
*	Webpage with KostalPiko values (all readonly)
*	Webpage for setting wifi credentials
*	Webpage for KostalPiko device credentials
*	Webpage for setting MQTT credentials
*	Use LED in flashing mode for special functions.
*	Storage of KostalPiko device values in flash memory
*	Overview of what is in onboard memory
*	Storage for 128 days
*	Use pushbutton for full reset 
*	Read KostalPiko values via CSV or JSON file
*	Status of KostalPiko
*	Programming via RS232 or OTA (password protected)

Keep in mind that I am not a software programmer. I am a hardware designer mainly designing FPGAs by means of writing VHDL.
So, there will be many parts to improve. At least this design is working
