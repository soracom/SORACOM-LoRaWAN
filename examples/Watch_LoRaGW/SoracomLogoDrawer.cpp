#include "Arduino.h"
#include <LiquidCrystal.h>
#include "SoracomLogoDrawer.h"

SoracomLogoDrawer::SoracomLogoDrawer(LiquidCrystal& liquidCrystal): lcd(liquidCrystal) {
}

void SoracomLogoDrawer::init() {
  const byte soracomTopLeft[] = {
    0b00000,
    0b00011,
    0b00110,
    0b01111,
    0b01000,
    0b11000,
    0b10000,
    0b10000
  };
  
  const byte soracomTopMiddle[] = {
    0b11111,
    0b00000,
    0b00000,
    0b11000,
    0b00111,
    0b00000,
    0b00000,
    0b00000
  };
  
  const byte soracomTopRight[] = {
    0b00000,
    0b11000,
    0b01100,
    0b00110,
    0b00010,
    0b10011,
    0b10001,
    0b10001
  };
  
  const byte soracomBottomLeft[] = {
    0b10000,
    0b10000,
    0b11001,
    0b01001,
    0b01100,
    0b00110,
    0b00011,
    0b00000
  };
  
  const byte soracomBottomMiddle[] = {
    0b00011,
    0b11110,
    0b00000,
    0b00001,
    0b10000,
    0b10010,
    0b01100,
    0b11111
  };
  
  const byte soracomBottomRight[] = {
    0b10001,
    0b00001,
    0b10011,
    0b11010,
    0b10110,
    0b01100,
    0b11000,
    0b00000
  };
  
  const byte soracomBottomMiddleBlink[] = {
    0b00011,
    0b11110,
    0b00000,
    0b00000,
    0b10000,
    0b10010,
    0b01100,
    0b11111
  };
  
  const byte soracomBottomRightBlink[] = {
    0b10001,
    0b00001,
    0b00011,
    0b10010,
    0b00110,
    0b01100,
    0b11000,
    0b00000
  };

  lcd.createChar(CHAR_TOP_LEFT, const_cast<byte*>(soracomTopLeft));
  lcd.createChar(CHAR_TOP_MIDDLE, const_cast<byte*>(soracomTopMiddle));
  lcd.createChar(CHAR_TOP_RIGHT, const_cast<byte*>(soracomTopRight));
  lcd.createChar(CHAR_BOTTOM_LEFT, const_cast<byte*>(soracomBottomLeft));
  lcd.createChar(CHAR_BOTTOM_MIDDLE, const_cast<byte*>(soracomBottomMiddle));
  lcd.createChar(CHAR_BOTTOM_RIGHT, const_cast<byte*>(soracomBottomRight));

  lcd.createChar(CHAR_BOTTOM_MIDDLE_BLINK, const_cast<byte*>(soracomBottomMiddleBlink));
  lcd.createChar(CHAR_BOTTOM_RIGHT_BLINK, const_cast<byte*>(soracomBottomRightBlink));
}

void SoracomLogoDrawer::drawLogo(int x, int y) {
  lcd.setCursor(x, y);
  lcd.write(CHAR_TOP_LEFT);
  lcd.write(CHAR_TOP_MIDDLE);
  lcd.write(CHAR_TOP_RIGHT);
  
  lcd.setCursor(x, y + 1);
  lcd.write(CHAR_BOTTOM_LEFT);
  lcd.write(CHAR_BOTTOM_MIDDLE);
  lcd.write(CHAR_BOTTOM_RIGHT); 

  lastX = x;
  lastY = y;
}

bool SoracomLogoDrawer::getBlink() {
  return blink;
}

bool SoracomLogoDrawer::blinkLogo() {
  return blinkLogo(lastX, lastY);
}

bool SoracomLogoDrawer::blinkLogo(int x, int y) {  
  lcd.setCursor(x + 1, y + 1);
  if (!blink) {
    lcd.write(CHAR_BOTTOM_MIDDLE_BLINK);
    lcd.write(CHAR_BOTTOM_RIGHT_BLINK);
  } else {
    lcd.write(CHAR_BOTTOM_MIDDLE);
    lcd.write(CHAR_BOTTOM_RIGHT);
  }
  blink = !blink;
  
  return blink;
}

