#include <motor_driver.h>

void robot_setup() {
  // Direction pins
  pinMode(LEFT_MOTOR_IN3,  OUTPUT);
  pinMode(LEFT_MOTOR_IN4,  OUTPUT);
  pinMode(RIGHT_MOTOR_IN1, OUTPUT);
  pinMode(RIGHT_MOTOR_IN2, OUTPUT);

  // PWM pins
  ledcAttach(LEFT_MOTOR_ENB,  freq, lresolution);
  ledcAttach(RIGHT_MOTOR_ENA, freq, lresolution);

  // Start stopped
  digitalWrite(LEFT_MOTOR_IN3,  LOW);
  digitalWrite(LEFT_MOTOR_IN4,  LOW);
  digitalWrite(RIGHT_MOTOR_IN1, LOW);
  digitalWrite(RIGHT_MOTOR_IN2, LOW);
  ledcWrite(LEFT_MOTOR_ENB,  0);
  ledcWrite(RIGHT_MOTOR_ENA, 0);
}

void setMotorSpeed(int i, int spd) {
  unsigned char reverse = 0;

  if (spd < 0) {
    spd = -spd;
    reverse = 1;
  }
  if (spd > 255) spd = 255;

  if (i == LEFT) {
    digitalWrite(LEFT_MOTOR_IN3, reverse == 0 ? HIGH : LOW);
    digitalWrite(LEFT_MOTOR_IN4, reverse == 0 ? LOW : HIGH);
    ledcWrite(LEFT_MOTOR_ENB, spd);
  } else {
    digitalWrite(RIGHT_MOTOR_IN1, reverse == 0 ? HIGH : LOW);
    digitalWrite(RIGHT_MOTOR_IN2, reverse == 0 ? LOW : HIGH);
    ledcWrite(RIGHT_MOTOR_ENA, spd);
  }
}

void setMotorSpeeds(int leftSpeed, int rightSpeed) {
  setMotorSpeed(LEFT,  leftSpeed);
  setMotorSpeed(RIGHT, rightSpeed);
}