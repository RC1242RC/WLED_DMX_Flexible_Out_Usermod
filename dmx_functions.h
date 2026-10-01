#include "wled.h"
#include "src/dependencies/dmx/SparkFunDMX.h"

/*
 * Support for DMX  output via serial (e.g. MAX485).
 * Change the output pin in src/dependencies/ESPDMX.cpp, if needed (ESP8266)
 * Change the output pin in src/dependencies/SparkFunDMX.cpp, if needed (ESP32)
 * ESP8266 Library from:
 * https://github.com/Rickgg/ESP-Dmx
 * ESP32 Library from:
 * https://github.com/sparkfun/SparkFunDMX
 */

void handleDMXOutput(int nDMXChannels, int nDMXGroups, int SpecificValueStart, uint16_t groupMap[64], uint16_t valueMap[64], SparkFunDMX& dmx)
{
  uint8_t brightness = strip.getBrightness();

  for (int DMXAddr = 0; DMXAddr < nDMXChannels; DMXAddr++) {   

    uint32_t in = strip.getPixelColor(groupMap[DMXAddr]);     // get the colors for the individual fixtures as suggested by Aircoookie in issue #462
    
    switch (valueMap[DMXAddr]) {
      case (300):        // Red
        dmx.write(DMXAddr, (R(in) * brightness) / 255);
        break;
      case (301):        // Green
        dmx.write(DMXAddr, (G(in) * brightness) / 255);
        break;
      case (302):        // Blue
        dmx.write(DMXAddr, (B(in) * brightness) / 255);
        break;
      case (303):        // White
        dmx.write(DMXAddr, (W(in) * brightness) / 255);
        break;
      default:
        dmx.write(DMXAddr, (valueMap[DMXAddr] - SpecificValueStart));
    }
  }

  dmx.update();        // update the DMX bus
}

void initDMXOutput(SparkFunDMX& dmx) {
  dmx.initWrite(512);  // initialize with bus length
}
