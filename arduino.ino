#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <hidboot.h>
#include <usbhub.h>
#include "RTClib.h"  // RTC Library

// USB Host Shield setup
USB usb;
HIDBoot<USB_HID_PROTOCOL_KEYBOARD> keyboard(&usb);

// LCD Setup
LiquidCrystal_I2C lcd(0x27, 16, 4);

// Thermal Printer Serial
SoftwareSerial printerSerial(8, 9);

// ESP32 Serial Communication
SoftwareSerial espSerial(10, 11); // RX, TX with ESP32

// RTC Module
RTC_DS3231 rtc;

String scannedBarcode;
float totalBill = 0.0;
String checkoutBarcode = "3366"; // Barcode for checkout

// Product storage
struct Product {
    String name;
    float price;
    int quantity;
};
Product cart[10];
int cartSize = 0;

void setup() {
    Serial.begin(115200);
    printerSerial.begin(115200);
    espSerial.begin(115200);

    // LCD setup
    lcd.begin(16, 4);
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Smart Cart Ready");

    // USB Host Shield Initialization
    if (usb.Init() == -1) {
        Serial.println("USB Host Shield Initialization Failed!");
        while (1); // Halt if initialization fails
    }

    keyboard.SetReportParser(0, (HIDReportParser*)&keyboard);

    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));  // Set RTC to compile time
}

void loop() {
    usb.Task(); // Handle USB tasks

    // Check if a key has been pressed
    if (keyboard.available()) {
        String barcode = keyboard.readStringUntil('\n'); // Read the barcode
        processBarcode(barcode); // Process the scanned barcode
    }

    // Check for response from ESP32
    if (espSerial.available()) {
        String response = espSerial.readStringUntil('\n');
        response.trim();
        Serial.println("Received response: " + response);  // Debug print
        processResponse(response);
        delay(200);  // Add a small delay to avoid rapid loops
    }
}

// Process Barcode
void processBarcode(String barcode) {
    barcode.trim();
    Serial.println("Scanned Barcode: " + barcode);  // Debug print to check the scanned barcode

    if (barcode == checkoutBarcode) {
        printBill();
    } else {
        Serial.println("Sending barcode to ESP32: " + barcode);  // Debug print
        espSerial.println(barcode);  // Send barcode to ESP32
    }
}

// Process Response from ESP32
void processResponse(String response) {
    if (response == "NOT_FOUND") {
        Serial.println("Product Not Found!");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Product Not Found");
    } else {
        int commaIndex = response.indexOf(',');
        if (commaIndex == -1) {
            Serial.println("Invalid response format");
            return; // Exit if the response format is invalid
        }

        String productName = response.substring(0, commaIndex);
        float productPrice = response.substring(commaIndex + 1).toFloat();

        addToCart(productName, productPrice);
        totalBill += productPrice;

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(productName);
        lcd.setCursor(0, 1);
        lcd.print("Price: $" + String(productPrice));
        Serial.println("Added: " + productName);
    }
}

// Add Product to Cart
void addToCart(String name, float price) {
    for (int i = 0; i < cartSize; i++) {
        if (cart[i].name == name) {
            cart[i].quantity++;
            return; // Exit if product already exists in the cart
        }
    }
    if (cartSize < 10) { // Check if there is space in the cart
        cart[cartSize] = {name, price, 1};
        cartSize++;
    } else {
        Serial.println("Cart is full! Cannot add more products.");
    }
}

// Print the final bill
void printBill() {
    Serial.println("Printing Bill...");

    DateTime now = rtc.now();
    String dateTime = String(now.day()) + "/" + String(now.month()) + "/" + String(now.year()) + " " +
                      String (now.hour()) + ":" + String(now.minute());

    String transactionID = String(now.unixtime());

    printerSerial.println("***************");
    printerSerial.println("  Smart Cart  ");
    printerSerial.println("***************");
    printerSerial.println("Date: " + dateTime);
    printerSerial.println("Transaction ID: " + transactionID);
    printerSerial.println("----------------");

    for (int i = 0; i < cartSize; i++) {
        printerSerial.println(cart[i].name);
        printerSerial.println("Qty: " + String(cart[i].quantity) + "  Price: $" + String(cart[i].price * cart[i].quantity));
        printerSerial.println("----------------");
    }

    printerSerial.println("Total: $" + String(totalBill));
    printerSerial.println("----------------");
    printerSerial.println("Thank You!");
    printerSerial.println("================");
}
