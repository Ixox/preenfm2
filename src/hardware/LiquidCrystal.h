#ifndef LiquidCrystal_h
#define LiquidCrystal_h

#include "RingBuffer.h"
#include "stm32f4xx_gpio.h"


#ifndef BOOTLOADER
#define LCDACTION_BUFFER_SIZE 128
#else
#define LCDACTION_BUFFER_SIZE 1
#endif

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00


struct LCDAction  {
    unsigned char value;
    unsigned char mode;
    unsigned char clear;
};

struct pin {
	GPIO_TypeDef* gpio;
	unsigned char pinNumber;
};

class LiquidCrystal {
public:
  LiquidCrystal();
  virtual ~LiquidCrystal() {};

  void begin(unsigned char cols, unsigned char rows);

  void clear();
  void home();

  void noDisplay();
  void display();
  void noBlink();
  void blink();
  void noCursor();
  void cursor();
  void scrollDisplayLeft();
  void scrollDisplayRight();
  void leftToRight();
  void rightToLeft();
  void autoscroll();
  void noAutoscroll();

  void createChar(unsigned char, unsigned char[]);
  void setCursor(unsigned char, unsigned char);
  void command(unsigned char);

  void setRealTimeAction(bool realTime) { this->realTimeDisplay = realTime; }
  bool hasActions() { return (lcdActions.getCount()>0); }
  void clearActions() { lcdActions.clear(); }
  LCDAction nextAction();

  // copied from Print
  void write(unsigned char);
  void print(char c);
  void print(const char str[]);
  void print(int n);
  void print(unsigned int n);
  void print(float f);
  void printWithOneDecimal(float f);
  void realTimeAction(LCDAction *action, void (*callback)());

private:
  void send(unsigned char, bool);
  void sendInitCommand(unsigned char value);
  void delay(unsigned char delay);
  void write4bits(unsigned char);
  void pulseEnable(int delay);

  bool realTimeDisplay;

  struct pin _rs_pin; // LOW: command.  HIGH: character.
  struct pin _enable_pin; // activated by a HIGH pulse.
  struct pin _data_pins[4];

  unsigned char _displayfunction;
  unsigned char _displaycontrol;
  unsigned char _displaymode;

  unsigned char _initialized;

  unsigned char _numlines,_currline;
  int delayAfterCommand;
  RingBuffer<LCDAction, LCDACTION_BUFFER_SIZE> lcdActions;
};

#endif
