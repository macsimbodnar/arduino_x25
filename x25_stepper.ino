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

#define SENSOR_PIN      A0
#define AVERAGE_FACTOR  0         // коэффициент сглаживания показаний (0 = не сглаживать)

#define POLLING_RATE    500       // Milliseconds 

#define STEPS          (315*3)    // Standard X25.168 range 315 degrees at 1/3 degree steps

#define MOTOR_ZERO      20
#define MOTOR_MAX       255
#define SENSOR_ZERO     0
#define SENSOR_MAX      380

// MOTOR 50% THRASHOLD
#define MOTOR_HALF_PERCENT_TH   165
#define SENSOR_HALF             ((SENSOR_MAX - SENSOR_ZERO) / 2)

#define IN_RESERVE_TH   34        // More or less position 45 on the motor


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

  // Check sensors and update the positon only after polling rate expires
  if (now - last_check > POLLING_RATE) {

    // Read sensor value
    int val = analogRead(SENSOR_PIN);

    if (val >= SENSOR_ZERO && val <= SENSOR_MAX) {
      // Invert the value
      val = SENSOR_MAX - val;

      // Update only if the values differs
      if (abs(val - last_sensor_val) > AVERAGE_FACTOR) {
        last_sensor_val = val;

        check_if_reserve(last_sensor_val);

        // Using different mapping if the sensor is under or above the 50%
        if (last_sensor_val > SENSOR_HALF) {
          // We are over the 50%
          motor_position = map(last_sensor_val, SENSOR_HALF, SENSOR_MAX, MOTOR_HALF_PERCENT_TH + 1, MOTOR_MAX);
        } else {
          // We are under 50%
          motor_position = map(last_sensor_val, SENSOR_ZERO, SENSOR_HALF, MOTOR_ZERO, MOTOR_HALF_PERCENT_TH);
        }

        // Check if the motor position is in the renge
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

  // the motor only moves when you call update
  motor.update();
}


void check_if_reserve(int sensor_val) {
  int new_led_status;
  if (sensor_val > IN_RESERVE_TH) {
    // OK
    new_led_status = HIGH;
  } else {
    // In reserve, turn on LED
    new_led_status = LOW;
  }

  // Change the led status only if differrent from the previous check
  if (new_led_status != led_status) {
    led_status = new_led_status;
    digitalWrite(RESERV_LED_PIN, led_status);
  }
}
