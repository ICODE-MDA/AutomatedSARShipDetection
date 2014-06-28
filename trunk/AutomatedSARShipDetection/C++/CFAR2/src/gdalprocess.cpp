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

#include "gdalprocess.h"

GDALProcess::~GDALProcess()
{

}

enum
{
    MASK_DISABLED,
    MASK_AUTO,
    MASK_USER
};


/************************************************************************/
/*                            ArgIsNumeric()                            */
/************************************************************************/

int GDALProcess::ArgIsNumeric( const char *pszArg )

{
    return CPLGetValueType(pszArg) != CPL_VALUE_STRING;
}

/************************************************************************/
/*                          InvertGeometries()                          */
/************************************************************************/

void GDALProcess::InvertGeometries( GDALDatasetH hDstDS, 
                              std::vector<OGRGeometryH> &ahGeometries )

{
    OGRGeometryH hCollection = 
        OGR_G_CreateGeometry( wkbGeometryCollection );

/* -------------------------------------------------------------------- */
/*      Create a ring that is a bit outside the raster dataset.         */
/* -------------------------------------------------------------------- */
    OGRGeometryH hUniversePoly, hUniverseRing;
    double adfGeoTransform[6];
    int brx = GDALGetRasterXSize( hDstDS ) + 2;
    int bry = GDALGetRasterYSize( hDstDS ) + 2;

    GDALGetGeoTransform( hDstDS, adfGeoTransform );

    hUniverseRing = OGR_G_CreateGeometry( wkbLinearRing );
    
    OGR_G_AddPoint_2D( 
        hUniverseRing, 
        adfGeoTransform[0] + -2*adfGeoTransform[1] + -2*adfGeoTransform[2],
        adfGeoTransform[3] + -2*adfGeoTransform[4] + -2*adfGeoTransform[5] );
                       
    OGR_G_AddPoint_2D( 
        hUniverseRing, 
        adfGeoTransform[0] + brx*adfGeoTransform[1] + -2*adfGeoTransform[2],
        adfGeoTransform[3] + brx*adfGeoTransform[4] + -2*adfGeoTransform[5] );
                       
    OGR_G_AddPoint_2D( 
        hUniverseRing, 
        adfGeoTransform[0] + brx*adfGeoTransform[1] + bry*adfGeoTransform[2],
        adfGeoTransform[3] + brx*adfGeoTransform[4] + bry*adfGeoTransform[5] );
                       
    OGR_G_AddPoint_2D( 
        hUniverseRing, 
        adfGeoTransform[0] + -2*adfGeoTransform[1] + bry*adfGeoTransform[2],
        adfGeoTransform[3] + -2*adfGeoTransform[4] + bry*adfGeoTransform[5] );
                       
    OGR_G_AddPoint_2D( 
        hUniverseRing, 
        adfGeoTransform[0] + -2*adfGeoTransform[1] + -2*adfGeoTransform[2],
        adfGeoTransform[3] + -2*adfGeoTransform[4] + -2*adfGeoTransform[5] );
                       
    hUniversePoly = OGR_G_CreateGeometry( wkbPolygon );
    OGR_G_AddGeometryDirectly( hUniversePoly, hUniverseRing );

    OGR_G_AddGeometryDirectly( hCollection, hUniversePoly );
    
/* -------------------------------------------------------------------- */
/*      Add the rest of the geometries into our collection.             */
/* -------------------------------------------------------------------- */
    unsigned int iGeom;

    for( iGeom = 0; iGeom < ahGeometries.size(); iGeom++ )
        OGR_G_AddGeometryDirectly( hCollection, ahGeometries[iGeom] );

    ahGeometries.resize(1);
    ahGeometries[0] = hCollection;
}

/************************************************************************/
/*                            ProcessLayer()                            */
/*                                                                      */
/*      Process all the features in a layer selection, collecting       */
/*      geometries and burn values.                                     */
/************************************************************************/

void GDALProcess::ProcessLayer( 
    OGRLayerH hSrcLayer, int bSRSIsSet, 
    GDALDatasetH hDstDS, std::vector<int> anBandList,
    std::vector<double> &adfBurnValues, int b3D, int bInverse,
    const char *pszBurnAttribute, char **papszRasterizeOptions,
    GDALProgressFunc pfnProgress, void* pProgressData )

{
/* -------------------------------------------------------------------- */
/*      Checkout that SRS are the same.                                 */
/*      If -a_srs is specified, skip the test                           */
/* -------------------------------------------------------------------- */
    if (!bSRSIsSet)
    {
        OGRSpatialReferenceH  hDstSRS = NULL;
        if( GDALGetProjectionRef( hDstDS ) != NULL )
        {
            char *pszProjection;
    
            pszProjection = (char *) GDALGetProjectionRef( hDstDS );
    
            hDstSRS = OSRNewSpatialReference(NULL);
            if( OSRImportFromWkt( hDstSRS, &pszProjection ) != CE_None )
            {
                OSRDestroySpatialReference(hDstSRS);
                hDstSRS = NULL;
            }
        }
    
        OGRSpatialReferenceH hSrcSRS = OGR_L_GetSpatialRef(hSrcLayer);
        if( hDstSRS != NULL && hSrcSRS != NULL )
        {
            if( OSRIsSame(hSrcSRS, hDstSRS) == FALSE )
            {
//                 fprintf(stderr,
//                         "Warning : the output raster dataset and the input vector layer do not have the same SRS.\n"
//                         "Results might be incorrect (no on-the-fly reprojection of input data).\n");
            }
        }
        else if( hDstSRS != NULL && hSrcSRS == NULL )
        {
//             fprintf(stderr,
//                     "Warning : the output raster dataset has a SRS, but the input vector layer SRS is unknown.\n"
//                     "Ensure input vector has the same SRS, otherwise results might be incorrect.\n");
        }
        else if( hDstSRS == NULL && hSrcSRS != NULL )
        {
//             fprintf(stderr,
//                     "Warning : the input vector layer has a SRS, but the output raster dataset SRS is unknown.\n"
//                     "Ensure output raster dataset has the same SRS, otherwise results might be incorrect.\n");
        }
    
        if( hDstSRS != NULL )
        {
            OSRDestroySpatialReference(hDstSRS);
        }
    }

/* -------------------------------------------------------------------- */
/*      Get field index, and check.                                     */
/* -------------------------------------------------------------------- */
    int iBurnField = -1;

    if( pszBurnAttribute )
    {
        iBurnField = OGR_FD_GetFieldIndex( OGR_L_GetLayerDefn( hSrcLayer ),
                                           pszBurnAttribute );
        if( iBurnField == -1 )
        {
            printf( "Failed to find field %s on layer %s, skipping.\n",
                    pszBurnAttribute, 
                    OGR_FD_GetName( OGR_L_GetLayerDefn( hSrcLayer ) ) );
            return;
        }
    }

/* -------------------------------------------------------------------- */
/*      Collect the geometries from this layer, and build list of       */
/*      burn values.                                                    */
/* -------------------------------------------------------------------- */
    OGRFeatureH hFeat;
    std::vector<OGRGeometryH> ahGeometries;
    std::vector<double> adfFullBurnValues;

    OGR_L_ResetReading( hSrcLayer );
    
    while( (hFeat = OGR_L_GetNextFeature( hSrcLayer )) != NULL )
    {
        OGRGeometryH hGeom;

        if( OGR_F_GetGeometryRef( hFeat ) == NULL )
        {
            OGR_F_Destroy( hFeat );
            continue;
        }

        hGeom = OGR_G_Clone( OGR_F_GetGeometryRef( hFeat ) );
        ahGeometries.push_back( hGeom );

        for( unsigned int iBand = 0; iBand < anBandList.size(); iBand++ )
        {
            if( adfBurnValues.size() > 0 )
                adfFullBurnValues.push_back( 
                    adfBurnValues[MIN(iBand,adfBurnValues.size()-1)] );
            else if( pszBurnAttribute )
            {
                adfFullBurnValues.push_back( OGR_F_GetFieldAsDouble( hFeat, iBurnField ) );
            }
            /* I have made the 3D option exclusive to other options since it
               can be used to modify the value from "-burn value" or
               "-a attribute_name" */
            if( b3D )
            {
                // TODO: get geometry "z" value
                /* Points and Lines will have their "z" values collected at the
                   point and line levels respectively. However filled polygons
                   (GDALdllImageFilledPolygon) can use some help by getting
                   their "z" values here. */
                adfFullBurnValues.push_back( 0.0 );
            }
        }
        
        OGR_F_Destroy( hFeat );
    }

/* -------------------------------------------------------------------- */
/*      If we are in inverse mode, we add one extra ring around the     */
/*      whole dataset to invert the concept of insideness and then      */
/*      merge everything into one geometry collection.                  */
/* -------------------------------------------------------------------- */
    if( bInverse )
    {
        if( ahGeometries.size() == 0 )
        {
            for( unsigned int iBand = 0; iBand < anBandList.size(); iBand++ )
            {
                if( adfBurnValues.size() > 0 )
                    adfFullBurnValues.push_back(
                        adfBurnValues[MIN(iBand,adfBurnValues.size()-1)] );
                else /* FIXME? Not sure what to do exactly in the else case, but we must insert a value */
                    adfFullBurnValues.push_back( 0.0 );
            }
        }

        InvertGeometries( hDstDS, ahGeometries );
    }

/* -------------------------------------------------------------------- */
/*      Perform the burn.                                               */
/* -------------------------------------------------------------------- */
    GDALRasterizeGeometries( hDstDS, anBandList.size(), &(anBandList[0]), 
                             ahGeometries.size(), &(ahGeometries[0]), 
                             NULL, NULL, &(adfFullBurnValues[0]), 
                             papszRasterizeOptions,
                             pfnProgress, pProgressData );

/* -------------------------------------------------------------------- */
/*      Cleanup geometries.                                             */
/* -------------------------------------------------------------------- */
    int iGeom;

    for( iGeom = ahGeometries.size()-1; iGeom >= 0; iGeom-- )
        OGR_G_DestroyGeometry( ahGeometries[iGeom] );
}

/************************************************************************/
/*                  CreateOutputDataset()                               */
/************************************************************************/

GDALDatasetH GDALProcess::CreateOutputDataset(std::vector<OGRLayerH> ahLayers,
                                 OGRSpatialReferenceH hSRS,
                                 int bGotBounds, OGREnvelope sEnvelop,
                                 GDALDriverH hDriver, const char* pszDstFilename,
                                 int nXSize, int nYSize, double dfXRes, double dfYRes,
                                 int bTargetAlignedPixels,
                                 int nBandCount, GDALDataType eOutputType,
                                 char** papszCreateOptions, std::vector<double> adfInitVals,
                                 int bNoDataSet, double dfNoData)
{
    int bFirstLayer = TRUE;
    char* pszWKT = NULL;
    GDALDatasetH hDstDS = NULL;
    unsigned int i;

    for( i = 0; i < ahLayers.size(); i++ )
    {
        OGRLayerH hLayer = ahLayers[i];

        if (!bGotBounds)
        {
            OGREnvelope sLayerEnvelop;

            if (OGR_L_GetExtent(hLayer, &sLayerEnvelop, TRUE) != OGRERR_NONE)
            {
                fprintf(stderr, "Cannot get layer extent\n");
                exit(2);
            }

            /* When rasterizing point layers and that the bounds have */
            /* not been explicitely set, voluntary increase the extent by */
            /* a half-pixel size to avoid missing points on the border */
            if (wkbFlatten(OGR_L_GetGeomType(hLayer)) == wkbPoint &&
                !bTargetAlignedPixels && dfXRes != 0 && dfYRes != 0)
            {
                sLayerEnvelop.MinX -= dfXRes / 2;
                sLayerEnvelop.MaxX += dfXRes / 2;
                sLayerEnvelop.MinY -= dfYRes / 2;
                sLayerEnvelop.MaxY += dfYRes / 2;
            }

            if (bFirstLayer)
            {
                sEnvelop.MinX = sLayerEnvelop.MinX;
                sEnvelop.MinY = sLayerEnvelop.MinY;
                sEnvelop.MaxX = sLayerEnvelop.MaxX;
                sEnvelop.MaxY = sLayerEnvelop.MaxY;

                if (hSRS == NULL)
                    hSRS = OGR_L_GetSpatialRef(hLayer);

                bFirstLayer = FALSE;
            }
            else
            {
                sEnvelop.MinX = MIN(sEnvelop.MinX, sLayerEnvelop.MinX);
                sEnvelop.MinY = MIN(sEnvelop.MinY, sLayerEnvelop.MinY);
                sEnvelop.MaxX = MAX(sEnvelop.MaxX, sLayerEnvelop.MaxX);
                sEnvelop.MaxY = MAX(sEnvelop.MaxY, sLayerEnvelop.MaxY);
            }
        }
        else
        {
            if (bFirstLayer)
            {
                if (hSRS == NULL)
                    hSRS = OGR_L_GetSpatialRef(hLayer);

                bFirstLayer = FALSE;
            }
        }
    }

    if (dfXRes == 0 && dfYRes == 0)
    {
        dfXRes = (sEnvelop.MaxX - sEnvelop.MinX) / nXSize;
        dfYRes = (sEnvelop.MaxY - sEnvelop.MinY) / nYSize;
    }
    else if (bTargetAlignedPixels && dfXRes != 0 && dfYRes != 0)
    {
        sEnvelop.MinX = floor(sEnvelop.MinX / dfXRes) * dfXRes;
        sEnvelop.MaxX = ceil(sEnvelop.MaxX / dfXRes) * dfXRes;
        sEnvelop.MinY = floor(sEnvelop.MinY / dfYRes) * dfYRes;
        sEnvelop.MaxY = ceil(sEnvelop.MaxY / dfYRes) * dfYRes;
    }

    double adfProjection[6];
    adfProjection[0] = sEnvelop.MinX;
    adfProjection[1] = dfXRes;
    adfProjection[2] = 0;
    adfProjection[3] = sEnvelop.MaxY;
    adfProjection[4] = 0;
    adfProjection[5] = -dfYRes;

    if (nXSize == 0 && nYSize == 0)
    {
        nXSize = (int)(0.5 + (sEnvelop.MaxX - sEnvelop.MinX) / dfXRes);
        nYSize = (int)(0.5 + (sEnvelop.MaxY - sEnvelop.MinY) / dfYRes);
    }

    hDstDS = GDALCreate(hDriver, pszDstFilename, nXSize, nYSize,
                        nBandCount, eOutputType, papszCreateOptions);
    if (hDstDS == NULL)
    {
        fprintf(stderr, "Cannot create %s\n", pszDstFilename);
        exit(2);
    }

    GDALSetGeoTransform(hDstDS, adfProjection);

    if (hSRS)
        OSRExportToWkt(hSRS, &pszWKT);
    if (pszWKT)
        GDALSetProjection(hDstDS, pszWKT);
    CPLFree(pszWKT);

    int iBand;
    /*if( nBandCount == 3 || nBandCount == 4 )
    {
        for(iBand = 0; iBand < nBandCount; iBand++)
        {
            GDALRasterBandH hBand = GDALGetRasterBand(hDstDS, iBand + 1);
            GDALSetRasterColorInterpretation(hBand, (GDALColorInterp)(GCI_RedBand + iBand));
        }
    }*/

    if (bNoDataSet)
    {
        for(iBand = 0; iBand < nBandCount; iBand++)
        {
            GDALRasterBandH hBand = GDALGetRasterBand(hDstDS, iBand + 1);
            GDALSetRasterNoDataValue(hBand, dfNoData);
        }
    }

    if (adfInitVals.size() != 0)
    {
        for(iBand = 0; iBand < MIN(nBandCount,(int)adfInitVals.size()); iBand++)
        {
            GDALRasterBandH hBand = GDALGetRasterBand(hDstDS, iBand + 1);
            GDALFillRaster(hBand, adfInitVals[iBand], 0);
        }
    }

    return hDstDS;
}

int GDALProcess::maskGEOTIFF( std::string inputFilename, int burnValue, std::string outputFilename)
{
    int i, b3D = FALSE;
    int bInverse = FALSE;
    const char *pszSrcFilename = NULL;
    const char *pszDstFilename = NULL;
    char **papszLayers = NULL;
    const char *pszBurnAttribute = NULL;
    std::vector<int> anBandList;
    std::vector<double> adfBurnValues;
    char **papszRasterizeOptions = NULL;
    char **papszCreateOptions = NULL;
    std::vector<double> adfInitVals;
    OGREnvelope sEnvelop;
    OGRSpatialReferenceH hSRS = NULL;
    
    GDALAllRegister();
    OGRRegisterAll();
    
    adfBurnValues.push_back(burnValue);
    pszSrcFilename = inputFilename.c_str();
    pszDstFilename = outputFilename.c_str();
    
    if( anBandList.size() == 0 )
       anBandList.push_back( 1 );

/* -------------------------------------------------------------------- */
/*      Open source vector dataset.                                     */
/* -------------------------------------------------------------------- */
    OGRDataSourceH hSrcDS;

    hSrcDS = OGROpen( pszSrcFilename, FALSE, NULL );
    papszLayers = CSLAddString(NULL, OGR_L_GetName(OGR_DS_GetLayer(hSrcDS, 0)));

/* -------------------------------------------------------------------- */
/*      Open target raster file.  Eventually we will add optional       */
/*      creation.                                                       */
/* -------------------------------------------------------------------- */
    GDALDatasetH hDstDS = NULL;
    
    hDstDS = GDALOpen( pszDstFilename, GA_Update );

/* -------------------------------------------------------------------- */
/*      Create output file if necessary.                                */
/* -------------------------------------------------------------------- */
    int nLayerCount = CSLCount(papszLayers);

/* -------------------------------------------------------------------- */
/*      Process each layer.                                             */
/* -------------------------------------------------------------------- */

    for( i = 0; i < nLayerCount; i++ )
    {
        OGRLayerH hLayer = OGR_DS_GetLayerByName( hSrcDS, papszLayers[i] );

        ProcessLayer( hLayer, hSRS != NULL, hDstDS, anBandList, 
                      adfBurnValues, b3D, bInverse, pszBurnAttribute,
                      papszRasterizeOptions, NULL, NULL );
    }

/* -------------------------------------------------------------------- */
/*      Cleanup                                                         */
/* -------------------------------------------------------------------- */

    OGR_DS_Destroy( hSrcDS );
    GDALClose( hDstDS );

    OSRDestroySpatialReference(hSRS);

    CSLDestroy( papszRasterizeOptions );
    CSLDestroy( papszLayers );
    CSLDestroy( papszCreateOptions );
    
    GDALDestroyDriverManager();
    OGRCleanupAll();

    return 0;
}

void GDALProcess::getLATLONG(std::string inputFilename,
		std::vector<double> &x,
		std::vector<double> &y,		
		std::vector<double> &Lats,
		std::vector<double> &Longs)
{
    GDALDataset  *poDataset;

    GDALAllRegister();

    poDataset = (GDALDataset *) GDALOpen(inputFilename.c_str(), GA_ReadOnly );

    if( poDataset != NULL )
    {
        if( poDataset->GetProjectionRef()  != NULL )
        {
          const char *sProj = poDataset->GetProjectionRef();
	  OGRSpatialReference oSourceSRS(sProj), oTargetSRS;
	  OGRCoordinateTransformation *poCT;
	  double                  x, y;
            
          oTargetSRS.importFromEPSG(4326);
            
          poCT = OGRCreateCoordinateTransformation( &oSourceSRS,
                                                  &oTargetSRS );
          x = 5;
          y = 5;

	  int ans =  (poCT == NULL);
            
	  std::cout << ans << std::endl;
          if( poCT == NULL || !poCT->Transform( 1, &x, &y ) )
              printf( "Transformation failed.\n" );
        else
              printf( "(%f,%f) -> (%f,%f)\n", 
                    5.0,
                    5.0,
                    x, y );
	}
    }

}


/// Conversion and merging of gdal_translat
int GDALProcess::writeGEOTIFF(std::string inputFilenameN1, 
				 std::string inputTiff, 
				 std::string outputGeotiff)
{
    GDALDatasetH	hDataset, hOutDS;
    int			i;
    int			nRasterXSize, nRasterYSize;
    const char		*pszSource=NULL, *pszDest=NULL, *pszFormat = "GTiff";
    GDALDriverH		hDriver;
    int			*panBandList = NULL; /* negative value of panBandList[i] means mask band of ABS(panBandList[i]) */
    int         nBandCount = 0, bDefBands = TRUE;
    double		adfGeoTransform[6];
    GDALDataType	eOutputType = GDT_Unknown;
    int			nOXSize = 0, nOYSize = 0;
    char		*pszOXSize=NULL, *pszOYSize=NULL;
    char                **papszCreateOptions = NULL;
    int                 anSrcWin[4], bStrict = FALSE;
    const char          *pszProjection;
    int                 bScale = FALSE, bHaveScaleSrc = FALSE, bUnscale=FALSE;
    double	        dfScaleSrcMin=0.0, dfScaleSrcMax=255.0;
    double              dfScaleDstMin=0.0, dfScaleDstMax=255.0;
    double              dfULX, dfULY, dfLRX, dfLRY;
    char                **papszMetadataOptions = NULL;
    char                *pszOutputSRS = NULL;
    int                 bQuiet = TRUE, bGotBounds = FALSE;
    double              adfULLR[4] = { 0,0,0,0 };
    int                 bSetNoData = FALSE;
    int                 bUnsetNoData = FALSE;
    double		dfNoDataReal = 0.0;
    int                 nRGBExpand = 0;
    int                 eMaskMode = MASK_AUTO;
    int                 nMaskBand = 0; /* negative value means mask band of ABS(nMaskBand) */
    int                 bStats = FALSE, bApproxStats = FALSE;
    int                 bErrorOnPartiallyOutside = FALSE;
    int                 bErrorOnCompletelyOutside = FALSE;


    anSrcWin[0] = 0;
    anSrcWin[1] = 0;
    anSrcWin[2] = 0;
    anSrcWin[3] = 0;

    dfULX = dfULY = dfLRX = dfLRY = 0.0;
    
GDALAllRegister();

    pszFormat = "GTiff";
    pszSource = inputTiff.c_str();
    pszDest = outputGeotiff.c_str();

    
    GDALDatasetH *hDataset2 = (GDALDatasetH*) GDALOpen(inputFilenameN1.c_str(),GA_ReadOnly);
    nGCPCount = GDALGetGCPCount( hDataset2 );
/* -------------------------------------------------------------------- */
/*      Attempt to open source file.                                    */
/* -------------------------------------------------------------------- */

    hDataset = GDALOpenShared( pszSource, GA_ReadOnly );
    
    if( hDataset == NULL )
    {
        fprintf( stderr,
                 "GDALOpen failed - %d\n%s\n",
                 CPLGetLastErrorNo(), CPLGetLastErrorMsg() );
        GDALDestroyDriverManager();
        exit( 1 );
    }

/* -------------------------------------------------------------------- */
/*      Collect some information from the source file.                  */
/* -------------------------------------------------------------------- */
    nRasterXSize = GDALGetRasterXSize( hDataset );
    nRasterYSize = GDALGetRasterYSize( hDataset );

    if( !bQuiet )
        printf( "Input file size is %d, %d\n", nRasterXSize, nRasterYSize );

    if( anSrcWin[2] == 0 && anSrcWin[3] == 0 )
    {
        anSrcWin[2] = nRasterXSize;
        anSrcWin[3] = nRasterYSize;
    }

/* -------------------------------------------------------------------- */
/*	Build band list to translate					*/
/* -------------------------------------------------------------------- */
    if( nBandCount == 0 )
    {
        nBandCount = GDALGetRasterCount( hDataset );
        panBandList = (int *) CPLMalloc(sizeof(int)*nBandCount);
        for( i = 0; i < nBandCount; i++ )
            panBandList[i] = i+1;
    }
    else
    {
        for( i = 0; i < nBandCount; i++ )
        {
            if( ABS(panBandList[i]) > GDALGetRasterCount(hDataset) )
            {
                fprintf( stderr, 
                         "Band %d requested, but only bands 1 to %d available.\n",
                         ABS(panBandList[i]), GDALGetRasterCount(hDataset) );
                GDALDestroyDriverManager();
                exit( 2 );
            }
        }

        if( nBandCount != GDALGetRasterCount( hDataset ) )
            bDefBands = FALSE;
    }

/* -------------------------------------------------------------------- */
/*      Compute the source window from the projected source window      */
/*      if the projected coordinates were provided.  Note that the      */
/*      projected coordinates are in ulx, uly, lrx, lry format,         */
/*      while the anSrcWin is xoff, yoff, xsize, ysize with the         */
/*      xoff,yoff being the ulx, uly in pixel/line.                     */
/* -------------------------------------------------------------------- */
    if( dfULX != 0.0 || dfULY != 0.0 
        || dfLRX != 0.0 || dfLRY != 0.0 )
    {
        double	adfGeoTransform[6];

        GDALGetGeoTransform( hDataset, adfGeoTransform );

        anSrcWin[0] = (int) 
            ((dfULX - adfGeoTransform[0]) / adfGeoTransform[1] + 0.001);
        anSrcWin[1] = (int) 
            ((dfULY - adfGeoTransform[3]) / adfGeoTransform[5] + 0.001);

        anSrcWin[2] = (int) ((dfLRX - dfULX) / adfGeoTransform[1] + 0.5);
        anSrcWin[3] = (int) ((dfLRY - dfULY) / adfGeoTransform[5] + 0.5);

        if( !bQuiet )
            fprintf( stdout, 
                     "Computed -srcwin %d %d %d %d from projected window.\n",
                     anSrcWin[0], 
                     anSrcWin[1], 
                     anSrcWin[2], 
                     anSrcWin[3] );
    }

/* -------------------------------------------------------------------- */
/*      Verify source window dimensions.                                */
/* -------------------------------------------------------------------- */
    if( anSrcWin[2] <= 0 || anSrcWin[3] <= 0 )
    {
        fprintf( stderr,
                 "Error: %s-srcwin %d %d %d %d has negative width and/or height.\n",
                 ( dfULX != 0.0 || dfULY != 0.0 || dfLRX != 0.0 || dfLRY != 0.0 ) ? "Computed " : "",
                 anSrcWin[0],
                 anSrcWin[1],
                 anSrcWin[2],
                 anSrcWin[3] );
        exit( 1 );
    }

/* -------------------------------------------------------------------- */
/*      Verify source window dimensions.                                */
/* -------------------------------------------------------------------- */
    else if( anSrcWin[0] < 0 || anSrcWin[1] < 0 
        || anSrcWin[0] + anSrcWin[2] > GDALGetRasterXSize(hDataset)
        || anSrcWin[1] + anSrcWin[3] > GDALGetRasterYSize(hDataset) )
    {
        int bCompletelyOutside = anSrcWin[0] + anSrcWin[2] <= 0 ||
                                    anSrcWin[1] + anSrcWin[3] <= 0 ||
                                    anSrcWin[0] >= GDALGetRasterXSize(hDataset) ||
                                    anSrcWin[1] >= GDALGetRasterYSize(hDataset);
        int bIsError = bErrorOnPartiallyOutside || (bCompletelyOutside && bErrorOnCompletelyOutside);
        if( !bQuiet || bIsError )
        {
            fprintf( stderr,
                 "%s: %s-srcwin %d %d %d %d falls %s outside raster extent.%s\n",
                 (bIsError) ? "Error" : "Warning",
                 ( dfULX != 0.0 || dfULY != 0.0 || dfLRX != 0.0 || dfLRY != 0.0 ) ? "Computed " : "",
                 anSrcWin[0],
                 anSrcWin[1],
                 anSrcWin[2],
                 anSrcWin[3],
                 (bCompletelyOutside) ? "completely" : "partially",
                 (bIsError) ? "" : " Going on however." );
        }
        if( bIsError )
            exit(1);
    }

/* -------------------------------------------------------------------- */
/*      Find the output driver.                                         */
/* -------------------------------------------------------------------- */
    hDriver = GDALGetDriverByName( pszFormat );
    if( hDriver == NULL )
    {
        int	iDr;
        
        printf( "Output driver `%s' not recognised.\n", pszFormat );
        printf( "The following format drivers are configured and support output:\n" );
        for( iDr = 0; iDr < GDALGetDriverCount(); iDr++ )
        {
            GDALDriverH hDriver = GDALGetDriver(iDr);

            if( GDALGetMetadataItem( hDriver, GDAL_DCAP_CREATE, NULL ) != NULL
                || GDALGetMetadataItem( hDriver, GDAL_DCAP_CREATECOPY,
                                        NULL ) != NULL )
            {
                printf( "  %s: %s\n",
                        GDALGetDriverShortName( hDriver  ),
                        GDALGetDriverLongName( hDriver ) );
            }
        }
        printf( "\n" );
        
        GDALClose( hDataset );
        CPLFree( panBandList );
        GDALDestroyDriverManager();
        CSLDestroy( papszCreateOptions );
        exit( 1 );
    }
    
/* -------------------------------------------------------------------- */
/*      The short form is to CreateCopy().  We use this if the input    */
/*      matches the whole dataset.  Eventually we should rewrite        */
/*      this entire program to use virtual datasets to construct a      */
/*      virtual input source to copy from.                              */
/* -------------------------------------------------------------------- */


    int bSpatialArrangementPreserved = (
           anSrcWin[0] == 0 && anSrcWin[1] == 0
        && anSrcWin[2] == GDALGetRasterXSize(hDataset)
        && anSrcWin[3] == GDALGetRasterYSize(hDataset)
        && pszOXSize == NULL && pszOYSize == NULL );

    if( eOutputType == GDT_Unknown 
        && !bScale && !bUnscale
        && CSLCount(papszMetadataOptions) == 0 && bDefBands 
        && eMaskMode == MASK_AUTO
        && bSpatialArrangementPreserved
        && nGCPCount == 0 && !bGotBounds
        && pszOutputSRS == NULL && !bSetNoData && !bUnsetNoData
        && nRGBExpand == 0 && !bStats )
    {
        
        hOutDS = GDALCreateCopy( hDriver, pszDest, hDataset, 
                                 bStrict, papszCreateOptions, 
                                 NULL, NULL );

        if( hOutDS != NULL )
            GDALClose( hOutDS );
        
        GDALClose( hDataset );

        CPLFree( panBandList );

        CSLDestroy( papszCreateOptions );

        return hOutDS == NULL;
    }

/* -------------------------------------------------------------------- */
/*      Establish some parameters.                                      */
/* -------------------------------------------------------------------- */
    if( pszOXSize == NULL )
    {
        nOXSize = anSrcWin[2];
        nOYSize = anSrcWin[3];
    }
    else
    {
        nOXSize = (int) ((pszOXSize[strlen(pszOXSize)-1]=='%' 
                          ? CPLAtofM(pszOXSize)/100*anSrcWin[2] : atoi(pszOXSize)));
        nOYSize = (int) ((pszOYSize[strlen(pszOYSize)-1]=='%' 
                          ? CPLAtofM(pszOYSize)/100*anSrcWin[3] : atoi(pszOYSize)));
    }

/* ==================================================================== */
/*      Create a virtual dataset.                                       */
/* ==================================================================== */
    VRTDataset *poVDS;
        
/* -------------------------------------------------------------------- */
/*      Make a virtual clone.                                           */
/* -------------------------------------------------------------------- */
    poVDS = (VRTDataset *) VRTCreate( nOXSize, nOYSize );

    if( nGCPCount == 0 )
    {
        if( pszOutputSRS != NULL )
        {
            poVDS->SetProjection( pszOutputSRS );
        }
        else
        {
            pszProjection = GDALGetProjectionRef( hDataset );
            if( pszProjection != NULL && strlen(pszProjection) > 0 )
                poVDS->SetProjection( pszProjection );
        }
    }

    if( bGotBounds )
    {
        adfGeoTransform[0] = adfULLR[0];
        adfGeoTransform[1] = (adfULLR[2] - adfULLR[0]) / nOXSize;
        adfGeoTransform[2] = 0.0;
        adfGeoTransform[3] = adfULLR[1];
        adfGeoTransform[4] = 0.0;
        adfGeoTransform[5] = (adfULLR[3] - adfULLR[1]) / nOYSize;

        poVDS->SetGeoTransform( adfGeoTransform );
    }

    else if( GDALGetGeoTransform( hDataset, adfGeoTransform ) == CE_None 
        && nGCPCount == 0 )
    {
        adfGeoTransform[0] += anSrcWin[0] * adfGeoTransform[1]
            + anSrcWin[1] * adfGeoTransform[2];
        adfGeoTransform[3] += anSrcWin[0] * adfGeoTransform[4]
            + anSrcWin[1] * adfGeoTransform[5];
        
        adfGeoTransform[1] *= anSrcWin[2] / (double) nOXSize;
        adfGeoTransform[2] *= anSrcWin[3] / (double) nOYSize;
        adfGeoTransform[4] *= anSrcWin[2] / (double) nOXSize;
        adfGeoTransform[5] *= anSrcWin[3] / (double) nOYSize;
        
        poVDS->SetGeoTransform( adfGeoTransform );
    }

       
    
    if( GDALGetGCPCount( hDataset2 ) > 0 )
    {
        GDAL_GCP *pasGCPs;
        int       nGCPs = GDALGetGCPCount( hDataset2 );

        pasGCPs = GDALDuplicateGCPs( nGCPs, GDALGetGCPs( hDataset2 ) );

        for( i = 0; i < nGCPs; i++ )
        {
            pasGCPs[i].dfGCPPixel -= anSrcWin[0];
            pasGCPs[i].dfGCPLine  -= anSrcWin[1];
            pasGCPs[i].dfGCPPixel *= (nOXSize / (double) anSrcWin[2] );
            pasGCPs[i].dfGCPLine  *= (nOYSize / (double) anSrcWin[3] );
        }
            
        poVDS->SetGCPs( nGCPs, pasGCPs,
                        GDALGetGCPProjection( hDataset2 ) );

        GDALDeinitGCPs( nGCPs, pasGCPs );
        CPLFree( pasGCPs );
    }
    GDALClose( hDataset2 );
    
/* -------------------------------------------------------------------- */
/*      To make the VRT to look less awkward (but this is optional      */
/*      in fact), avoid negative values.                                */
/* -------------------------------------------------------------------- */
    int anDstWin[4];
    anDstWin[0] = 0;
    anDstWin[1] = 0;
    anDstWin[2] = nOXSize;
    anDstWin[3] = nOYSize;

    FixSrcDstWindow( anSrcWin, anDstWin,
                     GDALGetRasterXSize(hDataset),
                     GDALGetRasterYSize(hDataset) );

/* -------------------------------------------------------------------- */
/*      Transfer generally applicable metadata.                         */
/* -------------------------------------------------------------------- */
    char** papszMetadata = CSLDuplicate(((GDALDataset*)hDataset)->GetMetadata());
    if ( bScale || bUnscale || eOutputType != GDT_Unknown )
    {
        /* Remove TIFFTAG_MINSAMPLEVALUE and TIFFTAG_MAXSAMPLEVALUE */
        /* if the data range may change because of options */
        char** papszIter = papszMetadata;
        while(papszIter && *papszIter)
        {
            if (EQUALN(*papszIter, "TIFFTAG_MINSAMPLEVALUE=", 23) ||
                EQUALN(*papszIter, "TIFFTAG_MAXSAMPLEVALUE=", 23))
            {
                CPLFree(*papszIter);
                memmove(papszIter, papszIter+1, sizeof(char*) * (CSLCount(papszIter+1)+1));
            }
            else
                papszIter++;
        }
    }
    poVDS->SetMetadata( papszMetadata );
    CSLDestroy( papszMetadata );
    AttachMetadata( (GDALDatasetH) poVDS, papszMetadataOptions );

    const char* pszInterleave = GDALGetMetadataItem(hDataset, "INTERLEAVE", "IMAGE_STRUCTURE");
    if (pszInterleave)
        poVDS->SetMetadataItem("INTERLEAVE", pszInterleave, "IMAGE_STRUCTURE");

/* -------------------------------------------------------------------- */
/*      Transfer metadata that remains valid if the spatial             */
/*      arrangement of the data is unaltered.                           */
/* -------------------------------------------------------------------- */
    if( bSpatialArrangementPreserved )
    {
        char **papszMD;

        papszMD = ((GDALDataset*)hDataset)->GetMetadata("RPC");
        if( papszMD != NULL )
            poVDS->SetMetadata( papszMD, "RPC" );

        papszMD = ((GDALDataset*)hDataset)->GetMetadata("GEOLOCATION");
        if( papszMD != NULL )
            poVDS->SetMetadata( papszMD, "GEOLOCATION" );
    }

    int nSrcBandCount = nBandCount;

    if (nRGBExpand != 0)
    {
        GDALRasterBand  *poSrcBand;
        poSrcBand = ((GDALDataset *) 
                     hDataset)->GetRasterBand(ABS(panBandList[0]));
        if (panBandList[0] < 0)
            poSrcBand = poSrcBand->GetMaskBand();
        GDALColorTable* poColorTable = poSrcBand->GetColorTable();
        if (poColorTable == NULL)
        {
            fprintf(stderr, "Error : band %d has no color table\n", ABS(panBandList[0]));
            GDALClose( hDataset );
            CPLFree( panBandList );
            GDALDestroyDriverManager();
            CSLDestroy( papszCreateOptions );
            exit( 1 );
        }
        
        /* Check that the color table only contains gray levels */
        /* when using -expand gray */
        if (nRGBExpand == 1)
        {
            int nColorCount = poColorTable->GetColorEntryCount();
            int nColor;
            for( nColor = 0; nColor < nColorCount; nColor++ )
            {
                const GDALColorEntry* poEntry = poColorTable->GetColorEntry(nColor);
                if (poEntry->c1 != poEntry->c2 || poEntry->c1 != poEntry->c2)
                {
                    fprintf(stderr, "Warning : color table contains non gray levels colors\n");
                    break;
                }
            }
        }

        if (nBandCount == 1)
            nBandCount = nRGBExpand;
        else if (nBandCount == 2 && (nRGBExpand == 3 || nRGBExpand == 4))
            nBandCount = nRGBExpand;
        else
        {
            fprintf(stderr, "Error : invalid use of -expand option.\n");
            exit( 1 );
        }
    }

    int bFilterOutStatsMetadata =
        (bScale || bUnscale || !bSpatialArrangementPreserved || nRGBExpand != 0);

/* ==================================================================== */
/*      Process all bands.                                              */
/* ==================================================================== */
    for( i = 0; i < nBandCount; i++ )
    {
        VRTSourcedRasterBand   *poVRTBand;
        GDALRasterBand  *poSrcBand;
        GDALDataType    eBandType;
        int             nComponent = 0;

        int nSrcBand;
        if (nRGBExpand != 0)
        {
            if (nSrcBandCount == 2 && nRGBExpand == 4 && i == 3)
                nSrcBand = panBandList[1];
            else
            {
                nSrcBand = panBandList[0];
                nComponent = i + 1;
            }
        }
        else
            nSrcBand = panBandList[i];

        poSrcBand = ((GDALDataset *) hDataset)->GetRasterBand(ABS(nSrcBand));

/* -------------------------------------------------------------------- */
/*      Select output data type to match source.                        */
/* -------------------------------------------------------------------- */
        if( eOutputType == GDT_Unknown )
            eBandType = poSrcBand->GetRasterDataType();
        else
            eBandType = eOutputType;

/* -------------------------------------------------------------------- */
/*      Create this band.                                               */
/* -------------------------------------------------------------------- */
        poVDS->AddBand( eBandType, NULL );
        poVRTBand = (VRTSourcedRasterBand *) poVDS->GetRasterBand( i+1 );
        if (nSrcBand < 0)
        {
            poVRTBand->AddMaskBandSource(poSrcBand);
            continue;
        }

/* -------------------------------------------------------------------- */
/*      Do we need to collect scaling information?                      */
/* -------------------------------------------------------------------- */
        double dfScale=1.0, dfOffset=0.0;

        if( bScale && !bHaveScaleSrc )
        {
            double	adfCMinMax[2];
            GDALComputeRasterMinMax( poSrcBand, TRUE, adfCMinMax );
            dfScaleSrcMin = adfCMinMax[0];
            dfScaleSrcMax = adfCMinMax[1];
        }

        if( bScale )
        {
            if( dfScaleSrcMax == dfScaleSrcMin )
                dfScaleSrcMax += 0.1;
            if( dfScaleDstMax == dfScaleDstMin )
                dfScaleDstMax += 0.1;

            dfScale = (dfScaleDstMax - dfScaleDstMin) 
                / (dfScaleSrcMax - dfScaleSrcMin);
            dfOffset = -1 * dfScaleSrcMin * dfScale + dfScaleDstMin;
        }

        if( bUnscale )
        {
            dfScale = poSrcBand->GetScale();
            dfOffset = poSrcBand->GetOffset();
        }

/* -------------------------------------------------------------------- */
/*      Create a simple or complex data source depending on the         */
/*      translation type required.                                      */
/* -------------------------------------------------------------------- */
        if( bUnscale || bScale || (nRGBExpand != 0 && i < nRGBExpand) )
        {
            poVRTBand->AddComplexSource( poSrcBand,
                                         anSrcWin[0], anSrcWin[1],
                                         anSrcWin[2], anSrcWin[3],
                                         anDstWin[0], anDstWin[1],
                                         anDstWin[2], anDstWin[3],
                                         dfOffset, dfScale,
                                         VRT_NODATA_UNSET,
                                         nComponent );
        }
        else
            poVRTBand->AddSimpleSource( poSrcBand,
                                        anSrcWin[0], anSrcWin[1],
                                        anSrcWin[2], anSrcWin[3],
                                        anDstWin[0], anDstWin[1],
                                        anDstWin[2], anDstWin[3] );

/* -------------------------------------------------------------------- */
/*      In case of color table translate, we only set the color         */
/*      interpretation other info copied by CopyBandInfo are            */
/*      not relevant in RGB expansion.                                  */
/* -------------------------------------------------------------------- */
        if (nRGBExpand == 1)
        {
            poVRTBand->SetColorInterpretation( GCI_GrayIndex );
        }
        else if (nRGBExpand != 0 && i < nRGBExpand)
        {
            poVRTBand->SetColorInterpretation( (GDALColorInterp) (GCI_RedBand + i) );
        }

/* -------------------------------------------------------------------- */
/*      copy over some other information of interest.                   */
/* -------------------------------------------------------------------- */
        else
        {
            CopyBandInfo( poSrcBand, poVRTBand,
                          !bStats && !bFilterOutStatsMetadata,
                          !bUnscale,
                          !bSetNoData && !bUnsetNoData );
        }

/* -------------------------------------------------------------------- */
/*      Set a forcable nodata value?                                    */
/* -------------------------------------------------------------------- */
        if( bSetNoData )
        {
            double dfVal = dfNoDataReal;
            int bClamped = FALSE, bRounded = FALSE;

#define CLAMP(val,type,minval,maxval) \
    do { if (val < minval) { bClamped = TRUE; val = minval; } \
    else if (val > maxval) { bClamped = TRUE; val = maxval; } \
    else if (val != (type)val) { bRounded = TRUE; val = (type)(val + 0.5); } } \
    while(0)

            switch(eBandType)
            {
                case GDT_Byte:
                    CLAMP(dfVal, GByte, 0.0, 255.0);
                    break;
                case GDT_Int16:
                    CLAMP(dfVal, GInt16, -32768.0, 32767.0);
                    break;
                case GDT_UInt16:
                    CLAMP(dfVal, GUInt16, 0.0, 65535.0);
                    break;
                case GDT_Int32:
                    CLAMP(dfVal, GInt32, -2147483648.0, 2147483647.0);
                    break;
                case GDT_UInt32:
                    CLAMP(dfVal, GUInt32, 0.0, 4294967295.0);
                    break;
                default:
                    break;
            }
                
            if (bClamped)
            {
                printf( "for band %d, nodata value has been clamped "
                       "to %.0f, the original value being out of range.\n",
                       i + 1, dfVal);
            }
            else if(bRounded)
            {
                printf("for band %d, nodata value has been rounded "
                       "to %.0f, %s being an integer datatype.\n",
                       i + 1, dfVal,
                       GDALGetDataTypeName(eBandType));
            }
            
            poVRTBand->SetNoDataValue( dfVal );
        }

        if (eMaskMode == MASK_AUTO &&
            (GDALGetMaskFlags(GDALGetRasterBand(hDataset, 1)) & GMF_PER_DATASET) == 0 &&
            (poSrcBand->GetMaskFlags() & (GMF_ALL_VALID | GMF_NODATA)) == 0)
        {
            if (poVRTBand->CreateMaskBand(poSrcBand->GetMaskFlags()) == CE_None)
            {
                VRTSourcedRasterBand* hMaskVRTBand =
                    (VRTSourcedRasterBand*)poVRTBand->GetMaskBand();
                hMaskVRTBand->AddMaskBandSource(poSrcBand,
                                        anSrcWin[0], anSrcWin[1],
                                        anSrcWin[2], anSrcWin[3],
                                        anDstWin[0], anDstWin[1],
                                        anDstWin[2], anDstWin[3] );
            }
        }
    }

    if (eMaskMode == MASK_USER)
    {
        GDALRasterBand *poSrcBand =
            (GDALRasterBand*)GDALGetRasterBand(hDataset, ABS(nMaskBand));
        if (poSrcBand && poVDS->CreateMaskBand(GMF_PER_DATASET) == CE_None)
        {
            VRTSourcedRasterBand* hMaskVRTBand = (VRTSourcedRasterBand*)
                GDALGetMaskBand(GDALGetRasterBand((GDALDatasetH)poVDS, 1));
            if (nMaskBand > 0)
                hMaskVRTBand->AddSimpleSource(poSrcBand,
                                        anSrcWin[0], anSrcWin[1],
                                        anSrcWin[2], anSrcWin[3],
                                        anDstWin[0], anDstWin[1],
                                        anDstWin[2], anDstWin[3] );
            else
                hMaskVRTBand->AddMaskBandSource(poSrcBand,
                                        anSrcWin[0], anSrcWin[1],
                                        anSrcWin[2], anSrcWin[3],
                                        anDstWin[0], anDstWin[1],
                                        anDstWin[2], anDstWin[3] );
        }
    }
    else
    if (eMaskMode == MASK_AUTO && nSrcBandCount > 0 &&
        GDALGetMaskFlags(GDALGetRasterBand(hDataset, 1)) == GMF_PER_DATASET)
    {
        if (poVDS->CreateMaskBand(GMF_PER_DATASET) == CE_None)
        {
            VRTSourcedRasterBand* hMaskVRTBand = (VRTSourcedRasterBand*)
                GDALGetMaskBand(GDALGetRasterBand((GDALDatasetH)poVDS, 1));
            hMaskVRTBand->AddMaskBandSource((GDALRasterBand*)GDALGetRasterBand(hDataset, 1),
                                        anSrcWin[0], anSrcWin[1],
                                        anSrcWin[2], anSrcWin[3],
                                        anDstWin[0], anDstWin[1],
                                        anDstWin[2], anDstWin[3] );
        }
    }

/* -------------------------------------------------------------------- */
/*      Compute stats if required.                                      */
/* -------------------------------------------------------------------- */
    if (bStats)
    {
        for( i = 0; i < poVDS->GetRasterCount(); i++ )
        {
            double dfMin, dfMax, dfMean, dfStdDev;
            poVDS->GetRasterBand(i+1)->ComputeStatistics( bApproxStats,
                    &dfMin, &dfMax, &dfMean, &dfStdDev, GDALDummyProgress, NULL );
        }
    }

/* -------------------------------------------------------------------- */
/*      Write to the output file using CopyCreate().                    */
/* -------------------------------------------------------------------- */
    hOutDS = GDALCreateCopy( hDriver, pszDest, (GDALDatasetH) poVDS,
                             bStrict, papszCreateOptions, 
                             NULL, NULL );
    if( hOutDS != NULL )
    {
        int bHasGotErr = FALSE;
        CPLErrorReset();
        GDALFlushCache( hOutDS );
        if (CPLGetLastErrorType() != CE_None)
            bHasGotErr = TRUE;
        GDALClose( hOutDS );
        if (bHasGotErr)
            hOutDS = NULL;
    }
    
    GDALClose( (GDALDatasetH) poVDS );
        
    GDALClose( hDataset );

    CPLFree( panBandList );
    
    CPLFree( pszOutputSRS );

    CSLDestroy( papszCreateOptions );
    
    return hOutDS == NULL;
    
}


/************************************************************************/
/*                              SrcToDst()                              */
/************************************************************************/

void GDALProcess::SrcToDst( double dfX, double dfY,
                      int nSrcXOff, int nSrcYOff,
                      int nSrcXSize, int nSrcYSize,
                      int nDstXOff, int nDstYOff,
                      int nDstXSize, int nDstYSize,
                      double &dfXOut, double &dfYOut )

{
    dfXOut = ((dfX - nSrcXOff) / nSrcXSize) * nDstXSize + nDstXOff;
    dfYOut = ((dfY - nSrcYOff) / nSrcYSize) * nDstYSize + nDstYOff;
}

int GDALProcess::FixSrcDstWindow( int* panSrcWin, int* panDstWin,
                            int nSrcRasterXSize,
                            int nSrcRasterYSize )

{
    const int nSrcXOff = panSrcWin[0];
    const int nSrcYOff = panSrcWin[1];
    const int nSrcXSize = panSrcWin[2];
    const int nSrcYSize = panSrcWin[3];

    const int nDstXOff = panDstWin[0];
    const int nDstYOff = panDstWin[1];
    const int nDstXSize = panDstWin[2];
    const int nDstYSize = panDstWin[3];

    int bModifiedX = FALSE, bModifiedY = FALSE;

    int nModifiedSrcXOff = nSrcXOff;
    int nModifiedSrcYOff = nSrcYOff;

    int nModifiedSrcXSize = nSrcXSize;
    int nModifiedSrcYSize = nSrcYSize;

/* -------------------------------------------------------------------- */
/*      Clamp within the bounds of the available source data.           */
/* -------------------------------------------------------------------- */
    if( nModifiedSrcXOff < 0 )
    {
        nModifiedSrcXSize += nModifiedSrcXOff;
        nModifiedSrcXOff = 0;

        bModifiedX = TRUE;
    }

    if( nModifiedSrcYOff < 0 )
    {
        nModifiedSrcYSize += nModifiedSrcYOff;
        nModifiedSrcYOff = 0;
        bModifiedY = TRUE;
    }

    if( nModifiedSrcXOff + nModifiedSrcXSize > nSrcRasterXSize )
    {
        nModifiedSrcXSize = nSrcRasterXSize - nModifiedSrcXOff;
        bModifiedX = TRUE;
    }

    if( nModifiedSrcYOff + nModifiedSrcYSize > nSrcRasterYSize )
    {
        nModifiedSrcYSize = nSrcRasterYSize - nModifiedSrcYOff;
        bModifiedY = TRUE;
    }

/* -------------------------------------------------------------------- */
/*      Don't do anything if the requesting region is completely off    */
/*      the source image.                                               */
/* -------------------------------------------------------------------- */
    if( nModifiedSrcXOff >= nSrcRasterXSize
        || nModifiedSrcYOff >= nSrcRasterYSize
        || nModifiedSrcXSize <= 0 || nModifiedSrcYSize <= 0 )
    {
        return FALSE;
    }

    panSrcWin[0] = nModifiedSrcXOff;
    panSrcWin[1] = nModifiedSrcYOff;
    panSrcWin[2] = nModifiedSrcXSize;
    panSrcWin[3] = nModifiedSrcYSize;

/* -------------------------------------------------------------------- */
/*      If we haven't had to modify the source rectangle, then the      */
/*      destination rectangle must be the whole region.                 */
/* -------------------------------------------------------------------- */
    if( !bModifiedX && !bModifiedY )
        return TRUE;

/* -------------------------------------------------------------------- */
/*      Now transform this possibly reduced request back into the       */
/*      destination buffer coordinates in case the output region is     */
/*      less than the whole buffer.                                     */
/* -------------------------------------------------------------------- */
    double dfDstULX, dfDstULY, dfDstLRX, dfDstLRY;

    SrcToDst( nModifiedSrcXOff, nModifiedSrcYOff,
              nSrcXOff, nSrcYOff,
              nSrcXSize, nSrcYSize,
              nDstXOff, nDstYOff,
              nDstXSize, nDstYSize,
              dfDstULX, dfDstULY );
    SrcToDst( nModifiedSrcXOff + nModifiedSrcXSize, nModifiedSrcYOff + nModifiedSrcYSize,
              nSrcXOff, nSrcYOff,
              nSrcXSize, nSrcYSize,
              nDstXOff, nDstYOff,
              nDstXSize, nDstYSize,
              dfDstLRX, dfDstLRY );

    int nModifiedDstXOff = nDstXOff;
    int nModifiedDstYOff = nDstYOff;
    int nModifiedDstXSize = nDstXSize;
    int nModifiedDstYSize = nDstYSize;

    if( bModifiedX )
    {
        nModifiedDstXOff = (int) ((dfDstULX - nDstXOff)+0.001);
        nModifiedDstXSize = (int) ((dfDstLRX - nDstXOff)+0.001)
            - nModifiedDstXOff;

        nModifiedDstXOff = MAX(0,nModifiedDstXOff);
        if( nModifiedDstXOff + nModifiedDstXSize > nDstXSize )
            nModifiedDstXSize = nDstXSize - nModifiedDstXOff;
    }

    if( bModifiedY )
    {
        nModifiedDstYOff = (int) ((dfDstULY - nDstYOff)+0.001);
        nModifiedDstYSize = (int) ((dfDstLRY - nDstYOff)+0.001)
            - nModifiedDstYOff;

        nModifiedDstYOff = MAX(0,nModifiedDstYOff);
        if( nModifiedDstYOff + nModifiedDstYSize > nDstYSize )
            nModifiedDstYSize = nDstYSize - nModifiedDstYOff;
    }

    if( nModifiedDstXSize < 1 || nModifiedDstYSize < 1 )
        return FALSE;
    else
    {
        panDstWin[0] = nModifiedDstXOff;
        panDstWin[1] = nModifiedDstYOff;
        panDstWin[2] = nModifiedDstXSize;
        panDstWin[3] = nModifiedDstYSize;

        return TRUE;
    }
}


void GDALProcess::AttachMetadata( GDALDatasetH hDS, char **papszMetadataOptions )

{
    int nCount = CSLCount(papszMetadataOptions);
    int i;

    for( i = 0; i < nCount; i++ )
    {
        char    *pszKey = NULL;
        const char *pszValue;
        
        pszValue = CPLParseNameValue( papszMetadataOptions[i], &pszKey );
        GDALSetMetadataItem(hDS,pszKey,pszValue,NULL);
        CPLFree( pszKey );
    }

    CSLDestroy( papszMetadataOptions );
}

void GDALProcess::CopyBandInfo( GDALRasterBand * poSrcBand, GDALRasterBand * poDstBand,
                          int bCanCopyStatsMetadata, int bCopyScale, int bCopyNoData )

{
    int bSuccess;
    double dfNoData;

    if (bCanCopyStatsMetadata)
    {
        poDstBand->SetMetadata( poSrcBand->GetMetadata() );
    }
    else
    {
        char** papszMetadata = poSrcBand->GetMetadata();
        char** papszMetadataNew = NULL;
        for( int i = 0; papszMetadata != NULL && papszMetadata[i] != NULL; i++ )
        {
            if (strncmp(papszMetadata[i], "STATISTICS_", 11) != 0)
                papszMetadataNew = CSLAddString(papszMetadataNew, papszMetadata[i]);
        }
        poDstBand->SetMetadata( papszMetadataNew );
        CSLDestroy(papszMetadataNew);
    }

    poDstBand->SetColorTable( poSrcBand->GetColorTable() );
    poDstBand->SetColorInterpretation(poSrcBand->GetColorInterpretation());
    if( strlen(poSrcBand->GetDescription()) > 0 )
        poDstBand->SetDescription( poSrcBand->GetDescription() );

    if (bCopyNoData)
    {
        dfNoData = poSrcBand->GetNoDataValue( &bSuccess );
        if( bSuccess )
            poDstBand->SetNoDataValue( dfNoData );
    }

    if (bCopyScale)
    {
        poDstBand->SetOffset( poSrcBand->GetOffset() );
        poDstBand->SetScale( poSrcBand->GetScale() );
    }

    poDstBand->SetCategoryNames( poSrcBand->GetCategoryNames() );
    if( !EQUAL(poSrcBand->GetUnitType(),"") )
        poDstBand->SetUnitType( poSrcBand->GetUnitType() );
}


/// GDAL WARP CODE
int GDALProcess::GDALExit( int nCode )
{
  const char  *pszDebug = CPLGetConfigOption("CPL_DEBUG",NULL);
  if( pszDebug && (EQUAL(pszDebug,"ON") || EQUAL(pszDebug,"") ) )
  {  
    GDALDumpOpenDatasets( stderr );
    CPLDumpSharedList( NULL );
  }

  GDALDestroyDriverManager();

#ifdef OGR_ENABLED
  OGRCleanupAll();
#endif

  exit( nCode );
}

char* GDALProcess::SanitizeSRS( const char *pszUserInput )

{
    OGRSpatialReferenceH hSRS;
    char *pszResult = NULL;

    CPLErrorReset();
    
    hSRS = OSRNewSpatialReference( NULL );
    if( OSRSetFromUserInput( hSRS, pszUserInput ) == OGRERR_NONE )
        OSRExportToWkt( hSRS, &pszResult );
    else
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Translating source or target SRS failed:\n%s",
                  pszUserInput );
        GDALExit( 1 );
    }
    
    OSRDestroySpatialReference( hSRS );

    return pszResult;
}

int GDALProcess::warpGEOTIFF(std::string inputFilename, std::string warpFormat, std::string outputFilename)
{
    double	       dfMinX=0.0, dfMinY=0.0, dfMaxX=0.0, dfMaxY=0.0;
    double	       dfXRes=0.0, dfYRes=0.0;
    int             bTargetAlignedPixels = FALSE;
    int             nForcePixels=0, nForceLines=0, bQuiet = TRUE;
    int             bEnableDstAlpha = FALSE, bEnableSrcAlpha = FALSE;

    GDALDatasetH	hDstDS;
    const char         *pszFormat = "GTiff";
    char 		*pszSrcFilename = new char[inputFilename.size()+1];
    char               *pszDstFilename = new char[outputFilename.size()+1];
    char 		*pszSRS = SanitizeSRS(warpFormat.c_str());
    int                 i;
    void               *hTransformArg, *hGenImgProjArg=NULL, *hApproxArg=NULL;
    char               **papszWarpOptions = NULL;
    double             dfErrorThreshold = 0.125;
    double             dfWarpMemoryLimit = 0.0;
    GDALTransformerFunc pfnTransformer = NULL;
    char                **papszCreateOptions = NULL;
    GDALDataType        eOutputType = GDT_Unknown, eWorkingType = GDT_Unknown; 
    GDALResampleAlg     eResampleAlg = GRA_NearestNeighbour;
    int                 bMulti = FALSE;
    char                **papszTO = NULL;
    int                  bHasGotErr = FALSE;

    GDALAllRegister();
    papszTO = CSLSetNameValue( papszTO, "DST_SRS", pszSRS );
    CPLFree( pszSRS );
    strcpy(pszSrcFilename, inputFilename.c_str());
    strcpy(pszDstFilename, outputFilename.c_str());
    
/* -------------------------------------------------------------------- */
/*      Does the output dataset already exist?                          */
/* -------------------------------------------------------------------- */

    CPLPushErrorHandler( CPLQuietErrorHandler );
    hDstDS = GDALOpen( pszDstFilename, GA_Update );
    CPLPopErrorHandler();

/* -------------------------------------------------------------------- */
/*      If not, we need to create it.                                   */
/* -------------------------------------------------------------------- */
    int   bInitDestSetForFirst = FALSE;

    void* hUniqueTransformArg = NULL;
    GDALDatasetH hUniqueSrcDS = NULL;

    if( hDstDS == NULL )
    {
	
        hDstDS = GDALWarpCreateOutput( pszSrcFilename, pszDstFilename,pszFormat,
                                       papszTO, &papszCreateOptions, 
                                       eOutputType, &hUniqueTransformArg,
                                       &hUniqueSrcDS, 
				        nForcePixels,  nForceLines,
					bQuiet,  bTargetAlignedPixels, 
					bEnableDstAlpha, 
					bEnableSrcAlpha, 
					dfXRes,  dfYRes, 
					dfMinX,  dfMinY,
					dfMaxX,  dfMaxY);
        papszWarpOptions = CSLSetNameValue(papszWarpOptions,
                                               "INIT_DEST", "0");
        bInitDestSetForFirst = TRUE;

        CSLDestroy( papszCreateOptions );
        papszCreateOptions = NULL;
    }
 
    if( hDstDS == NULL )
        GDALExit( 1 );

/* -------------------------------------------------------------------- */
/*      Loop over all source files, processing each in turn.            */
/* -------------------------------------------------------------------- */
    int iSrc = 0;

        GDALDatasetH hSrcDS;
       
/* -------------------------------------------------------------------- */
/*      Open this file.                                                 */
/* -------------------------------------------------------------------- */
        hSrcDS = GDALOpen( pszSrcFilename, GA_ReadOnly );
	    
        if( hSrcDS == NULL )
            GDALExit( 2 );

/* -------------------------------------------------------------------- */
/*      Get the metadata of the first source DS and copy it to the      */
/*      destination DS. Copy Band-level metadata and other info, only   */
/*      if source and destination band count are equal. Any values that */
/*      conflict between source datasets are set to pszMDConflictValue. */
/* -------------------------------------------------------------------- */
        char **papszMetadata = NULL;
        const char *pszSrcInfo = NULL;
        GDALRasterBandH hSrcBand = NULL;
        GDALRasterBandH hDstBand = NULL;

            /* copy metadata from first dataset */
            if ( iSrc == 0 )
            {
                CPLDebug("WARP", "Copying metadata from first source to destination dataset");
                /* copy dataset-level metadata */
                papszMetadata = GDALGetMetadata( hSrcDS, NULL );
                if ( CSLCount(papszMetadata) > 0 ) {
                    if ( GDALSetMetadata( hDstDS, papszMetadata, NULL ) != CE_None )
                         fprintf( stderr, 
                                  "Warning: error copying metadata to destination dataset.\n" );
                }
                /* copy band-level metadata and other info */
                if ( GDALGetRasterCount( hSrcDS ) == GDALGetRasterCount( hDstDS ) )              
                {
                    for ( int iBand = 0; iBand < GDALGetRasterCount( hSrcDS ); iBand++ )
                    {
                        hSrcBand = GDALGetRasterBand( hSrcDS, iBand + 1 );
                        hDstBand = GDALGetRasterBand( hDstDS, iBand + 1 );
                        /* copy metadata */
                        papszMetadata = GDALGetMetadata( hSrcBand, NULL);              
                        if ( CSLCount(papszMetadata) > 0 )
                            GDALSetMetadata( hDstBand, papszMetadata, NULL );
                        /* copy other info (Description, Unit Type) - what else? */
                        
                            pszSrcInfo = GDALGetDescription( hSrcBand );
                            if(  pszSrcInfo != NULL && strlen(pszSrcInfo) > 0 )
                                GDALSetDescription( hDstBand, pszSrcInfo );  
                            pszSrcInfo = GDALGetRasterUnitType( hSrcBand );
                            if(  pszSrcInfo != NULL && strlen(pszSrcInfo) > 0 )
                                GDALSetRasterUnitType( hDstBand, pszSrcInfo );  
                    }
                }
            }

/* -------------------------------------------------------------------- */
/*      Create a transformation object from the source to               */
/*      destination coordinate system.                                  */
/* -------------------------------------------------------------------- */
        if (hUniqueTransformArg)
            hTransformArg = hGenImgProjArg = hUniqueTransformArg;
        else
            hTransformArg = hGenImgProjArg =
                GDALCreateGenImgProjTransformer2( hSrcDS, hDstDS, papszTO );
        
        if( hTransformArg == NULL )
            GDALExit( 1 );
        
        pfnTransformer = GDALGenImgProjTransform;

/* -------------------------------------------------------------------- */
/*      Warp the transformer with a linear approximator unless the      */
/*      acceptable error is zero.                                       */
/* -------------------------------------------------------------------- */
        if( dfErrorThreshold != 0.0 )
        {
            hTransformArg = hApproxArg = 
                GDALCreateApproxTransformer( GDALGenImgProjTransform, 
                                             hGenImgProjArg, dfErrorThreshold);
            pfnTransformer = GDALApproxTransform;
        }

/* -------------------------------------------------------------------- */
/*      Clear temporary INIT_DEST settings after the first image.       */
/* -------------------------------------------------------------------- */
        if( bInitDestSetForFirst && iSrc == 1 )
            papszWarpOptions = CSLSetNameValue( papszWarpOptions, 
                                                "INIT_DEST", NULL );

/* -------------------------------------------------------------------- */
/*      Setup warp options.                                             */
/* -------------------------------------------------------------------- */
        GDALWarpOptions *psWO = GDALCreateWarpOptions();

        psWO->papszWarpOptions = CSLDuplicate(papszWarpOptions);
        psWO->eWorkingDataType = eWorkingType;
        psWO->eResampleAlg = eResampleAlg;

        psWO->hSrcDS = hSrcDS;
        psWO->hDstDS = hDstDS;

        psWO->pfnTransformer = pfnTransformer;
        psWO->pTransformerArg = hTransformArg;

        if( !bQuiet )
            psWO->pfnProgress = GDALTermProgress;

        if( dfWarpMemoryLimit != 0.0 )
            psWO->dfWarpMemoryLimit = dfWarpMemoryLimit;

/* -------------------------------------------------------------------- */
/*      Setup band mapping.                                             */
/* -------------------------------------------------------------------- */
        psWO->nBandCount = GDALGetRasterCount(hSrcDS);
        psWO->panSrcBands = (int *) CPLMalloc(psWO->nBandCount*sizeof(int));
        psWO->panDstBands = (int *) CPLMalloc(psWO->nBandCount*sizeof(int));

        for( i = 0; i < psWO->nBandCount; i++ )
        {
            psWO->panSrcBands[i] = i+1;
            psWO->panDstBands[i] = i+1;
        }

/* -------------------------------------------------------------------- */
/*      Initialize and execute the warp.                                */
/* -------------------------------------------------------------------- */
        GDALWarpOperation oWO;

        if( oWO.Initialize( psWO ) == CE_None )
        {
            CPLErr eErr;
            if( bMulti )
                eErr = oWO.ChunkAndWarpMulti( 0, 0, 
                                       GDALGetRasterXSize( hDstDS ),
                                       GDALGetRasterYSize( hDstDS ) );
            else
                eErr = oWO.ChunkAndWarpImage( 0, 0, 
                                       GDALGetRasterXSize( hDstDS ),
                                       GDALGetRasterYSize( hDstDS ) );
            if (eErr != CE_None)
                bHasGotErr = TRUE;
        }

/* -------------------------------------------------------------------- */
/*      Cleanup                                                         */
/* -------------------------------------------------------------------- */
        if( hApproxArg != NULL )
            GDALDestroyApproxTransformer( hApproxArg );
        
        if( hGenImgProjArg != NULL )
            GDALDestroyGenImgProjTransformer( hGenImgProjArg );
        
        GDALDestroyWarpOptions( psWO );

        GDALClose( hSrcDS );

/* -------------------------------------------------------------------- */
/*      Final Cleanup.                                                  */
/* -------------------------------------------------------------------- */
    CPLErrorReset();
    GDALFlushCache( hDstDS );
    if( CPLGetLastErrorType() != CE_None )
        bHasGotErr = TRUE;
    GDALClose( hDstDS );
      
    CPLFree( pszDstFilename );
    CPLFree( pszSrcFilename );
    CSLDestroy( papszWarpOptions );
    CSLDestroy( papszTO );

    GDALDestroyDriverManager();
             
    return (bHasGotErr) ? 1 : 0;
}

GDALDatasetH 
GDALProcess::GDALWarpCreateOutput( char *papszSrcFile, const char *pszFilename, 
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
		      double dfMaxX, double dfMaxY)


{
    GDALDriverH hDriver;
    GDALDatasetH hDstDS;
    void *hTransformArg;
    GDALColorTableH hCT = NULL;
    double dfWrkMinX=0, dfWrkMaxX=0, dfWrkMinY=0, dfWrkMaxY=0;
    double dfWrkResX=0, dfWrkResY=0;
    int nDstBandCount = 0;
    std::vector<GDALColorInterp> apeColorInterpretations;

    /* If (-ts and -te) or (-tr and -te) are specified, we don't need to compute the suggested output extent */
    int    bNeedsSuggestedWarpOutput = 
                  !( ((nForcePixels != 0 && nForceLines != 0) || (dfXRes != 0 && dfYRes != 0)) &&
                     !(dfMinX == 0.0 && dfMinY == 0.0 && dfMaxX == 0.0 && dfMaxY == 0.0) );

    *phTransformArg = NULL;
    *phSrcDS = NULL;

/* -------------------------------------------------------------------- */
/*      Find the output driver.                                         */
/* -------------------------------------------------------------------- */
    hDriver = GDALGetDriverByName( pszFormat );
    if( hDriver == NULL 
        || GDALGetMetadataItem( hDriver, GDAL_DCAP_CREATE, NULL ) == NULL )
    {
        int	iDr;
        
        printf( "Output driver `%s' not recognised or does not support\n", 
                pszFormat );
        printf( "direct output file creation.  The following format drivers are configured\n"
                "and support direct output:\n" );

        for( iDr = 0; iDr < GDALGetDriverCount(); iDr++ )
        {
            GDALDriverH hDriver = GDALGetDriver(iDr);

            if( GDALGetMetadataItem( hDriver, GDAL_DCAP_CREATE, NULL) != NULL )
            {
                printf( "  %s: %s\n",
                        GDALGetDriverShortName( hDriver  ),
                        GDALGetDriverLongName( hDriver ) );
            }
        }
        printf( "\n" );
        GDALExit( 1 );
    }

/* -------------------------------------------------------------------- */
/*      Loop over all input files to collect extents.                   */
/* -------------------------------------------------------------------- */
    int     iSrc = 0;
    char    *pszThisTargetSRS = (char*)CSLFetchNameValue( papszTO, "DST_SRS" );
    if( pszThisTargetSRS != NULL )
        pszThisTargetSRS = CPLStrdup( pszThisTargetSRS );

    GDALDatasetH hSrcDS;
    const char *pszThisSourceSRS = CSLFetchNameValue(papszTO,"SRC_SRS");

    hSrcDS = GDALOpen( papszSrcFile, GA_ReadOnly );
    if( hSrcDS == NULL )
            GDALExit( 1 );

/* -------------------------------------------------------------------- */
/*      Check that there's at least one raster band                     */
/* -------------------------------------------------------------------- */
        if( eDT == GDT_Unknown )
            eDT = GDALGetRasterDataType(GDALGetRasterBand(hSrcDS,1));

/* -------------------------------------------------------------------- */
/*      If we are processing the first file, and it has a color         */
/*      table, then we will copy it to the destination file.            */
/* -------------------------------------------------------------------- */
        if( iSrc == 0 )
        {
            nDstBandCount = GDALGetRasterCount(hSrcDS);
            hCT = GDALGetRasterColorTable( GDALGetRasterBand(hSrcDS,1) );
            if( hCT != NULL )
            {
                hCT = GDALCloneColorTable( hCT );
            }

            for(int iBand = 0; iBand < nDstBandCount; iBand++)
            {
                apeColorInterpretations.push_back(
                    GDALGetRasterColorInterpretation(GDALGetRasterBand(hSrcDS,iBand+1)) );
            }
        }

/* -------------------------------------------------------------------- */
/*      Get the sourcesrs from the dataset, if not set already.         */
/* -------------------------------------------------------------------- */
        if( pszThisSourceSRS == NULL )
        {
            const char *pszMethod = CSLFetchNameValue( papszTO, "METHOD" );

            if( GDALGetProjectionRef( hSrcDS ) != NULL 
                && strlen(GDALGetProjectionRef( hSrcDS )) > 0
                && (pszMethod == NULL || EQUAL(pszMethod,"GEOTRANSFORM")) )
                pszThisSourceSRS = GDALGetProjectionRef( hSrcDS );
            
            else if( GDALGetGCPProjection( hSrcDS ) != NULL
                     && strlen(GDALGetGCPProjection(hSrcDS)) > 0 
                     && GDALGetGCPCount( hSrcDS ) > 1 
                     && (pszMethod == NULL || EQUALN(pszMethod,"GCP_",4)) )
                pszThisSourceSRS = GDALGetGCPProjection( hSrcDS );
            else if( pszMethod != NULL && EQUAL(pszMethod,"RPC") )
                pszThisSourceSRS = SRS_WKT_WGS84;
            else
                pszThisSourceSRS = "";
        }

        if( pszThisTargetSRS == NULL )
            pszThisTargetSRS = CPLStrdup( pszThisSourceSRS );
        
/* -------------------------------------------------------------------- */
/*      Create a transformation object from the source to               */
/*      destination coordinate system.                                  */
/* -------------------------------------------------------------------- */
        hTransformArg = 
            GDALCreateGenImgProjTransformer2( hSrcDS, NULL, papszTO );
        
        if( hTransformArg == NULL )
        {
            CPLFree( pszThisTargetSRS );
            GDALClose( hSrcDS );
            return NULL;
        }
        
        GDALTransformerInfo* psInfo = (GDALTransformerInfo*)hTransformArg;

/* -------------------------------------------------------------------- */
/*      Get approximate output definition.                              */
/* -------------------------------------------------------------------- */
        if( bNeedsSuggestedWarpOutput )
        {
            double adfThisGeoTransform[6];
            double adfExtent[4];
            int    nThisPixels, nThisLines;

            if ( GDALSuggestedWarpOutput2( hSrcDS, 
                                        psInfo->pfnTransform, hTransformArg, 
                                        adfThisGeoTransform, 
                                        &nThisPixels, &nThisLines, 
                                        adfExtent, 0 ) != CE_None )
            {
                CPLFree( pszThisTargetSRS );
                GDALClose( hSrcDS );
                return NULL;
            }
            
            if ( CPLGetConfigOption( "CHECK_WITH_INVERT_PROJ", NULL ) == NULL )
            {
                double MinX = adfExtent[0];
                double MaxX = adfExtent[2];
                double MaxY = adfExtent[3];
                double MinY = adfExtent[1];
                int bSuccess = TRUE;
                
                /* Check that the the edges of the target image are in the validity area */
                /* of the target projection */
    #define N_STEPS 20
                int i,j;
                for(i=0;i<=N_STEPS && bSuccess;i++)
                {
                    for(j=0;j<=N_STEPS && bSuccess;j++)
                    {
                        double dfRatioI = i * 1.0 / N_STEPS;
                        double dfRatioJ = j * 1.0 / N_STEPS;
                        double expected_x = (1 - dfRatioI) * MinX + dfRatioI * MaxX;
                        double expected_y = (1 - dfRatioJ) * MinY + dfRatioJ * MaxY;
                        double x = expected_x;
                        double y = expected_y;
                        double z = 0;
                        /* Target SRS coordinates to source image pixel coordinates */
                        if (!psInfo->pfnTransform(hTransformArg, TRUE, 1, &x, &y, &z, &bSuccess) || !bSuccess)
                            bSuccess = FALSE;
                        /* Source image pixel coordinates to target SRS coordinates */
                        if (!psInfo->pfnTransform(hTransformArg, FALSE, 1, &x, &y, &z, &bSuccess) || !bSuccess)
                            bSuccess = FALSE;
                        if (fabs(x - expected_x) > (MaxX - MinX) / nThisPixels ||
                            fabs(y - expected_y) > (MaxY - MinY) / nThisLines)
                            bSuccess = FALSE;
                    }
                }
                
                /* If not, retry with CHECK_WITH_INVERT_PROJ=TRUE that forces ogrct.cpp */
                /* to check the consistency of each requested projection result with the */
                /* invert projection */
                if (!bSuccess)
                {
                    CPLSetConfigOption( "CHECK_WITH_INVERT_PROJ", "TRUE" );
                    CPLDebug("WARP", "Recompute out extent with CHECK_WITH_INVERT_PROJ=TRUE");

                    if( GDALSuggestedWarpOutput2( hSrcDS, 
                                        psInfo->pfnTransform, hTransformArg, 
                                        adfThisGeoTransform, 
                                        &nThisPixels, &nThisLines, 
                                        adfExtent, 0 ) != CE_None )
                    {
                        CPLFree( pszThisTargetSRS );
                        GDALClose( hSrcDS );
                        return NULL;
                    }
                }
            }

    /* -------------------------------------------------------------------- */
    /*      Expand the working bounds to include this region, ensure the    */
    /*      working resolution is no more than this resolution.             */
    /* -------------------------------------------------------------------- */
            if( dfWrkMaxX == 0.0 && dfWrkMinX == 0.0 )
            {
                dfWrkMinX = adfExtent[0];
                dfWrkMaxX = adfExtent[2];
                dfWrkMaxY = adfExtent[3];
                dfWrkMinY = adfExtent[1];
                dfWrkResX = adfThisGeoTransform[1];
                dfWrkResY = ABS(adfThisGeoTransform[5]);
            }
            else
            {
                dfWrkMinX = MIN(dfWrkMinX,adfExtent[0]);
                dfWrkMaxX = MAX(dfWrkMaxX,adfExtent[2]);
                dfWrkMaxY = MAX(dfWrkMaxY,adfExtent[3]);
                dfWrkMinY = MIN(dfWrkMinY,adfExtent[1]);
                dfWrkResX = MIN(dfWrkResX,adfThisGeoTransform[1]);
                dfWrkResY = MIN(dfWrkResY,ABS(adfThisGeoTransform[5]));
            }
        }

//         if (iSrc == 0 && papszSrcFiles[1] == NULL)
//         {
//             *phTransformArg = hTransformArg;
//             *phSrcDS = hSrcDS;
//         }
//         else
//         {
            GDALDestroyGenImgProjTransformer( hTransformArg );
            GDALClose( hSrcDS );
//         }
//     }



    /* -------------------------------------------------------------------- */
/*      Did we have any usable sources?                                 */
/* -------------------------------------------------------------------- */
    if( nDstBandCount == 0 )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "No usable source images." );
        CPLFree( pszThisTargetSRS );
        return NULL;
    }

/* -------------------------------------------------------------------- */
/*      Turn the suggested region into a geotransform and suggested     */
/*      number of pixels and lines.                                     */
/* -------------------------------------------------------------------- */
    double adfDstGeoTransform[6] = { 0, 0, 0, 0, 0, 0 };
    int nPixels = 0, nLines = 0;
    
    if( bNeedsSuggestedWarpOutput )
    {
        adfDstGeoTransform[0] = dfWrkMinX;
        adfDstGeoTransform[1] = dfWrkResX;
        adfDstGeoTransform[2] = 0.0;
        adfDstGeoTransform[3] = dfWrkMaxY;
        adfDstGeoTransform[4] = 0.0;
        adfDstGeoTransform[5] = -1 * dfWrkResY;

        nPixels = (int) ((dfWrkMaxX - dfWrkMinX) / dfWrkResX + 0.5);
        nLines = (int) ((dfWrkMaxY - dfWrkMinY) / dfWrkResY + 0.5);
    }

/* -------------------------------------------------------------------- */
/*      Did the user override some parameters?                          */
/* -------------------------------------------------------------------- */
    if( dfXRes != 0.0 && dfYRes != 0.0 )
    {
        if( dfMinX == 0.0 && dfMinY == 0.0 && dfMaxX == 0.0 && dfMaxY == 0.0 )
        {
            dfMinX = adfDstGeoTransform[0];
            dfMaxX = adfDstGeoTransform[0] + adfDstGeoTransform[1] * nPixels;
            dfMaxY = adfDstGeoTransform[3];
            dfMinY = adfDstGeoTransform[3] + adfDstGeoTransform[5] * nLines;
        }
        
        if ( bTargetAlignedPixels )
        {
            dfMinX = floor(dfMinX / dfXRes) * dfXRes;
            dfMaxX = ceil(dfMaxX / dfXRes) * dfXRes;
            dfMinY = floor(dfMinY / dfYRes) * dfYRes;
            dfMaxY = ceil(dfMaxY / dfYRes) * dfYRes;
        }

        nPixels = (int) ((dfMaxX - dfMinX + (dfXRes/2.0)) / dfXRes);
        nLines = (int) ((dfMaxY - dfMinY + (dfYRes/2.0)) / dfYRes);
        adfDstGeoTransform[0] = dfMinX;
        adfDstGeoTransform[3] = dfMaxY;
        adfDstGeoTransform[1] = dfXRes;
        adfDstGeoTransform[5] = -dfYRes;
    }

    else if( nForcePixels != 0 && nForceLines != 0 )
    {
        if( dfMinX == 0.0 && dfMinY == 0.0 && dfMaxX == 0.0 && dfMaxY == 0.0 )
        {
            dfMinX = dfWrkMinX;
            dfMaxX = dfWrkMaxX;
            dfMaxY = dfWrkMaxY;
            dfMinY = dfWrkMinY;
        }

        dfXRes = (dfMaxX - dfMinX) / nForcePixels;
        dfYRes = (dfMaxY - dfMinY) / nForceLines;

        adfDstGeoTransform[0] = dfMinX;
        adfDstGeoTransform[3] = dfMaxY;
        adfDstGeoTransform[1] = dfXRes;
        adfDstGeoTransform[5] = -dfYRes;

        nPixels = nForcePixels;
        nLines = nForceLines;
    }

    else if( nForcePixels != 0 )
    {
        if( dfMinX == 0.0 && dfMinY == 0.0 && dfMaxX == 0.0 && dfMaxY == 0.0 )
        {
            dfMinX = dfWrkMinX;
            dfMaxX = dfWrkMaxX;
            dfMaxY = dfWrkMaxY;
            dfMinY = dfWrkMinY;
        }

        dfXRes = (dfMaxX - dfMinX) / nForcePixels;
        dfYRes = dfXRes;

        adfDstGeoTransform[0] = dfMinX;
        adfDstGeoTransform[3] = dfMaxY;
        adfDstGeoTransform[1] = dfXRes;
        adfDstGeoTransform[5] = -dfYRes;

        nPixels = nForcePixels;
        nLines = (int) ((dfMaxY - dfMinY + (dfYRes/2.0)) / dfYRes);
    }

    else if( nForceLines != 0 )
    {
        if( dfMinX == 0.0 && dfMinY == 0.0 && dfMaxX == 0.0 && dfMaxY == 0.0 )
        {
            dfMinX = dfWrkMinX;
            dfMaxX = dfWrkMaxX;
            dfMaxY = dfWrkMaxY;
            dfMinY = dfWrkMinY;
        }

        dfYRes = (dfMaxY - dfMinY) / nForceLines;
        dfXRes = dfYRes;

        adfDstGeoTransform[0] = dfMinX;
        adfDstGeoTransform[3] = dfMaxY;
        adfDstGeoTransform[1] = dfXRes;
        adfDstGeoTransform[5] = -dfYRes;

        nPixels = (int) ((dfMaxX - dfMinX + (dfXRes/2.0)) / dfXRes);
        nLines = nForceLines;
    }

    else if( dfMinX != 0.0 || dfMinY != 0.0 || dfMaxX != 0.0 || dfMaxY != 0.0 )
    {
        dfXRes = adfDstGeoTransform[1];
        dfYRes = fabs(adfDstGeoTransform[5]);

        nPixels = (int) ((dfMaxX - dfMinX + (dfXRes/2.0)) / dfXRes);
        nLines = (int) ((dfMaxY - dfMinY + (dfYRes/2.0)) / dfYRes);

        dfXRes = (dfMaxX - dfMinX) / nPixels;
        dfYRes = (dfMaxY - dfMinY) / nLines;

        adfDstGeoTransform[0] = dfMinX;
        adfDstGeoTransform[3] = dfMaxY;
        adfDstGeoTransform[1] = dfXRes;
        adfDstGeoTransform[5] = -dfYRes;
    }

/* -------------------------------------------------------------------- */
/*      Do we want to generate an alpha band in the output file?        */
/* -------------------------------------------------------------------- */
    if( bEnableSrcAlpha )
        nDstBandCount--;

    if( bEnableDstAlpha )
        nDstBandCount++;

/* -------------------------------------------------------------------- */
/*      Create the output file.                                         */
/* -------------------------------------------------------------------- */
    hDstDS = GDALCreate( hDriver, pszFilename, nPixels, nLines, 
                         nDstBandCount, eDT, *ppapszCreateOptions );
    
    if( hDstDS == NULL )
    {
        CPLFree( pszThisTargetSRS );
        return NULL;
    }

/* -------------------------------------------------------------------- */
/*      Write out the projection definition.                            */
/* -------------------------------------------------------------------- */
    GDALSetProjection( hDstDS, pszThisTargetSRS );
    GDALSetGeoTransform( hDstDS, adfDstGeoTransform );

    if (*phTransformArg != NULL)
        GDALSetGenImgProjTransformerDstGeoTransform( *phTransformArg, adfDstGeoTransform);

    CPLFree( pszThisTargetSRS );
    return hDstDS;
}


void 
GDALProcess::RemoveConflictingMetadata( GDALMajorObjectH hObj, char **papszMetadata, 
                           const char *pszValueConflict )
{
    if ( hObj == NULL ) return;

    char *pszKey = NULL; 
    const char *pszValueRef; 
    const char *pszValueComp; 
    char ** papszMetadataRef = CSLDuplicate( papszMetadata );
    int nCount = CSLCount( papszMetadataRef ); 

    for( int i = 0; i < nCount; i++ ) 
    { 
        pszValueRef = CPLParseNameValue( papszMetadataRef[i], &pszKey ); 
        pszValueComp = GDALGetMetadataItem( hObj, pszKey, NULL );
        if ( ( pszValueRef == NULL || pszValueComp == NULL ||
               ! EQUAL( pszValueRef, pszValueComp ) ) &&
             ! EQUAL( pszValueComp, pszValueConflict ) ) {
            GDALSetMetadataItem( hObj, pszKey, pszValueConflict, NULL ); 
        }
        CPLFree( pszKey ); 
    } 

    CSLDestroy( papszMetadataRef );
}

