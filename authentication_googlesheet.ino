#include <Wire.h> 
#include <Adafruit_GFX.h> 
#include <Adafruit_SH110X.h> 
#include <Keypad.h> 
#include <WiFi.h> 
#include <HTTPClient.h> 
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
#define OLED_RESET    -1 
#define I2C_ADDRESS   0x3C 
#define IR_PIN 38   
#define GREEN_LED 7 
#define RED_LED 8 
#define BUZZER 6 
// WiFi Credentials 
const char* ssid = "Sinch"; 
const char* password = "Sinchana"; 
// Google Script Web App URL 
const char* scriptURL = 
"https://script.google.com/macros/s/AKfycbzTEEiCTLE_zfGDybjz9rOpbnVfqsiw0UU77Yx
N9kRLfwkZUgxmkpFTxLGJGQy6qzc6bQ/exec"; 
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, 
&Wire, OLED_RESET); 
// Keypad Setup 
const byte ROWS = 4; 
const byte COLS = 4; 
char keys[ROWS][COLS] = { 
{'1', '2', '3', 'A'}, 
{'4', '5', '6', 'B'}, 
{'7', '8', '9', 'C'}, 
{'*', '0', '#', 'D'} 
}; 
byte rowPins[ROWS] = {9, 10, 11, 12}; 
byte colPins[COLS] = {13, 14, 15, 16};   
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS); 
struct User { 
String id; 
String name; 
String password; 
bool isAdmin; 
}; 
User users[] = { 
{"AAAA", "Sinchu", "4304", true},   
{"ABAB", "Abhay", "1234", false}, 
{"ABCB", "Subhay", "1234", false}, 
{"ADDA", "Jujay", "1234", false} 
}; 
int totalUsers = sizeof(users) / sizeof(users[0]); 
String resetCode = "9999";   
bool systemLocked = false; 
bool lastUserWasAdmin = false;   
 
// Function Prototypes 
void displayFormattedMessage(String message); 
String getUserInput(bool masked); 
void sendToGoogleSheet(String id, String name, String access); 
 
void setup() { 
  Serial.begin(115200); 
  pinMode(IR_PIN, INPUT); 
  pinMode(GREEN_LED, OUTPUT); 
  pinMode(RED_LED, OUTPUT); 
  pinMode(BUZZER, OUTPUT); 
 
  if (!display.begin(I2C_ADDRESS, true)) { 
    Serial.println("OLED initialization failed!"); 
    while (1); 
  } 
 
  Serial.println("OLED initialized successfully!"); 
  display.clearDisplay(); 
  displayFormattedMessage("No Object"); 
 
  // WiFi Connect 
  WiFi.begin(ssid, password); 
  Serial.print("Connecting to WiFi"); 
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); 
    Serial.print("."); 
  } 
  Serial.println("Connected to WiFi!"); 
} 
 
void loop() { 
  if (systemLocked) { 
    Serial.println("System Locked!"); 
    displayFormattedMessage("System Locked"); 
 
    if (!lastUserWasAdmin) { 
      Serial.println("Access Denied! Only Admin Can Reset."); 
      displayFormattedMessage("Locked! Admin Only"); 
      while (true);   
    } 
 
    Serial.println("Admin: Enter Reset Code..."); 
    displayFormattedMessage("Enter Reset Code"); 
 
    while (true) { 
      String enteredCode = getUserInput(true);   
      if (enteredCode == resetCode) { 
        Serial.println("System Unlocked!"); 
        displayFormattedMessage("System Unlocked"); 
        delay(3000); 
        systemLocked = false; 
        displayFormattedMessage("No Object"); 
        return; 
      } else { 
        Serial.println("Wrong Reset Code!"); 
        displayFormattedMessage("Wrong Code!"); 
      } 
    } 
  } 
 
  int ir_value = digitalRead(IR_PIN); 
 
  if (ir_value == LOW) {   
    Serial.println("\nObject Detected!"); 
    displayFormattedMessage("Object Detected!"); 
    delay(2000); 
 
    Serial.println("Enter Your ID..."); 
    displayFormattedMessage("Enter ID"); 
 
    int idAttempts = 0; 
    String enteredID; 
    String userName = ""; 
    bool isAdmin = false; 
    String correctPassword = ""; 
 
    while (idAttempts < 3) { 
      enteredID = getUserInput(false);   
      Serial.print("Entered ID: "); 
      Serial.println(enteredID); 
 
      bool validUser = false; 
      for (int i = 0; i < totalUsers; i++) { 
        if (users[i].id == enteredID) { 
          userName = users[i].name; 
          correctPassword = users[i].password; 
          isAdmin = users[i].isAdmin; 
          validUser = true; 
          break; 
        } 
      } 
 
      if (validUser) { 
        Serial.print("User Identified - "); 
        Serial.println(userName); 
        displayFormattedMessage("User Located"); 
        lastUserWasAdmin = isAdmin; 
        break; 
      } else { 
        idAttempts++; 
        Serial.println("Invalid ID!"); 
        displayFormattedMessage("Invalid ID"); 
        digitalWrite(RED_LED, HIGH); 
        tone(BUZZER, 1000); 
        delay(500); 
        digitalWrite(RED_LED, LOW); 
        noTone(BUZZER); 
 
        // Log unauthorized attempt 
        sendToGoogleSheet(enteredID, "Unauthorized", "Denied"); 
      } 
    } 
 
    if (userName == "") { 
      Serial.println("Sorry! System Locked."); 
      displayFormattedMessage("Sorry!"); 
      delay(2000); 
      Serial.println("System Locked!"); 
      displayFormattedMessage("System Locked"); 
      systemLocked = true; 
      return; 
    } 
 
    Serial.println("Enter Password..."); 
    displayFormattedMessage("Enter Password"); 
 
    int passwordAttempts = 0; 
 
    while (passwordAttempts < 6) { 
      if (passwordAttempts == 3) {   
        Serial.println("Too many failed attempts! Wait 10 seconds..."); 
        displayFormattedMessage("Wait 10 sec"); 
        for (int i = 10; i > 0; i--) { 
          displayFormattedMessage("Wait " + String(i)); 
          delay(1000); 
        } 
        Serial.println("You can try again now."); 
        displayFormattedMessage("Try Again"); 
      } 
 
      String enteredPassword = getUserInput(true);   
 
      if (enteredPassword == correctPassword) { 
        Serial.println("Access Granted!"); 
        displayFormattedMessage("Access Granted"); 
 
        digitalWrite(GREEN_LED, HIGH); 
        delay(3000); 
 
        Serial.print("Welcome "); 
        Serial.println(userName); 
        displayFormattedMessage("Welcome " + userName); 
        delay(3000); 
 
        digitalWrite(GREEN_LED, LOW); 
 
        // Log successful access 
        sendToGoogleSheet(enteredID, userName, "Granted"); 
        return; 
      } else { 
        passwordAttempts++; 
        Serial.println("Wrong Password!"); 
        displayFormattedMessage("Wrong Password"); 
        digitalWrite(RED_LED, HIGH); 
        tone(BUZZER, 1000); 
        delay(500); 
        digitalWrite(RED_LED, LOW); 
        noTone(BUZZER); 
      } 
    } 
 
    Serial.println("Access Denied! System Locked."); 
    displayFormattedMessage("Access Denied"); 
    delay(3000); 
    displayFormattedMessage("System Locked"); 
    systemLocked = true; 
 
    // Log denied access 
    sendToGoogleSheet(enteredID, userName, "Denied"); 
  } else {   
    Serial.println("No Object."); 
    displayFormattedMessage("No Object"); 
  } 
 
  delay(1000); 
} 
 
void displayFormattedMessage(String message) { 
  display.clearDisplay(); 
  display.setTextSize(2); 
  display.setTextColor(SH110X_WHITE); 
  display.setCursor(0, 10); 
 
  String word = ""; 
  for (int i = 0; i < message.length(); i++) { 
    if (message[i] == ' ') { 
      display.println(word); 
      word = ""; 
    } else { 
      word += message[i]; 
    } 
  } 
  display.println(word); 
  display.display(); 
} 
 
String getUserInput(bool masked) { 
  String input = ""; 
  String maskedInput = ""; 
 
  Serial.print("Entering: "); 
  while (true) { 
    char key = keypad.getKey(); 
 
    if (key) { 
      if (key == '*') { 
        if (input.length() > 0) { 
          input.remove(input.length() - 1); 
          maskedInput.remove(maskedInput.length() - 1); 
          Serial.print("\b \b"); 
        } 
      } else if (key == '#') { 
        Serial.println(); 
        return input; 
      } else { 
        input += key; 
        maskedInput += "*"; 
        Serial.print(key); 
      } 
 
      displayFormattedMessage(masked ? maskedInput : input); 
      delay(300); 
    } 
  } 
} 
 
void sendToGoogleSheet(String id, String name, String access) { 
  if (WiFi.status() == WL_CONNECTED) { 
    HTTPClient http; 
    String url = String(scriptURL) + "?id=" + id + "&name=" + name + "&access=" + access; 
    http.begin(url); 
    int httpCode = http.GET(); 
    if (httpCode > 0) { 
      Serial.println("Data sent to Google Sheet"); 
    } else { 
      Serial.println("Failed to send data to Google Sheet"); 
    } 
    http.end(); 
  } else { 
    Serial.println("WiFi not connected"); 
  } 
} 
