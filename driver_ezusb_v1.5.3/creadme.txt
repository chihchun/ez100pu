EZUSB(EZ100PU/EZMINI)版本1.5.3 PC/SC智慧卡讀卡機驅動程式安裝步驟:

   1. 執行環境檢測: ./check_env
      若顯示"PC/SC Daemon Not Ready", 請到http://www.linuxnet.com/middle.html
      下載安裝 pcsc-lite 套件
      若顯示"Error! USB Device File System Not Mounted",請依指示載入USB檔案系統

      注意: 若無法執行,請用"chmod 777 check_env"將該script改成執行檔
	
   2. 執行安裝程式 : ./install

      注意: 1. 若無法執行,請用"chmod 777 install"將該script改成執行檔
            2. 須有root權限才能正確安裝

   3. 重新開機
   
   4. 重新啟動後插入卡片, 若讀卡機燈號顯示為紅色, 表示安裝成功
   
   注意事項: 1. Linux Kernel 版本建議2.4以上
             2. 如果已安裝1.3.4以前版本之驅動程式, 請先移除再進行安裝
                要移除之前的版本: 
                       請移除在/etc/reader.conf內關於EZ100PU驅動程式之設定
             3. 此板驅動程式要求PCSCLITE必須使用LIBUSB, 若LIBHAL版之PCSCLIE,則驅
                動程式無法正確運作
