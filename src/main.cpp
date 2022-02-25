#include <Arduino.h>
#include <Adafruit_SleepyDog.h>
#include <Tic.h>

// Project Includes
#include "Version.h"
#include <pinout.h>
#include <audio.h>
#include <radio.h>

// Button Header
// Include the Bounce2 library found here :
// https://github.com/thomasfredericks/Bounce2
#include <Bounce2.h>

// Library to read rotary shaft encoder (for user input, not attached to motor)
#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>

// INSTANTIATE A Bounce OBJECT
Bounce bounce = Bounce();

// SET A VARIABLE TO STORE THE LED STATE
int ledState = LOW;

// Instantiate Motor Controller Object
TicI2C motor;

// Encoder Setup
Encoder shaftEncoder(ENCODER_A, ENCODER_B);
// Last position of rotary shaft encoder for user input
long oldPosition = -999;
long lastChangeTime = 0;
#define ENCODER_BUFFER_SIZE 3
long encBuffer[ENCODER_BUFFER_SIZE];
int bufPos = 0;

long avgInterval(long interval)
{
  encBuffer[bufPos] = interval;
  bufPos++;
  if (ENCODER_BUFFER_SIZE <= bufPos)
  {
    bufPos = 0;
  }
  long sum = 0;
  for (int i=0; i<ENCODER_BUFFER_SIZE; i++)
  {
    sum = sum + encBuffer[i];
  }
  long avg = long(sum / ENCODER_BUFFER_SIZE);
  return avg;
}


// ███████╗███████╗████████╗██╗   ██╗██████╗
// ██╔════╝██╔════╝╚══██╔══╝██║   ██║██╔══██╗
// ███████╗█████╗     ██║   ██║   ██║██████╔╝
// ╚════██║██╔══╝     ██║   ██║   ██║██╔═══╝
// ███████║███████╗   ██║   ╚██████╔╝██║
// ╚══════╝╚══════╝   ╚═╝    ╚═════╝ ╚═╝

void setup()
{
  Serial.begin(9600);
  // while (!Serial)
  // {
  //   ; // wait for serial port to connect. Needed for native USB port only
  // }
  Serial.printf("\nProject version v%s, built %s\n", VERSION, BUILD_TIMESTAMP);
  Serial.println("Setup function commencing...");
  //vsAudioSetup();
  delay(100);
  radioSetup();

  // BOUNCE SETUP

  // SELECT ONE OF THE FOLLOWING :
  // 1) IF YOUR INPUT HAS AN INTERNAL PULL-UP
  bounce.attach(BOUNCE_PIN, INPUT_PULLUP); // USE INTERNAL PULL-UP
  // 2) IF YOUR INPUT USES AN EXTERNAL PULL-UP
  //bounce.attach( BOUNCE_PIN, INPUT ); // USE EXTERNAL PULL-UP

  // DEBOUNCE INTERVAL IN MILLISECONDS
  bounce.interval(5); // interval in ms

  // LED SETUP
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, ledState);

  // Motor Setup
  Wire.begin();
  delay(20);
  motor.exitSafeStart();
  motor.goHomeReverse();


  Watchdog.enable(4000);
  Serial.println("Setup Complete");
}

// ██╗      ██████╗  ██████╗ ██████╗
// ██║     ██╔═══██╗██╔═══██╗██╔══██╗
// ██║     ██║   ██║██║   ██║██████╔╝
// ██║     ██║   ██║██║   ██║██╔═══╝
// ███████╗╚██████╔╝╚██████╔╝██║
// ╚══════╝ ╚═════╝  ╚═════╝ ╚═╝

void loop()
{
  long newPosition = shaftEncoder.read();
  //Serial.println(newPosition);
  if (newPosition != oldPosition)
  {
    long interval = avgInterval(millis() - lastChangeTime);
    // Serial.print("Interval: ");
    // Serial.print(interval);
    lastChangeTime = millis();
    if (newPosition > oldPosition)
    {
      //Serial.println(F("Surfacing"));
      long speed = int((1000 / interval) * -5000000);
      motor.setTargetVelocity(speed);
      // Serial.print("Tgt Speed: ");
      // Serial.println(speed);
    }
    else
    {
      //Serial.println(F("Diving"));
      long speed = int((1000 / interval) * 5000000);
      motor.setTargetVelocity(speed);
      // Serial.print("Tgt Speed: ");
      // Serial.println(speed);
    }
    oldPosition = newPosition;
    // Serial.print("Cur Speed: ");
    // Serial.print(motor.getCurrentVelocity());
    // Serial.print(" Position: ");
    // Serial.println(motor.getCurrentPosition());
  }
  else
  {
    long idleTime = millis() - lastChangeTime;
    if (idleTime > 200)
    {
      motor.setTargetVelocity(0);
      //Serial.println("------------------------------Stopped---------------------------");
    }
    
  }

  motor.resetCommandTimeout();
  Watchdog.reset();
}