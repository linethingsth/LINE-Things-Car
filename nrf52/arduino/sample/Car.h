#ifndef CAR_H
#define CAR_H

#include "Arduino.h"

// default value pin for nRF52840

#define MOTORA_PIN 15
#define MOTORB_PIN 25

#define MOTORAF_PIN 15
#define MOTORAB_PIN 16
#define MOTORBF_PIN 25
#define MOTORBB_PIN 26

#define SENSORL_PIN A0
#define SENSORR_PIN A3

#define SPEED_SCALE 50

class Car
{
	public :
	  //Car();
	  Car(int pinMotorAF, int pinMotorAB, int pinMotorBF, int pinMotorBB, int sensorL, int sensorR);
	  void stop();
	  void setWheelsStraight();
	  void stopAndSetWheelsStraight();

	  void goForward(int speed);
	  void goBackward(int speed);
	  
	  void turnLeft(int value);
	  void turnRight(int value);

	  int readLeftSensor();
	  int readRightSensor();

	private :
	  void setup();

	  int motorAF_ = 0;
	  int motorAB_ = 0;
	  int motorBF_ = 0;
	  int motorBB_ = 0;

	  int sensorL_ = 0;
	  int sensorR_ = 0;
};

#endif
