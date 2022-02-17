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

// ███████╗███████╗████████╗██╗   ██╗██████╗
// ██╔════╝██╔════╝╚══██╔══╝██║   ██║██╔══██╗
// ███████╗█████╗     ██║   ██║   ██║██████╔╝
// ╚════██║██╔══╝     ██║   ██║   ██║██╔═══╝
// ███████║███████╗   ██║   ╚██████╔╝██║
// ╚══════╝╚══════╝   ╚═╝    ╚═════╝ ╚═╝

void setup()
{
  Serial.begin(9600);
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }
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
  // Update the Bounce instance (YOU MUST DO THIS EVERY LOOP)
  bounce.update();

  // <Bounce>.changed() RETURNS true IF THE STATE CHANGED (FROM HIGH TO LOW OR LOW TO HIGH)
  if (bounce.changed())
  {
    // THE STATE OF THE INPUT CHANGED
    // GET THE STATE
    int debouncedInput = bounce.read();
    // IF THE CHANGED VALUE IS LOW
    if (debouncedInput == LOW)
    {
      //ledState = !ledState;            // SET ledState TO THE OPPOSITE OF ledState
      digitalWrite(LED_PIN, HIGH); // WRITE THE NEW ledState
      motor.setTargetVelocity(20000000);
      // startAudio(); // Don't send radio envent while audio is playing!
      // delay(1000);
      // stopAudio();
      sendGoEvent(1); // Does not work inside VS1053 audio startPlayingFile!
    }
    else if (debouncedInput == HIGH)
    {
      digitalWrite(LED_PIN, LOW); // WRITE THE NEW ledState
      motor.setTargetVelocity(-20000000);
    }
  }

  long newPosition = shaftEncoder.read();
  //Serial.println(newPosition);
  if (newPosition != oldPosition)
  {
    long interval = millis() - lastChangeTime;
    //Serial.println();
    lastChangeTime = millis();
    if (newPosition > oldPosition)
    {
      //Serial.println(F("Surfacing"));
      motor.setTargetVelocity(int(50 / interval) * -5000000);
    }
    else
    {
      //Serial.println(F("Diving"));
      motor.setTargetVelocity(int(50 / interval) * 5000000);
    }
    oldPosition = newPosition;
    //Serial.println(newPosition);
  }
  else
  {
    motor.setTargetVelocity(0);
  }

  motor.resetCommandTimeout();
  Watchdog.reset();
}