#include "PNGImage.h"

#include "rtweekend.h"
#include <opencv2/imgcodecs.hpp>

PNGImage::PNGImage(const int imageWidth, const int imageHeight) :
	imageWidth(imageWidth), imageHeight(imageHeight)
{
	pixels = std::make_unique<cv::Mat>(cv::Mat::zeros(imageHeight, imageWidth, CV_8UC3));
}

void PNGImage::SetPixel(int x, int y, float r, float g, float b, int samplesPerPixel)
{
	// gamma correction
	float scale = 1.0f / samplesPerPixel;
	r = sqrt(scale * r);
	g = sqrt(scale * g);
	b = sqrt(scale * b);

	int rInt = static_cast<uchar>(256 * clamp(r, 0.0f, 0.999f));
	int gInt = static_cast<uchar>(256 * clamp(g, 0.0f, 0.999f));
	int bInt = static_cast<uchar>(256 * clamp(b, 0.0f, 0.999f));

	cv::Vec3b& color = this->pixels->at<cv::Vec3b>(this->imageHeight - 1 - y, x);
	color[0] = bInt; 
	color[1] = gInt; 
	color[2] = rInt;

	/*cv::Mat imgChannels[3];
	cv::split(*this->pixels, imgChannels);

	imgChannels[0].at<uchar>(this->imageHeight - 1 - y, x) = bInt;
	imgChannels[1].at<uchar>(this->imageHeight - 1 - y, x) = gInt;
	imgChannels[2].at<uchar>(this->imageHeight - 1 - y, x) = rInt;

	cv::merge(imgChannels, 3, *this->pixels);*/
}

void PNGImage::SaveImage(cv::String& fileName) const
{
	cv::imwrite(fileName, *this->pixels);
}