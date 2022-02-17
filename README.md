# attiny_brushed_esc
Brushed motor driver with Digispark clone and driver boards

### Parts
* [Digispark ATTiny clone](https://digistump.com/wiki/digispark/quickref) [Taobao](https://item.taobao.com/item.htm?id=528589975290)
* ["36V 360W" full bridge board](http://akelc.com/DCMotorHBridge/show_34.html) [Taobao](https://item.taobao.com/item.htm?id=20509935647)
* Wires with XH terminations for signal
* Thick enough wires for power

I have derated the board down to 24V and 250W. According to the seller the board does not require heatsinking if the load is under 12A. Between 12 and 15A some "simple heatsinking" is needed. Between 15 and 20A it needs "beefy heatsinking".

### Connection
Digispark P0 - Full bridge "IN1" (Full bridge forward)

Digispark P1 - Full bridge "PWM" and LED on Digispark (Full bridge PWM)

Digispark P2 - Full bridge "IN2" (Full bridge backward)

Digispark P3 - Not connected (Got some pull ups or downs on Digispark board for USB which is unused in this project)

Digispark P4 - External servo signal input (1100us to 1900us)

Digispark P5 - Not connected (Reset pin on Digispark)

### Programming the Digispark
The Digispark is best programmed with SPI and an external programmer. I used ArduinoISP which is bundled with the Arduino IDE.
1. Install Digispark board support in Arduino IDE. Board manager URL is "http://digistump.com/package_digistump_index.json".
2. Set board type to "Digispark (Default - 16.5 MHz)".
3. Connect the Digispark to the programmer as follows:
| Digispark pin name | Pin desc | Signal name | ArduinoISP pin |
| --- | --- | --- | --- |
| D0 | DI | MOSI | 11 |
| D1 | DO | MISO | 12 |
| D2 | SCK | SCK | 13 |
| D5 | RESET | | 10 (Default) |
| GND | GND| | Any GND |
| 5V | 5V | | Any 5V |
Lol fuck it check the source text instead of the rendered markdown thanks

4. Burn "digispark_pwm_controller.ino" with the "Upload with programmer" option

My Digispark clone does not have special fuse settings so reset works. 

### Customizing
PWM frequency, servo input range, deadzone and other settings can be changed in the Arduino sketch. It takes only a few minutes to understand it so have fun.
