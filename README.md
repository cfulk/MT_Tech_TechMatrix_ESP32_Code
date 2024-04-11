<h1>MT Tech TechMatrix ESP32 Code</h1>
<p>
  This is the Arduino/C++ program that sits on the ESP32C3 that allows a user to interface with a TechMatrix board using either BLE data from the companion app or the CLI menu system from a PC UART connection.
</p>
<h2>How to Use</h2>
<ol>
  <li>
    Install Arduino IDE and configure it for programming the SEEED Studio ESP32C3. Instructions for this can be found <a href="https://github.com/Seeed-Studio/wiki-documents/blob/docusaurus-version/docs/Sensor/SeeedStudio_XIAO/SeeedStudio_XIAO_ESP32C3/XIAO_ESP32C3_Getting_Started.md">here</a>.
  </li>
  <li>
    Download "ESP32_Main_Code.ino" and open it with Arduino IDE.
  </li>
  <li>
    Connect the MT Tech ESP32 adapter module to the PC running Arduino IDE via a USB-C cable capable of transferring data.
  </li>
  <li>
    Press the "Upload" button in Arduino IDE.
  </li>
  <li>
    Once the code is compiled and uploaded, the adapter module can be connected to a TechMatrix board, and it can be interfaced with via the companion app over Bluetooth or via serial monitoring software, like PuTTY, over the USB-C connection. The USB-C cable can now be disconnected, as long as the adapter module has an alternate power source.
  </li>
</ol>
