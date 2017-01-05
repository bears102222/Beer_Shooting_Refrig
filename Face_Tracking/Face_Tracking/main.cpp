#include <opencv2\opencv.hpp>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SerialPort.h"
#include <string>
//
using namespace std;
using namespace cv;
char *port_name = "\\\\.\\COM6";
String classifer = "C:/OpenCV-3.1.0/opencv/sources/data/haarcascades/haarcascade_frontalface_default.xml";
SerialPort arduino(port_name);
Mat frame, gray_frame, hsv_frame;
std::vector<Rect> found_faces;
int frame_width, frame_height;
VideoCapture cap(1);
CascadeClassifier face;
bool condition, movement = false;
int last_move_left, last_move_right = 0;

//Sends message to Arduino over Serial
void send_message(string message) {
	char *c_string = new char[message.size() + 1];
	copy(message.begin(), message.end(), c_string);
	c_string[message.size()] = '\n';
	arduino.writeSerialPort(c_string, MAX_DATA_LENGTH);
	delete[] c_string;
}
//Confirm face result is actually a face. 
bool search_forFace() {
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
	if (arduino.isConnected()) cout << "Connection Established" << endl;
	else cout << "ERROR, check port name";

	face.load(classifer);
	//cap.set(CV_CAP_PROP_FRAME_WIDTH, 300);
	//cap.set(CV_CAP_PROP_FRAME_HEIGHT, 300);
	cap.read(frame);
	frame_width = frame.size().width;
	frame_height = frame.size().height;
	while (1) {

		//Haar Classifer Code
		cap.read(frame);
		rectangle(frame, Point(frame_width *0.30, 0), Point(frame_width *0.70, frame_height), Scalar(0, 0, 255), 2, LINE_8, 0);
		cvtColor(frame, gray_frame, COLOR_BGR2GRAY);
		face.detectMultiScale(gray_frame, found_faces, 1.1, 4, CV_HAAR_DO_CANNY_PRUNING | CASCADE_SCALE_IMAGE, Size(30, 30));


		//Draw Boxes around faces red unless its in the target zone
		if(found_faces.size() > 0)
		condition = (found_faces[0].x > frame_width *0.30) && (found_faces[0].x < frame_width *0.70);
		
		if (found_faces.size() > 0  && (condition ||search_forFace() == true)) {

			for (size_t i = 0; i < found_faces.size(); i++) {

				//If this condition is met the camera should slightly shift
				if (condition == false) {
					rectangle(frame, Point(found_faces[i].x, found_faces[i].y), Point(found_faces[i].x + found_faces[i].width, found_faces[i].y + found_faces[i].height), Scalar(0, 0, 255), 2, LINE_8, 0);
					if ((found_faces[i].x < frame_width*0.3) && movement == false) {
						movement = true;
						send_message("Shift_Left");
					}

					if ((found_faces[i].x > frame_width*0.7) && movement == false) {
						movement = true;
						send_message("Shift_Right");
					}
					movement = true;
					
				}
				//If this condition is met, it should shoot the can and tells Arduino to stop moving.
				if (condition == true){
					rectangle(frame, Point(found_faces[i].x, found_faces[i].y), Point(found_faces[i].x + found_faces[i].width, found_faces[i].y + found_faces[i].height), Scalar(0, 255, 0), 2, LINE_8, 0);
					rectangle(frame, Point(frame_width *0.30, 0), Point(frame_width *0.70, frame_height), Scalar(0, 255, 0), 2, LINE_8, 0);
					if (movement == true) {
						movement = false;
						waitKey(1000);
						send_message("Stop");
					}
					}
			}
		}
		
		imshow("window", frame);
		waitKey(5);
		
	}

}
