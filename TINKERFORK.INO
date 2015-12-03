/*****************************************************************************
 * Farzad Izadi Kharazi, EEC284 Fall2008, fizadikh@ucdavis.edu
 * This is the firmware for the Spark Core for my project.  It is comprised of
 * three main parts.  The first part is the stock "Tinker" firmware with the
 * following changes:
 *      1.  Added a function movestepper & registered it for cloud access
 *      2.  Removed cloud access for digitalread
 * The second part is the arduino stepper library, which has been
 * modified by someone to work with the Particle Core.  The third part is
 * my custom "driveStepperMotor" function.  It accepts in a string command
 * from the cloud, and then parses it into three main parts:
 *      1.  Motor rotation direction
 *      2.  Motor RPM
 *      3.  Number of steps desired
 * There is a lot of string cutting and conversion because the Particle
 * Core API can only receive strings via published functions.
 * *****************************************************************************/


#include <string>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#define STEPSPERREV 48 //stepper motor steps per revolution
#define ARDUINO_H

/* Beginning of Tinker code****************************************************/

/* Function prototypes -------------------------------------------------------*/
int tinkerDigitalRead(String pin);
int tinkerDigitalWrite(String command);
int tinkerAnalogRead(String pin);
int tinkerAnalogWrite(String command);
int driveStepperMotor(String command);

void setup()
{
	//Register all the Tinker functions - can only have 4 max at a time - Farzad
	//Particle.function("digitalread", tinkerDigitalRead);
	Particle.function("digitalwrite", tinkerDigitalWrite);
	Particle.function("analogread", tinkerAnalogRead);
	Particle.function("analogwrite", tinkerAnalogWrite);
	Particle.function("movestepper", driveStepperMotor);

}

/* This function loops forever --------------------------------------------*/
void loop()
{
	//Empty - we use function call from android API to call functions directly
}

/*******************************************************************************
 * Function Name  : tinkerDigitalRead
 * Description    : Reads the digital value of a given pin
 * Input          : Pin
 * Output         : None.
 * Return         : Value of the pin (0 or 1) in INT type
                    Returns a negative number on failure
 *******************************************************************************/
int tinkerDigitalRead(String pin)
{
	//convert ascii to integer
	int pinNumber = pin.charAt(1) - '0';
	//Sanity check to see if the pin numbers are within limits
	if (pinNumber< 0 || pinNumber >7) return -1;

	if(pin.startsWith("D"))
	{
		pinMode(pinNumber, INPUT_PULLDOWN);
		return digitalRead(pinNumber);
	}
	else if (pin.startsWith("A"))
	{
		pinMode(pinNumber+10, INPUT_PULLDOWN);
		return digitalRead(pinNumber+10);
	}
	return -2;
}

/*******************************************************************************
 * Function Name  : tinkerDigitalWrite
 * Description    : Sets the specified pin HIGH or LOW
 * Input          : Pin and value
 * Output         : None.
 * Return         : 1 on success and a negative number on failure
 *******************************************************************************/
int tinkerDigitalWrite(String command)
{
	bool value = 0;
	//convert ascii to integer
	int pinNumber = command.charAt(1) - '0';
	//Sanity check to see if the pin numbers are within limits
	if (pinNumber< 0 || pinNumber >7) return -1;

	if(command.substring(3,7) == "HIGH") value = 1;
	else if(command.substring(3,6) == "LOW") value = 0;
	else return -2;

	if(command.startsWith("D"))
	{
		pinMode(pinNumber, OUTPUT);
		digitalWrite(pinNumber, value);
		return 1;
	}
	else if(command.startsWith("A"))
	{
		pinMode(pinNumber+10, OUTPUT);
		digitalWrite(pinNumber+10, value);
		return 1;
	}
	else return -3;
}

/*******************************************************************************
 * Function Name  : tinkerAnalogRead
 * Description    : Reads the analog value of a pin
 * Input          : Pin
 * Output         : None.
 * Return         : Returns the analog value in INT type (0 to 4095)
                    Returns a negative number on failure
 *******************************************************************************/
int tinkerAnalogRead(String pin)
{
	//convert ascii to integer
	int pinNumber = pin.charAt(1) - '0';
	//Sanity check to see if the pin numbers are within limits
	if (pinNumber< 0 || pinNumber >7) return -1;

	if(pin.startsWith("D"))
	{
		return -3;
	}
	else if (pin.startsWith("A"))
	{
		return analogRead(pinNumber+10);
	}
	return -2;
}

/*******************************************************************************
 * Function Name  : tinkerAnalogWrite
 * Description    : Writes an analog value (PWM) to the specified pin
 * Input          : Pin and Value (0 to 255)
 * Output         : None.
 * Return         : 1 on success and a negative number on failure
 *******************************************************************************/
int tinkerAnalogWrite(String command)
{
	//convert ascii to integer
	int pinNumber = command.charAt(1) - '0';
	//Sanity check to see if the pin numbers are within limits
	if (pinNumber< 0 || pinNumber >7) return -1;

	String value = command.substring(3);

	if(command.startsWith("D"))
	{
		pinMode(pinNumber, OUTPUT);
		analogWrite(pinNumber, value.toInt());
		return 1;
	}
	else if(command.startsWith("A"))
	{
		pinMode(pinNumber+10, OUTPUT);
		analogWrite(pinNumber+10, value.toInt());
		return 1;
	}
	else return -2;
}

/* Stepper Motor Library****************************************************/

/*
  Stepper.h - - Stepper library for Wiring/Arduino - Version 0.4

  Original library     (0.1) by Tom Igoe.
  Two-wire modifications   (0.2) by Sebastian Gassner
  Combination version   (0.3) by Tom Igoe and David Mellis
  Bug fix for four-wire   (0.4) by Tom Igoe, bug fix from Noah Shibley

  Drives a unipolar or bipolar stepper motor using  2 wires or 4 wires

  When wiring multiple stepper motors to a microcontroller,
  you quickly run out of output pins, with each motor requiring 4 connections. 

  By making use of the fact that at any time two of the four motor
  coils are the inverse  of the other two, the number of
  control connections can be reduced from 4 to 2. 

  A slightly modified circuit around a Darlington transistor array or an L293 H-bridge
  connects to only 2 microcontroler pins, inverts the signals received,
  and delivers the 4 (2 plus 2 inverted ones) output signals required
  for driving a stepper motor.

  The sequence of control signals for 4 control wires is as follows:

  Step C0 C1 C2 C3
     1  1  0  1  0
     2  0  1  1  0
     3  0  1  0  1
     4  1  0  0  1

  The sequence of controls signals for 2 control wires is as follows
  (columns C1 and C2 from above):

  Step C0 C1
     1  0  1
     2  1  1
     3  1  0
     4  0  0

  The circuits can be found at 
  http://www.arduino.cc/en/Tutorial/Stepper
*/

// library interface description
class Stepper {
  public:
    // constructors:
    Stepper(int number_of_steps, int motor_pin_1, int motor_pin_2);
    Stepper(int number_of_steps, int motor_pin_1, int motor_pin_2, int motor_pin_3, int motor_pin_4);

    // speed setter method:
    void setSpeed(long whatSpeed);

    // mover method:
    void step(int number_of_steps);

    int version(void);

  private:
    void stepMotor(int this_step);

    int direction;        // Direction of rotation
    int speed;          // Speed in RPMs
    unsigned long step_delay;    // delay between steps, in ms, based on speed
    int number_of_steps;      // total number of steps this motor can take
    int pin_count;        // whether you're driving the motor with 2 or 4 pins
    int step_number;        // which step the motor is on

    // motor pin numbers:
    int motor_pin_1;
    int motor_pin_2;
    int motor_pin_3;
    int motor_pin_4;

    long last_step_time;      // time stamp in ms of when the last step was taken
};

/*
  Stepper.cpp - - Stepper library for Wiring/Arduino - Version 0.4

  Original library     (0.1) by Tom Igoe.
  Two-wire modifications   (0.2) by Sebastian Gassner
  Combination version   (0.3) by Tom Igoe and David Mellis
  Bug fix for four-wire   (0.4) by Tom Igoe, bug fix from Noah Shibley  

  Drives a unipolar or bipolar stepper motor using  2 wires or 4 wires

  When wiring multiple stepper motors to a microcontroller,
  you quickly run out of output pins, with each motor requiring 4 connections. 

  By making use of the fact that at any time two of the four motor
  coils are the inverse  of the other two, the number of
  control connections can be reduced from 4 to 2. 

  A slightly modified circuit around a Darlington transistor array or an L293 H-bridge
  connects to only 2 microcontroler pins, inverts the signals received,
  and delivers the 4 (2 plus 2 inverted ones) output signals required
  for driving a stepper motor.

  The sequence of control signals for 4 control wires is as follows:

  Step C0 C1 C2 C3
     1  1  0  1  0
     2  0  1  1  0
     3  0  1  0  1
     4  1  0  0  1

  The sequence of controls signals for 2 control wires is as follows
  (columns C1 and C2 from above):

  Step C0 C1
     1  0  1
     2  1  1
     3  1  0
     4  0  0

/*
 * two-wire constructor.
 * Sets which wires should control the motor.
 */
Stepper::Stepper(int number_of_steps, int motor_pin_1, int motor_pin_2)
{
  this->step_number = 0;      // which step the motor is on
  this->speed = 0;        // the motor speed, in revolutions per minute
  this->direction = 0;      // motor direction
  this->last_step_time = 0;    // time stamp in ms of the last step taken
  this->number_of_steps = number_of_steps;    // total number of steps for this motor

  // Arduino pins for the motor control connection:
  this->motor_pin_1 = motor_pin_1;
  this->motor_pin_2 = motor_pin_2;

  // setup the pins on the microcontroller:
  pinMode(this->motor_pin_1, OUTPUT);
  pinMode(this->motor_pin_2, OUTPUT);

  // When there are only 2 pins, set the other two to 0:
  this->motor_pin_3 = 0;
  this->motor_pin_4 = 0;

  // pin_count is used by the stepMotor() method:
  this->pin_count = 2;
}


/*
 *   constructor for four-pin version
 *   Sets which wires should control the motor.
 */

Stepper::Stepper(int number_of_steps, int motor_pin_1, int motor_pin_2, int motor_pin_3, int motor_pin_4)
{
  this->step_number = 0;      // which step the motor is on
  this->speed = 0;        // the motor speed, in revolutions per minute
  this->direction = 0;      // motor direction
  this->last_step_time = 0;    // time stamp in ms of the last step taken
  this->number_of_steps = number_of_steps;    // total number of steps for this motor

  // Arduino pins for the motor control connection:
  this->motor_pin_1 = motor_pin_1;
  this->motor_pin_2 = motor_pin_2;
  this->motor_pin_3 = motor_pin_3;
  this->motor_pin_4 = motor_pin_4;

  // setup the pins on the microcontroller:
  pinMode(this->motor_pin_1, OUTPUT);
  pinMode(this->motor_pin_2, OUTPUT);
  pinMode(this->motor_pin_3, OUTPUT);
  pinMode(this->motor_pin_4, OUTPUT);

  // pin_count is used by the stepMotor() method:  
  this->pin_count = 4;  
}

/*
  Sets the speed in revs per minute

*/
void Stepper::setSpeed(long whatSpeed)
{
  this->step_delay = 60L * 1000L / this->number_of_steps / whatSpeed;
}

/*
  Moves the motor steps_to_move steps.  If the number is negative, 
   the motor moves in the reverse direction.
 */
void Stepper::step(int steps_to_move)
{  
  int steps_left = abs(steps_to_move);  // how many steps to take

  // determine direction based on whether steps_to_mode is + or -:
  if (steps_to_move > 0) {this->direction = 1;}
  if (steps_to_move < 0) {this->direction = 0;}


  // decrement the number of steps, moving one step each time:
  while(steps_left > 0) {
  // move only if the appropriate delay has passed:
  if (millis() - this->last_step_time >= this->step_delay) {
      // get the timeStamp of when you stepped:
      this->last_step_time = millis();
      // increment or decrement the step number,
      // depending on direction:
      if (this->direction == 1) {
        this->step_number++;
        if (this->step_number == this->number_of_steps) {
          this->step_number = 0;
        }
      } 
      else { 
        if (this->step_number == 0) {
          this->step_number = this->number_of_steps;
        }
        this->step_number--;
      }
      // decrement the steps left:
      steps_left--;
      // step the motor to step number 0, 1, 2, or 3:
      stepMotor(this->step_number % 4);
    }
  }
}

/*
 * Moves the motor forward or backwards.
 */
void Stepper::stepMotor(int thisStep)
{
  if (this->pin_count == 2) {
    switch (thisStep) {
      case 0: /* 01 */
      digitalWrite(motor_pin_1, LOW);
      digitalWrite(motor_pin_2, HIGH);
      break;
      case 1: /* 11 */
      digitalWrite(motor_pin_1, HIGH);
      digitalWrite(motor_pin_2, HIGH);
      break;
      case 2: /* 10 */
      digitalWrite(motor_pin_1, HIGH);
      digitalWrite(motor_pin_2, LOW);
      break;
      case 3: /* 00 */
      digitalWrite(motor_pin_1, LOW);
      digitalWrite(motor_pin_2, LOW);
      break;
    } 
  }
  if (this->pin_count == 4) {
    switch (thisStep) {
      case 0:    // 1010
      digitalWrite(motor_pin_1, HIGH);
      digitalWrite(motor_pin_2, LOW);
      digitalWrite(motor_pin_3, HIGH);
      digitalWrite(motor_pin_4, LOW);
      break;
      case 1:    // 0110
      digitalWrite(motor_pin_1, LOW);
      digitalWrite(motor_pin_2, HIGH);
      digitalWrite(motor_pin_3, HIGH);
      digitalWrite(motor_pin_4, LOW);
      break;
      case 2:    //0101
      digitalWrite(motor_pin_1, LOW);
      digitalWrite(motor_pin_2, HIGH);
      digitalWrite(motor_pin_3, LOW);
      digitalWrite(motor_pin_4, HIGH);
      break;
      case 3:    //1001
      digitalWrite(motor_pin_1, HIGH);
      digitalWrite(motor_pin_2, LOW);
      digitalWrite(motor_pin_3, LOW);
      digitalWrite(motor_pin_4, HIGH);
      break;
    } 
  }
}

/*
  version() returns the version of the library:
*/
int Stepper::version(void)
{
  return 4;
}


/* Beginning of Farzad's Code********************************************************************/
int driveStepperMotor(String command)
{
    Stepper stepperMotor1(STEPSPERREV, D3, D2, D1, D0);
    int speedIndex = 0, stepsIndex = 0;
    char temp[20];
    
    //this steps through the command message and pulls out the speed and steps indexes in the string
    for(unsigned int index = 2; index < command.length(); index++){
        if (command.charAt(index) == 'f'){ //the 'f' is a terminating character for the speed portion of the message
            speedIndex = index;
        }
        else if (command.charAt(index) == 'g'){  //similarly, 'g' terminates the step portion
            stepsIndex = index;
            break;
        }
    }
    
    if (speedIndex == 0 || stepsIndex == 0) {return 0;}  //if you get an invalid command, exit
    
    //pull the snippets that have the values we want and convert them to integer
    unsigned int speed = atoi(command.substring(2, speedIndex));
    unsigned int steps = atoi(command.substring(speedIndex+1, stepsIndex));
    
    //check the motor direction and call the steping function to move the motors
    if (command.substring(0,2) == "cw"){
        stepperMotor1.setSpeed(speed);
        stepperMotor1.step(steps);
    }
    else if (command.substring(0,2) == "cc"){
        stepperMotor1.setSpeed(speed);
        stepperMotor1.step(-steps);
    }
    else {
        return 0;
    }
    
    return 1;
}
