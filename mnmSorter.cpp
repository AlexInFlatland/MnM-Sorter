#include <iostream>

#include<opencv/cvaux.h>
#include<opencv/highgui.h>
#include<opencv/cxcore.h>

#include<stdio.h>
#include<stdlib.h>

// Need to include this for serial port communication
#include <Windows.h>

using namespace std;

// Main detection loop
int runCycle();

// Number of M&Ms to load. This will determine program execution length.
int mnmCount = 7;

// This is the color array
// The first item is the name of the color
// The second item is the unload position counting from the camera to the load servo
// 3 - Lower Hue
// 4 - Upper Hue
// 5 - Lower Saturation
// 6 - Upper Saturation
// 7 - Lower Value
// 8 - Upper Value
string colors[6][8] = {
	{"Blue", "0", "75", "130", "90", "256", "110", "256"},
	{"Orange", "1", "0", "22", "255", "256", "110", "256"},
	{"Brown", "2", "0", "180", "30", "256", "0", "130"},
	{"Green", "3", "38", "75", "50", "256", "110", "256"},
	{"Red", "4", "0", "180", "235", "253", "110", "256"},
	{"Yellow", "5", "20", "30", "220", "256", "110", "256"}
};

// Setup serial port connection and needed variables.
HANDLE hSerial = CreateFile(L"COM3", GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

char outputChars[] = "l";
DWORD btsIO;

char charCheckForEscKey;			// char for checking key press (Esc exits program)

CvCapture* p_capWebcam;						// we will assign our web cam video stream to this later . . .

CvSize size640x480 = cvSize(640, 480);			// use a 640 x 480 size for all windows, also make sure your webcam is set to 640x480 !!

IplImage* p_imgOriginal;			// pointer to an image structure, this will be the input image from webcam
IplImage* p_imgProcessed;			// pointer to an image structure, this will be the processed image
IplImage* p_imgHSV;

int main(int argc, char* argv[])
{
	// Setup serial port connection and needed variables.
	if (hSerial !=INVALID_HANDLE_VALUE)
    {
        printf("Port opened! \n");

        DCB dcbSerialParams;
        GetCommState(hSerial,&dcbSerialParams);

        dcbSerialParams.BaudRate = CBR_9600;
        dcbSerialParams.ByteSize = 8;
        dcbSerialParams.Parity = NOPARITY;
        dcbSerialParams.StopBits = ONESTOPBIT;

        SetCommState(hSerial, &dcbSerialParams);
	}
	else
	{
		if (GetLastError() == ERROR_FILE_NOT_FOUND)
		{
			printf("Serial port doesn't exist! \n");
		}

		printf("Error while setting up serial port! \n");
	}


	p_capWebcam = cvCaptureFromCAM(0);	// 0 => use 1st webcam, may have to change to a different number if you have multiple cameras

	if(p_capWebcam == NULL) {			// if capture was not successful . . .
		printf("error: capture is NULL \n");	// error message to standard out . . .
		getchar();								// getchar() to pause for the user to see the message . . .
		return(-1);								// exit program
	}

	p_imgOriginal = cvQueryFrame(p_capWebcam);		// get frame from webcam

	p_imgProcessed = cvCreateImage(size640x480,			// 640 x 480 pixels (CvSize struct from earlier)
								   IPL_DEPTH_8U,		// 8-bit color depth
								   1);					// 1 channel (grayscale), if this was a color image, use 3

	p_imgHSV = cvCreateImage(size640x480, IPL_DEPTH_8U, 3);

	cvNamedWindow("Original", CV_WINDOW_AUTOSIZE);

	cvShowImage("Original", p_imgOriginal);

	// The small 10ms delay here helps cvShowImage to update the window
	// The image isn't updated properly without it
	charCheckForEscKey = cvWaitKey(10);				// delay (in ms), and get key press, if any
	if(charCheckForEscKey == 27) return 0;				// if Esc key (ASCII 27) was pressed, jump out of while loop

	runCycle();
}

int runCycle()
{
	// Load candy
	outputChars[0] = 'l';
	WriteFile(hSerial, outputChars, strlen(outputChars), &btsIO, NULL);

	// Move print head to camera
	outputChars[0] = 'm';
	WriteFile(hSerial, outputChars, strlen(outputChars), &btsIO, NULL);

	// Wait for serial input notifying that the printer head has reached the camera
	// The input itself isn't important as long as we get it
	if (!ReadFile(hSerial, outputChars, 1, &btsIO, NULL))
	{
		printf("Error: Couldn't read serial input \n");
	}

	// Due to a quirk in OpenCV, I'm grabbing two frames here,
	// as grabbing only one resulted in a delay while getting the image
	p_imgOriginal = cvQueryFrame(p_capWebcam);		// get frame from webcam
	p_imgOriginal = cvQueryFrame(p_capWebcam);		// get frame from webcam

	if(p_imgOriginal == NULL) {					// if frame was not captured successfully . . .
	printf("error: frame is NULL \n");		// error message to std out
	getchar();
	}

	// Show image in the window
	cvShowImage("Original", p_imgOriginal);

	// The small 10ms delay here helps cvShowImage to update the window
	// The image isn't updated properly without it
	charCheckForEscKey = cvWaitKey(10);				// delay (in ms), and get key press, if any
	if(charCheckForEscKey == 27) return 0;				// if Esc key (ASCII 27) was pressed, jump out of while loop

	// Change the color model from RGB (BGR) to HSV
	// I find it's easier to distinguish between colors using the HSV model
	cvCvtColor(p_imgOriginal, p_imgHSV, CV_BGR2HSV);

	int colorNonZero = 0;
	int colorDeployPos = -1;

	// This is where the magic happens. In this loop I'm taking the HSV image
	// and checking for pixels of each individual color. The cvCountNonZero
	// method then takes the resulting binary image and gives back the number
	// of white pixels (non-zero = non-black). I'm then using this value to
	// determine which color produces the largest number of non-black pixels
	// and thus I determine the color of the M&M in the image.
	for (int i = 0; i < 6; i++)
	{
		cvInRangeS(p_imgHSV,				// function input
		cvScalar(stoi(colors[i][2]),  stoi(colors[i][4]),  stoi(colors[i][6])),			// min filtering value (if color is greater than or equal to this)
		cvScalar(stoi(colors[i][3]), stoi(colors[i][5]), stoi(colors[i][7])),			// max filtering value (if color is less than this)
		p_imgProcessed);				// function output

		if (colorNonZero < cvCountNonZero(p_imgProcessed))
		{
			colorNonZero = cvCountNonZero(p_imgProcessed);
			colorDeployPos = stoi(colors[i][1]);
		}
	}

	cout << "Color: " << colors[colorDeployPos][0] << "\n";

	// Sometimes there might be issues with the load procedure so no
	// candy arrives for detection. If this happens the deploy position
	// is - 1 so I return to try and pick up the candy again.
	if (colorDeployPos == -1)
	{
		mnmCount++;
	}
	else
	{
		// Step away from the camera onto the first unload position
		outputChars[0] = 's';
		WriteFile(hSerial, outputChars, strlen(outputChars), &btsIO, NULL);

		// Check on which position the candy needs to be deployed. Based on
		// the deploy position instruct the stepper to move the neccessary
		// steps (number of times) to reach it's goal destination.
		for (int i = 0 ; i < colorDeployPos; i++)
		{
			outputChars[0] = 'n';
			WriteFile(hSerial, outputChars, strlen(outputChars), &btsIO, NULL);
		}

		// Unload the candy
		outputChars[0] = 'u';
		WriteFile(hSerial, outputChars, strlen(outputChars), &btsIO, NULL);
	}

	// Go to the home position where it all started
	outputChars[0] = 'h';
	WriteFile(hSerial, outputChars, strlen(outputChars), &btsIO, NULL);

	// Wait for serial input notifying that the printer head has reached the home position
	// The input itself isn't important as long as we get it
	if (!ReadFile(hSerial, outputChars, 1, &btsIO, NULL))
	{
		printf("Error: Couldn't read serial input \n");
	}

	// One candy down
	mnmCount--;

	// If there are more, repeat the procedure
	if(mnmCount > 0)
		runCycle();
	else
	{
		//Release memory and close serial connection
		cvReleaseCapture(&p_capWebcam);					// release memory as applicable

		cvDestroyWindow("Original");

		// This closes the Serial Port
		CloseHandle(hSerial);
	}

	return 0;
}