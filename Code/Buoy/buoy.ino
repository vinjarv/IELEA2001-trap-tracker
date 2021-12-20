#include <WiFi.h>
#include <HTTPClient.h>
#include <SoftwareSerial.h>
#include <Adafruit_GPS.h>

// Config file for WiFi and Ubidots token
#include "credentials.h"

// WiFi - from config file
#define SSID _SSID
#define PASSWORD _PASSWORD

// Ubidots - from config file
#define UBIDOTS_TOKEN _UBIDOTS_TOKEN
#define UBIDOTS_HOST "industrial.api.ubidots.com"
#define URL "/api/v1.6/devices/ESP-test/"


#define BAUDRATE 115200         // Buoy <=> Trap serial
#define MAX_IMG_SIZE 98*1000    // 98kB
#define TRANSMIT_DELAY_S 30     // Seconds
#define trapserial Serial2
#define GPSECHO false           // Don't print GPS output to Serial0

WiFiClient c;
HTTPClient client;
SoftwareSerial GPSSerial(18, 19);
Adafruit_GPS gps(&GPSSerial);

unsigned long next_transmit;
char* payload;


void setup(){
    Serial.begin(115200);           // For debugging
    trapserial.begin(BAUDRATE);
    trapserial.setTimeout(10000);

    gps.begin(9600);
    gps.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);  // Minimal output
    gps.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);      // 1Hz update rate

    connect_wifi();
    // Permanently assign RAM for constructing the HTTP payload
    payload = (char*)malloc(100*1000);
}
    
void loop(){
    // Send data
    next_transmit = millis() + TRANSMIT_DELAY_S*1000;
    Serial.println("[INFO] Sending data");
    data_to_ubidots();

    // Wait for next transmit time
    if (next_transmit > millis()){
        delay(next_transmit - millis());
    }
}

// Send JSON data string to Ubidots
void data_to_ubidots(){
    Serial.println("Largest free block:");
    Serial.println(heap_caps_get_largest_free_block(MALLOC_CAP_32BIT));
    Serial.println("Available heap: " + String(ESP.getFreeHeap()));
    size_t payload_size;
    Serial.printf("Memory allocated at %d\r\n", payload);

    // Ubidots fixed payload strings
    const char* PAYLOAD_START = "{";
    const char* TEMP_START = "\"temp\":{\"value\":";
    const char* TEMP_END = "}, ";
    const char* LOC_START = "\"location\":{\"value\":1, \"context\":";
    const char* LOC_END = "}, ";
    const char* IMAGE_START = "\"image\":{\"value\":1, \"context\":{\"image_b64\":\"";
    const char* IMAGE_END = "\"}}";
    const char* PAYLOAD_END = "}\r\n";
    strcpy(payload, PAYLOAD_START);
    payload_size = strlen(PAYLOAD_START);

    // Add temperature to payload
    strcat(payload, TEMP_START);
    payload_size += strlen(TEMP_START);

    float temp_float = get_temperature();
    char* temp_str = (char*)malloc(50);
    sprintf(temp_str, "%.2f", temp_float);
    strcat(payload, temp_str);
    payload_size += strlen(temp_str);
    Serial.printf("Temp string: %s\r\n", temp_str);
    free(temp_str);

    strcat(payload, TEMP_END);
    payload_size += strlen(TEMP_END);

    // Add GPS data to payload
    strcat(payload, LOC_START);
    payload_size += strlen(LOC_START);

    char* GPS_str = (char*)malloc(50);
    get_GPS_string(GPS_str);
    strcat(payload, GPS_str);
    payload_size += strlen(GPS_str);
    Serial.printf("GPS string: %s\r\n", GPS_str);
    free(GPS_str);

    strcat(payload, LOC_END);
    payload_size += strlen(LOC_END);

    // Add image to payload
    strcat(payload, IMAGE_START);
    payload_size += strlen(IMAGE_START);

    char* image_start = (payload + payload_size);
    // get_image appends to payload at pointer image_start
    uint image_size = get_image(image_start);
    payload_size += image_size;
    Serial.printf("Image appended, %dB\r\n", image_size);

    strcat(payload, IMAGE_END);
    payload_size += strlen(IMAGE_END);


    // End payload
    strcat(payload, PAYLOAD_END);
    payload_size += strlen(PAYLOAD_END);

    // Connect to ubidots and transmit
    if(client.begin(c, UBIDOTS_HOST, 80, URL, false)){
        Serial.println("[HTTP] Connection started");
        client.addHeader("X-Auth-Token", UBIDOTS_TOKEN);
        client.addHeader("Content-Type", "application/json");
        int httpcode = client.POST((uint8_t*)payload, payload_size);
        Serial.println("[HTTP] POST complete");
        Serial.printf("[HTTP] POST code: %d\r\n", httpcode);
        String received_payload = client.getString();
        Serial.printf("[HTTP] Payload: %s \r\n", received_payload.c_str());
        Serial.println("");
    }
    client.end();
}

// Request temperature, return degrees C as float
float get_temperature(){
    send_command("temp");
    String temp_string = trapserial.readStringUntil('\n');
    return temp_string.toFloat();
}

// Request image, put in memory, return total bytes
uint get_image(char* buffer){
    send_command("image");
    bool done = false;
    uint bytes_copied = 0;
    while (!done && bytes_copied <= MAX_IMG_SIZE){
        // Wait for data
        while(trapserial.available() <= 0){}

        char c = trapserial.read();
        if (c == '\n' || c == '\r'){
            done = true;
        }else{
            *(buffer + bytes_copied) = c;
            bytes_copied++;
        }
    }
    if (bytes_copied >= MAX_IMG_SIZE){
        Serial.println("[INFO] Max image size reached!");
    }
    // Null-terminate string
    *(buffer + bytes_copied) = '\0';
    return bytes_copied;
}

// Read data from GPS module, format as JSON string
void get_GPS_string(char* output_str){
    GPSSerial.flush();
    const char* nmeacode = "$GPRMC";
    gps.waitForSentence(nmeacode);
    gps.parse(gps.lastNMEA());
    float lat = gps.latitude / 100;
    float lng = gps.longitude / 100;

    sprintf(output_str, "{\"lat\":%.4f, \"lng\":%.4f}", lat, lng);    
}

// Send a command to trap node over serial
void send_command(String command){
    // Clear serial buffers
    trapserial.flush();
    delay(250);
    while(trapserial.available() > 0){
        trapserial.read();
    }

    // Read and send newline until received ready signal
    String received = "";
    unsigned long sent_time = 0;
    while (received != String("ready")){
        if (millis() > sent_time + 2500){
            trapserial.println("");
            sent_time = millis();
        }
        received = trapserial.readStringUntil('\n');
        trapserial.read(); // Clear newline char
        received.trim(); // Remove additional char
    }

    // Send command
    trapserial.println(command);
}

// Connect to AP
void connect_wifi(){
    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID, PASSWORD);
    Serial.print("Connecting to AP ");
    while (!WiFi.isConnected()){
        Serial.print('.');
        delay(500);
    }
    Serial.println("\nConnected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
}