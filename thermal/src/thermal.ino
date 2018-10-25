#include "Arduino.h"
#include "EEPROM.h"
#include "SPI.h"
#include "Wire.h"
#include "oled.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"

	// pin def
#define encodeA 2
#define encodeB 3
#define encodeP 4
#define holster_air 7
#define holster_irn 8
#define sel_air 9
#define sel_irn 0
#define fan_detect 1
#define temp_air A7
#define temp_irn A6

#define heat_air 6
#define heat_irn 5

#define ss 14
#define mosi 15
#define miso 16
#define sck 17

#define sda 27
#define scl 28

/*
*	Settings are stored in EEPROM:
*		0 - Soldering temp
*		1 - Soldering idle temp
*		2 - Soldering idle timeout
*		3 - Air temp
*		4 - Air idle temp
*		5 - Air idle timeout
*		6 - Degrees Celcius or Farenheit

*
*	They should not be overwritten unless the value is different to what's there
*/
#define display_framerate_divider 4		/* limits processing spent updating screen	0 = 1ms per frame, 1 = 2ms per frame, 2 = 4ms, 3 = 8ms, 4 = 16ms, etc.*/

using namespace std;

/* Global Variables */
const uint8_t duty_log_depth = 32;
uint16_t lastsetting[6];
uint16_t last_display_calc = 0;
uint16_t duty_log_times[duty_log_depth];
uint8_t duty_log_act[duty_log_depth];
uint16_t iron_temp_target = 0;
uint8_t settings[7];
uint16_t prev_duty_time = 0;
uint8_t duty_cyc = 0;
bool irn_active = false;
/* Global Variable ends*/


/*		TO DO:
*	Change duty_log time calculations to record change times rather than 1 sample per ms
*	Set up uart user interface
*	Set up visual interface once screen technology is known
*	Set up & test control loop functions
*/

void display_init(){	// set up display for use

}





void settings_init(){		// configure initial device settings
		// idea: imediately start heater for quick time to max temp?
}


void setup(){			//	arduino setup function
	pinMode(encodeA, INPUT);
	pinMode(encodeB, INPUT);
	pinMode(encodeP, INPUT);
	pinMode(holster_air, INPUT);
	pinMode(holster_irn, INPUT);
	pinMode(sel_air, INPUT);
	pinMode(sel_irn, INPUT);
	pinMode(fan_detect, INPUT);
	pinMode(temp_air, INPUT);
	pinMode(temp_irn, INPUT);

	pinMode(heat_air, OUTPUT);
	pinMode(heat_irn, OUTPUT);

	analogReference(INTERNAL);
	analogRead(temp_irn);
	analogRead(temp_irn);
	analogRead(temp_irn);	/* flushing inaccurate readings*/

	digitalWrite(heat_irn, HIGH);
	Serial.begin(19200);
	display_init();
}


uint16_t iron_temp_cel(){		//	reads resistance from thermistor, returns temp in celcius

	float reading = (float) analogRead(temp_irn);
	float work = ((reading * 0.6988) - 267.73);						/* the two formula coefficients based on resistor selection go in here */
	Serial.print("Iron temp reading: ");					/* printing results */
	Serial.print(reading);
	Serial.print(" count to Celcius = ");
	Serial.println(work);
	return (uint16_t) work;
}


uint16_t cel_to_far(uint16_t cel){	// converts temp in Celcius to Farenheit

	return (uint16_t) ((cel*9)/5 + 32);
}


uint8_t duty_log(){		// returns percentage duty over a number of readings given by 'duty_log_depth'

	if(millis() == prev_duty_time)	//only calculates once per ms
	{
		return duty_cyc;
	}
	/*to anylize what Percent of the time the iron is active*/
	/*add active times, add inactive times, calculate fraction etc.*/
	float act_time = 0;
	float inact_time = 0;
	float fraction = 0;
	uint8_t duty = 0;

	for (uint8_t i = 0; i < (duty_log_depth-1); i++) {
		/*	need to run calculations for every entry up to last one inside for loop,
				from last entry to current time handled outside	*/

				act_time += ((float) (duty_log_times[i + 1] - duty_log_times[i]) * (float) (duty_log_act[i]) / 256);
				act_time += ((float) (duty_log_times[i + 1] - duty_log_times[i]) * (float) (duty_log_act[i]) / 256);

			//act_time += (duty_log_times[i+1] - duty_log_times[i]);
			//inact_time += (duty_log_times[i+1] - duty_log_times[i]);
	}
	/*	now we handle from the most recent entry from the current time	*/
		act_time += ((float) (millis() - duty_log_times[duty_log_depth]) * (float) duty_log_act[duty_log_depth] / 256);	/*	>> 8 == div by 256	*/
		inact_time += ((float) (millis() - duty_log_times[duty_log_depth]) * (float) (256 - duty_log_act[duty_log_depth]) / 256);

	fraction = (float) act_time / (float) inact_time;
	duty = (uint8_t) (fraction * 100);

	Serial.println("Duty cycle calculation:\n");
	Serial.print("Active time : ");
	Serial.println(act_time);
	Serial.print("Inactive time : ");
	Serial.println(inact_time);
	Serial.print("Gives duty of : ");
	Serial.println(fraction);

	/*	Itterate all the duty measurements	*/
	for (uint8_t c = 0; c < (duty_log_depth - 1); c++)
	{
		duty_log_times[c] = duty_log_times[c + 1];
		duty_log_act[c] = duty_log_act[c + 1];
	}
	duty_log_times[(duty_log_depth - 1)] = millis();
	duty_log_act[(duty_log_depth - 1)] = irn_active;

	prev_duty_time = millis();
	duty_cyc = duty;
	return duty_cyc;
}


int menu_mem(uint8_t menu){
	switch (menu){
		case 20:
		return 0;
		break;

		case 210:
		return 1;
		break;

		case 211:
		return 2;
		break;

		case 22:
		return 3;
		break;

		case 23:
		return 4;
		break;

		case 24:
		return 5;
		break;

		default:
		return 0;
		break;
	}
}


uint8_t eeprom(uint8_t menu){
	uint8_t address = menu_mem(menu);
	return EEPROM.read(address);
}


void eeprom(uint8_t menu, uint8_t value){
	uint8_t address = menu_mem(menu);
	EEPROM.update(address, value);
	if (eeprom(menu) != value)
	{
		/*	eeprom address is faulty	*/
		Serial.print("EEPROM write error address: ");
		Serial.println(address);
	}
}


void disp_currenttemp(uint16_t current){
	/*	draw large text of current iron temp and degrees C or F*/
}


void disp_target_temp(uint16_t target){
	/*	draw small text below 	current temp with target in terms of same units as currenttemp*/
}


void displayOut()	/*	Reads system state and displays relevent info on screen	*/{
	/*	Need a large active temp display,
				Smaller target temperature
	*/
	/*	only calculate a new frame every 2^n ms, n = display_framerate_divider	*/
	if ((last_display_calc >> display_framerate_divider) == (millis() >> display_framerate_divider))
	{

	}
}


void iron_heater(uint16_t target){
	if (iron_temp_cel() < iron_temp_target)
	{
		if ((iron_temp_target - iron_temp_cel()) > 16 )
		{
			analogWrite(heat_irn, 255);
		}
		else
		{
			analogWrite(heat_irn, (((iron_temp_target - iron_temp_cel()) << 4) -1));
		}

	}
	else
	{
		analogWrite(heat_irn, 0);
	}


}


void serialEvent(){		//	takes valid chars from serial buff
	String inputString = "";
	bool reading = true;
  while (Serial.available() && reading) {
    char inChar = (char)Serial.read();
		if ((uint8_t)inChar < 0x20) {
			Serial.print("Message recieved: ");
			Serial.println(inputString);
			reading = false;
    }
		else{
    	inputString += inChar;
		}
  }
	//serial_interface(inputString);
}


// The loop function is called in an endless loop
void loop(){


}
