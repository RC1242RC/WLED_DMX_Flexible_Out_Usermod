#include "wled.h"
#include "dmx_functions.h"
#include "src/dependencies/dmx/SparkFunDMX.h"

/*
 * Usermods allow you to add own functionality to WLED without touching core source files.
 * See the WLED docs: https://kno.wled.ge/advanced/custom-features/
 *
 * This is an example usermod. It demonstrates:
 *   - persistent settings via addToConfig() / readFromConfig()
 *   - JSON state read/write via addToJsonState() / readFromJsonState()
 *   - MQTT subscribe and message handling (guarded by WLED_DISABLE_MQTT)
 *   - button event handling
 *   - the Usermod Settings page via appendConfigData()
 *
 * To create your own usermod:
 *   1. Click "Use this template" on https://github.com/wled/wled-usermod-example to create your own repo.
 *   2. Rename the class and file to something descriptive.
 *   3. Reference your new repo in platformio_override.ini via custom_usermods.
 *
 * REGISTER_USERMOD() at the bottom self-registers the instance — no other
 * file edits are needed.
 */

//class name. Use something descriptive and leave the ": public Usermod" part :)
class DMXFlexibleOutUsermod : public Usermod {

  private:

    // Helper variables
    bool enabled = true;
    unsigned long lastTime = 0;
    bool initDone = false;
    int j;
    int k;
    int m;
    int temp;
    char stringBuffer[50]; // This must be long enough so that strings are not cut off
    

    // DMX variables
    // Elements of groupMap have the possible values 0 (no group), 1, 2, 3, ... (group number)
    // Elements of valueMap have the possible values: 300 (red), 301 (green), 302 (blue), 303 (white), or 0-255 for specific values
    // Currently brightness is not handled separately
    // Gap between specific values is given by SpecificValueIncrement
    const int maxDMXGroups = 32; // This overrides the strip length variable if there are too many LEDs
    const int nDMXChannels = 64; 
    int nDMXGroups;
    const int RGBStart = 300; // If this is changed then the switch statement in dmx_functions.h must also be changed
    const int SpecificValueStart = 0;
    const int SpecificValueIncrement = 5;
    uint16_t groupMap[64]; // These need to match nDMXChannels
    uint16_t valueMap[64]; // If changed, the arguments in dmx_functions.h must also be changed

    // Strings that are used multiple time (this will save some flash memory)
    static const char _name[];
    static const char _enabled[];
    static const char _channelMap[];
    static const char _channel[];
    static const char _group[];
    static const char _value[];
    static const char _groupSettingsScript[];
    static const char _valueSettingsScript[];
    static const char _groupDropdown[];
    static const char _valueDropdownInt[];
    static const char _valueDropdownString[];
    static const char _red[];
    static const char _green[];
    static const char _blue[];
    static const char _white[];
    static const char _brightness[];


  public:

    SparkFunDMX dmx;
    // byte DMXChannels _INIT(7);        // number of channels per fixture
    // byte DMXFixtureMap[15] _INIT_N(({ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }));
    // assigns the different channels to different functions. See wled21_dmx.ino for more information.
    // uint16_t DMXGap _INIT(10);          // gap between the fixtures. makes addressing easier because you don't have to memorize odd numbers when climbing up onto a rig.
    // uint16_t DMXStart _INIT(10);        // start address of the first fixture
    // uint16_t DMXStartLED _INIT(0);      // LED from which DMX fixtures start


    /*
     * setup() is called once at boot. WiFi is not yet connected at this point.
     * readFromConfig() is called prior to setup()
     * You can use it to initialize variables, sensors or similar.
     */
    void setup() override {
      // do your set-up here
      //Serial.println("Hello from my usermod!");
      initDMXOutput(dmx);
      initDone = true;
    }

    static um_data_t* getAudioData() {
      um_data_t *um_data;
      if (!UsermodManager::getUMData(&um_data, USERMOD_ID_AUDIOREACTIVE)) {
        // add support for no audio
        um_data = simulateSound(SEGMENT.soundSim);
      }
      return um_data;
    }


    /*
     * loop() is called continuously. Here you can check for events, read sensors, etc.
     * 
     * Tips:
     * 1. You can use "if (WLED_CONNECTED)" to check for a successful network connection.
     *    Additionally, "if (WLED_MQTT_CONNECTED)" is available to check for a connection to an MQTT broker.
     * 
     * 2. Try to avoid using the delay() function. NEVER use delays longer than 10 milliseconds.
     *    Instead, use a timer check as shown here.
     */
    void loop() override {
      // if usermod is disabled or called during strip updating just exit
      // NOTE: on very long strips strip.isUpdating() may always return true so update accordingly
      if (!enabled || strip.isUpdating()) return;

      handleDMXOutput(nDMXChannels, nDMXGroups, SpecificValueStart, groupMap, valueMap, dmx);

      //uint8_t  *binNum = (uint8_t*)&SEGENV.aux1, *maxVol = (uint8_t*)(&SEGENV.aux1+1); // just in case assignment
      //bool      samplePeak = false;
      //float     FFT_MajorPeak = 1.0;
      //uint8_t  *fftResult = nullptr;
      //float    *fftBin = nullptr;
      um_data_t *um_data = getAudioData();
      float volumeSmth    = *(float*)   um_data->u_data[0];
      Serial.print(volumeSmth);
      Serial.print(" ");
      int volumeRaw     = *(int16_t*)   um_data->u_data[1];
      Serial.print(volumeRaw);
      Serial.print(" ");
      uint8_t *fftResult     =  (uint8_t*) um_data->u_data[2];
      Serial.print(*fftResult);
      Serial.print(" ");
      uint8_t samplePeak    = *(uint8_t*) um_data->u_data[3];
      Serial.print(samplePeak);
      Serial.print(" ");
      float FFT_MajorPeak = *(float*)   um_data->u_data[4];
      Serial.print(FFT_MajorPeak);
      Serial.print(" ");
      float my_magnitude  = *(float*)   um_data->u_data[5];
      Serial.print(my_magnitude);
      Serial.print(" ");
      uint8_t *maxVol        =  (uint8_t*) um_data->u_data[6];  // requires UI element (SEGMENT.customX?), changes source element
      Serial.print(*maxVol);
      Serial.print(" ");
      uint8_t *binNum        =  (uint8_t*) um_data->u_data[7];  // requires UI element (SEGMENT.customX?), changes source element
      Serial.print(*binNum);
      Serial.println();

      // do your magic here
      if (millis() - lastTime > 1000) {
        //Serial.println("I'm alive!");
        lastTime = millis();
      }
    }


    /*
     * addToConfig() saves settings to cfg.json under the "um" object. WLED calls this whenever settings are saved.
     * The Usermod Settings page in the UI is generated automatically from the keys you write here.
     *
     * Usermod Settings Overview:
     * - Numeric values are treated as floats in the browser.
     *   - If the numeric value entered into the browser contains a decimal point, it will be parsed as a C float
     *     before being returned to the Usermod.  The float data type has only 6-7 decimal digits of precision, and
     *     doubles are not supported, numbers will be rounded to the nearest float value when being parsed.
     *     The range accepted by the input field is +/- 1.175494351e-38 to +/- 3.402823466e+38.
     *   - If the numeric value entered into the browser doesn't contain a decimal point, it will be parsed as a
     *     C int32_t (range: -2147483648 to 2147483647) before being returned to the usermod.
     *     Overflows or underflows are truncated to the max/min value for an int32_t, and again truncated to the type
     *     used in the Usermod when reading the value from ArduinoJson.
     * - Pin values can be treated differently from an integer value by using the key name "pin"
     *   - "pin" can contain a single or array of integer values
     *   - On the Usermod Settings page there is simple checking for pin conflicts and warnings for special pins
     *     - Red color indicates a conflict.  Yellow color indicates a pin with a warning (e.g. an input-only pin)
     *   - Tip: use int8_t to store the pin value in the Usermod, so a -1 value (pin not set) can be used
     *
     * To force a config write from loop(), call serializeConfig() — but use it sparingly (flash wear,
     * possible LED stutter). Never call it from a network callback.
     */
    void addToConfig(JsonObject& root) override
    {
      JsonObject top = root.createNestedObject(FPSTR(_name));
      top[FPSTR(_enabled)] = enabled;

      JsonObject channelJSON = top.createNestedObject(FPSTR(_channelMap));
      //JsonArray channelArray = channelJSON.createNestedArray("channelArray");
      for (j=1; j<=nDMXChannels; j++) {
        snprintf_P(stringBuffer, sizeof(stringBuffer), _group, j);
        channelJSON[stringBuffer] = groupMap[j];
        snprintf_P(stringBuffer, sizeof(stringBuffer), _value, j);
        channelJSON[stringBuffer] = valueMap[j];
      }
    }


    /*
     * readFromConfig() is called before setup() and again after settings are saved.
     * Return false if any expected keys were missing — WLED will then call addToConfig() to write the defaults.
     * getJsonValue(src, dest) copies the value if present and returns true; leaves dest unchanged if missing.
     * getJsonValue(src, dest, default) also assigns a default when the key is absent.
     */
    bool readFromConfig(JsonObject& root) override
    {
      if (strip.getLengthTotal() > maxDMXGroups) {
        nDMXGroups = maxDMXGroups;
      } else {
        nDMXGroups = strip.getLengthTotal();
      }

      JsonObject top = root[FPSTR(_name)];
      JsonObject channelJSON = top[FPSTR(_channelMap)];

      bool configComplete = (!top.isNull()) || (!channelJSON.isNull());

      configComplete &= getJsonValue(top[FPSTR(_enabled)], enabled);
      for (j=1; j<=nDMXChannels; j++) {
        snprintf_P(stringBuffer, sizeof(stringBuffer), _group, j);
        configComplete &= getJsonValue(channelJSON[stringBuffer], groupMap[j], 0);
        snprintf_P(stringBuffer, sizeof(stringBuffer), _value, j);
        configComplete &= getJsonValue(channelJSON[stringBuffer], valueMap[j], 0);
      }

      return configComplete;
    }


    /*
     * appendConfigData() is called when the Usermod Settings page renders.
     * Write JavaScript snippets to settingsScript to add helper text or dropdowns for your config fields.
     * addInfo('<ModName>:<key>', 1, '<html>') adds a tooltip/label next to the field.
     * addDropdown / addOption replace a plain text input with a <select>.
     */
    void appendConfigData(Print& settingsScript) override
    {
      if (strip.getLengthTotal() > maxDMXGroups) {
        nDMXGroups = maxDMXGroups;
      } else {
        nDMXGroups = strip.getLengthTotal();
      }

      settingsScript.print(F("addInfo('")); settingsScript.print(FPSTR(_name)); settingsScript.print(F(":enabled',1,'<i>The number of available groups is defined by the number of LEDs specified in LED and Hardware settings</i>');"));

      for (j=1; j<=nDMXChannels; j++) {

        // Options for group dropdown
        snprintf_P(stringBuffer, sizeof(stringBuffer), _groupSettingsScript, j);
        settingsScript.print(F("cc=addDropdown('")); settingsScript.print(FPSTR(_name)); settingsScript.print(F("','")); settingsScript.print(FPSTR(_channelMap)); settingsScript.print(stringBuffer);
        settingsScript.print(F("addOption(cc,'No group',0);"));
        for (k=1; k<=nDMXGroups; k++) {
          snprintf_P(stringBuffer, sizeof(stringBuffer), _groupDropdown, k, k);
          settingsScript.print(stringBuffer);
        }

        // Options for value dropdown
        snprintf_P(stringBuffer, sizeof(stringBuffer), _valueSettingsScript, j);
        settingsScript.print(F("ee=addDropdown('")); settingsScript.print(FPSTR(_name)); settingsScript.print(F("','")); settingsScript.print(FPSTR(_channelMap)); settingsScript.print(stringBuffer);
        snprintf_P(stringBuffer, sizeof(stringBuffer), _valueDropdownInt, 0, SpecificValueStart);
        settingsScript.print(stringBuffer);
        snprintf_P(stringBuffer, sizeof(stringBuffer), _valueDropdownString, _red, RGBStart);
        settingsScript.print(stringBuffer);
        snprintf_P(stringBuffer, sizeof(stringBuffer), _valueDropdownString, _green, RGBStart+1);
        settingsScript.print(stringBuffer);
        snprintf_P(stringBuffer, sizeof(stringBuffer), _valueDropdownString, _blue, RGBStart+2);
        settingsScript.print(stringBuffer);
        snprintf_P(stringBuffer, sizeof(stringBuffer), _valueDropdownString, _white, RGBStart+3);
        settingsScript.print(stringBuffer);
        // snprintf_P(stringBuffer, sizeof(stringBuffer), _valueDropdownString, _brightness, RGBStart+4);
        // settingsScript.print(stringBuffer);
        for (m=(SpecificValueStart+SpecificValueIncrement); m<(SpecificValueStart+255); m = m + SpecificValueIncrement) {
          snprintf_P(stringBuffer, sizeof(stringBuffer), _valueDropdownInt, m, m);
          settingsScript.print(stringBuffer);
        }
        snprintf_P(stringBuffer, sizeof(stringBuffer), _valueDropdownInt, 255, SpecificValueStart+255);
        settingsScript.print(stringBuffer);
      }
    }



    /**
     * onStateChanged() is used to detect WLED state change
     * @mode parameter is CALL_MODE_... parameter used for notifications
     */
    void onStateChange(uint8_t mode) override {
      // do something if WLED state changed (color, brightness, effect, preset, etc)
    }

};

// add more strings here to reduce flash memory usage
const char DMXFlexibleOutUsermod::_name[]    PROGMEM = "DMX Out";
const char DMXFlexibleOutUsermod::_channelMap[]    PROGMEM = "Channel Map";
const char DMXFlexibleOutUsermod::_channel[]    PROGMEM = "Channel ";
const char DMXFlexibleOutUsermod::_group[]    PROGMEM = "Channel %d Group";
const char DMXFlexibleOutUsermod::_value[]    PROGMEM = "Channel %d Value";
const char DMXFlexibleOutUsermod::_groupSettingsScript[]    PROGMEM = ":Channel %d Group');";
const char DMXFlexibleOutUsermod::_valueSettingsScript[]    PROGMEM = ":Channel %d Value');";
const char DMXFlexibleOutUsermod::_groupDropdown[]    PROGMEM = "addOption(cc,'Group %d',%d);";
const char DMXFlexibleOutUsermod::_valueDropdownInt[]    PROGMEM = "addOption(ee,'%d',%d);";
const char DMXFlexibleOutUsermod::_valueDropdownString[]    PROGMEM = "addOption(ee,'%s',%d);";
const char DMXFlexibleOutUsermod::_red[]    PROGMEM = "Red";
const char DMXFlexibleOutUsermod::_green[]    PROGMEM = "Green";
const char DMXFlexibleOutUsermod::_blue[]    PROGMEM = "Blue";
const char DMXFlexibleOutUsermod::_white[]    PROGMEM = "White";
const char DMXFlexibleOutUsermod::_brightness[]    PROGMEM = "Brightness";
const char DMXFlexibleOutUsermod::_enabled[] PROGMEM = "enabled";


static DMXFlexibleOutUsermod flexible_out_usermod;
REGISTER_USERMOD(flexible_out_usermod);
