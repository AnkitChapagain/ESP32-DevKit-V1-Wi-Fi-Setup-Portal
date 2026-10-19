# ESP32-DevKit-V1-Wi-Fi-Setup-Portal
A simple Arduino sketch for ESP32 DevKit V1 that creates a Wi-Fi setup hotspot on first boot, shows nearby Wi-Fi networks in a captive portal, saves credentials, and stops the setup hotspot after successful connection.
Features
- First boot creates a Wi-Fi access point named ESP32-Setup.
- Captive portal lets you select an available Wi-Fi network and enter its password.
- WiFiManager stores the selected credentials in ESP32 flash.
- Later boots reconnect using saved credentials.
- If saved Wi-Fi cannot be reached, the setup portal is shown again.
- Hold the reset button for 3 seconds to erase saved Wi-Fi settings and restart setup.
- A long press can also be detected during startup.
Hardware
- ESP32 DevKit V1
- USB data cable for the first upload
- One momentary push button
- Two jumper wires
Button wiring
Use an external button on GPIO 27:
- One button terminal -> GPIO 27
- Other button terminal -> GND
The code uses INPUT_PULLUP, so the button reads LOW when pressed. Do not connect GPIO 27 to 3.3 V or 5 V through the button.
Avoid using GPIO 0 (the BOOT button) for this purpose: holding it during reset can put the ESP32 into its serial bootloader.

Software setup
1. Install Arduino IDE.
2. In Tools -> Board -> Boards Manager, install esp32 by Espressif Systems.
3. In Sketch -> Include Library -> Manage Libraries, install WiFiManager by tzapu.
4. Open ESP32_WiFi_Setup.ino.
5. Select Tools -> Board -> esp32 -> ESP32 Dev Module.
6. Select the correct serial port under Tools -> Port.
7. Click Verify, then Upload.
8. Open Tools -> Serial Monitor and set baud rate to 115200.
First-time Wi-Fi setup
1. Power the ESP32.
2. On your phone or laptop, open Wi-Fi settings and connect to ESP32-Setup.
3. The phone may open the captive portal automatically.
4. If it does not, open a browser and visit http://192.168.4.1.
5. Select Configure WiFi (or the equivalent menu item).
6. Choose your Wi-Fi network, enter its password, and save.
7. Wait for the ESP32 to connect. The setup AP should disappear after a successful connection.
8. Check Serial Monitor for Wi-Fi connected and the device IP address.
The setup AP is open by default for ease of initial provisioning. To protect it with a password, set AP_PASSWORD in the sketch to a password of at least 8 characters. Then connect to the AP using that password.
Reset Wi-Fi settings
While the ESP32 is running, press and hold the external button for 3 seconds. The ESP32 clears the saved Wi-Fi credentials and restarts into setup mode.

If the ESP32 cannot connect to its saved Wi-Fi, you can also hold the button while powering on or resetting the board and keep holding it for 3 seconds. The sketch clears the saved credentials and then starts the setup portal.
How the portal behaves
- If no credentials are saved, the setup portal starts.
- If saved credentials connect successfully, the portal is not shown and the ESP32 joins the Wi-Fi network.
- If saved credentials fail to connect, WiFiManager opens the setup portal.
- The portal is configured with no timeout, so it stays available until setup succeeds or the device is restarted.
- The ESP32 stops its setup access point after successful connection.
Captive portal note
Phones and operating systems do not always automatically open captive portals. If the page does not open by itself, connect to ESP32-Setup and manually visit http://192.168.4.1. Temporarily disabling mobile data or choosing “stay connected” to the ESP32 setup network may help on some phones.
Serial monitor troubleshooting
- No ESP32-Setup network: Check power, upload success, and serial output.
- COM port not visible: Try a known data-capable USB cable and install the USB-to-serial driver required by your board.
- Portal page not opening: Visit http://192.168.4.1 manually.
- Wrong Wi-Fi password: The connection will fail; use the button reset procedure to reopen provisioning.
- Setup network remains visible: Wait for the connection attempt to finish and inspect Serial Monitor.
- Button does not work: Verify the button connects GPIO 27 to GND and is held for at least 3 seconds.
Notes
- This project only configures Wi-Fi. It does not include OTA firmware updates; OTA can be added after connectivity is working.
- WiFiManager is a third-party library. Its portal wording and captive portal behavior can vary slightly by library version.
- For unattended installations, consider adding authenticated provisioning, firmware signing, and secure OTA rollback before deployment.
