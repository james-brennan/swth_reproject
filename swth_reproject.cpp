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
    const char * Filename = "L2_of_MER_RRG_1PRACR20080607_123851_000026442069_00167_32786_0000_h19v10_sdr_10.tif";
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
    int Xsize = swDataset->GetRasterXSize();
    int Ysize = swDataset->GetRasterYSize();
    int dims = 2;

    //double* coords[] = new double[Xsize][Ysize][dims];

    // try using vector

    std::vector<std::vector<std::vector<double> > > coords;

    // Set up sizes. (HEIGHT x WIDTH)
    coords.resize(Xsize);
    for (int i = 0; i < Xsize; ++i) {
      coords[i].resize(Ysize);

      for (int j = 0; j < Xsize; ++j)
        coords[i][j].resize(dims);
    }


    // 4. Compute coodinates for every element of grid...
    double u;
    double v;
    for (int x=0; x < Xsize; x++) {
      for (int y=0; y < Ysize; y++) {
        // calculate new values
        double yi = (double) y;
        double xi = (double) x;
        u = GeoTransform[0] + GeoTransform[1]*xi + GeoTransform[2]*yi;
        v = GeoTransform[3] + GeoTransform[4]*xi + GeoTransform[5]*yi;
        // now put into coords array
        coords[x][y][0]= u;
        coords[x][y][1]= u;

      }
    }
    // 5. Save it out to a file



    const char *pszFormat = "GTiff";
    GDALDriver *poDriver;
    poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);

    const char * pszDstFilename = "Coords.tif";
    GDALDataset *poDstDS;
    char **papszOptions = NULL;
    poDstDS = poDriver->Create( pszDstFilename, Xsize, Ysize, 2, GDT_Float32, papszOptions );

    // get format right
    GDALRasterBand *poBand;

    poDstDS->SetGeoTransform( GeoTransform );
    poDstDS->SetProjection( ProjectionInfo );
    poBand = poDstDS->GetRasterBand(1);
    poBand->RasterIO( GF_Write, 0, 0, Xsize, Ysize,
                      coords, Xsize, Ysize, GDT_Float32, 0, 0 );
    /* Once we're done, close properly the dataset */
    GDALClose( (GDALDatasetH) poDstDS );


}
