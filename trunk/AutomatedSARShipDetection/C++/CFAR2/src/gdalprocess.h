/** 
 *
 * Programmed and Developed By:
 * Colin Schwegmann (colin.schwegmann@gmail.com)
 * For the Completion of Masters for
 * CSIR / University Of Pretoria
 * 
 * 2013
 *  
**/

#ifndef GDALPROCESS_H
#define GDALPROCESS_H

#include "gdal.h"
#include "gdal_alg.h"
#include "gdalwarper.h"
#include "cpl_vsi.h"
#include "cpl_conv.h"
#include "cpl_string.h"
#include "cpl_multiproc.h"
#include "gdal_priv.h"
#include "ogr_spatialref.h"
#include "ogr_srs_api.h"
#include "vrt/vrtdataset.h"
#include "commonutils.h"

// Include some basic libraries for handling
#include <string>
#include <iostream>
#include <sstream>

class GDALProcess
{

public:
GDALProcess(){};  

/// Taken from gdal_rasterize
int ArgIsNumeric( const char *pszArg );
void InvertGeometries( GDALDatasetH hDstDS, 
                              std::vector<OGRGeometryH> &ahGeometries );
void ProcessLayer( 
    OGRLayerH hSrcLayer, int bSRSIsSet, 
    GDALDatasetH hDstDS, std::vector<int> anBandList,
    std::vector<double> &adfBurnValues, int b3D, int bInverse,
    const char *pszBurnAttribute, char **papszRasterizeOptions,
    GDALProgressFunc pfnProgress, void* pProgressData );

int maskGEOTIFF( std::string inputFilename, int burnValue, std::string outputFilename);

/// Taken from gdal_translate
int writeGEOTIFF(std::string inputFilenameN1, std::string inputTiff, std::string outputGeotiff);

void SrcToDst( double dfX, double dfY,
                      int nSrcXOff, int nSrcYOff,
                      int nSrcXSize, int nSrcYSize,
                      int nDstXOff, int nDstYOff,
                      int nDstXSize, int nDstYSize,
                      double &dfXOut, double &dfYOut );
int FixSrcDstWindow( int* panSrcWin, int* panDstWin,
                            int nSrcRasterXSize,
                            int nSrcRasterYSize );
void AttachMetadata( GDALDatasetH, char ** );
void CopyBandInfo( GDALRasterBand * poSrcBand, GDALRasterBand * poDstBand,
                            int bCanCopyStatsMetadata, int bCopyScale, int bCopyNoData );


/// GDALWARP
int warpGEOTIFF(std::string inputFilename,
		std::string warpFormat,
		std::string outputFile);

// MYOWN
void getLATLONG(std::string inputFilename,
		std::vector<double> &x,
		std::vector<double> &y,		
		std::vector<double> &Lats,
		std::vector<double> &Longs);

GDALDatasetH 
GDALWarpCreateOutput( char *papszSrcFiles, const char *pszFilename, 
                      const char *pszFormat, char **papszTO, 
                      char ***ppapszCreateOptions, GDALDataType eDT,
                      void ** phTransformArg,
                      GDALDatasetH* phSrcDS,
		       int nForcePixels, int nForceLines,
                      int bQuiet, int bTargetAlignedPixels, 
		      int bEnableDstAlpha, 
		      int bEnableSrcAlpha, 
		      double dfXRes, double dfYRes, 
		      double dfMinX, double dfMinY,
		      double dfMaxX, double dfMaxY);

GDALDatasetH CreateOutputDataset(std::vector<OGRLayerH> ahLayers,
                                 OGRSpatialReferenceH hSRS,
                                 int bGotBounds, OGREnvelope sEnvelop,
                                 GDALDriverH hDriver, const char* pszDstFilename,
                                 int nXSize, int nYSize, double dfXRes, double dfYRes,
                                 int bTargetAlignedPixels,
                                 int nBandCount, GDALDataType eOutputType,
                                 char** papszCreateOptions, std::vector<double> adfInitVals,
                                 int bNoDataSet, double dfNoData);

void 
RemoveConflictingMetadata( GDALMajorObjectH hObj, char **papszMetadata,
                           const char *pszValueConflict );
char* SanitizeSRS( const char *pszUserInput );
int GDALExit( int nCode );

virtual ~GDALProcess();

private:

  int nGCPCount;
};

#endif // GDALPROCESS_H
