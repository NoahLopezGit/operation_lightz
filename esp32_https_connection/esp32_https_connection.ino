/*
  Complete project details: https://RandomNerdTutorials.com/esp32-https-requests/
  Based on the BasicHTTPSClient.ino example found at Examples > BasicHttpsClient
*/

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

// includes for IR control
#define DISABLE_CODE_FOR_RECEIVER
#include "PinDefinitionsAndMore.h" // where did this come from?
#include <IRremote.hpp> // include the library

// Replace with your network credentials
const char* ssid = "your_ssid";
const char* password = "your_password";

// hue config
const char* hue_token = "your_hue_api_token";
const char *light_id = "your_light_id"; // existing light id to tie controller to

// hue color definitions - this is cursed
#define Green 16716015
#define None 16764975
#define White 16764975
#define Red 16748655
#define Blue 16732335
uint32_t color_picker_array[10][10] = {
  {None,None,None,None,None,None,None,None,None,None},
  {Green,None,None,None,None,None,None,None,None,None},
  {Green,Green,Green,None,None,None,None,None,None,None},
  {Green,Green,Green,Green,None,None,None,None,None,None},
  {Green,Green,Green,Green,Green,None,None,None,None,None},
  {Blue,Green,Green,Green,Green,Red,None,None,None,None},
  {Blue,Blue,Blue,White,Red,Red,Red,None,None,None},
  {Blue,Blue,Blue,Red,Red,Red,Red,Red,None,None},
  {Blue,Blue,Blue,Red,Red,Red,None,None,None,None},
  {None,Blue,Blue,Red,None,None,None,None,None,None}
};

// data holders
int dataIndex = 0; // Index to track where to store data

// json start sequence
char start_sequence[] = "data: ";

void handle_packet(JsonDocument& doc) {
  // serializeJson(doc, Serial);
  int num_events = doc.size();
  for (int i=0; i<num_events; i++) {
    int num_data = doc[i]["data"].size();
    for (int j=0; j<num_data; j++) {
      const char *doc_id = doc[i]["data"][j]["id"];
      if (strcmp(light_id,doc_id)==0){
        //get the light status, if it exists
        JsonObject obj = doc[i]["data"][j].as<JsonObject>();
        check_command_keys(obj);
      }      
    }
  }
}

void check_command_keys(JsonObject& obj) {
  //create an array of pointers to char arrays
  char on[] = "on";
  char color[] = "color";
  for (JsonPair p : obj) {
    const char *key = p.key().c_str(); // is a JsonString
    if (strcmp(key, on)==0){
      JsonObject obj_tmp = obj[on].as<JsonObject>();
      set_output_status(obj_tmp);
    } else if (strcmp(key, color)==0) {
      JsonObject obj_tmp = obj[color].as<JsonObject>();
      set_color(obj_tmp);
    }
  }
}

void set_output_status(JsonObject& obj) {
  uint8_t nbits = 32;
  bool on_status = obj["on"];
  Serial.print("Setting Output Status: ");
  Serial.println(on_status);
  if (on_status==true){
    uint32_t command=16769055;// = 0xFFE01F;
    IrSender.sendNEC(command,nbits);
  } else if (on_status==false) {
    uint32_t command=16736415;// = 0xFF609F;
    IrSender.sendNEC(command,nbits);
  }
}

//todo tf is this color representation
void set_color(JsonObject& obj){
  uint8_t nbits = 32;
  float x = obj["xy"]["x"];
  float y = obj["xy"]["y"];
  // check for membership in color picker array
  int x_index = int(ceil(x*10)-1+0.5); 
  int y_index = (9-int(ceil(y*10)-1+0.5)); //have to 9-n b/c color picker array is reversed
  uint32_t color_command = color_picker_array[y_index][x_index]; // x_index is column wise, 2nd index
  Serial.println("Setting Color:");
  Serial.print(x);
  Serial.print(",");
  Serial.println(y);
  Serial.print(x_index);
  Serial.print(",");
  Serial.println(y_index);
  Serial.println(color_command);
  IrSender.sendNEC(color_command,nbits);
}

void setup() {
  Serial.begin(115200);
  Serial.println();

  // print IR send pin
  Serial.println(IR_SEND_PIN); // gpio9 on esp32-c3 super mini

  // Initialize Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

void loop() {
  WiFiClientSecure* client = new WiFiClientSecure;
  if (client) {
    // set secure client without certificate
    client->setInsecure();
    //create an HTTPClient instance
    HTTPClient https;

    //Initializing an HTTPS communication using the secure client
    Serial.print("[HTTPS] begin...\n");
    if (https.begin(*client, "https://192.168.1.4/eventstream/clip/v2")) {
      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      https.addHeader("Accept", "text/event-stream");
      https.addHeader("Connection", "Keep-Alive");
      https.addHeader("hue-application-key", hue_token);
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
                handle_packet(doc);
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
