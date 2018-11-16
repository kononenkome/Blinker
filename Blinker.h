#include <Arduino.h> 
#include <Ticker.h> 

//#define DEBUGPRINT

#ifdef DEBUGPRINT
#define DEBUG_PRINT(x)  Serial.println (x)
#else
#define DEBUG_PRINT(x)
#endif

class Blinker : public Ticker {
  private:
    uint32_t pin;
    uint32_t count;
    uint32_t series; 
    uint32_t series_period;
    Ticker series_ticker;

    void setPin(uint32_t level) {
      detach_all();
      digitalWrite(pin, level);
    }
    static void toggle_internal(Blinker* b) { 
      digitalWrite(b->pin, !digitalRead(b->pin)); 
    }
     static void toggle_internal_series(Blinker* b) { 
      DEBUG_PRINT("toggle_internal_series " + String(b->series) + " " + String(b->count));
      if (b->count < b->series) {
        DEBUG_PRINT("blink");
        toggle_internal(b); 
        b->count++;
      } else {
        DEBUG_PRINT("detach series");        
        b->series_ticker.detach();
        b->count = 0;
      }
    }   
    static void series_internal(Blinker* b) {
      DEBUG_PRINT("series_internal");
      b->series_ticker.attach_ms<Blinker*>(b->series_period, toggle_internal_series, b);
    }

  public:
    Blinker() {
      pin = 0;
      detach_all(); 
    }

    void setup(uint32_t _pin = LED_BUILTIN)  { 
      pin = _pin;
      pinMode(pin, OUTPUT); 
    };
    void detach_all() {
      detach();
      series_ticker.detach();
    }
    void on() { 
      setPin(LOW); //on WeMos D1 mimi LED_BUILTIN light on LOW
    }
    void off() { 
      setPin(HIGH); 
    }
    void toggle() {
      detach_all();
      toggle_internal(this);
    }
    void blink(uint32_t milliseconds, uint32_t _series = 0, uint32_t _series_period = 100) {
      if (_series) {
        blink_series(milliseconds, _series, _series_period);
        return;
      }
      detach_all();
      attach_ms<Blinker*>(milliseconds, toggle_internal, this);
    }
    void blink_series(uint32_t milliseconds, uint32_t _series = 1, uint32_t _series_period = 100) {
      detach_all();
      series = _series * 2;
      count = 0;
      series_period = _series_period;
#ifdef DEBUGPRINT      
      if (repeat * series_ms >= milliseconds) {
        DEBUG_PRINT("WARNING: blinker series (" + String(series * series_period) + ") longer than period " + milliseconds);
      }
#endif      
      attach_ms<Blinker*>(milliseconds, series_internal, this);
    }
};