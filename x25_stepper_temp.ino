//----------------------------------------------------------------------
// https://github.com/clearwater/SwitecX25
//
// Using the SwitchX25 library.
// It zero's the motor, sets the position to mid-range
// and waits for serial input to indicate new motor positions.
//
// Open the serial monitor and try entering values
// between 0 and 944.
//
// Note that the maximum speed of the motor will be determined
// by how frequently you call update().  If you put a big slow
// serial.println() call in the loop below, the motor will move
// very slowly!
//----------------------------------------------------------------------
#include <SwitecX25.h>

// #define LOGS            // Comment out this line if want to remove serial code!


/**
   DEFINES
*/
// Motor pis
#define MOTOR_PIN1      4
#define MOTOR_PIN2      5
#define MOTOR_PIN3      6
#define MOTOR_PIN4      7

#define RESERV_LED_PIN  8

#define SENSOR_PIN      A0        // датчик ДУТ
#define AVERAGE_FACTOR  0         // коэффициент сглаживания показаний (0 = не сглаживать)

#define POLLING_RATE    500       // Milliseconds 

#define STEPS          (315*3)    // Standard X25.168 range 315 degrees at 1/3 degree steps

#define MOTOR_ZERO      35   //положение стрелки мин.
#define MOTOR_MAX       285  //положение стрелки макс.
#define SENSOR_ZERO     14   //цифра ДУТ мин.
#define SENSOR_MAX      454  //цифра ДУТ макс.

// MOTOR 50% THRASHOLD
#define MOTOR_HALF_PERCENT_TH   165 //положение стрелки  *1/2* бака

#define IN_RESERVE_TH   35        // лампа сигнализации резерва


/**
   GLOBALS
*/
// For motors connected to digital pins 4,5,6,7
SwitecX25 motor(STEPS, MOTOR_PIN1, MOTOR_PIN2, MOTOR_PIN3, MOTOR_PIN4);
unsigned long last_check  = 0;
int motor_position        = 0;
int last_sensor_val       = 0;
int led_status            = LOW;

/**
   SETUP
*/
void setup(void)
{
#ifdef LOGS
  Serial.begin(9600);
#endif
  digitalWrite(RESERV_LED_PIN, led_status);
  pinMode(RESERV_LED_PIN, OUTPUT);
  
  // Run the motor against the stops
  motor.zero();
  // Set the motor to the defined zero
  motor.setPosition(MOTOR_ZERO);

  // Init last_check
  last_check = millis();

#ifdef LOGS
  Serial.println("GO!");
#endif
}


/**
   MAIN LOOP
*/
void loop(void)
{
  // Read past time
  unsigned long now = millis();

  // Check sensors and update the position only after polling rate expires
  if (now - last_check > POLLING_RATE) {

    // Read sensor value
    int val = analogRead(SENSOR_PIN);

    if (val >= SENSOR_ZERO && val <= SENSOR_MAX) {
      // Invert the value
      val = SENSOR_MAX - val;

      // Update only if the values differs
      if (abs(val - last_sensor_val) > AVERAGE_FACTOR) {
        last_sensor_val = val;

        // Using different mapping
        if (last_sensor_val < 50) {
          motor_position = map(last_sensor_val, SENSOR_ZERO, 50, 260, 330);
        } else if (last_sensor_val >= 50 && last_sensor_val < 100) {
          motor_position = map(last_sensor_val, 50, 100, 205, 260);
        } else if (last_sensor_val >= 100 && last_sensor_val < 150) {
          motor_position = map(last_sensor_val, 100, 150, 163, 205);
        } else if (last_sensor_val >= 150 && last_sensor_val < 200) {
          motor_position = map(last_sensor_val, 150, 200, 130, 163);
        } else if (last_sensor_val > 200) {
          motor_position = map(last_sensor_val, 200, SENSOR_MAX, 163, MOTOR_MAX);
        }
 
        // Check if the motor position is in the range
        if (motor_position <= MOTOR_MAX && motor_position >= MOTOR_ZERO) {
          // Set new position
          motor.setPosition(motor_position);
          last_check = now;
        }
#ifdef LOGS
        else {
          Serial.println("Motor out of range!");
        }
#endif
      }
    }
#ifdef LOGS
    else {
      Serial.println("Sensor out of range!");
    }
#endif

#ifdef LOGS
    char buff[50];
    sprintf(buff, "RAW|AVERAGE SENSOR: %04d|%04d  ->  MOTOR: %04d", val, last_sensor_val, motor_position);
    Serial.println(buff);
#endif
  }

  // The motor only moves when you call update
  motor.update();
}
