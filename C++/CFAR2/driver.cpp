#define LAT 0
#define LON 1


/// Load OpenCV to read image and use OpenCV's GUI interface
#include "opencv/cv.h"
#include "opencv/highgui.h"

/// Standard io 
#include <iostream>
#include <sstream>
#include <numeric>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <iterator>

/// within this program ossimCommon is used for ossimGetScalarSizeInBytes.
/// This header will have some common globabl inline and non-inline functions

/// Include base ossim files and filename handling
#include "ossim/base/ossimCommon.h"
#include "ossim/base/ossimFilename.h"

/// Used to the name of the type of data within the handler
#include "ossim/base/ossimScalarTypeLut.h"

/// Used to search factories to find the correct one to load image 
#include "ossim/imaging/ossimImageHandlerRegistry.h"

/// Base pointer for the passed back object type
#include "ossim/imaging/ossimImageHandler.h"

/// Creates all the important factories used to read files
#include "ossim/init/ossimInit.h"

/// Add the dynamic (.so file) plugin support
/// For .N1 file GDAL plugin is required.
#include "ossim/plugin/ossimPluginLibrary.h"

/// Projection info for writing geotiff files
#include "ossim/projection/ossimMapProjection.h"
#include "ossim/projection/ossimMapProjectionInfo.h"

/// Writiting headers
#include "ossim/imaging/ossimImageFileWriter.h"
#include "ossim/imaging/ossimImageWriterFactoryRegistry.h"
#include "ossim/imaging/ossimImageSourceFactoryRegistry.h"
#include "ossim/imaging/ossimTiffWriter.h"
#include "ossim/imaging/ossimSingleImageChain.h"

/// Include filters
#include "src/ossimSimpleFilter.h"
#include "src/ossimGlobalFilter.h"
#include "src/ossimCFARFilter.h"
#include "src/ossimWaveletFilter.h"
#include "src/ossimSDFilter.h"

/// Include gdal
#include "src/gdalprocess.h"

using namespace std;

void processSD(std::string &inputName);

int main(int argc, char** argv)
{
  
	/// Check that total of 2 arguments are passed to the program
	if(argc != 2){
		cout << "./driver.out <text_file>" << endl;
		return 0;
	}
	
	std::string inputFilename;
	std::string inputFilenameSHP;
	std::string outputFolder;
	std::string convertType;
	std::string warpFormat = "WGS84";

	int processingType = 0; // 0 = none, 1 = global, 2 = cfar.....
	int globalThreshold = 0;
	int scaleValue = 35;
	int burnValue = 0;
	int guardSize = 5;
	int neighbourSize = 7;

	double cfarThreshold = 2.5;
	
	std::vector< std::string > inputFilenamesSHP;
	std::vector< std::string > inputFileNames;
	std::vector< std::string > inputNames;
	std::vector< std::string > inputNameFinals;
	std::vector< std::string > tempNames;
	
	std::ifstream infile(argv[1]);
	std::string line;
	vector<string> tokens;
	while (std::getline(infile, line))
	{
	    std::istringstream iss(line);
	    std::istringstream ss;
	    copy(istream_iterator<string>(iss),
		  istream_iterator<string>(),
		  back_inserter<vector<string> >(tokens));
	    
	    int tokenSize = tokens.size();
	    
	    switch (tokenSize)
	    {
	      case 3:
		std::cout << "Running conversion" << std::endl;
		inputFilename = tokens.at(0);
		inputFilenameSHP = tokens.at(1);
		outputFolder = tokens.at(2);
		processingType = 0;
		convertType = "";
	      break;
	      case 5:
		std::cout << "Running global thresholding ship detection" << std::endl;
		inputFilename = tokens.at(0);
		inputFilenameSHP = tokens.at(1);
		outputFolder = tokens.at(2);
		ss.str(tokens.at(4));
		ss >> globalThreshold;
		processingType = 1;
		convertType = "";
	      break;
	      case 7:
		std::cout << "Running CFAR thresholding ship detection" << std::endl;
		inputFilename = tokens.at(0);
		inputFilenameSHP = tokens.at(1);
		outputFolder = tokens.at(2);
		ss.str(tokens.at(4));
		ss >> guardSize;
		ss.str(tokens.at(5));
		ss >> neighbourSize;
		ss.str(tokens.at(6));
		ss >> cfarThreshold;
		processingType = 2;
		convertType = "cfar/";
	      break;
	      default:
		break;
	    }
	    
	std::cout << "Processing image: " << inputFilename << std::endl;
	
	/// Register all the required file names
	ossimString drivePart,pathPart, filePart, extPart;
	ossimFilename(inputFilename.c_str()).split(drivePart,pathPart,filePart,extPart);
	
	std::string inputName = outputFolder + convertType + filePart + ".tiff";
	std::string inputNameFinal = outputFolder + convertType  + filePart + "Final.tiff";
	std::string tempFileName = outputFolder + convertType  + filePart + "TEMP.tiff";

	inputFileNames.push_back(inputFilename);
	inputFilenamesSHP.push_back(inputFilenameSHP);
	inputNames.push_back(inputName);
	inputNameFinals.push_back(inputNameFinal);
	tempNames.push_back(tempFileName);

	/// Also load ossim plugin system and GDAL plugin (for .N1 file support).
	ossimInit::instance()->initialize();

	/// Initialise single image chain
	ossimRefPtr<ossimSingleImageChain> sic = new ossimSingleImageChain();
	
	/// Check if image is null
	ossimImageHandler *testHandler = ossimImageHandlerRegistry::instance()->open(ossimFilename(inputFilename.c_str()));
	if(testHandler == NULL) 
	{
	 cout << "Image file cannot be opened" << endl;
	 exit(1);
	}
	else
	{
	    testHandler->close();
	    if (sic->open(ossimFilename(inputFilename.c_str())))
	    {
	      
	      /// Create a handle to the image.
	      ossimImageHandler *handler = ossimImageHandlerRegistry::instance()->open(ossimFilename(inputFilename.c_str()));
	      
	      //sic->createRenderedChain();
	      
	      /// Get ossim-type pointer to image data
	      ossimRefPtr<ossimImageData> imageSourceData;
      
	      /// Get image tile and use entire sattelite image.
	      ossimIrect tileRect = handler->getBoundingRect(0);
	      
	      // Get the image data from the handler for the tile, in this case the entire image
	      /// Use the handler to grab the image data and place it in the source file
	      imageSourceData = handler->getTile(tileRect);
	       
	      /// Create filter
	      if(processingType == 0)
	      {
		ossimSimpleFilter *filter = new ossimSimpleFilter(imageSourceData.get());
		filter->setScaleValue(scaleValue);
		filter->connectMyInputTo(0,handler);
		
		/// Write to tiff
		ossimTiffWriter *writer = new ossimTiffWriter();
		writer->setFilename(inputName);
		writer->setGeotiffFlag(true);
		writer->setOutputImageType("tiff_tiled_band_separate");
		
		/// Connect and execute
		writer->connectMyInputTo(filter);
		writer->execute();
		writer->close();	    
		
	      }
	      else
	      if(processingType == 1)
	      {
		ossimGlobalFilter *filter = new ossimGlobalFilter(imageSourceData.get());
		filter->setScaleValue(scaleValue);
		filter->setThreshold(globalThreshold);
		filter->connectMyInputTo(0,handler);
		
		/// Write to tiff
		ossimTiffWriter *writer = new ossimTiffWriter();
		writer->setFilename(inputName);
		writer->setGeotiffFlag(true);
		writer->setOutputImageType("tiff_tiled_band_separate");
		
		/// Connect and execute
		writer->connectMyInputTo(filter);
		writer->execute();
		writer->close();
		
	      }
	      else
	      if(processingType == 2)
	      {
		ossimCFARFilter *filter = new ossimCFARFilter(imageSourceData.get());
		filter->setScaleValue(scaleValue);
		filter->setGuardSize(guardSize);
		filter->setNeighbourSize(neighbourSize);
		filter->setThreshold(cfarThreshold);
		filter->setCFARMethod(0);		// O = OpenCV, 1 = indexing
		filter->connectMyInputTo(0,handler);
		
		/// Write to tiff
		ossimTiffWriter *writer = new ossimTiffWriter();
		writer->setFilename(inputName);
		writer->setGeotiffFlag(true);
		writer->setOutputImageType("tiff_tiled_band_separate");
		
		/// Connect and execute
		writer->connectMyInputTo(filter);
		writer->execute();
		writer->close();
	      }
	      else
	      {
		ossimSimpleFilter *filter = new ossimSimpleFilter(imageSourceData.get());
		filter->setScaleValue(scaleValue);
		filter->connectMyInputTo(0,handler);
		
		/// Write to tiff
		ossimTiffWriter *writer = new ossimTiffWriter();
		writer->setFilename(inputName);
		writer->setGeotiffFlag(true);
		writer->setOutputImageType("tiff_tiled_band_separate");
		
		/// Connect and execute
		writer->connectMyInputTo(filter);
		writer->execute();
		writer->close();	  
	      }

	      handler->close();
	      
	    }
	    
	    sic->close();
	    tokens.clear();
	    
	  std::cout << std::endl;
	  std::cout << std::endl;
	  }
}
	  std::string inputName;
	  std::string inputNameFinal;
	  std::string tempFileName;
	  
	  for (unsigned int i = 0; i < tempNames.size(); i++)
	  {
	    inputFilename = inputFileNames.at(i);
	    inputFilenameSHP = inputFilenamesSHP.at(i);
	    inputName = inputNames.at(i);
	    inputNameFinal = inputNameFinals.at(i);
	    tempFileName = tempNames.at(i);
	    
	    //Process sd afterwards (temporary)
	    processSD(inputName);
	    
	    // Use GDAL Processor to process image into masked geotiff images
	    GDALProcess *gdalProcessor = new GDALProcess();
	    std::cout << "Processing Image (Georeferencing)" << std::endl;
	    double t = (double) cv::getTickCount();
	    gdalProcessor->writeGEOTIFF(inputFilename,inputName, tempFileName);
	    t = ((double)cv::getTickCount() - t)/cv::getTickFrequency();
	    std::cout << "Processing Image (Georeferencing) completed in: " << t << " seconds" << std::endl;
	    
	    std::cout << "Processing Image (Warping to WGS84)" << std::endl;
	    t = (double) cv::getTickCount();
	    gdalProcessor->warpGEOTIFF(tempFileName, warpFormat, inputNameFinal);
	    t = ((double)cv::getTickCount() - t)/cv::getTickFrequency();
	    std::cout << "Processing Image (Warping to WGS84) completed in: " << t << " seconds" << std::endl;
	    
	    std::cout << "Processing Image (Land Masking)" << std::endl;
	    t = (double) cv::getTickCount();
	    gdalProcessor->maskGEOTIFF(inputFilenameSHP,burnValue,inputNameFinal);
	    t = ((double)cv::getTickCount() - t)/cv::getTickFrequency();
	    std::cout << "Processing Image (Land Masking) completed in: " << t << " seconds" << std::endl;
	   
	  }
	
	return 0;
	
}

void processSD(std::string &inputName)
{
  cv::Mat inputImage, outputImage;
  double bandwidth = 10;
  int spacing = 2;
  int sdType = 0;
  double bw = bandwidth;
  double rate = 0.5;
  int iterMax = 1000;
  
  
  //Open Image and then make it a binary image
  inputImage = cv::imread(inputName.c_str(), CV_LOAD_IMAGE_GRAYSCALE);   // Read the file
  inputImage = inputImage > 0;
  
  //As it stands there isn't a way to access the whole satellite image at once through OSSIM so
  //process it after it has been processed by tile.
  ossimSDFilter *filter2 = new ossimSDFilter();
  filter2->setScaleValue(1.0);
  filter2->setSDType(sdType);
  filter2->setSpacing(spacing);
  filter2->setBandwidth(bw);
  filter2->setDescendRate(rate);
  filter2->setMaxIterations(iterMax);
  filter2->simpleSD(inputImage, outputImage);
  delete(filter2);
  
  cv::imwrite(inputName.c_str(), outputImage);
 
}
