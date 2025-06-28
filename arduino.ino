//Arduino Uno Code For Smart Cart

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <hidboot.h>
#include <usbhub.h>
#include "RTClib.h"  // RTC Library
#include <qrcode.h>  // Include QR Code library

// USB Host Shield setup
USB usb;
HIDBoot<USB_HID_PROTOCOL_KEYBOARD> keyboard(&usb); // Keep using HIDBoot

// LCD Setup
LiquidCrystal_I2C lcd(0x27, 16, 4); // Initialize LCD with I2C address 0x27

// Thermal Printer Serial
SoftwareSerial printerSerial(8, 9); // RX, TX pins for the thermal printer

// ESP32 Serial Communication
SoftwareSerial espSerial(10, 11); // RX, TX with ESP32

// RTC Module
RTC_DS3231 rtc; // Create an RTC object

String scannedBarcode;
float totalBill = 0.0;
String checkoutBarcode = "3366"; // Barcode for checkout

// Product storage
struct Product {
    String name;
    float price;
    int quantity;
};
Product cart[10]; // Array to hold products in the cart
int cartSize = 0; // Current number of products in the cart

// Forward declaration of processBarcode
void processBarcode(String barcode);

// Custom report parser for keyboard input
class MyKeyboardReportParser : public HIDReportParser {
public:
    void OnKeyPress(uint8_t mod, uint8_t key) {
        // Handle key press
        if (key == 0x28) { // Enter key
            processBarcode(scannedBarcode); // Process the scanned barcode
            scannedBarcode = ""; // Clear the scanned barcode
        } else {
            scannedBarcode += (char)key; // Append the character to the scanned barcode
        }
    }

    // Implement the pure virtual function from HIDReportParser
    void Parse(USBHID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf) override {
        // You can leave this empty or implement it if needed
    }
};

// Create an instance of the keyboard report parser
MyKeyboardReportParser keyboardParser;

void setup() {
    Serial.begin(115200); // Start serial communication for debugging
    printerSerial.begin(115200); // Start serial communication with the printer
    espSerial.begin(115200); // Start serial communication with the ESP32

    // LCD setup
    lcd.begin(16, 4); // Initialize the LCD
    lcd.backlight(); // Turn on the backlight
    lcd.setCursor(0, 0);
    lcd.print("Smart Cart Ready"); // Display initial message

    // USB Host Shield Initialization
    if (usb.Init() == -1) {
        Serial.println("USB Host Shield Initialization Failed!");
        while (1); // Halt if initialization fails
    }

    keyboard.SetReportParser(0, &keyboardParser); // Set up keyboard parser

    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));  // Set RTC to compile time
}

void loop() {
    usb.Task(); // Handle USB tasks

    // Check for response from ESP32
    if (espSerial.available()) {
        String response = espSerial.readStringUntil('\n');
        response.trim();
        Serial.println("Received response: " + response);  // Debug print
        processResponse(response);
        delay(200); // Add a small delay to avoid rapid loops
    }
}

// Process Barcode
void processBarcode(String barcode) {
    barcode.trim();
    Serial.println("Scanned Barcode: " + barcode);  // Debug print to check the scanned barcode

    if (barcode == checkoutBarcode) {
        printBill(); // Print the bill if the checkout barcode is scanned
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

        addToCart(productName, productPrice); // Add product to cart
        totalBill += productPrice; // Update total bill

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(productName); // Display product name on LCD
        lcd.setCursor(0, 1); // Corrected line
        lcd.print("Price: $" + String(productPrice)); // Display product price
        Serial.println("Added: " + productName);
    }
}

// Add Product to Cart
void addToCart(String name, float price) {
    for (int i = 0; i < cartSize; i++) {
        if (cart[i].name == name) {
            cart[i].quantity++; // Increment quantity if product already exists
            return; // Exit if product already exists in the cart
        }
    }
    if (cartSize < 10) { // Check if there is space in the cart
        cart[cartSize] = {name, price, 1}; // Add new product to cart
        cartSize++;
    } else {
        Serial.println("Cart is full! Cannot add more products.");
    }
}

// Print the final bill
void printBill() {
    Serial.println("Printing Bill...");

    DateTime now = rtc.now(); // Get current date and time from RTC
    String dateTime = String(now.day()) + "/" + String(now.month()) + "/" + String(now.year()) + " " +
                      String(now.hour()) + ":" + String(now.minute());

    String transactionID = String(now.unixtime()); // Generate a transaction ID based on the current time

    // Print bill header
    printerSerial.println("***************");
    printerSerial.println("  Smart Cart  ");
    printerSerial.println("***************");
    printerSerial.println("Date: " + dateTime);
    printerSerial.println("Transaction ID: " + transactionID);
    printerSerial.println("----------------");

    // Print each product in the cart
    for (int i = 0; i < cartSize; i++) {
        printerSerial.println(cart[i].name);
        printerSerial.println("Qty: " + String(cart[i].quantity) + "  Price: $" + String(cart[i].price * cart[i].quantity));
        printerSerial.println("----------------");
    }

    // Print total amount
    printerSerial.println("Total: $" + String(totalBill));
    printerSerial.println("----------------");

    // Generate UPI QR code for payment
    String upiID = "yourupiid@bank"; // Replace with your UPI ID
    String paymentInfo = "upi://pay?pa=" + upiID + "&pn=YourName&mc=1234&tid=" + transactionID + "&am=" + String(totalBill) + "&cu=INR&url=https://yourwebsite.com"; // Format UPI URL
    QRCode qrcode; // Create QR code object
    uint8_t qrcodeData[qrcode_getBufferSize(3)]; // Buffer for QR code data
    qrcode_initText(&qrcode, qrcodeData, 3, ECC_LOW, paymentInfo.c_str()); // Initialize QR code

    // Print QR code (assuming printer supports printing bitmaps)
    for (int y = 0; y < qrcode.size; y++) {
        for (int x = 0; x < qrcode.size; x++) {
            if (qrcode_getModule(&qrcode, x, y)) {
                printerSerial.print("##"); // Print filled module
            } else {
                printerSerial.print("  "); // Print empty module
            }
        }
        printerSerial.println(); // New line after each row
    }

    printerSerial.println("Thank You!");
    printerSerial.println("================");
}
