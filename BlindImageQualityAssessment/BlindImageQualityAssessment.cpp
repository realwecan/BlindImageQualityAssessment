// BlindImageQualityAssessment.cpp : 定义控制台应用程序的入口点。
//

//#include "stdafx.h"
//#include <opencv2/core/core.hpp>
//#include <opencv2/highgui/highgui.hpp>
#include "brisque.h"
#include <iostream>

//using namespace cv;
using namespace std;

//rescaling based on training data i libsvm
float rescale_vector[36][2];

int read_range_file(string prefix) {
	//check if file exists
	char buff[100];
	int i;
	string range_fname = prefix + "allrange";
	FILE* range_file = fopen(range_fname.c_str(), "r");
	if (range_file == NULL) return 1;
	//assume standard file format for this program	
	fgets(buff, 100, range_file);
	fgets(buff, 100, range_file);
	//now we can fill the array
	for (i = 0; i < 36; ++i) {
		float a, b, c;
		fscanf(range_file, "%f %f %f", &a, &b, &c);
		rescale_vector[i][0] = b;
		rescale_vector[i][1] = c;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	
	//Mat image;
	//image = imread(argv[1], IMREAD_COLOR); // Read the file

	//if (!image.data) // Check for invalid input
	//{
	//	cout << "Could not open or find the image" << std::endl;
	//	return -1;
	//}

	//namedWindow("Display window", WINDOW_AUTOSIZE); // Create a window for display.
	//imshow("Display window", image); // Show our image inside it.

	//waitKey(0); // Wait for a keystroke in the window

	string prefix = argv[0];
	prefix = prefix.substr(0, prefix.find_last_of("/\\") + 1);

	//read in the allrange file to setup internal scaling array
	if (read_range_file(prefix)) {
		cerr << "unable to open allrange file" << endl;
		return -1;
	}

	float qualityscore;
	qualityscore = computescore(prefix, argv[1]);

	cout << qualityscore;

	return 0;
}

