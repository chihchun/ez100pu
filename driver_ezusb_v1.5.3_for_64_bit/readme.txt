EZUSB(EZ100PU/EZMINI) Driver v1.5.3(for 64-bit) Installation guide for Linux:

   For installing EZUSB PC/SC driver, please follow these steps -

   1. Execute enviroment check program: ./check_env
      If the screen shows "PC/SC Daemon Not Ready", please goto the website 
      http://www.linuxnet.com/middle.html to download and install the pcsclite package.
      If the scrren shows "Error! USB Device File System Not Mounted", please mount USB
      file system according to the instruction.
	
      Notes: If the script cannot execute, please use "chmod 777 check_env" to change it 
             to executable file.

   2. Execute installation program : ./install      

      Notes: 1. If the script cannot execute, please use "chmod 777 install" to change it 
                to executable file.
             2. You need to have root privilege.
      
   3. Reboot the system.

   4. After reboot, insert a card. If the led of the reader turns red, the installation 
      of EZUSB driver is successful.

   Notice: 1. The recommaned version of kernel is 2.4 or higher.
           2. If you have installed EZUSB driver v1.3.4 or lower, please reinstall it first 
              before executing "install" program.
	      To uninstall v1.3.4 or lower, remove the settings of EZUSB driver in the 
              /etc/reader.conf.
           3. The driver requires PCSCLITE with being building with libusb. If PCSCLITE built 
              with libhal, the driver will not work.

