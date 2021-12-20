#include "esp_camera.h"
#include "src/custombase64.h"
#include <DallasTemperature.h>
#include <OneWire.h>


#define TEMP_PIN 15
#define FLASH_PIN 4
#define MAX_IMG_SIZE 98*1000    //98kB
#define BAUDRATE 115200         // Trap <=> Buoy serial

OneWire onewire(TEMP_PIN);
DallasTemperature temp_sensor(&onewire);

// Buffer to store base64 image
unsigned char* encoded_img;


void setup()
{
	Serial.begin(BAUDRATE);

    init_camera();
    pinMode(FLASH_PIN, OUTPUT);
    // Assign PSRAM for the base64 conversion of image
    encoded_img = (unsigned char*)ps_malloc(500*1000);

    // Initialize onewire DS18B20 temperature sensor
    temp_sensor.begin();
}


void loop()
{
    // Send "ready" only if RX serial buffer is empty
    if (Serial.available() <= 0){
        delay(10);
        Serial.printf("ready\n");
    }

    // Wait for command
    while(Serial.available() <= 0){};
    String command = Serial.readStringUntil('\n');
    command.trim();

    // Send data according to command
    if (command == "image"){
        send_image();
    }
    if (command == "temp"){
        send_temp();
    }
    if (command == "light"){
        send_light();
    }
}


// Get image, convert to base64 and transmit over serial
void send_image(){
    digitalWrite(FLASH_PIN, HIGH);
    delay(10);
    // Capture image to framebuffer
    camera_fb_t* fb = NULL;
    fb = esp_camera_fb_get();
    digitalWrite(FLASH_PIN, LOW);
    if(!fb) {
        Serial.println("Camera capture failed");
        esp_camera_fb_return(fb);
        return;
    }

    // Check if image size is small enough to transmit
    // Base64 encoding increases size by 4 bytes per 3 bytes in binary format
    if (fb->len >= ulong(MAX_IMG_SIZE * 4.0/3.0)){
        // TODO: Reduce quality until image size fits?
        Serial.println("Image too large");
        return;
    }

    // Framebuffer (jpg) to base64-encoded string
    size_t encoded_len;
    custombase64_h::base64_encode_P(fb->buf, encoded_img, fb->len, &encoded_len);
    esp_camera_fb_return(fb);

    // Write to serial
    Serial.write(encoded_img, encoded_len);
    Serial.print('\n');
}

// Read temperature sensor data, transmit over serial
void send_temp(){
    temp_sensor.requestTemperatures();
    float temp_c = temp_sensor.getTempCByIndex(0);
    Serial.printf("%.2f\n", temp_c);
}

// Read light level sensor, transmit over serial
void send_light(){
    // Dummy value
    Serial.printf("42\n");
}

// Camera setup/settings
void init_camera(){
    #define CAMERA_MODEL_AI_THINKER
    // Pin definition for CAMERA_MODEL_AI_THINKER 
    #define PWDN_GPIO_NUM     32
    #define RESET_GPIO_NUM    -1
    #define XCLK_GPIO_NUM      0
    #define SIOD_GPIO_NUM     26
    #define SIOC_GPIO_NUM     27

    #define Y9_GPIO_NUM       35
    #define Y8_GPIO_NUM       34
    #define Y7_GPIO_NUM       39
    #define Y6_GPIO_NUM       36
    #define Y5_GPIO_NUM       21
    #define Y4_GPIO_NUM       19
    #define Y3_GPIO_NUM       18
    #define Y2_GPIO_NUM        5
    #define VSYNC_GPIO_NUM    25 
    #define HREF_GPIO_NUM     23
    #define PCLK_GPIO_NUM     22

    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = FRAMESIZE_XGA;
    config.jpeg_quality = 10;
    config.fb_count = 1;

    // Init Camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
    }
}