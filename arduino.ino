//Arduino Uno Code For Smart Cart

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <hidboot.h>
#include <usbhub.h>
#include <SoftwareSerial.h>
#include "Adafruit_Thermal.h"

LiquidCrystal_I2C lcd(0x27, 16, 4);
USB usb;
HIDBoot<USB_HID_PROTOCOL_KEYBOARD> keyboard(&usb);

// Thermal printer on pins 5 (RX), 6 (TX)
SoftwareSerial mySerial(5, 6); // RX, TX
Adafruit_Thermal printer(&mySerial);

String barcode = "";
unsigned long lastCharTime = 0;
const unsigned long timeout = 500;

struct Product {
  String barcode;
  String productName;
  float price;
  int count;
};

Product products[] = {
  {"00000", "Product A", 20.0}, // Add Product
  {"00000", "Product B", 30.0}, // Add Product
  {"00000", "Product C", 15.0}  // Add Product
};

int numProducts = sizeof(products) / sizeof(products[0]);

int getProductIndexByBarcode(String bc) {
  for (int i = 0; i < numProducts; i++) {
    if (products[i].barcode == bc) {
      return i;
    }
  }
  return -1;
}

class KeyboardParser : public KeyboardReportParser {
  void OnKeyDown(uint8_t mod, uint8_t key) override {
    uint8_t ascii = OemToAscii(mod, key);
    if (ascii) {
      char c = (char)ascii;
      barcode += c;
      lastCharTime = millis();
      Serial.print(c);
    }
  }
};

KeyboardParser parser;

void printCartSummary() {
  float grandTotal = 0;
  int totalItems = 0;

  printer.setSize('M');
  printer.justify('C');
  printer.println(F("==== INVOICE ===="));
  Serial.print(F("==== INVOICE ===="));
  printer.println();

  printer.justify('L');
  for (int i = 0; i < numProducts; i++) {
    if (products[i].count > 0) {
      float total = products[i].price * products[i].count;
      printer.print(products[i].productName);
      printer.print(" x");
      Serial.print("x");
      printer.print(products[i].count);
      Serial.print(products[i].count);
      printer.println();
      Serial.println();
      printer.print("  @ ₹");
      Serial.print("  @ ₹");
      printer.print(products[i].price, 2);
      Serial.print(products[i].price, 2);
      printer.print(" = ₹");
      Serial.print("  = ₹");
      printer.println(total, 2);
      Serial.println(total, 2);

      grandTotal += total;
      totalItems += products[i].count;
    }
  }

  printer.println();
  Serial.println();
  printer.println(F("----------------"));
  Serial.println(F("----------------"));
  printer.print("Total Items: ");
  Serial.print("Total Items: ");
  printer.println(totalItems);
  Serial.println(totalItems);
  printer.print("Total Amount: ₹");
  Serial.print("Total Amount: ₹");
  printer.println(grandTotal, 2);
  Serial.println(grandTotal, 2);
  printer.println(F("================"));
  Serial.println(F("================"));
  printer.feed(3);
 
}

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Scan barcode...");
  lcd.setCursor(0, 1);
  lcd.print("Waiting...");

  mySerial.begin(19200); // Baud rate for printer
  printer.begin();

  if (usb.Init() == -1) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("USB init failed");
    while (1);
  }

  keyboard.SetReportParser(0, &parser);
}

void loop() {
  usb.Task();

  if (barcode.length() > 0 && (millis() - lastCharTime > timeout)) {
    Serial.println("\n[Scanned]: " + barcode);

    if (barcode == "3366") {   //Check Out Barcode
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Printing Bill...");
      printCartSummary();  // 🧾 Trigger Thermal Print
      delay(3000);
    } else {
      int index = getProductIndexByBarcode(barcode);
      lcd.clear();

      if (index != -1) {
        products[index].count++;
        float itemTotal = products[index].count * products[index].price;

        float grandTotal = 0;
        int totalItems = 0;
        for (int i = 0; i < numProducts; i++) {
          grandTotal += products[i].count * products[i].price;
          totalItems += products[i].count;
        }

        lcd.setCursor(0, 0);
        lcd.print(products[index].productName.substring(0, 13));
        lcd.setCursor(14, 0);
        lcd.print("*" + String(products[index].count));

        lcd.setCursor(0, 1);
        lcd.print("Unit: ₹" + String(products[index].price, 2));

        lcd.setCursor(0, 2);
        lcd.print("ItemTotal: ₹" + String(itemTotal, 2));

        lcd.setCursor(0, 3);
        lcd.print("Total:₹" + String(grandTotal, 2));
        lcd.setCursor(10, 3);
        lcd.print("Items:" + String(totalItems));
      } else {
        lcd.setCursor(0, 0);
        lcd.print("Unknown Barcode");
        lcd.setCursor(0, 1);
        lcd.print(barcode);
      }
    }

    barcode = "";
  }
}
