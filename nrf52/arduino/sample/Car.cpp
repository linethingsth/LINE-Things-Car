#include "car.h"

#define ADC_MAX 1024

//Car::Car()
//{
//	motorAF_ = MOTORAF_PIN;
//	motorAB_ = MOTORAB_PIN;
//	motorBF_ = MOTORBF_PIN;
//	motorBB_ = MOTORBB_PIN;
//	sensorL_ = SENSORL_PIN;
//	sensorR_ = SENSORR_PIN;
//
//	setup();
//}

Car::Car(int pinMotorAF, int pinMotorAB, int pinMotorBF, int pinMotorBB, int sensorL, int sensorR)
{
	motorAF_ = pinMotorAF;
	motorAB_ = pinMotorAB;
	motorBF_ = pinMotorBF;
	motorBB_ = pinMotorBB;
	sensorL_ = sensorL;
	sensorR_ = sensorR;

	setup();
}

void Car::setup()
{
	pinMode(motorAF_, OUTPUT);
	pinMode(motorAB_, OUTPUT);
	pinMode(motorBF_, OUTPUT);
	pinMode(motorBB_, OUTPUT);
	stopAndSetWheelsStraight();

	pinMode(sensorL_, INPUT);
	pinMode(sensorR_, INPUT);
}

void Car::stop()
{
	analogWrite(motorAF_, 0);
	analogWrite(motorAB_, 0);
}

void Car::setWheelsStraight()
{
	analogWrite(motorBF_, 0);
	analogWrite(motorBB_, 0);
}

void Car::stopAndSetWheelsStraight()
{
	stop();
	setWheelsStraight();
}

void Car::goForward(int speed)
{
	analogWrite(motorAF_, speed * SPEED_SCALE);
	analogWrite(motorAB_, 0);
}

void Car::goBackward(int speed)
{
	analogWrite(motorAF_, 0);
	analogWrite(motorAB_, speed * SPEED_SCALE);
}

void Car::turnLeft(int value)
{
	analogWrite(motorBF_, value * SPEED_SCALE);
	analogWrite(motorBB_, 0);	
}

void Car::turnRight(int value)
{
	analogWrite(motorBF_, 0);
	analogWrite(motorBB_, value * SPEED_SCALE);
}

int Car::readLeftSensor()
{
  //Serial.printf("Left sensor value %d\n", analogRead(sensorL_));
	return map(ADC_MAX - (uint16_t)analogRead(sensorL_), 0, ADC_MAX, 0, 100);
}

int Car::readRightSensor()
{
  //Serial.printf("Right sensor value %d\n", analogRead(sensorR_));
	return map(ADC_MAX - (uint16_t)analogRead(sensorR_), 0, ADC_MAX, 0, 100);
}
