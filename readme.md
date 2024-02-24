## ESP8266 Implementation
This chip has "built-in TCP/IP Networking Software".

### How to Program?
[Setting Up the Arduino IDE to Program ESP8266 : 3 Steps - Instructables](https://www.instructables.com/Setting-Up-the-Arduino-IDE-to-Program-ESP8266/)

The ESP8266 can be programmed through the Arduino IDE. 

1. Add the latest stable release of the ESP866 libraries to Arduino's "Additional Board Manager URLs". ```http://arduino.esp8266.com/stable/package_esp8266com_index.json```
2. The board I have is this one [Amazon.com: HiLetgo 3pcs ESP8266 NodeMCU CP2102 ESP-12E Development Board Open Source Serial Module Works Great for Arduino IDE/Micropython (Large) : Electronics](https://www.amazon.com/HiLetgo-Internet-Development-Wireless-Micropython/dp/B081CSJV2V/ref=sr_1_3?crid=3RY5L1PWV5FGI&dib=eyJ2IjoiMSJ9.gJShu3rQeKD8EK_mYUdf6cd4BpJCiIA6K-ygA0Pvs5nZCA8Fda1XucdQ68P8_r_yCtdOFaBMpkvhYCfFKoqqf6l1l83HW92eW5BHEfuQVcX6ao_6qMOjelS0dLL4Fv8mFD1CoPZfnWPy1sVTvfUxuP577Omi-V0C2BWmJklYvZhQcE_zBLhQ_V38iVCE4ElQM2pQ-J61loRnFOb-RhFFrhNDoz9O-WTVWdAj4Zbr25M.a6ClmRteiC9xIb58TLIvnpTA2p6NXxhyJ9eUxw0oe9k&dib_tag=se&keywords=hiletgo+esp8266&qid=1708305050&sprefix=hiletgo+esp8266%2Caps%2C146&sr=8-3)
	1. This uses the NodeMCU 1.0 (ESP-12E) board type/config in Arduino.
### Doing a basic get request
1. After installing the ESP8266 Arduino library they have a good example in ESP8266HTTPClient > BasicHTTPSClient
	1. Got this one working with the following commands
	2. *This is different than the original https.begin format which uses \*client, "hostname", and port args.*
```c++
    if (https.begin(*client, "https://192.168.1.4/clip/v2/resource/device")) {  // HTTPS
      Serial.print("Connected");
      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      https.addHeader("hue-application-key", "***********************************************");
      int httpCode = https.GET();
      ...
  };
```

Now need to get the event stream working... the event stream endpoint is ```/eventstream/clip/v2```. Also pass these headers
```json
"Accept": "text/event-stream",
"Connection": "Keep-Alive"
```

### Getting the Event Stream
There is a good example in ESP8266HTTPClient > StreamHTTPSClient. I had to change a few things:
1.  event stream endpoint
```c++
if(https.begin(*client, "https://192.168.1.4/eventstream/clip/v2")) {...};
```
2. These headers were added
```c++
      https.addHeader("Accept", "text/event-stream");
      https.addHeader("Connection", "Keep-Alive");
      https.addHeader("hue-application-key", "***********************************************");
```
3. connection insecure
```c++
client->setInsecure();
```
4. Note this program still uses ```https.GET()``` and not ```https.getStream()```

**Here is the whole file:**
```c++
/**
   StreamHTTPClient.ino

    Created on: 24.05.2015

*/

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

ESP8266WiFiMulti WiFiMulti;

void setup() {

  Serial.begin(115200);
  // Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("Big Ls Clubhouse", "wittyraft698");
}

void loop() {
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);

    bool mfln = client->probeMaxFragmentLength("tls.mbed.org", 443, 1024); //what is this?
    Serial.printf("\nConnecting");
    Serial.printf("Maximum fragment Length negotiation supported: %s\n", mfln ? "yes" : "no");
    if (mfln) { client->setBufferSizes(1024, 1024); }

    Serial.print("[HTTPS] begin...\n");

    // configure server and url
    const uint8_t fingerprint[20] = { 0x15, 0x77, 0xdc, 0x04, 0x7c, 0x00, 0xf8, 0x70, 0x09, 0x34, 0x24, 0xf4, 0xd3, 0xa1, 0x7a, 0x6c, 0x1e, 0xa3, 0xe0, 0x2a };

    // client->setFingerprint(fingerprint);
    client->setInsecure();


    HTTPClient https;

    if (https.begin(*client, "https://192.168.1.4/eventstream/clip/v2")) {

      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      https.addHeader("Accept", "text/event-stream");
      https.addHeader("Connection", "Keep-Alive");
      https.addHeader("hue-application-key", "***********************************************");
      int httpCode = https.GET();
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK) {

          // get length of document (is -1 when Server sends no Content-Length header)
          int len = https.getSize();

          // create buffer for read
          static uint8_t buff[128] = { 0 };

          // read all data from server
          while (https.connected() && (len > 0 || len == -1)) {
            // get available data size
            size_t size = client->available();

            if (size) {
              // read up to 128 byte
              int c = client->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));

              // write it to Serial
              Serial.write(buff, c);

              if (len > 0) { len -= c; }
            }
            delay(1);
          }

          Serial.println();
          Serial.print("[HTTPS] connection closed or file end.\n");
        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }

      https.end();
    } else {
      Serial.printf("Unable to connect\n");
    }
  }

  Serial.println("Wait 10s before the next round...");
  delay(10000);
}
```

### Parsing JSON from the event stream
First I need to answer the question, where does the current readBytes() method come from (being used by the event stream example).

The client is calling the readBytes method like so. This is a special implementation of the normal object oriented structure (object.method). *The client in this case is a pointer to an object*.
```c++
client->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
```
I believe it was created here in the code `std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);`
This is smart pointer (pointer which auto deletes when it isn't being used anymore), `client` (uses the format `std::unique_ptr<Obj> name(new Obj);`). The actual object being created is the `BearSSL::WifiClientSecure` object.
The `BearSSL::WifiClientSecure` [extends the WiFiClient class](https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/bearssl-client-secure-class.html#:~:text=It%20extends%20WiFiClient%20and%20so%20can%20be%20used%20with%20minimal%20changes%20to%20code%20that%20does%20unsecured%20communications). Not just any WiFiClient class, but the ESP8266WIFI WiFiClient class. The specific method being used in client->readBytes() can be found [here](https://arduino-esp8266.readthedocs.io/en/latest/reference.html#streams:~:text=The%20readBytes(buffer,g.%20HardwareSerial%3A%3A.). This method is of the following format,
```c++
?::readBytes(buff, len);
```
where the buff is a character array and the length is the amount of bytes to read to (and modify?) the buff.

I don't necessarily have to use this buff, I need to find the smallest chunk of data needed to get the event change information.
Actually, the easiest approach (hardware allows for it) would be to load the whole object into json and then analyze it. Need to see if I can find a serial stream to json library. Referencing this tutorial: [[arduinojson_deserialization_tutorial.pdf]].

[ArduinoJson supports parsing from stream](https://arduinojson.org/#:~:text=Parse%20From%20Stream,reader%20types.) and even lists support for WiFiClients. They say "ArduinoJson can parse directly from an input [`Stream`](https://www.arduino.cc/reference/en/language/functions/communication/stream/) or [`std::istream`](https://en.cppreference.com/w/cpp/io/basic_istream)". The ESP WifiClientSecure class is derived from Stream. So this should be usable with the current code.
![[esp8266 WiFiClientSecure is Stream.png]]
Section 3.10.2 Reading from an HTTP response has information relevant to what I'm trying to do. It seems that this will only work if it passed the Stream when you know that the next serial stream is the valid json. I think I need to first search for the start sequence of the json serial stream and then start the parsing. This code from ChatGPT seems find the start sequence. I think there is an edge case where part of the start sequence is transmitted and immediately followed by the actual start sequence (i.e., "DADATA: " if the start sequence is "DATA: "), but I think this case isn't possible with the format of the transmitted data.
```c++
const char startSequence[] = "SD:"; // Your desired start sequence
char receivedData[100]; // Adjust the array size as needed
int dataIndex = 0; // Index to track where to store data

void setup() {
  Serial.begin(9600);
}

void loop() {
  while (Serial.available()) {
    char incomingChar = Serial.read();

    // Check if the incoming character matches the start sequence
    if (incomingChar == startSequence[dataIndex]) {
      dataIndex++; // Move to the next character in the start sequence
      if (dataIndex == strlen(startSequence)) {
        // Start sequence found, start storing data
        while (Serial.available()) {
          incomingChar = Serial.read();
          if (incomingChar == '\n') {
            // End marker (newline) encountered, process the data
            receivedData[dataIndex] = '\0'; // Null-terminate the array
            Serial.print("Received data: ");
            Serial.println(receivedData);
            // Now you can further process 'receivedData' as needed
            // Reset the index for the next start sequence
            dataIndex = 0;
            break;
          }
          // Store the character in the array
          receivedData[dataIndex++] = incomingChar;
          if (dataIndex >= sizeof(receivedData) - 1) {
            // Array full, handle overflow or reset the index
            // (e.g., discard data or start over)
            dataIndex = 0;
          }
        }
      }
    } else {
      // Reset the index if the incoming character doesn't match
      dataIndex = 0;
    }
  }
}
```

After adapting the start sequence code above to my StreamHTTPSClient example and adding the json deserializer I was able to effectively parse the incoming json packets. Note, I wasn't sure if the deserializer could auto detect if the end of the json object was reached and stop parsing, but it seems it can. Also ArduinoJson has a nice function to print the json object serialized to the Serial output (`serializeJson(doc, Serial);`)
```c++
/**
   StreamHTTPClient.ino

    Created on: 24.05.2015

*/

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

ESP8266WiFiMulti WiFiMulti;
int dataIndex = 0; // Index to track where to store data
char start_sequence[] = "data: ";

void setup() {

  Serial.begin(115200);
  // Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("Big Ls Clubhouse", "wittyraft698");
}

void loop() {
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);

    bool mfln = client->probeMaxFragmentLength("tls.mbed.org", 443, 1024); //what is this?
    Serial.printf("\nConnecting");
    Serial.printf("Maximum fragment Length negotiation supported: %s\n", mfln ? "yes" : "no");
    if (mfln) { client->setBufferSizes(1024, 1024); }

    Serial.print("[HTTPS] begin...\n");
    client->setInsecure();
    HTTPClient https;

    if (https.begin(*client, "https://192.168.1.4/eventstream/clip/v2")) {
      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      https.addHeader("Accept", "text/event-stream");
      https.addHeader("Connection", "Keep-Alive");
      https.addHeader("hue-application-key", "***********************************************");
      int httpCode = https.GET();
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK) {

          // get length of document (is -1 when Server sends no Content-Length header)
          int len = https.getSize();

          // read all data from server
          while (https.connected() && (len > 0 || len == -1)) {
            Stream& response = https.getStream(); //This doesn't have to be initialized everytime a start sequence is found.
            
	        //read incoming char, eval against start sequence. if start sequence, parse json object and print to serial.
            char incomingChar = client->read();
            if (incomingChar == start_sequence[dataIndex]) {
              dataIndex++; // Move to the next character in the start sequence
              if (dataIndex == strlen(start_sequence)) {
                // Allocate the JsonDocument in the heap
                DynamicJsonDocument doc(2048);
                // Deserialize the JSON document in the response
                deserializeJson(doc, response); //this is assuming it knows when to stop. Assumption seems correct.
                serializeJson(doc, Serial); //Nice function to serialize json object and print to Serial
                Serial.println();
              }
            } else {
              // Reset the index if the incoming character doesn't match
              dataIndex = 0;
            }
            delay(1);
          }
          Serial.println();
          Serial.print("[HTTPS] connection closed or file end.\n");
        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }

      https.end();
    } else {
      Serial.printf("Unable to connect\n");
    }
  }

  Serial.println("Wait 10s before the next round...");
  delay(10000);
}
```

**What does that arrow notation do?**
this is a shorthand notation to directly access the method of an object *through* the a pointer of that object. For example, in the below code `ptr->printValue()` is equivalent to `(*ptr).printValue()`.
```c++
#include <iostream>

struct MyClass {
    int value;
    void printValue() {
        std::cout << "Value: " << value << std::endl;
    }
};

int main() {
    MyClass obj; // Create an object of MyClass
    obj.value = 42;

    MyClass* ptr = &obj; // Create a pointer to obj

    // Using the arrow operator to access obj's member through the pointer
    ptr->printValue();

    // Using the dot operator directly on the object
    obj.printValue();

    return 0;
}
```

### Handling JSON events
**Ignore all of the following rambling I found the issue: In redefining the handle_packet function I lost the deserializeJson function which actually writes the Json data to the created JsonDocument object. If this function isn't called the empty JsonDocument object will be null (as per documentation); I was then passing this null value to the handle packet function and it was printing the null object through serial. After fixing this issue the pass by reference defined below was working.**
If I define a function to handle the JsonDocument how can I pass the JsonDocument to it? Here is what I have so far. This would be an example of pass by reference. You would call the function like so `handle_packet(doc);`. The &doc here usually means get the memory address of the doc object (standard notation is like this: `int* ptr = &val;`). This doesn't seem to work; The program compiles but the output of this function is null when it should be the json serial stream.
```c++
void handle_packet(JsonDocument& doc) {
  serializeJson(doc, Serial);
  Serial.println();
}
```

I'm not sure exactly what the below signatures are describing, but it suggests to me that the function uses pass by reference for the JsonDocument obj. Note we are creating a DynamicJsonDocument object not a JsonDocument object, but these objects are compatible in some way (where I think static/dynamicJsonDocument are the same JsonDocument object but the notation affects how the object is created in some way).
![[deserializeJson func signatures.png]]
1. Need to identify specific light we want to sync IR light to.
	1. Look for json events with that specific light.

The json data is returned as an array of event objects. Need to iterate over this array, searching for objects which contain the ID of the specific light I want to use. 

### Sending IR Signal to Control the Globe (Spoofing Magic LED Remote Controller)
w/ ESP8266

### Color Handler
Hue uses the "CIE color space" to define the colors it sets. This is a 2d space which the color spectrum is mapped to.
My first thought is that I should create a 2d array of colors values which close to the magic controllers available colors. Then, as a color update is pushed, find the closest color in that map to send as an IR command.

Mapping this out I realize it becomes more complicated. Can I use the shortest square root path?

Ok, some good research here: [A Beginner’s Guide to (CIE) Colorimetry | by Chandler Abraham | Color and Imaging | Medium](https://medium.com/hipster-color-science/a-beginners-guide-to-colorimetry-401f1830b65a)

I have conncluded that the color system hue uses [doesn't have a good linear relationships between colors](http://www.colorbasics.com/CIESystem/#:~:text=CIE%201931%20Gamut,the%20source%20colors). For this reason, I think it will be more effective to define x/y 'zones' for each color. If a given coordinate falls under a particular zone, then it will set the corresponding color for that zone.

Square zones will be easiest, but the 'colorspace' for hue is a triangle. For now lets choose only three colors to set, red, blue, green and define 4 zones for which they exist. Based on the below diagram, we can use the following ranges:

- Red: 
	- 
- Green:
- Blue
	- 0 < x < 0.3
	- 0 < y < 0.3
- White - will override rgb spaces

Actually, a better option might be to divide this diagram into different blocks and set a color for each block.

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

![[CIE Color Space.png]]