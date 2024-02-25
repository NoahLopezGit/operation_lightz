## Project Overview
Controlling a 3rd party IR remote controlled LED through the Phillips-Hue lighting system

Demos:
- [On/Off control demo](https://www.youtube.com/shorts/3DD_msGkm7w?feature=share)
- [Color control demo](https://www.youtube.com/shorts/cillKHZvEkQ?feature=share)

Hardware Needeed:
- [ESP8266 Microcrontroller](https://www.amazon.com/HiLetgo-Internet-Development-Wireless-Micropython/dp/B081CSJV2V/ref=sr_1_3?crid=2FN80O4DEJ35Q&dib=eyJ2IjoiMSJ9.4xEr54v6vHUSws_RbogXUUQ0-qx0V7rAto5k-V99Mo0B0nICbDVzAQhl7oN_vez6h6I_iQWVLfWSQE1FCVuprA5YJ1QEEvuTtAwUl-Pv1QlIUN76l8YZnHXtN_q3rcoP7MQUSQHvrSm5hWsLkkhQXWyyjSHtN6FbixxEHky-_DTnwwE4OpbVHigxTFzypAsI_txCKg245Xo4zAAZbrvLMWYMtuxcqqW9dJRCkVbgO3s.iHiyCP1_ftZqlOarHK6rW__RVd4USzGyAUK-tuX9USc&dib_tag=se&keywords=esp8266&qid=1708795372&sprefix=esp826%2Caps%2C241&sr=8-3)
- [Arduino Compatible IR Transmitter](https://www.amazon.com/Digital-Receiver-Transmitter-Electronic-Building/dp/B08X2MFS6S/ref=sr_1_3?crid=LV2HYJJ67B2B&dib=eyJ2IjoiMSJ9._vf72KYZ3KT9icycUs9mdM0CsoL-vlhHUhWBduX01chbdRf9ogw1yb3ScaPJKxriGIoUuo9_0lG49d2js_uIMINOJ3cXmQ7Jv-eysLc-SBEYQwJJ2mn_LoATZjl5XkzK-37B2MUVydVT8sQhzrY_6B4ak7v9PqmwDMIVparTQFXOoewNgNVv1AIhPWM5GN-xfqfgi74LaiLus1CnnDIEduFiifhG1LYH8rDXCy72Bg8.Eq7Ml8uMY73ev3FV3fXqfBEB8Ew0eEBwu8JQwosaKzA&dib_tag=se&keywords=ir+transmitter+arduino&qid=1708795775&sprefix=ir+transmitter+arduin+o%2Caps%2C187&sr=8-3)

Concept of operations:
1. The ESP8266 is used to connect to Philips-Hue's existing lighting API.
   1. The ESP8266 uses the HTTPS connection to establish the 'eventstream' endpoint. Any lighting changes are updated through this event stream
2. The event stream is parsed for changes to a specific light 
   1. This will need to be an existing smart light in the hue system; The IR remote controlled LED is synchronized with this light in the hue system.
3. Changes like on/off state and color are then converted to their corresponding IR command and sent through the IR transmitter module.

This project is working with this [Levitating Moon Lamp](https://www.amazon.com/VGAzer-Levitating-Floating-Spinning-Printing/dp/B07CR2JYPH/ref=sr_1_7?crid=3GX85ZF9NJFY5&dib=eyJ2IjoiMSJ9.Xn7z8MvuMuKHdWTVc01bqDk6qeGk6q3t2jc2lKmJgvNPJDdJbwNo5Wd_wTgc0rj-eTg7f8K0xyGAuzkzToHNE6rdS4E5WU8yvd1s4K2fPcjvmEJowMOmqKtgY8wjbjjUaCRZgjgu05ICUPWctfe8N9gs2oKWsY--IUp4kIQdsvirCf36yZ7B7CdCKoaiIm90TvYdw7rTX1grbETf3302U3O-K-C_dYDdHkDh5p_qTTIRexIgwcxvOUetipo6wAFYn4rVa0yH6CbapVVR0KCfPrbdxRIEGH8BHqnJUz-nSOY.R6E22jOTyq1MgoRUMVWY0s4bHgMr_b9oUrgh_gqicnc&dib_tag=se&keywords=levitating+moon+lamp&qid=1708794988&sprefix=levitating+m%2Caps%2C179&sr=8-7). Many other IR controlled LEDs use similar control schemes so controlling them is just a matter of figuring out the specific IR protocol and command patterns.
![Levitating Moon Lamp|500](https://m.media-amazon.com/images/I/61rLaPELHJL._AC_SX679_.jpg)

# Setting up the event stream from the Phillips-Hue API
First, you need to connect to the [hue bridge](https://www.amazon.com/Philips-Hue-Smart-Google-Assistant/dp/B016H0QZ7I/ref=sr_1_1?crid=2UVHT1W5TJJI6&dib=eyJ2IjoiMSJ9.6i4ofTweJSbH-34Kwnx3BefcnJiy8GGDpsduZGs3uSJvn4pbmIHII-4k7hXLDps6zCyrM3lxaAWwggefoNw6oWu5US4sAY01cUmpikWtZ17A_5xj3hGUAiclp_eYNEyL4OKrJtfLTsKClCnTZlU9LPMESXPRthza8fVoPK99z-ZxH9TWruTKXkyDqkkUYsuGD5di03QcTvPu8ACMvu4dciyExwn9JAGCarFidQ3-P3201sf-dA6Y3sNw8E6o9ok2g9z2GJDDKlvP929ZnE0Ke1OOOb-d8rzkCUtBhEwqzIs.Lo1waRPaiGU50nbrKfLyHd2gRS24PQZ529kNLqefCEg&dib_tag=se&keywords=philips+hue+bridge&qid=1708895090&sprefix=phillips+hue+bridg%2Caps%2C179&sr=8-1) which supports the [Phillips-Hue API v2](https://developers.meethue.com/new-hue-api/). Most bridges nowadays support this API version, but if you have an old one it may not. The API is HTTPS based, you will need a device capable of making HTTPS requests (i.e., supports secure socket layer, SSL protocol). There are some libraries which you allow HTTPS connections through microcontrollers like the Ardunio Uno, but the bottom line is that the program size needed to fully support the SSL protocol is too large for most microcontrollers. To perform this connection it is recommended to use a microcontroller which has the space/support for the SSL protocol built in (enter ESP8266). 

**Before attempting to connect https with a microcontroller I recommend trying to connect to your bridge through python GET requests or some HTTPS request application like [Postman](https://www.postman.com/).**
## Making an HTTPS request to the Phillips-Hue bridge
A hue-application-key is needed connect to the Phillips-Hue bridge. Follow the directions on the Phillips-Hue Developer portal for connecting to your bridge for info on how to get this key. When this key is obtained, a simple request can be made to test the connection.
```powershell
curl --insecure -N -H 'hue-application-key: your-application-key' https://{your-bridge-ip}/clip/v2/resource/device
```
**Note, the hue-application-key is being passed to the https request as a header**. This can also be done using the python requests library like so:
```python
import requests, json


bridge_ip = 'your-bridge-ip'
url = f"https://{bridge_ip}/clip/v2/resource/device"

headers = {"hue-application-key":"your-application-key"}
r = requests.get(url, headers=headers, verify=False)

# Access response properties
print(r.status_code)  # HTTP status code
print(r.headers)     # Headers
print(r.content)     # Raw content (bytes)
print(r.text)        # Content as a string
```
**Note, in both cases these methods disable the SSL certificate verification (with curl `--insecure`; with python `verify=False`)**. Phillips-Hue recommends that your validate their provided private SSL certificate (see the developer portal HTTPS connections tab), but I have run into many issues with attempting this and it is not worth the effort. For implementations on your local network, there really isn't an issue with disabling the SSL certificate verification.

The response content should be serialized JSON containing information about your Phillips-Hue network.

## Implementing the HTTPS connection on the ESP8266
**The ESP8266 can be programmed through the Arduino IDE.** It is compatible for programming with the Arduino IDE and there many libraries for the ESP8266 which can be loaded into this IDE.
1. Add the latest stable release of the ESP866 libraries to Arduino's "Additional Board Manager URLs". ```http://arduino.esp8266.com/stable/package_esp8266com_index.json```
2. The board I have is this one: [ESP8266 NodeMCU CP2102 ESP-12E](https://www.amazon.com/HiLetgo-Internet-Development-Wireless-Micropython/dp/B081CSJV2V/ref=sr_1_3?crid=3RY5L1PWV5FGI&dib=eyJ2IjoiMSJ9.gJShu3rQeKD8EK_mYUdf6cd4BpJCiIA6K-ygA0Pvs5nZCA8Fda1XucdQ68P8_r_yCtdOFaBMpkvhYCfFKoqqf6l1l83HW92eW5BHEfuQVcX6ao_6qMOjelS0dLL4Fv8mFD1CoPZfnWPy1sVTvfUxuP577Omi-V0C2BWmJklYvZhQcE_zBLhQ_V38iVCE4ElQM2pQ-J61loRnFOb-RhFFrhNDoz9O-WTVWdAj4Zbr25M.a6ClmRteiC9xIb58TLIvnpTA2p6NXxhyJ9eUxw0oe9k&dib_tag=se&keywords=hiletgo+esp8266&qid=1708305050&sprefix=hiletgo+esp8266%2Caps%2C146&sr=8-3)
	1. This uses the NodeMCU 1.0 (ESP-12E) board type/config in Arduino. **This board selection provides the necessary libraries and examples to use the HTTPs functionality**

### Doing a basic GET request
After selecting the NodeMCU 1.0 (ESP-12E) in the board manager the Arduino IDE installs the necessary libraries to use this board. They have a good example for HTTPS requests in Node MCU 1.0 (ESP-12E) > ESP8266HTTPClient > BasicHTTPSClient. Got this one working with the following changes:
1. Add bridge ip/endpoint to https.begin
```c++
    if (https.begin(*client, "https://your-bridge-ip/clip/v2/resource/device")) {  // HTTPS
```
*This is different than the original https.begin format which uses \*client, "hostname", and port args.*
3. Set connection to insecure
```c++
client->setInsecure();
```

### Getting the HTTPS event stream
There is another good example in Node MCU 1.0 (ESP-12E) > ESP8266HTTPClient > StreamHTTPSClient which can be modified to fit the needs of this project. I had to change a few things:
1.  event stream endpoint
```c++
if(https.begin(*client, "https://192.168.1.4/eventstream/clip/v2")) {...};
```
2. These headers were added
```c++
      https.addHeader("Accept", "text/event-stream");
      https.addHeader("Connection", "Keep-Alive");
      https.addHeader("hue-application-key", "your-application-key");
```
3. Set connection to insecure
```c++
client->setInsecure();
```
4. Note, this program still uses ```https.GET()``` and not ```https.getStream()```

This program prints each character essentially as it is received from the HTTPs stream. For this project it is easiest to parse JSON objects from this event stream which we can check for the relevant light commands.

## Parsing JSON from the event stream
 The [ArduinoJSON](https://arduinojson.org/) can be used to deserialize (extract) the JSON ojbects from the event stream. The easiest approach (hardware allows for it) would be to load the whole object into json and then analyze it. Need to see if I can find a serial stream to json library. Referencing this tutorial: [Deserialization tutorial | ArduinoJson 6](https://arduinojson.org/v6/doc/deserialization/).

[ArduinoJson supports parsing from stream](https://arduinojson.org/#:~:text=Parse%20From%20Stream,reader%20types.) and even lists support for WiFiClients. They say "ArduinoJson can parse directly from an input [`Stream`](https://www.arduino.cc/reference/en/language/functions/communication/stream/) or [`std::istream`](https://en.cppreference.com/w/cpp/io/basic_istream)". The ESP WifiClientSecure class is derived from Stream. So this should be usable with the current code.

**Find start sequence of JSON serial stream "DATA: "; Pass stream to JSON de-serializer once start is found.**
Section 3.10.2 Reading from an HTTP response has information relevant to what I'm trying to do. It seems that this will only work if it passed the Stream when you know that the next serial stream is the valid json. I think I need to first search for the start sequence of the json serial stream and then start the parsing. This code from ChatGPT seems find the start sequence. I think there is an edge case where part of the start sequence is transmitted and immediately followed by the actual start sequence (i.e., "DADATA: " if the start sequence is "DATA: "), but I think this case isn't possible with the format of the transmitted data.

```c++
const char startSequence[] = "SD:"; // Your desired start sequence
char receivedData[100];             // Adjust the array size as needed
int dataIndex = 0;                  // Index to track where to store data

void setup() {
    Serial.begin(9600);
}

void loop()
{
    while (Serial.available()) {
        char incomingChar = Serial.read();
        // Check if the incoming character matches the start sequence
        if (incomingChar == startSequence[dataIndex]) {
            dataIndex++; // Move to the next character in the start sequence
            if (dataIndex == strlen(startSequence)) {
                // Start sequence found, do thing
                while (Serial.available()) {
                    ...
                }
            }
        }
    } else {
        // Reset the index if the incoming character doesn't match
        dataIndex = 0;
    }
}
```

After adapting the start sequence code above to my StreamHTTPSClient example and adding the json deserializer I was able to effectively parse the incoming json packets. **Note, I wasn't sure if the deserializer could auto detect if the end of the json object was reached and stop parsing, but it seems it can**. Also ArduinoJson has a nice function to print the json object serialized to the Serial output (`serializeJson(doc, Serial);`)

### Extracting Data from the JSON object
I am looking for packets which have a certain "id" (Phillips-Hue's v2 API identification of different lights in your network). Once the whole JSON object is loaded, it can be accessed to get only packets which contain changes for the relevant light. These changes can then handled, and the corresponding commands sent to the IR controller.
**Example event stream:**
```json
: hi



id: 1708404744: 0

data: [
    {
        "creationtime": "2024-02-20T04:52:24Z",
        "data": [
            {
                "id": "a7b863de-bce8-4933-86c2-34c8ade5cb79",
                "id_v1": "/lights/2",
                "on": {
                    "on": false
                },
                "owner": {
                    "rid": "4be3ce23-a0b2-4b52-a617-155d7685c97f",
                    "rtype": "device"
                },
                "type": "light"
            },
            {
                "id": "7ba3c2e7-1b1b-4189-941f-b434d60c75cb",
                "id_v1": "/lights/3",
                "on": {
                    "on": false
                },
                "owner": {
                    "rid": "33a4714d-347e-409a-9b55-2d1c02ee91c0",
                    "rtype": "device"
                },
                "type": "light"
            },
            {
                "id": "54cd2081-b12a-4a68-b9ec-08643af1be98",
                "id_v1": "/lights/1",
                "on": {
                    "on": false
                },
                "owner": {
                    "rid": "32955bfa-5b17-4d0f-95be-e04055d1f7b2",
                    "rtype": "device"
                },
                "type": "light"
            },
            {
                "id": "238fca10-1896-44ed-8047-a3b6eac70842",
                "id_v1": "/lights/5",
                "on": {
                    "on": false
                },
                "owner": {
                    "rid": "ae4464f3-c195-446b-b789-424aed521370",
                    "rtype": "device"
                },
                "type": "light"
            }
        ],
        "id": "7b343c5e-f374-4c82-a024-0915647c998a",
        "type": "update"
    }
]
```

In this case I want to get the ID with `obj[0]["data"][0]["id"]`. When I find the correct packets I can then get the light state change with `obj[0]["data"][0]["on"]` or `obj[0]["data"][0]["color"]`.

Through some accessing/algorithm logic found in the main .ino program this can be accomplished. A couple of notes for using ArduinoJSON:
1. A JSON Object (JsonDocument in ArduinoJSON) can be loaded from the event stream by first creating a DynamicJsonDocument object and passing this to the deserializeJson() function along with the HTTPS stream (accessed here with `Stream& response = https.getStream();`.
```c++
// Allocate the JsonDocument in the heap
DynamicJsonDocument doc(2048);
// Deserialize the JSON document in the response
deserializeJson(doc, response); //this is assuming it knows when to stop. Assumption seems correct.
```
2. A JsonObject can be extracted from an enveloping JsonDocument like so. This can be passed to a function *by reference*:
```c++
JsonObject obj = doc[i]["data"][j].as<JsonObject>();
check_command_keys(obj);

void check_command_keys(JsonObject& obj) {
	//do something
	serializeJson(obj, Serial);
}
```

When the appropriate light status change data has been extracted, it can be connected to the corresponding IR commands to control the moon light.

## Sending IR Signal to Control the Globe (Spoofing Magic LED Remote Controller)
 My code uses this library for the IR control [Arduino-IRremote/Arduino-IRremote](https://github.com/Arduino-IRremote/Arduino-IRremote). The IR control codes are based on code found here: [github repo](https://github.com/robertmoro/MagicLightingRemote). This project provides the specific IR commands for the moon lamp I am using, other IR controlled lights might use a different IR protocol and command codes.

Command Protocol: [NEC](https://exploreembedded.com/wiki/NEC_IR_Remote_Control_Interface_with_8051)
Command Values (Note, these are defined as uint32_t values in my code):

| Command     | Hex Value |
| :---------- | :-------- |
| Bright_UP   | FFA05F    |
| Bright_DOWN | FF20DF    |
| Shut_OFF    | FF609F    |
| Turn_ON     | FFE01F    |
| RED         | FF906F    |
| GREEN       | FF10EF    |
| BLUE        | FF50AF    |
| WHITE       | FFD02F    |
| FLASH       | FFF00F    |
| STROBE      | FFE817    |
| FADE        | FFD827    |
| SMOOTH      | FFC837    |
| RED2        | FFB04F    |
| RED3        | FFA857    |
| RED4        | FF9867    |
| RED5        | FF8877    |
| GREEN2      | FF30CF    |
| GREEN3      | FF28D7    |
| GREEN4      | FF18E7    |
| GREEN5      | FF08F7    |
| BLUE2       | FF708F    |
| BLUE3       | FF6897    |
| BLUE4       | FF58A7    |
| BLUE5       | FF48B7    |
The IR transmission code is adapted from the IRremote libraries `SimpleSender.ino`.
An IR command can be sent through the IR transmitter as follows:
```c++
#define DISABLE_CODE_FOR_RECEIVER
#include "PinDefinitionsAndMore.h"
#include <IRremote.hpp> // include the library

...

uint8_t nbits = 32;
uint32_t command=16736415;// = 0xFF609F;
IrSender.sendNEC(command,nbits);
```

The IRtransmitter module has Vcc (power), GND (Ground), and DAT pins. On the ESP8266 these pins should be connected to any 3V3 and corresponding GND pin; DAT needs to be connected to a specific pin on the ESP8266 which can support pulse width modulation (PWM). That pin on the ESP8266 is PIN12 (GPIO12 or D6). The specific pin can be found with the following code section in the `SimpleSender.ino` program.
```c++
// Just to know which program is running on my Arduino
Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));
Serial.print(F("Send IR signals at pin "));
Serial.println(IR_SEND_PIN);
```

![ESP8266 Pin Reference](https://www.electrorules.com/wp-content/uploads/2021/07/NodeMCU-ESP8266-Pinout.jpg)
**A couple of notes:**
1. The `IrSender.sendNEC(command,nbits);` in the `SimpleSender.ino` uses a different function overload input: `IrSender.sendNEC(0x00, sCommand, sRepeats);`.

### Translating the Phillips-Hue Color Data into different IR controller color codes
**Right now my solution to getting a color is bad and needs to be improved**
Hue uses the "CIE color space" to define the colors it sets. This is a 2d space which the color spectrum is mapped to. The data from the API looks like this:
```json
"color": {
	"xy": {
		"x": 0.5665,
		"y": 0.393
	}
```
X and Y here are coordinates on the CIE color map:
![CIE 1931 xy Chromaticity Chart](https://img.favpng.com/9/22/23/cie-1931-color-space-chromaticity-gamut-png-favpng-ZCCcQzz086Yhwyiwg1KHqGHi9.jpg)

Some good research here: [A Beginner’s Guide to (CIE) Colorimetry | by Chandler Abraham | Color and Imaging | Medium](https://medium.com/hipster-color-science/a-beginners-guide-to-colorimetry-401f1830b65a)
My first thought is that I should create a 2d array of colors values which close to the magic controllers available colors. Then, as a color update is pushed, find the closest color in that map to send as an IR command. Mapping this out I realize it becomes more complicated. Can I use the shortest square root path?

I have concluded that the color system hue uses [doesn't have a good linear relationships between colors](http://www.colorbasics.com/CIESystem/#:~:text=CIE%201931%20Gamut,the%20source%20colors). For this reason, I think it will be more effective to define x/y 'zones' for each color (and not use square root linear path). If a given coordinate falls under a particular zone, then it will set the corresponding color for that zone.

Here are the following manually defined color zones:

| 1   | None  | None  | None  | None  | None  | None | None | None | None | None |
| --- | ----- | ----- | ----- | ----- | ----- | ---- | ---- | ---- | ---- | ---- |
| 0.9 | Green | None  | None  | None  | None  | None | None | None | None | None |
| 0.8 | Green | Green | Green | None  | None  | None | None | None | None | None |
| 0.7 | Green | Green | Green | Green | None  | None | None | None | None | None |
| 0.6 | Green | Green | Green | Green | Green | None | None | None | None | None |
| 0.5 | Blue  | Green | Green | Green | Green | Red  | None | None | None | None |
| 0.4 | Blue  | Blue  | Blue  | White | Red   | Red  | Red  | None | None | None |
| 0.3 | Blue  | Blue  | Blue  | Red   | Red   | Red  | Red  | Red  | None | None |
| 0.2 | Blue  | Blue  | Blue  | Red   | Red   | Red  | None | None | None | None |
| 0.1 | None  | Blue  | Blue  | Red   | None  | None | None | None | None | None |
| 0   | 0.1   | 0.2   | 0.3   | 0.4   | 0.5   | 0.6  | 0.7  | 0.8  | 0.9  | 1    |
