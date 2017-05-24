#ifndef VIEW_MANAGER_h
#define VIEW_MANAGER_h

#include <LiquidCrystal.h>
#include "Statistics.h"

enum ViewMode { 
  VIEW_MODE_COMBO, 
  VIEW_MODE_COUNT,
  VIEW_MODE_RATE, 
  
  // sentry (bampei) 
  VIEW_MODE_END,
  VIEW_MODE_BEGIN = VIEW_MODE_COMBO,
};

/**
 * Handles lcd view except Logo area (3x2)
 */
class ViewManager {
public:
  ViewManager(Statistics& statistics, LiquidCrystal& liquidCrystal) : stat(statistics), lcd(liquidCrystal) {    
  }

  void showGreetings() {
    //     01234567890abcde
    lcd.setCursor(4, 0);
    lcd.print("Starting");
    lcd.setCursor(4, 1); 
    lcd.print("  GW Monitor");     
  }

  void updateView() {
    switch (viewMode) {
      case VIEW_MODE_COMBO:
        updateComboView();
        break;
      case VIEW_MODE_RATE:
        updateRateView();
        break;
      case VIEW_MODE_COUNT:
        updateCountView();
        break;
      default:
        Serial.print("Unexpected viewMode=");
        Serial.println(viewMode);
        break;
    }
  }
  
  void updateComboView() {
    if (stat.getLastOk()) {
      //        01234567890abcde
      lcd.setCursor(4, 0);
      lcd.print("tx_ok          ");
      lcd.setCursor(4, 1);
      lcd.print(stat.getSequenceCount());
      lcd.print(" combo        ");
    } else {
      lcd.setCursor(4, 0);
      lcd.print("err            ");
      lcd.setCursor(4, 1);
      lcd.print(stat.getSequenceCount());
      lcd.print(" combo        ");
    }
  }
  
  void updateRateView() {
    //     01234567890abcde
    lcd.setCursor(4, 0);
    lcd.print("ok rate     ");
    lcd.setCursor(4, 1);
    lcd.print("=           ");  
    lcd.setCursor(5, 1);
    lcd.print(stat.getOkRate());  
  }
  
  void updateCountView() {
    //     01234567890abcde
    lcd.setCursor(4, 0);
    lcd.print(" ok=        ");
    lcd.setCursor(8, 0);
    lcd.print(stat.getOkCount());
    
    lcd.setCursor(4, 1);
    lcd.print("err=        ");
    lcd.setCursor(8, 1);
    lcd.print(stat.getErrCount());
  }
  
  ViewMode rotateViewMode() {
    // rotate enum view mode
    viewMode = static_cast<ViewMode>(static_cast<int>(viewMode) + 1);
    if (viewMode == VIEW_MODE_END)
      viewMode = VIEW_MODE_BEGIN;
      
    return viewMode;
  }
  
private:
  ViewMode viewMode = VIEW_MODE_COMBO;
  Statistics& stat;
  LiquidCrystal& lcd;
};

#endif


