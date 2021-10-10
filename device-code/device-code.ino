#include "WiFiEsp.h"
#include "secrets.h"
#include "adCustomThingspeak.h"
#include "DHT.h"
#include "SoftwareSerial.h"
#include <LiquidCrystal.h>
#define REDPIN 10
#define GREENPIN 9
#define BLUEPIN 8
#define BUTTON A1
#define DHTPIN A5
#define LDR A0
#define PIEZO A3
#define DHTTYPE DHT11
#define ESP_BAUDRATE  19200
#define READ_INTERVAL 60000

LiquidCrystal lcd(12, 11, 5, 4, 3, 2); // LCD setup
DHT dht(DHTPIN, DHTTYPE);

WiFiEspClient  client;
SoftwareSerial Serial1(6, 7); // RX, TX

float tempSum, humidSum, lightSum;
int tempCount, humidCount, lightCount;
//int temp, light, humid;
int uploadFails = 0;
bool shouldUpload = true;
bool discreteMode = false;
int uploadInterval = 0;
int timeTracker = 0;
char ssid[] = SECRET_SSID;   // your network SSID (name)
char pass[] = SECRET_PASS;

void lcd_print(String value, int cx, int cy, bool lcdClear = false) {
  if (lcdClear == true) {
    lcd.clear();
  }
  lcd.setCursor(cx, cy);
  lcd.print(value);

}

void reset_vars(){
  uploadFails = 0;
    uploadInterval = 20;
    tempSum = 0;
    humidSum = 0;
    lightSum = 0;
    tempCount = 0;
    humidCount = 0;
    lightCount = 0;
}

void upload_data(String postData) {
  if (!shouldUpload) {
    reset_vars();
    
    return;
  }
  // Connect or reconnect to WiFi
  if (WiFi.status() != WL_CONNECTED) {
    lcd_print("Connecting...", 0, 0, true);
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      delay(3000);
    }
  }
  if (postData != "f") {
    int x = ThingSpeak.writeFields(postData, SECRET_KEY);
    lcd_print("Upload Status=", 0, 0, true);
    lcd_print(String(x), 0, 1);
    reset_vars();
  }
  //    lcd_print("Error uploading...", 0, 0, true);
  else {
    uploadFails += 1;
    uploadInterval = 2;
    async_delay(2000);
  }
  delay(1000);

}

void setEspBaudRate(unsigned long baudrate) {
  long rates[6] = {115200, 74880, 57600, 38400, 19200, 9600};
  lcd_print("Initializing...", 0, 0, true);
  for (int i = 0; i < 6; i++) {
    Serial1.begin(rates[i]);
    delay(100);
    Serial1.print("AT+UART_DEF=");
    Serial1.print(baudrate);
    Serial1.print(",8,1,0,0\r\n");
    delay(100);
  }

  Serial1.begin(baudrate);
}


void update_RGB(float t = -1) {
  Serial.println(discreteMode);
  Serial.println(t);
  if (!discreteMode && t != -1) {
    if (t <= 10.0f) {
      rgb(0, 0, 255);
    }
    else if (14.0f < t && t <= 25.0f) {
      rgb(0, 255, 0);
    }
    else {
      rgb(255, 0, 0);
    }
  }
  else {
    rgb(0, 0, 0);
  }

}

void async_delay(unsigned int ms) {
  for (unsigned int i = 0; i < ms; i += 100) {
    if (digitalRead(BUTTON) == 1) {
      button_handler();
      i += 4000;
    }
    delay(100);
  }

}

void button_handler() {
  if (uploadFails >= 5) {
    update_data();
    uploadFails = 0;
    update_lcd(tempSum / tempCount, humidCount / humidSum);
    delay(3000);
    return;
  }
  int c = 0;
  while (digitalRead(BUTTON) == 1 && c < 3000) {
    c += 100;
    delay(100);
  }
  if (c >= 3000) {
    shouldUpload = !shouldUpload;
    if (shouldUpload) {
      lcd_print("Resuming sensor", 1, 0, true);
      lcd_print("data uploads", 2, 1);
    }
    else {
      lcd_print("Pausing sensor", 1, 0, true);
      lcd_print("data uploads", 2, 1);
    }
  }
  else {
    discreteMode = !discreteMode;
    if (discreteMode) {
      lcd_print("In Discrete", 2, 0, true);
      lcd_print("Mode", 5, 1);

    }
    else {
      lcd_print("In Indicator", 1, 0, true);
      lcd_print("Mode", 5, 1);
    }
    update_RGB(tempSum/tempCount);

  }
  delay(3000);
  update_lcd(tempSum / tempCount, humidSum / humidCount);
  delay(1000);

}

float update_data() {
  float light = analogRead(LDR);
  float temp = dht.readTemperature();
  float humid = dht.readHumidity();
  if (isnan(light) || isnan(humid) || isnan(temp)) {
    if (!isnan(tempSum / tempCount) && !isnan(humidSum / humidCount)) {
      update_lcd(tempSum / tempCount, humidSum / humidCount);
      return tempSum / tempCount;
    }
    else {
      lcd_print("Error reading", 1, 0, true);
      lcd_print("Sensors", 2, 1);
      return -1;
    }
  }
  tempSum += temp;
  lightSum += light;
  humidSum += humid;
  tempCount++;
  lightCount++;
  humidCount++;

  if (uploadFails <= 5) {
    update_lcd(temp, humid);
  }
  return temp;

}

void update_lcd(float temp, float humidity) {
  String tempString = String(temp, 1);
  String humidString = String(humidity, 1);
  lcd_print("Temp:", 0, 0, true);
  lcd_print(tempString, 10, 0);
  lcd_print("C", 10 + tempString.length(), 0);
  lcd_print("Humidity:", 0, 1);
  lcd_print(humidString, 10, 1);
  lcd_print("%", humidString.length() + 10, 1);

}


void rgb(int red, int green, int blue) {
  analogWrite(REDPIN, red);
  analogWrite(GREENPIN, green);
  analogWrite(BLUEPIN, blue);

}

String parse_post_data() {
  String postData;
  float tempAvg = tempSum / tempCount;
  float humidAvg = humidSum / humidCount;
  float lightAvg = lightSum / lightCount;
  if (isnan(humidAvg) || isnan(lightAvg) || isnan(tempAvg)) {
    postData = "f";
  }
  postData = "temp=";
  postData.concat(String(tempAvg, 1));
  postData.concat("&humid=");
  postData.concat(String(humidAvg, 1));
  postData.concat("&light=");
  postData.concat(String(lightAvg));
  return postData;
}

void setup() {
  pinMode(PIEZO, OUTPUT);
  pinMode(REDPIN, OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(BLUEPIN, OUTPUT);
  pinMode(BUTTON, INPUT);
  pinMode(LDR, INPUT);
  lcd.begin(16, 2);
  dht.begin();

  // initialize serial for ESP module
  setEspBaudRate(ESP_BAUDRATE);

  lcd_print(" Searching for ", 0, 0, true);
  lcd_print("   ESP8266...   ", 0, 1);


  // initialize ESP module
  WiFi.init(&Serial1);
  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    lcd_print(" Error! Module ", 0, 0, true);
    lcd_print("    Not Found   ", 0, 1);
    // don't continue - reboot required
    while (true);
  }
  lcd_print(" Module Located ", 0, 0, true);
  Serial.begin(115200);
  ThingSpeak.begin(client);  // Initialize ThingSpeak
}

void loop() {

  float t = update_data();
  update_RGB(t);
  Serial.println(timeTracker);
  Serial.println(uploadInterval);
  async_delay(READ_INTERVAL);
  timeTracker++;
  if (timeTracker >= uploadInterval) {
    upload_data(parse_post_data());
    timeTracker = 0;
  }
}
