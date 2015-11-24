// swath_reprojector.cpp
// Program to reproject swath data to
// modis sin grid

#include <iostream>

#include "gdal_priv.h"
#include "cpl_conv.h" // for CPLMalloc()
#include "gdalwarper.h"


int main()
{
    // 1. Load the dataset
    const char * Filename = "NETCDF:\"L2_of_MER_RRG_1PRACR20080629_124716_000026412069_00482_33101_0000.nc\":sdr_6";
    GDALDataset  *swDataset;
    GDALAllRegister();
    swDataset = (GDALDataset *) GDALOpen( Filename, GA_ReadOnly );
    if( swDataset == NULL )
    {
      std::cout << "Could not open dataset " << swDataset << std::endl;
      exit(1);
    }
    // 2. Get projection information
    double        GeoTransform[6];
    const char *        ProjectionInfo;
    swDataset->GetGeoTransform( GeoTransform ); // load it
    std::cout << "The GeoTransform was sucessfully loaded: " <<  GeoTransform[0] << std::endl;
    ProjectionInfo = swDataset->GetProjectionRef();
    std::cout << "The projection was sucessfully loaded: " <<  ProjectionInfo << std::endl;

    // 3. Prepare coordinate transform grid
    //    1. make arrays of coordinate informati
    int nCols = swDataset->GetRasterXSize();
    int nRows = swDataset->GetRasterYSize();
    int dims = 2;


    // Make output file...
    const char *pszFormat = "GTiff";
    GDALDriver *poDriver;
    poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);

    const char * pszDstFilename = "Coords.tif";
    GDALDataset *poDstDS;
    char **papszOptions = NULL;
    poDstDS = poDriver->Create( pszDstFilename, nCols, nRows, 2, GDT_Float32, papszOptions );
    // get format right
    GDALRasterBand *poBand;

    poDstDS->SetGeoTransform( GeoTransform );
    poDstDS->SetProjection( ProjectionInfo );


    GDALRasterBand *lonBand;
    GDALRasterBand *latBand;

    // Do it row by row for now
    float *coordsX = (float*) CPLMalloc(sizeof(float)*nCols);
    float *coordsY = (float*) CPLMalloc(sizeof(float)*nCols);


    // 4. Compute coodinates for every element of grid...
    double u;
    double v;
    for (int i=0; i < nRows; i++) {
      for (int j=0; j < nCols; j++) {
        // calculate new values
        double yi = (double) j;
        double xi = (double) i;
        u = GeoTransform[0] + GeoTransform[1]*xi + GeoTransform[2]*yi;
        v = GeoTransform[3] + GeoTransform[4]*xi + GeoTransform[5]*yi;
        // now put into coords array
        coordsX[j]= u;
        coordsY[j]= v;
      }
    // write out to file after each row...
    lonBand = poDstDS->GetRasterBand(1);
    latBand = poDstDS->GetRasterBand(2);
    lonBand->RasterIO( GF_Write, 0, i, nCols, 1,
                      coordsX, nCols, 1, GDT_Float32, 0, 0 );
    latBand->RasterIO( GF_Write, 0, i, nCols, 1,
                      coordsY, nCols, 1, GDT_Float32, 0, 0 );
    }
    // 5. Save it out to a file

    /* Once we're done, close properly the dataset */
    GDALClose( (GDALDatasetH) poDstDS );

}
