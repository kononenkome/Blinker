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
    bool reverse; //WeMos use LOW for ON builin led. reserse = true say that ON is LOW.
    bool single; //only one blink flag

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
        if (b->single) {
          b->detach_all();
          b->single = false;
        } else {    
          b->series_ticker.detach();
        }
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
      reverse = false;
      single = false;
      detach_all(); 
    }

    void setup(uint32_t _pin = LED_BUILTIN, bool _reverse = false)  { 
      pin = _pin;
      reverse = _reverse;
      pinMode(pin, OUTPUT); 
    };
    void detach_all() {
      detach();
      series_ticker.detach();
    }
    void on() {  
      setPin(reverse ? LOW : HIGH); //on WeMos D1 mini LED_BUILTIN light on LOW
    }
    void off() { 
      setPin(reverse ? HIGH : LOW); 
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

      attach_ms<Blinker*>(milliseconds, series_internal, this);
    }
    void blink_once(uint32_t milliseconds) {
      detach_all();
      series = 2;
      count = 0;
      single = true;
      
      attach_ms<Blinker*>(milliseconds, toggle_internal_series, this);
    }
};