#pragma once

#include <opencv2/core.hpp>

class PNGImage
{
public:
	PNGImage(const int imageWidth, const int imageHeight);
	void SetPixel(int x, int y, float r, float g, float b, int samplesPerPixel);
	void SaveImage(cv::String &fileName) const;

private:
	std::unique_ptr<cv::Mat> pixels;
	int imageWidth;
	int imageHeight;
};

