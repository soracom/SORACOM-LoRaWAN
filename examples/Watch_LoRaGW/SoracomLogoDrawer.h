/**
 * Sample Usage:
 * 
 * LiquidCrystal lcd(LCD_RS, LCD_ENABLE, LCD_DB4, LCD_DB5, LCD_DB6, LCD_DB7);
 * SoracomLogoDrawer logoDrawer(lcd);
 * logoDrawer.init();
 * logoDrawer.drawLogo(0, 0);
 * logoDrawer.blinkLogo();
 */
#include "Arduino.h"

#ifndef SORACOM_LOGO_DRAWER_h
#define SORACOM_LOGO_DRAWER_h
#include <LiquidCrystal.h>

class SoracomLogoDrawer{
public:
  // char should be 0 to 7
  static const uint8_t CHAR_TOP_LEFT = 0;
  static const uint8_t CHAR_TOP_MIDDLE = 1;
  static const uint8_t CHAR_TOP_RIGHT = 2;
  static const uint8_t CHAR_BOTTOM_LEFT = 3;
  static const uint8_t CHAR_BOTTOM_MIDDLE = 4;
  static const uint8_t CHAR_BOTTOM_RIGHT = 5;
  static const uint8_t CHAR_BOTTOM_MIDDLE_BLINK = 6;
  static const uint8_t CHAR_BOTTOM_RIGHT_BLINK = 7;

  SoracomLogoDrawer(LiquidCrystal& liquidCrystal);

  /**
   * Register logo data as glyph chars
   */
  void init();

  /**
   * Draw the logo at the specified position
   */
  void drawLogo(int x, int y);
  
  /**
   * Toggle logo blinking
   */
  bool blinkLogo();
  bool blinkLogo(int x, int y);

  bool getBlink();
  
private:
  LiquidCrystal& lcd;
  bool blink = false;
  int lastX = 0;
  int lastY = 0;
};

#endif

