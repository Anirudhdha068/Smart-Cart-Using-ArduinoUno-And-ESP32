//ESP32 Code For Smart Cart

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* ssid = "xyz"; // Your WiFi SSID
const char* password = "1234567890"; // Your WiFi password

// Google Sheets API credentials
const char* apiKey = "AIzaSyCiTnrH49fgDlz-Cr57DhxkF_jJTgPL-Z8"; // Your Google Sheets API key
const char* spreadsheetId = "1ayVLaSG0FPxFfTZ_3wo_8rql-cN_Mhh8_f-6cywWFWw"; // Your Google Sheets ID
const char* range = "SmartCart!A2:C"; // The range of cells to read from the Google Sheet

// Variable to store the rows of data retrieved from Google Sheets
JsonArray rows;

void setup() {
  // Start the Serial communication for debugging
  Serial.begin(115200);      // Set baud rate to 115200 for faster communication

  // Connect to WiFi
  WiFi.begin(ssid, password); // Start WiFi connection

  // Wait until the ESP32 is connected to WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000); // Wait for 1 second
    Serial.println("Connecting to WiFi..."); // Print status
  }

  Serial.println("Connected to WiFi"); // Print success message
  getDataFromSheet(); // Fetch data from Google Sheets
}

void loop() {
  // Check if there is data available from the Serial input
  if (Serial.available()) {
    String barcode = Serial.readStringUntil('\n'); // Read the barcode until a newline character
    barcode.trim(); // Remove any leading or trailing whitespace
    Serial.println("Received barcode: " + barcode); // Print the received barcode for debugging
    processBarcode(barcode); // Process the scanned barcode
    delay(200); // Add a small delay to avoid rapid loops
  }
}

// Function to fetch data from Google Sheets
void getDataFromSheet() {
  // Check if the ESP32 is connected to WiFi
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http; // Create an HTTPClient object
    // Construct the URL for the Google Sheets API request
    String url = "https://sheets.googleapis.com/v4/spreadsheets/" + String(spreadsheetId) + "/values/" + String(range) + "?key=" + String(apiKey);
    http.begin(url); // Initialize the HTTP request
    http.setTimeout(20000); // Set timeout to 20 seconds

    int httpCode = http.GET(); // Send the GET request
    Serial.print("HTTP Response code: "); // Print the HTTP response code
    Serial.println(httpCode);

    // Check if the response is successful
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString(); // Get the response payload
      Serial.println("Received data:"); // Print received data for debugging
      Serial.println(payload);

      // Create a JSON document to parse the response
      DynamicJsonDocument doc(4096); // Allocate memory for the JSON document
      auto error = deserializeJson(doc, payload); // Parse the JSON

      // Check for JSON parsing errors
      if (error) {
        Serial.println("JSON Parse Error: " + String(error.c_str())); // Print error message
      } else {
        rows = doc["values"].as<JsonArray>(); // Store the rows of data in the 'rows' variable
      }
    } else {
      Serial.println("Error on HTTP request"); // Print error message if the request fails
      Serial.println(http.errorToString(httpCode)); // Print the error string
    }
    http.end(); // End the HTTP request
  } else {
    Serial.println("WiFi not connected"); // Print message if WiFi is not connected
  }
}

// Function to process the scanned barcode
void processBarcode(String barcode) {
  // Loop through each row of data retrieved from Google Sheets
  for (JsonArray row : rows) {
    // Check if the scanned barcode matches the first column (barcode) in the row
    if (barcode == row[0].as<String>()) {
      String productName = row[1].as<String>(); // Get the product name from the second column
      float productPrice = row[2].as<float>(); // Get the product price from the third column

      // Construct the response string with product name and price
      String response = productName + "," + String(productPrice);
      Serial.println(" Sending response: " + response);  // Debug print
      Serial.println(response);  // Send response to Arduino Uno
      return; // Exit after sending the response
    }
  }
  Serial.println("Sending response: NOT_FOUND");  // Debug print
  Serial.println("NOT_FOUND"); // Send not found response
}
