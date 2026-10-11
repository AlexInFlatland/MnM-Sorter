#include <Stepper.h>
#include <Servo.h>

// Change this to fit the number of steps per revolution for your motor
const int stepsPerRevolution = 48;  

// Initialize the stepper library on pins 8 through 11:
Stepper stepper(stepsPerRevolution, 8,9,10,11);

int stepperPosition = 0;

// Initialize the load and unload servos
Servo loadServo;
Servo unloadServo;
             
int incomingByte = 0;   // for incoming serial data                

void setup() {
  Serial.begin(9600); // // opens serial port, sets data rate to 9600 bps

  // Attaches the load servo on pin 6 to the loadServo object
  loadServo.attach(6);
  loadServo.write(0);

  // Attaches the unload servo on pin 5 to the unloadServo object
  unloadServo.attach(5);
  unloadServo.write(176);

  // Set the speed of the stepper motor at 200 rpm
  stepper.setSpeed(200);
}

void loop()
{
    if (Serial.available() > 0)
    {
      // read the incoming byte:
      incomingByte = Serial.read();
      
      switch(incomingByte)
      {
        // Move the candy to the camera
        case 'm':
        stepper.step(-500);
        // Update current stepper position
        stepperPosition+=500;
        // Done moving the stepper.
        // It doesn't really matter which letter we output here
        // as the OpenCV code just expects something (anything)
        // so that it knows that the move procedure ended
        Serial.write('d');
        // This small delay makes sure that the candy doesn't move
        // while the camera is taking an image. I'm just making sure
        // that the image I take will be steady
        delay(500); 
        break;
        
        // 'Home' the printer head to the loading position
        // I don't have a sensor for the home position and that's
        // why I'm tracking the stepper position with a variable
        // knowing that position = 0 is home.
        case 'h':
        stepper.step(stepperPosition);
        stepperPosition-=stepperPosition;
        // Again this write just informs the OpenCV code
        // that the print head is home and we can repeat the
        // detection cycle
        Serial.write('d');
        break;
        
        // Step away from the camera and onto the first
        // unloading point. This initial step is slightly
        // 'larger' than the other ones as it needs to
        // clear the camera so it doesn't hit it during
        // unload in the first location
        case 's':
        stepper.step(100);
        stepperPosition-=100;
        break;
        
        // Next step. After the first initial step we use
        // this 'smaller' step to move between all the
        // other unloading points.
        // Note: After the first step we have 400 steps to
        // get to the home position. This step is at 80 so
        // as to get exactly 5 steps till home thus evenly
        // distributing the unload locations.
        case 'n':
        stepper.step(80);
        stepperPosition-=80;
        break;
        
        // Unload. This moves the servo responsible for
        // candy unloading. It rotates the unloader down
        // and then moves it back to the horizontal position
        case 'u':
        unloadServo.write(140);
        // This small delay makes sure the candy is out of
        // the unloader before the latter moves back
        delay(500); 
        unloadServo.write(176);
        break;
        
        // Load. This moves the servo responsible for
        // candy loading.
        case 'l':
        loadServo.write(50);
        // This small delay makes sure the servo can
        // catch up with the code
        delay(120); 
        loadServo.write(0);
        // This delay gives time for the dropped candy
        // to settle in before starting to move the
        // printer head to the camera
        delay(500); 
        break;
      }
    }
}