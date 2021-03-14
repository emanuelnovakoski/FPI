#include <stdio.h>
#include <math.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

#define B 0
#define G 1
#define R 2
#define RWEIGHT 0.299
#define GWEIGHT 0.587
#define BWEIGHT 0.114
#define SX 2
#define SY 3


using namespace cv;

Mat imageIn, imageOut;
char* filename;
int quantizationAmount = 0;
int brightness = 0;
int contrast = 0;



// Here are defined the auxiliary functions
int maximum(int* v, int size)
{
	int max = 0;
	for(int i = 0; i<size; i++)
	{
		if (v[i] > max)
		{
			max = v[i];
		}
	}
	
	return max;
			
}

int clamp(int value, int min, int max)
{
	if (value < min)
		return min;
	if (value > max)
		return max;
	return value;
}

// Here are defined the callbacks
void _on_vertical_flip(int state, void* userData)
{
	int sizey = imageIn.rows;

	Mat temp;
	temp.create(1, imageIn.cols, imageIn.type());

	for (int i=0;i<(sizey/2);i++)
	{
		
		imageOut.row(sizey - i - 1).copyTo(temp.row(0));
		imageOut.row(i).copyTo(imageOut.row(sizey - i - 1));
		temp.row(0).copyTo(imageOut.row(i));
		
	}

	imshow("Output Image", imageOut);
    imshow("Input Image", imageIn);
}

void _on_reset(int state, void* userData)
{
	imageOut = imageIn.clone();
	imshow("Output Image", imageOut);
    imshow("Input Image", imageIn);
}

void _on_horizontal_flip(int state, void* userData)
{
	int sizex = imageIn.cols;

	Mat temp;
	temp.create(imageIn.rows, 1, imageIn.type());

	for (int i=0;i<(sizex/2);i++)
	{
		imageOut.col(sizex - i - 1).copyTo(temp.col(0));
		imageOut.col(i).copyTo(imageOut.col(sizex - i - 1));
		temp.col(0).copyTo(imageOut.col(i));
	}

	imshow("Output Image", imageOut);
    imshow("Input Image", imageIn);
}

void _on_save(int state, void* userData)
{
	imwrite(strcat(filename, "_t1Edited"), imageOut);
}

void _on_luminance(int state, void* userData)
{
	int sizex = imageOut.cols;
	int sizey = imageOut.rows;
	for (int i=0;i<sizey;i++)
		for (int j=0;j<sizex;j++)
		{
			Vec3b pixel = imageOut.at<Vec3b>(i,j);
			float luminance = RWEIGHT * pixel.val[R] + GWEIGHT * pixel.val[G] + BWEIGHT * pixel.val[B];
			pixel.val[R] = luminance;
			pixel.val[G] = luminance;
			pixel.val[B] = luminance;
			imageOut.at<Vec3b>(i,j) = pixel;
		}


	imshow("Output Image", imageOut);
    imshow("Input Image", imageIn);
}


void _on_grey_quantization(int state, void* userData)
{
	int sizex = imageOut.cols;
	int sizey = imageOut.rows;
	
	int lower = 256;
	int higher = -1;
	
	if(quantizationAmount == 0)
	{
		std::cout << "Quantization cannot be 0\n";
		return;
	}
	
	// convert image to grayscale
	_on_luminance(state, userData); 
	
	// get limits
	for (int i=0;i<sizey;i++)
		for (int j=0;j<sizex;j++)
		{
			Vec3b pixel = imageOut.at<Vec3b>(i,j);
			if (pixel.val[R] > higher)
				higher = pixel.val[R];
			if (pixel.val[R] < lower)
				lower = pixel.val[R];
		}	
	
	
	// get bin sizes
	int tam_int = higher - lower + 1;
	
	if (quantizationAmount >= tam_int)
		return;
	
	int tb = tam_int/quantizationAmount;
	
	
	// set bin
	for (int i=0;i<sizey;i++)
		for (int j=0;j<sizex;j++)
		{
			Vec3b pixel = imageOut.at<Vec3b>(i,j);
			int newVal = (pixel.val[R] - lower)/tb;
			newVal = lower + newVal*tb;
			pixel.val[R] = newVal;
			pixel.val[G] = newVal;
			pixel.val[B] = newVal;
			imageOut.at<Vec3b>(i,j) = pixel;			
		}
		

	imshow("Output Image", imageOut);
    imshow("Input Image", imageIn);
}

void _on_trackbar(int state, void* userData) {}

void _on_histogram(int state, void* userData)
{
	namedWindow("Histogram", WINDOW_AUTOSIZE);
	_on_luminance(state, userData);
	Mat *histogram = new Mat(256, 256, imageIn.type());
	int sizex = imageOut.cols;
	int sizey = imageOut.rows;
	int count[256] = {0};
	
	for (int i=0; i<sizey; i++)
	{
		for (int j=0; j<sizex; j++)
		{
			Vec3b pixel = imageOut.at<Vec3b>(i,j);
			count[pixel.val[R]]++;
		}	
	}
	int max = maximum(count, 256);

	for (int i=0; i<256; i++)
	{
		float barSize = (float)count[i]/max;
		int limitPixel = floor(barSize*256);
		for (int j=0; j<limitPixel; j++)
		{
			Vec3b pixel = histogram->at<Vec3b>(i,j);
			pixel.val[R] = 0;
			pixel.val[G] = 0;
			pixel.val[B] = 0;
			histogram->at<Vec3b>(255-j,i) = pixel;
		}

		for (int j=limitPixel; j<255; j++)
		{
			Vec3b pixel = histogram->at<Vec3b>(i,j);
			pixel.val[R] = 255;
			pixel.val[G] = 255;
			pixel.val[B] = 255;
			histogram->at<Vec3b>(255-j,i) = pixel;
		}

	}
	imshow("Histogram", *histogram);
}

void _on_brightness(int state, void* userData)
{
	int sizex = imageIn.cols;
	int sizey = imageIn.rows;	
	for (int i=0; i<sizey; i++)
	{
		for (int j=0; j<sizex; j++)
		{
			Vec3b pixel = imageIn.at<Vec3b>(i,j);
			pixel.val[R] = clamp(pixel.val[R] + brightness, 0, 255);
			pixel.val[G] = clamp(pixel.val[G] + brightness, 0, 255);
			pixel.val[B] = clamp(pixel.val[B] + brightness, 0, 255);
			imageOut.at<Vec3b>(i,j) = pixel;	
		}	
	}
	imshow("Output Image", imageOut);
}


void _on_contrast(int state, void* userData)
{
	int sizex = imageIn.cols;
	int sizey = imageIn.rows;	
	for (int i=0; i<sizey; i++)
	{
		for (int j=0; j<sizex; j++)
		{
			Vec3b pixel = imageIn.at<Vec3b>(i,j);
			pixel.val[R] = clamp(pixel.val[R]*contrast, 0, 255);
			pixel.val[G] = clamp(pixel.val[G]*contrast, 0, 255);
			pixel.val[B] = clamp(pixel.val[B]*contrast, 0, 255);
			imageOut.at<Vec3b>(i,j) = pixel;	
		}	
	}
	imshow("Output Image", imageOut);
}

void _on_negative(int state, void* userData)
{
	int sizex = imageIn.cols;
	int sizey = imageIn.rows;	
	for (int i=0; i<sizey; i++)
	{
		for (int j=0; j<sizex; j++)
		{
			Vec3b pixel = imageOut.at<Vec3b>(i,j);
			pixel.val[R] = 255 - pixel.val[R];
			pixel.val[G] = 255 - pixel.val[G];
			pixel.val[B] = 255 - pixel.val[B];
			imageOut.at<Vec3b>(i,j) = pixel;	
		}	
	}
	imshow("Output Image", imageOut);
}

void _on_histogram_equalization(int state, void* userData)
{
	_on_luminance(state, userData);
	int sizex = imageOut.cols;
	int sizey = imageOut.rows;
	int histogram[256] = {0}, histCum[256] = {0};
	
	
	for (int i=0; i<sizey; i++)
	{
		for (int j=0; j<sizex; j++)
		{
			Vec3b pixel = imageIn.at<Vec3b>(i,j);
			histogram[pixel.val[R]]++;
		}	
	}	
	
	float a = 255.0 / (sizex*sizey);
	std::cout << a << "\n";
	
	histCum[0] = a*histogram[0];
	for (int i=1; i<256; i++)
	{
		histCum[i] = histCum[i-1] + (a*histogram[i]);
		std::cout << i << " " << histCum[i] << "\n";
	}
	
	for (int i=0; i<sizey; i++)
	{
		for (int j=0; j<sizex; j++)
		{
			Vec3b pixel = imageOut.at<Vec3b>(i,j);
			pixel.val[R] = histCum[pixel.val[R]];
			pixel.val[G] = histCum[pixel.val[G]];
			pixel.val[B] = histCum[pixel.val[B]];
			imageOut.at<Vec3b>(i,j) = pixel;	
		}	
	}	
	imshow("Output Image", imageOut);
}


//TODO TRIM IMAGE
void _on_zoom_out(int state, void* userData)
{
	int sizex = imageOut.cols;
	int sizey = imageOut.rows;
	
	for (int i=0; i<sizey; i++)
	{
		for (int j=0; j<sizex; j++)	
		{
			int sum[3] = {0};
			int count = 0;
			for(int ii=0; ii<SY; ii++)
			{
				if (ii+(i*SY) > sizey) continue;
				for(int jj=0; jj<SX; jj++)
				{
					if (jj+(j*SX) > sizex) continue;
					sum[R] += (imageOut.at<Vec3b>((i*SY)+ii,(j*SX)+jj)).val[R];
					sum[G] += (imageOut.at<Vec3b>((i*SY)+ii,(j*SX)+jj)).val[G];
					sum[B] += (imageOut.at<Vec3b>((i*SY)+ii,(j*SX)+jj)).val[B];
					count++;
				}
			}
			Vec3b pixel = imageOut.at<Vec3b>(i,j);
			pixel[R] = (float)sum[R]/count;
			pixel[G] = (float)sum[G]/count;
			pixel[B] = (float)sum[B]/count;
			imageOut.at<Vec3b>(i,j) = pixel;	
									
		}
	}
	imshow("Output Image", imageOut);
}

void _on_zoom_in(int state, void* userData)
{
	int sizex = imageOut.cols;
	int sizey = imageOut.rows;
	Mat *out = new Mat(sizey*2, sizex*2, imageIn.type());
	int newsizex = imageOut.cols*2;
	int newsizey = imageOut.rows*2;
	
	for (int i=0; i<sizey; i++)
	{
		for (int j=0; j<sizex; j++)	
		{
			Vec3b pixel = imageIn.at<Vec3b>(i,j);
			out->at<Vec3b>(i*2,j*2) = pixel;
		}
	}	
	
	for (int i=1; i<newsizey-1; i+=2)
	{
		for (int j=0; j<newsizex-1; j+=2)	
		{
			Vec3b pixel1 = out->at<Vec3b>(i-1,j);
			Vec3b pixel2 = out->at<Vec3b>(i+1,j);
			Vec3b pixeln;
			pixeln.val[R] = (pixel1.val[R] + pixel2.val[R])/2;
			pixeln.val[G] = (pixel1.val[G] + pixel2.val[G])/2;
			pixeln.val[B] = (pixel1.val[B] + pixel2.val[B])/2;
			out->at<Vec3b>(i,j) = pixeln;
		}
	}

	for (int i=0; i<newsizey-1; i+=2)
	{
		for (int j=1; j<newsizex-1; j+=2)	
		{
			Vec3b pixel1 = out->at<Vec3b>(i,j-1);
			Vec3b pixel2 = out->at<Vec3b>(i,j+1);
			Vec3b pixeln;
			pixeln.val[R] = (pixel1.val[R] + pixel2.val[R])/2;
			pixeln.val[G] = (pixel1.val[G] + pixel2.val[G])/2;
			pixeln.val[B] = (pixel1.val[B] + pixel2.val[B])/2;
			out->at<Vec3b>(i,j) = pixeln;
		}
	}

	for (int i=1; i<newsizey-1; i+=2)
	{
		for (int j=1; j<newsizex-1; j+=2)	
		{
			Vec3b pixel1 = out->at<Vec3b>(i,j-1);
			Vec3b pixel2 = out->at<Vec3b>(i,j+1);
			Vec3b pixel3 = out->at<Vec3b>(i-1,j);
			Vec3b pixel4 = out->at<Vec3b>(i+1,j);
			Vec3b pixeln;
			pixeln.val[R] = (pixel1.val[R] + pixel2.val[R] + pixel3.val[R] + pixel4.val[R])/4;
			pixeln.val[G] = (pixel1.val[G] + pixel2.val[G] + pixel3.val[G] + pixel4.val[G])/4;
			pixeln.val[B] = (pixel1.val[B] + pixel2.val[B] + pixel3.val[B] + pixel4.val[B])/4;
			out->at<Vec3b>(i,j) = pixeln;
		}
	}

	imageOut.release();
	imageOut = *out;
	imshow("Output Image", imageOut);
}

void _on_rotate_right(int state, void* userData)
{
	int sizex = imageOut.cols;
	int sizey = imageOut.rows;	
	Mat *out = new Mat(sizex, sizey, imageOut.type());
	
	for (int i=0; i<sizey; i++)
	{
		for (int j=0; j<sizex; j++)	
		{
			Vec3b pixel = imageOut.at<Vec3b>(i,j);
			out->at<Vec3b>(j, sizey-i) = pixel;
		}
	}	
	imageOut.release();
	imageOut = *out;
	imshow("Output Image", imageOut);
}

void _on_rotate_left(int state, void* userData)
{
	int sizex = imageOut.cols;
	int sizey = imageOut.rows;	
	Mat *out = new Mat(sizex, sizey, imageOut.type());
	
	for (int i=0; i<sizey; i++)
	{
		for (int j=0; j<sizex; j++)	
		{
			Vec3b pixel = imageOut.at<Vec3b>(i,j);
			out->at<Vec3b>(j, sizey - i) = pixel;
		}
	}	
	imageOut.release();
	imageOut = *out;
	imshow("Output Image", imageOut);
}

int main(int argc, char** argv )
{
    if ( argc != 2 )
    {
        printf("usage: t1 <Image_Path>\n");
        return -1;
    }

	filename = argv[1];
    imageIn = imread(filename , 1 );
    if ( !imageIn.data )
    {
        printf("No image data \n");
        return -1;
    }
	imageOut = imageIn.clone();
    namedWindow("Input Image", WINDOW_AUTOSIZE );
    namedWindow("Output Image", WINDOW_AUTOSIZE);
    imshow("Input Image", imageIn);
	imshow("Output Image", imageOut);
	
	createButton("Flip Vertically", _on_vertical_flip);
	createButton("Flip Horizontally", _on_horizontal_flip);
	createButton("Reset Image", _on_reset);
	createButton("Save Output", _on_save, NULL, QT_NEW_BUTTONBAR);
	createButton("Luminance", _on_luminance);
	createTrackbar("Quantization Amount", "Input Image", &quantizationAmount, 255, _on_trackbar);
	createButton("Gray Scale Quantization", _on_grey_quantization);
	createButton("Histogram", _on_histogram, NULL, QT_NEW_BUTTONBAR);
	createTrackbar("Brightness", "Input Image", &brightness, 512, _on_brightness);
	setTrackbarMin("Brightness", "Input Image", -255);
	setTrackbarMax("Brightness", "Input Image", 255);
	createTrackbar("Contrast", "Input Image", &contrast, 255, _on_contrast);
	createButton("Negative", _on_negative);
	createButton("Histogram Equalization", _on_histogram_equalization);
	createButton("Zoom Out", _on_zoom_out, NULL, QT_NEW_BUTTONBAR);
	createButton("Zoom In", _on_zoom_in);
	createButton("Rotate Right", _on_rotate_right, NULL, QT_NEW_BUTTONBAR);
	createButton("Rotate Left", _on_rotate_left);

	waitKey(0);
	
    return 0;
}


