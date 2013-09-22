#include "FileProcessing.h"
#include <stdexcept>

std::map<std::string, FileProcessing::Function > FileProcessing::tFunc;
std::map<std::string, int> FileProcessing::strtoFormat;
std::map<std::string, int> FileProcessing::normType;


int FileProcessing::use_PNG(const char* filename,const char* imageName,int complevel){

    // Mat mat(480, 640, CV_8UC4);
    //  createAlphaMat(mat); //Matrix  Do we want alpha channel for something!!

	Mat *image;
	image = pool->getImage(imageName);

	vector<int> compression_params;

	compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
	compression_params.push_back(complevel);
	
	// Add Extension to filename
	 char fname[150];
    strcpy( fname, filename);
	strcat(fname,".png");

	try{
		 Mat image2;
		 double min, max;
		 cv::minMaxLoc(*image, &min, &max); 
			
		 if(image->type()==CV_8U) 
			 image->convertTo(image2,CV_8UC1,255.0/(max - min), -min * 255.0/(max - min));
		 else image->convertTo(image2,CV_16UC1,65535.0/(max - min), -min * 65535.0/(max- min));
         imwrite(fname, image2, compression_params);
    }
    catch (const std::runtime_error& ex)
    {
        fprintf(stderr, "Exception converting image to PNG format: %s\n", ex.what());
        return -2211;
    }
    return 0;
}

/*********************************************************************************************************/
int FileProcessing::use_JP2(const char* filename,const char* imageName,int complevel){

	Mat *image;
	image = pool->getImage(imageName);

	vector<int> compression_params;

	compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
	compression_params.push_back(complevel);
	
	// Add Extension to filename
	 char fname[150];
    strcpy( fname, filename);
	strcat(fname,".jp2");

	try{
		 Mat image2;
		 double min, max;
		 cv::minMaxLoc(*image, &min, &max); 
			
		 if(image->type()==CV_8U) 
			 image->convertTo(image2,CV_8UC1,255.0/(max - min), -min * 255.0/(max - min));
		 else image->convertTo(image2,CV_16UC1,65535.0/(max - min), -min * 65535.0/(max- min));
         imwrite(fname, image2, compression_params);
    }
    catch (runtime_error& ex)
    {
        fprintf(stderr, "Exception converting image to JPEG2000 format: %s\n", ex.what());
        return -2211;
    }
    return 0;
}

/*******************************************************************************************************/
int FileProcessing::use_JPG(const char* filename,const char* imageName,int complevel){

	Mat *image;
	image = pool->getImage(imageName);

	vector<int> compression_params;

	compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
	compression_params.push_back(complevel);
	// Add Extension to filename
	 char fname[150];
    strcpy( fname, filename );
	strcat(fname,".jpg");

	 try {
         Mat image2;
		 double min, max;
		 minMaxLoc(*image, &min, &max); 
		 // Normalization is done for safety
		 image->convertTo(image2,CV_8UC1,255.0/(max - min), -min * 255.0/(max - min));
         imwrite(fname, image2, compression_params);
    }
    catch (const runtime_error& ex)
    {
        fprintf(stderr, "Exception converting image to JPG format: %s\n", ex.what());
        return -2031;
    }
    return 0;
}

int FileProcessing::use_TIFF(const char* filename,const char* imageName){

	Mat *image;
	image = pool->getImage(imageName);

	// Add Extension to filename
	 char fname[150];
    strcpy( fname, filename);
	strcat(fname,".tiff");

	 try {
		 Mat image2;
		 double min, max;
		 minMaxLoc(*image, &min, &max); 
		 // Normalization is done for safety
		 if(image->type()==CV_8U) image->convertTo(image2,CV_8UC1,255.0/(max - min), -min * 255.0/(max - min));
		 else	image->convertTo(image2,CV_16UC1,65535.0/(max - min), -min * 65535.0/(max- min));
         imwrite(fname, image2);
    }
    catch (runtime_error& ex) {
        fprintf(stderr, "Exception converting image to TIFF format: %s\n", ex.what());
        return -2211;
}
}

int FileProcessing::use_PM(const char* filename,const char* imageName,int code){

	Mat *image;
	image = pool->getImage(imageName);
	
	// Add Extension to filename
	 char fname[150];
    strcpy( fname, filename);
	switch(code)
	{
	case(FILEFORMATS::PBM) :strcat(fname,".pbm"); break;
	case(FILEFORMATS::PPM) :strcat(fname,".ppm"); break;
	case(FILEFORMATS::PGM) :strcat(fname,".pgm"); break;
	}
	try {
		 Mat image2;
		 double min, max;
		 minMaxLoc(*image, &min, &max); 
		 // Normalization is done for safety
		 image->convertTo(image2,CV_8UC1,255.0/(max - min), -min * 255.0/(max - min));
         imwrite(fname, image2);
    }
    catch (runtime_error& ex) {
        fprintf(stderr, "Exception converting image to Portable format (PPM,PBM or PGM): %s\n", ex.what());
        return -2211;
}
}

