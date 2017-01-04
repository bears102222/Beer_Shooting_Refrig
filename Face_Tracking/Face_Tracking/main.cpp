#include <opencv2\opencv.hpp>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SerialPort.h"
#include <string>

using namespace std;
using namespace cv;
char *port_name = "\\\\.\\COM6";
String classifer = "C:/OpenCV-3.1.0/opencv/sources/data/haarcascades/haarcascade_frontalface_default.xml";
SerialPort arduino(port_name);
Mat frame, gray_frame, hsv_frame;
std::vector<Rect> found_faces;

VideoCapture cap(1);
CascadeClassifier face;


//Sends message to Arduino over Serial
void send_message(string message) {
	char *c_string = new char[message.size() + 1];
	copy(message.begin(), message.end(), c_string);
	c_string[message.size()] = '\n';
	arduino.writeSerialPort(c_string, MAX_DATA_LENGTH);
	delete[] c_string;
}
bool search_forFace() {
	cap.set(CV_CAP_PROP_FRAME_WIDTH, 300);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 300);
	int success = 0;
	face.load(classifer);
	for (int i = 0; i < 3; i++) {
		cap.read(frame);
		waitKey(10);
		cvtColor(frame, gray_frame, COLOR_BGR2GRAY);
		face.detectMultiScale(gray_frame, found_faces, 1.1, 4, CV_HAAR_DO_CANNY_PRUNING | CASCADE_SCALE_IMAGE, Size(30, 30));
		if (found_faces.size() > 0) {
			//printf("width: %d length: %d \n", found_faces[0].width, found_faces[0].height);
			success = success + 1;
			if (success >= 2)
				return true;
		}
	}
	return false;
}


int main() {
	face.load(classifer);
	cap.set(CV_CAP_PROP_FRAME_WIDTH, 300);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 300);
	while (1) {
		cap.read(frame);
		
		cvtColor(frame, gray_frame, COLOR_BGR2GRAY);
		face.detectMultiScale(gray_frame, found_faces, 1.1, 4, CV_HAAR_DO_CANNY_PRUNING | CASCADE_SCALE_IMAGE, Size(30, 30));
		if (found_faces.size() > 0  && search_forFace() == true) {
			for (size_t i = 0; i < found_faces.size(); i++) {
				rectangle(frame, Point(found_faces[i].x, found_faces[i].y), Point(found_faces[i].x + found_faces[i].width, found_faces[i].y + found_faces[i].height), Scalar(255, 0, 255), 2, LINE_8, 0);
			}
		}
		
		imshow("window", frame);
		waitKey(5);
		
	}

}
