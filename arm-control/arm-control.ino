/************************************************************************
* File Name          : Arm Control
* Authors             : Tom Stpehens / Kristopher Kendall
* Description        : Simplistic serial protocol for working with the arm
* License            : 
* Copyright(C) 2015 HID Global. All right reserved.
*************************************************************************/
#include <EEPROM.h>
#include <UF_uArm.h>

#define VERSION "AC-1.00"

const int NO_DATA = -10000;

UF_uArm uarm;           // initialize the uArm library 
int  lastHeight  = 0, lastStretch = 0, lastRotation = 0, lastWrist = 0;
bool debug = false;

void setup() 
{
  Serial.begin(115200);  // start serial port at 9600 bps
  uarm.init();          // initialize the uArm position
  uarm.setServoSpeed(SERVO_R, 15);  // 0=full speed, 1-255 slower to faster, no difference for >= 127
  uarm.setServoSpeed(SERVO_L, 15);  // 0=full speed, 1-255 slower to faster, no difference for >= 127
  uarm.setServoSpeed(SERVO_ROT, 15); // 0=full speed, 1-255 slower to faster , no difference for >= 127
  uarm.setServoSpeed(SERVO_HAND, 15); // Rotation speed of wrist servo.
  delay(500);
}


int asciiToInt(int i) {
  int val = NO_DATA;
  
  if (i == '-')
    val = '-';
  else {
    i -= 48;
    if(i >= 0 && i < 10)
       val = i;
  }
  
  return val;
}


int readNext() {
  int val = 0;
  bool dataFound = false;
  bool invert = false;
  
  char dataVal = 0;
  delay(10);
  while (Serial.available())  
  {
      dataVal = (char)Serial.read();
      if((dataVal == '\n') || (dataVal == '\r')) 
      {
       Serial.flush();
       break;
      } 
      int nextDigit = asciiToInt(dataVal);
      if(nextDigit != NO_DATA)
      {
        dataFound = true;
        if(nextDigit == '-')
          invert = true;
        else {
          val *= 10;
          val += nextDigit;
        }
      }
  }
  
  if(invert)
    val = -val;
    
  return dataFound ? val : NO_DATA;
}

// #define DEBUG_CODE  // Uncomment this line to enable debugging. Currently creates system instability. Memory problems.
#define SUCTION_CODE // Comment out this line to enable the pinch end-effector and disable the suction cup.


/* main loop looking for incoming serial commands. */
void loop()
{
  uarm.calibration();
  
  if(Serial.available())
  {
    byte cmd = Serial.read();
    cmd = tolower(cmd);
    bool valid = false;
  
    switch(cmd) {
      case 's':
        valid = setStretch();
        break;
      case 'h':
        valid = setHeight();
        break;
      case 'r':
        valid = setRotation();
        break;
      case 'w':
        valid = setWrist();
        break;
      case 'v':
        valid = setVelocity();
        break;
      case 'x':
        valid = setVelocityRot();
        break;
      case 'y':
        valid = setVelocityWrist();
        break;
      case '!':
        valid = reset();
        break;
      case 'b':
        valid = runBeeper();
        break;
      case 'd':
        valid = toggleDebug();
        break;
      case '?':
        valid = getPos();   
        break;
#if defined(SUCTION_CODE)
      case 'p':
        valid = runPump();
        break;
      case 'o':
        valid = offPump();
        break;
#else
      case 'g':
        valid = pinch();
        break;
      case 'v':
        valid = releasePinch();
        break;
#endif
#if defined(DEBUG_CODE)
      case 'q':
        valid = help();
        break;
#endif
      case '\r':
      case '\n':
        return;
     default:
        if(debug)
          Serial.println("Unknown command.");
        break;
    }
    Serial.println((valid ? 'K' : 'N'));
  }
}
#if defined(DEBUG_CODE)
bool help() {
  Serial.println("Help on Available Commands:");
  Serial.println("Enter case insensitive letter followed by parameter, no spaces.");
  Serial.println("s = Set Stretch Position (0 to 210)");
  Serial.println("h = Set Height Position (150(highest) to -180(lowest), Table Height ~ -85)");
  Serial.println("r = Set Rotation Position (-90 to 90)");
  Serial.println("w = Twist the wrist action (-90 to 90)");
  Serial.println("g = Grab an object with the pinching end-effector");
  Serial.println("w = Release an object with the pinching end-effector");
  return true;
}
#endif

bool setStretch() {
  int pos = readNext();
  
  if(pos == NO_DATA)
    return false;
  
  if(debug) {
    Serial.print("Stretch pos: ");
    Serial.println(pos, DEC);
  }
  
  uarm.setPosition(pos, lastHeight, lastRotation, lastWrist);
  lastStretch = pos;
  
  return true;
}

bool setHeight() {
  int pos = readNext();
  
  if(pos == NO_DATA)
    return false;
  
  if(debug) {
    Serial.print("Height pos: ");
    Serial.println(pos, DEC);
  }
  
  uarm.setPosition(lastStretch, pos, lastRotation, lastWrist);
  lastHeight = pos;
  
  return true;
}

bool setRotation() {
  int pos = readNext();
  
  if(pos == NO_DATA)
    return false;
  
  if(debug) {
    Serial.print("Rotation pos: ");
    Serial.println(pos, DEC);
  }
  
  uarm.setPosition(lastStretch, lastHeight, pos, lastWrist);
  lastRotation = pos;
  
  return true;
}

bool setWrist() {
  int pos = readNext();
  
  if(pos == NO_DATA)
    return false;
  
  if(debug) {
    Serial.print("Wrist pos: ");
    Serial.println(pos, DEC);
  }
  
  uarm.setPosition(lastStretch, lastHeight, lastRotation, pos);
  lastWrist = pos;
  
  return true;
}

#if defined(SUCTION_CODE)
bool runPump() {   
  if(debug) {
    Serial.println("Pump ON ");
  }
    uarm.pumpOn(); 
    uarm.valveOff();
    return true;
}

bool offPump() {
  if(debug) {
    Serial.println("Pump OFF ");
  } 
    uarm.pumpOff();
    uarm.valveOn();
    return true;
}
#else
bool pinch() {  
  if(debug) {
    Serial.println("Grab");
  }  
  uarm.gripperCatch();
  return true;
}

bool releasePinch() {  
  if(debug) {
    Serial.println("Drop");
  }  
  uarm.gripperRelease();
  return true;
}
#endif


bool getPos() {
 
  Serial.print("Current Pos: Stretch = ");
  Serial.print(lastStretch);
  Serial.print(", Height = ");
  Serial.print(lastHeight);
  Serial.print(", Rotation = ");
  Serial.print(lastRotation);
  Serial.print(", Wrist = ");
  Serial.println(lastWrist);
 return true; 
}

bool setVelocity() {
  int v = readNext();
  
  if(v == NO_DATA)
    return false;
  
  if(debug) {
    Serial.print("Set Speed: ");
    Serial.println(v, DEC);
  }
  
  uarm.setServoSpeed(SERVO_R, v);
  uarm.setServoSpeed(SERVO_L, v);
  
  return true;
}


bool setVelocityRot() {
  int x = readNext();
  
  if(x == NO_DATA)
    return false;
  
  if(debug) {
    Serial.print("Set Rot Speed: ");
    Serial.println(x, DEC);
  }

  uarm.setServoSpeed(SERVO_ROT, x);
  
  return true;
}

bool setVelocityWrist() {
  int y = readNext();
  
  if(y == NO_DATA)
    return false;
  
  if(debug) {
    Serial.print("Set Wrist Speed: ");
    Serial.println(y, DEC);
  }

  uarm.setServoSpeed(SERVO_HAND, y);

  return true;
}

bool reset() {
  uarm.setPosition(0,0,0,0);
  
  if(debug)
    Serial.println("All pos reset.");
  
  lastStretch = 0;
  lastHeight = 0;
  lastRotation = 0;
  lastWrist = 0;
  
  return true;
}

bool runBeeper() {
  int times = readNext();
  
  if(times == NO_DATA)
    return false;
  
  if(debug) {
    Serial.print("Beep Times: ");
    Serial.println(times, DEC);
  }
  
  uarm.alert(times, 200, 100);  
  return true;
}

bool toggleDebug() {
  debug=!debug;  
  Serial.print("Debug ");
  Serial.println((debug ? "Enabled" : "Disabled"));
  return true;
}
