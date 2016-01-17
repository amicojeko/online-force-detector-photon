// This is part of the Online Force Detector Project

// Copyright (C) 2016  Stefano Guglielmetti

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

#include <vector>
#include <string>

#include "application.h"
#include "neopixel/neopixel.h"

SYSTEM_MODE(AUTOMATIC);

#define PIXEL_PIN D2
#define PIXEL_COUNT 8
#define PIXEL_TYPE WS2812B

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

int errors     = 0;
int max_errors = 5;
int elements   = 8;

void setup() {
  Particle.subscribe("hook-response/online_status", gotOnlineStatus, MY_DEVICES);
  Particle.subscribe("hook-response/test",          gotOnlineStatus, MY_DEVICES);

  #ifdef DEBUG
    Particle.publish("DEBUG", "App Started");
  #endif

  pixels.begin();

  rainbow(20);
  fadeout(20);
}

void loop() {

  #ifdef DEBUG
    Particle.publish("DEBUG", "Requesting online status");
  #endif

  Particle.publish("online_status");

  if(errors >= max_errors){
      panic();
  }

  delay(60000);
}

std::vector<float> parseColors(std::string colors_string){
  std::vector<float> colors;
  std::string delimiter = ";";
  size_t pos = 0;
  String token;

  while ((pos = colors_string.find(delimiter)) != std::string::npos && colors.size() <= 255) {
    token = String(colors_string.substr(0, pos).c_str());
    colors.push_back(token.toFloat());
    colors_string.erase(0, pos + delimiter.length());
  }

  return colors;
}

void gotOnlineStatus(const char *event, const char *data) {
  if (data){

    #ifdef DEBUG
      Particle.publish("DEBUG", "Got online status");
    #endif

    std::string input       = std::string(data);
    std::size_t timeout     = input.find("<title>504 Gateway Time-out</title>");
    std::size_t bad_gateway = input.find("<title>502 Bad Gateway</title>");
    std::vector<float> colors;

    if (timeout != -1) {

      #ifdef DEBUG
        Particle.publish("DEBUG", "TIMEOUT");
      #endif

      errors++;
      return;
    }

    if (bad_gateway != -1) {

      #ifdef DEBUG
        Particle.publish("DEBUG", "BAD GATEWAY");
      #endif

      errors++;
      return;
    }

    errors = 0;

    colors = parseColors(input);

    for(int i = 0; i < elements; i++) {
      pixels.setBrightness(255);
      pixels.setPixelColor(i, colors[i]);
      pixels.show();
    }

  } else {

    #ifdef DEBUG
      Particle.publish("DEBUG", "NO DATA");
    #endif

  }
}

void panic() {

    #ifdef DEBUG
    Particle.publish("DEBUG", "PANIC!!! ERRORS LIMIT REACHED!!!");
    #endif

    return;
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  Particle.publish("DEBUG", "Rainbow start");
  for(j=0; j<256; j++) {
    for(i=0; i<pixels.numPixels(); i++) {
      pixels.setPixelColor(i, Wheel((i+j) & 255));
    }
    pixels.show();
    delay(wait);
  }
  Particle.publish("DEBUG", "Rainbow end");
}

void fadeout(uint8_t wait) {
  Particle.publish("DEBUG", "Fade Out start");
  for(uint8_t i=255; i>0; i--) {
    pixels.setBrightness(i);
    pixels.show();
    delay(wait);
  }
  Particle.publish("DEBUG", "Fade Out end");
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
