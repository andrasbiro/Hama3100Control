# Hama 3100 fast forward/rewind

The Hama 3100 is a gread radio/network player, but it doesn't have rewind/fast forward control on it - that's ususally not needed, or at the worst case, you can use the mobile app, but I'm mostly using it to listen to audiobooks from my NAS, where each chapter can be up the an hour long, and to get my phone, launch the app and so on simply takes too much time.

Instead, this application was designed for a Wemos board, with a prototyping shield and a rotary encoder on it. The rotary encoder fast forwards/rewinds, and the integrated button reconnects to the radio. It also has OTA enabled.

I'm using it for the Hama 3100, but it should work with any frontier silicon based device.

The project is for platformIO, but it should also work in Arduino: just copy the contents of main.cpp into an ino file, and install the dependencies:

Dependences:
- ESP8266 Arduiono support: https://github.com/esp8266/Arduino
- Encoder: https://github.com/PaulStoffregen/Encoder
- WifiManager: https://github.com/tzapu/WiFiManager