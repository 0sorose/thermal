#define oled_clk 10
#define oled_dc 11
#define oled_rst 12
#define oled_cs 13

using namespace std;

class oled{
public:
  oled();
  ~oled();
  void update();
  void restart();
  void temp(uint8_t temp);
  void target(uint8_t target);
  void power(uint8_t power);
  void status(char status, bool value);

private:
  uint8_t d_temp;     // iron temp
  uint8_t d_target;   // temp target
  uint8_t d_power;    // % actuve heating time
  bool hot;       //can cause burns
  bool docked;
  bool idle;      // no temp variation within idle time
  bool error;
  bool ready;     // within %5 of target temp
  bool celcius;   // 0 = Farenheit 1 = Celcius
  void layout();  //draw static gui objects
};
