/// Load OpenCV to read image and use OpenCV's GUI interface
#include "opencv/cv.h"
#include "opencv/highgui.h"

/// Standard io 
#include <string>


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
#include "src/ossimCFARFilter.h"

int main(int argc, char** argv)
{
	// Set defualt values. 
	int scaleValue = 35;	// Used to scale values within SAR image
	int guardSize = 5;	// Guard window size (square)
	int neighbourSize = 7; // Neighbourhood window size (square) neighbourSize >= guardSize

	
	double cfarThreshold = 2.5; // Set CFAR threshold such that if target pixel > mean(pixels in neighbourhood)*threshold, then pixel is marked as true, else false. 

	std::string inputFilename = "/home/student/Programming/SD/data/sardata/ASA_WSM_1PNPDE20120222_075128_000001223112_00035_52204_4260.N1";
	std::string outputFileName = "results/cfar/ASAR_WSM_CFAR_RESULT.tiff";
	
	if (argc > 11) 
	{ 
	 // Check number of arguments and if not the required amount, inform user and exit. 
         std::cout << "Usage is: driver -fi <inputFilenameAndPath> -fo <outputFilenameAndPath> -g <length in pixels> -b <length in pixels> -t <decimal threshold>" << std::endl; 
 	 exit(0);
    	} 
	else 
	{ 
		std::cout << std::endl;
	
                /*
		  Iterate through each of the arguments and check each argument and value
		*/
		for (int i = 1; i < argc; i++) 
		{ 
		    if (i + 1 != argc) 
		    {	// Check that we have not reached end of argument list
			
			if (strcmp("-fi", argv[i]) == 0) 
			{
		            inputFilename = argv[i + 1];
		        } 
			else if (strcmp("-g", argv[i]) == 0) 
			{
		            guardSize = atoi(argv[i + 1]);
		        } 
			else if (strcmp("-b", argv[i]) == 0) 
			{
		            neighbourSize = atoi(argv[i + 1]);
		        } 
			else if (strcmp("-t", argv[i]) == 0) 
			{
		            cfarThreshold = atof(argv[i + 1]);
		        }
			else if (strcmp("-fo", argv[i]) == 0) 
			{
		            outputFileName = argv[i + 1];
		        }  
		    }
		}
		
		if (guardSize > neighbourSize)
		{
		  std::cout << "Please ensure Guard Window Length < Background Window Length" << std::endl; 
 	          exit(0);
		}

		if (cfarThreshold == 0 || cfarThreshold < 0)
		{
		  std::cout << "Please ensure CFAR threshold > 0" << std::endl; 
 	 	  exit(0);
		}
		
		std::cout << "Input file path and Name: " << inputFilename << std::endl;
		std::cout << "Output file path and Name: " << outputFileName << std::endl;
		std::cout << "Guard Window Length: " << guardSize << std::endl;
		std::cout << "Background Window Length: " << guardSize << std::endl;
		std::cout << "CFAR Threshold: " << cfarThreshold << std::endl;
		
		//return 0;
    	}
		


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
	      
	      //sic->createRenderedChain();	// Enabling this line causes my OSSIM to crash - I believe this is where the OSSIM
						// problem with georeferencing and ENVISAT ASAR (WSM) files comes into play.
	      
	      /// Get ossim-type pointer to image data
	      ossimRefPtr<ossimImageData> imageSourceData;
      
	      /// Get image tile and use entire sattelite image.
	      ossimIrect tileRect = handler->getBoundingRect(0);
	      
	      /// Use the handler to grab the image data
	      imageSourceData = handler->getTile(tileRect);
	       
	      /// Create filter (CFAR in this case)
	      ossimCFARFilter *filter = new ossimCFARFilter(imageSourceData.get());
	      filter->setScaleValue(scaleValue);
	      filter->setGuardSize(guardSize);
	      filter->setNeighbourSize(neighbourSize);
	      filter->setThreshold(cfarThreshold);
	      filter->setCFARMethod(0);			// O = OpenCV, 1 = indexing. Currently indexing does not work
							// and have to play around again to see what I need to fix with it
							// In the meantime use OpenCV version of CFAR - quite slow (about a minute for a 5k x 9k image)
	      filter->connectMyInputTo(0,handler);
		      
	      /// Write to tiff
	      ossimTiffWriter *writer = new ossimTiffWriter();
	      writer->setFilename(outputFileName);
	      writer->setGeotiffFlag(true);
	      writer->setOutputImageType("tiff_tiled_band_separate");
		      
	      /// Connect ossim writer and execute
	      writer->connectMyInputTo(filter);
	      writer->execute();
	      writer->close();

	      /// Close image handler 
	      handler->close();
	      
	    }
	    
	 sic->close();
	}

 return 0;
	
}
