/*******************************************************************************
#      ____               __          __  _      _____ _       _               #
#     / __ \              \ \        / / | |    / ____| |     | |              #
#    | |  | |_ __   ___ _ __ \  /\  / /__| |__ | |  __| | ___ | |__   ___      #
#    | |  | | '_ \ / _ \ '_ \ \/  \/ / _ \ '_ \| | |_ | |/ _ \| '_ \ / _ \     #
#    | |__| | |_) |  __/ | | \  /\  /  __/ |_) | |__| | | (_) | |_) |  __/     #
#     \____/| .__/ \___|_| |_|\/  \/ \___|_.__/ \_____|_|\___/|_.__/ \___|     #
#           | |                                                                #
#           |_|                                                                #
#                                                                              #
#                                (c) 2011 by                                   #
#           University of Applied Sciences Northwestern Switzerland            #
#                     Institute of Geomatics Engineering                       #
#                           robert.wueest@fhnw.ch                              #
********************************************************************************
*     Licensed under MIT License. Read the file LICENSE for more information   *
*******************************************************************************/

//-------------------------------------------------------
// NOTE: this tool will be renamed to ogPostprocess soon
//-------------------------------------------------------

#include "ogprocess.h"
#include "errors.h"
#include <boost/asio.hpp>
#include "app/ProcessingSettings.h"
#include "geo/MercatorQuadtree.h"
#include "geo/CoordinateTransformation.h"
#include "geo/ElevationLayerSettings.h"
#include "string/FilenameUtils.h"
#include "string/StringUtils.h"
#include "geo/ImageLayerSettings.h"
#include "io/FileSystem.h"
#include "image/ImageLoader.h"
#include "image/ImageWriter.h"
#include "app/Logger.h"
#include "math/mathutils.h"
#include <iostream>
#include <boost/program_options.hpp>
#include <sstream>
#include <omp.h>
#include <app/QueueManager.h>
#include "hillshading.h"
#include <math/vec3.h>

namespace po = boost::program_options;

//-------------------------------------------------------------------
// Job-Struct
struct SJob
{
   int xx;
   int yy;
   int lod;
};

// -------------------------------------------------------------------
// -- Global variables
   bool bError = false;
   std::string sLayerPath;
   std::string sJobQueueFile;
   int iLayerMaxZoom = 0;
   int iLayerMinZoom = 0;
   std::string sAlgorithm;
   std::string sProcessHostName;
   bool bVerbose = false;
   bool bGenerateJobs = false;
   bool bOverrideQueue = false;
   bool bOverrideTiles = true;
   bool bLockEnabled = false;
   bool bNormalMaps = false;
   bool bBorders = false;
   bool bSlope = false;
   bool bColored = false;
   bool bTextured = false;
   bool bNoData = false;
   int iAmount = 256;
   int inputX = 768;
   int inputY = 768;
   int outputX = 256;
   int outputY = 256;
   double z_depth = 1.0;
   double azimut = 315;
   double altitude = 45;
   double sscale = 1;
   double slopeScale = 1;
   bool bJPEG = false;
   int iX = 0;
   int iY = 0;
   std::string sTempTileDir;
   std::string sTileDir;
   boost::shared_ptr<MercatorQuadtree> qQuadtree;
   int64 layerTileX0, layerTileY0, layerTileX1, layerTileY1;
   QueueManager _QueueManager = QueueManager();
   boost::shared_array<ImageObject> pTextures;
// -------------------------------------------------------------------

//  Job function (called every thread/compute node)
void ProcessJob(const SJob& job, int layerLod)
{
   //std::cout << sCurrentQuadcode << "\n";
   HSProcessChunk pData;
   pData.layerLod = layerLod;
   pData.dfXMax = -1e20;
   pData.dfYMax = -1e20;
   pData.dfXMin = 1e20;
   pData.dfYMin = 1e20;
   pData.data.AllocateImage(inputX, inputY, -9999.0f);
   std::string sParentQuad = qQuadtree->TileCoordToQuadkey(job.xx,job.yy,job.lod);
   sParentQuad = StringUtils::Left(sParentQuad,layerLod);
   int64 parentX,parentY;
   int parentLod;
   MercatorQuadtree::QuadKeyToTileCoord(sParentQuad,parentX, parentY,parentLod);
   for (int ty=-1;ty<=1;ty++)
   {
      for (int tx=-1;tx<=1;tx++)
      {
         //std::string sQuadcode = qQuadtree->TileCoordToQuadkey(job.xx+tx,job.yy+ty,job.lod);
         std::string sQuadcode = qQuadtree->TileCoordToQuadkey(parentX+tx, parentY+ty,parentLod);
         std::string sTilefile = ProcessingUtils::GetTilePath(sTempTileDir, ".raw" , parentLod, parentX+tx, parentY+ty);
                  
         double sx0, sy1, sx1, sy0;
         qQuadtree->QuadKeyToMercatorCoord(sQuadcode, sx0, sy1, sx1, sy0);
               
         pData.dfXMax = math::Max<double>(pData.dfXMax, sx1);
         pData.dfYMax = math::Max<double>(pData.dfYMax, sy1);
         pData.dfXMin = math::Min<double>(pData.dfXMin, sx0);
         pData.dfYMin = math::Min<double>(pData.dfYMin, sy0);

         assert(sx0 < sx1);
         assert(sy0 < sy1);

         //std::cout << "   " << sTilefile << "\n";

         std::ifstream fin;
         fin.open(sTilefile.c_str(), std::ios::binary);
         int posX = (tx+1)*(inputX/3);
         int posY = (ty+1)*(inputY/3);
         int offX = 0;
         int offY = 0;
         if (fin.good())
         {
            while (!fin.eof())
            {    
               float value;
               fin.read((char*)&(value), sizeof(float));
               if (!fin.eof())
               {
                  pData.data.SetValue(posX+offX, posY+offY, value);
               }
               offX++;
               if(offX % 256 == 0)
               {
                  offX = 0;
                  offY++;
               }

            }
         }
         fin.close();
      }
   }
   // Generate tile
   process_hillshading(sTileDir, pData, qQuadtree, job.xx, job.yy, job.lod, z_depth, azimut, altitude,sscale,slopeScale, bSlope, bNormalMaps, outputX, outputY, bOverrideTiles, bLockEnabled, bNoData, bJPEG, bColored, bTextured, pTextures);
}

//------------------------------------------------------------------------------------

void ConvertJobs(std::vector<QJob>& input, std::vector<SJob>& output)
{
   output.clear();
   for (size_t i=0;i<input.size();i++)
   {  
      SJob job2;
      memcpy(&job2, input[i].data.get(), sizeof(SJob));
      output.push_back(job2);
   }
}

//------------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
	   //---------------------------------------------------------------------------
   // init options:

   boost::shared_ptr<ProcessingSettings> qSettings =  ProcessingUtils::LoadAppSettings();

   if (!qSettings)
   {
      std::cout << "[" << sProcessHostName<< "] " << "Error in configuration! Check setup.xml\n";
      return ERROR_CONFIG;
   }

   sProcessHostName = boost::asio::ip::host_name();
   po::options_description desc("Program-Options");
   desc.add_options()
      ("layername", po::value<std::string>(), "name of layer to generate data")
      ("maxlod", po::value<int>(), "maximum lod which has to be generated previously using ogAddData")
      ("minlod", po::value<int>(), "minimum lod which has to be generated previously using ogAddData")
      ("generatejobs","[optional] create a jobqueue which can be used in every process")
      ("overridejobqueue","[optional] overrides existing queue file if exist (only when generatejobs is set!)")
      ("normalmaps", "[optional] generate normal maps")
      ("slope", "[optional] integrate slope to map")
      ("slopescale", po::value<double>(),"[optional] define slope scale default 1")
      ("numthreads", po::value<int>(), "[optional] force number of threads")
      ("amount", po::value<int>(), "[opional] define amount of jobs to be read for one process at the time")
      ("zdepth", po::value<double>(), "[opional] hillshading z factor")
      ("azimut", po::value<double>(), "[opional] hillshading azimut")
      ("altitude", po::value<double>(), "[opional] hillshading altitude")
      ("scale", po::value<double>(), "[opional] hillshading scale")
      ("nooverride", "[opional] overriding existing tiles disabled")
      ("enablelocking", "[opional] lock files to prevent concurrency on parallel processes")
      ("verbose", "[optional] verbose output")
      ("processborders", "[optional] process border tiles")
      ("nodata", "[optional] include nodata values")
	   ("colored", "[optional] color the heigths")
	   ("textured", "[optional] generic textured heights")
      ("jpg", "[optional] save files in compressed JPEG quality(78) instead of PNG")
      ;

   po::variables_map vm;
   try
   {
      po::store(po::parse_command_line(argc, argv, desc), vm);
      po::notify(vm);
   }
   catch (std::exception&)
   {
      bError = true;
   }

   if(vm.count("layername"))
   {
	  std::string sLayerName = vm["layername"].as<std::string>();
	  sLayerPath = FilenameUtils::DelimitPath(qSettings->GetPath()) + sLayerName;
      /*sLayerPath = vm["layerpath"].as<std::string>();
      if(!(sLayerPath.at(sLayerPath.length()-1) == '\\' || sLayerPath.at(sLayerPath.length()-1) == '/'))
         sLayerPath = sLayerPath + "/";*/
   }
   else
      bError = true;
   if(vm.count("maxlod"))
      iLayerMaxZoom = vm["maxlod"].as<int>();
   else
      bError = true;
   if(vm.count("minlod"))
      iLayerMinZoom = vm["minlod"].as<int>();
   else
      bError = true;
   int numThreads = 1;
   if(vm.count("numthreads"))
   {
      int n = vm["numthreads"].as<int>();
      if (n>0 && n<65)
      {
         std::cout << "[" << sProcessHostName<< "] " << "Forcing number of threads to " << n << "\n";
         omp_set_num_threads(n);
         numThreads = n;
      }
   }
   if(vm.count("amount"))
      iAmount = vm["amount"].as<int>();
   if(vm.count("zdepth"))
      z_depth = vm["zdepth"].as<double>();
   if(vm.count("azimut"))
      azimut = vm["azimut"].as<double>();
   if(vm.count("altitude"))
      altitude = vm["altitude"].as<double>();
   if(vm.count("scale"))
      sscale = vm["scale"].as<double>();
   if(vm.count("verbose"))
      bVerbose = true;
   if(vm.count("generatejobs"))
      bGenerateJobs = true;
    if(vm.count("normalmaps"))
      bNormalMaps = true;
    if(vm.count("slopescale"))
      slopeScale = vm["slopescale"].as<double>();
    if(vm.count("slope"))
      bSlope = true;
    if(vm.count("nodata"))
      bNoData = true;
   if(vm.count("overridejobqueue"))
      bOverrideQueue = true;
   if(vm.count("nooverride"))
      bOverrideTiles = false;
    if(vm.count("enablelocking"))
      bLockEnabled = true;
	if(vm.count("colored"))
      bColored = true;
   if(vm.count("processborders"))
      bBorders = true;
   if(vm.count("jpg"))
      bJPEG = true;
	if(vm.count("textured"))
   {
      bTextured = true;
         // Coloring textures
      pTextures = boost::shared_array<ImageObject>(new ImageObject[6]);
      ImageLoader::LoadFromDisk(Img::Format_PNG, "data/ground.png", Img::PixelFormat_RGBA, pTextures[0]);
      ImageLoader::LoadFromDisk(Img::Format_PNG, "data/grass.png", Img::PixelFormat_RGBA, pTextures[1]);
      ImageLoader::LoadFromDisk(Img::Format_PNG, "data/snow.png", Img::PixelFormat_RGBA, pTextures[2]);
      ImageLoader::LoadFromDisk(Img::Format_PNG, "data/rock.png", Img::PixelFormat_RGBA, pTextures[3]);
      ImageLoader::LoadFromDisk(Img::Format_PNG, "data/desert.png", Img::PixelFormat_RGBA, pTextures[4]);
      ImageLoader::LoadFromDisk(Img::Format_PNG, "data/water.png", Img::PixelFormat_RGBA, pTextures[5]);
   }

	   if(bError)
   {
      std::cout << "[" << sProcessHostName<< "] " << "Wrong parameters!\n";
      std::cout << desc << "\n";
      return ERROR_PARAMS;
   }
   //---------------------------------------------------------------------------
   // -- Beginn process
   sTempTileDir = sLayerPath + "/temp/tiles/";
   sTileDir = sLayerPath + "/tiles/";

   boost::shared_ptr<ImageLayerSettings> qImageLayerSettings = ImageLayerSettings::Load(sLayerPath);

   if (!qImageLayerSettings)
   {
      std::cout << "[" << sProcessHostName<< "] " << "Failed retrieving image layer settings! Make sure to create it using 'createlayer'.\n"<< std::flush;
      return ERROR_IMAGELAYERSETTINGS;
   }
   int layermaxlod = qImageLayerSettings->GetMaxLod();
   
   qImageLayerSettings->GetTileExtent(layerTileX0, layerTileY0, layerTileX1, layerTileY1);
   if (bVerbose)
   {
      std::cout << "\n" << "[" << sProcessHostName<< "] " << "Raw Image Layer:\n";
      std::cout << "   name = " << qImageLayerSettings->GetLayerName() << "\n";
      std::cout << "   maxlod = " << layermaxlod << "\n";
      std::cout << "   extent = " << layerTileX0 << ", " << layerTileY0 << ", " << layerTileX1 << ", " << layerTileY1 << "\n" << std::flush;
   }
   //---------------------------------------------------------------------------
   // -- performance measurement
   int tileCount = 0;
   clock_t t_0, t_1;
   t_0 = clock();

  
   //---------------------------------------------------------------------------
   // -- Generate job queue
   qQuadtree = boost::shared_ptr<MercatorQuadtree>(new MercatorQuadtree());
   sJobQueueFile = sTileDir + "jobqueue.jobs";
   if(bGenerateJobs)
   {
      if(iLayerMaxZoom > layermaxlod)
      {
         for(size_t ll = layermaxlod; ll <iLayerMaxZoom; ll++)
         {
            layerTileX0 = layerTileX0*2;
            layerTileX1 = layerTileX1*2;
            layerTileY0 = layerTileY0*2;
            layerTileY1 = layerTileY1*2;
         }
      }
	  else if(iLayerMaxZoom < layermaxlod)
      {
         for(size_t ll = layermaxlod; ll > iLayerMaxZoom; ll--)
         {
            layerTileX0 = math::Floor(layerTileX0/2.0);
            layerTileX1 = math::Floor(layerTileX1/2.0);
            layerTileY0 = math::Floor(layerTileY0/2.0);
            layerTileY1 = math::Floor(layerTileY1/2.0);
         }
      }
      else
      {
         iLayerMaxZoom = layermaxlod;
         
      }
      if(iLayerMinZoom == 0)
      {
         iLayerMinZoom = layermaxlod;
      }
      for(size_t lod = iLayerMaxZoom; lod >= iLayerMinZoom; lod--)
      {

         if(lod < iLayerMaxZoom)
         {
            // update tile extents
            layerTileX0 = math::Floor(layerTileX0/2.0);
            layerTileX1 = math::Floor(layerTileX1/2.0);
            layerTileY0 = math::Floor(layerTileY0/2.0);
            layerTileY1 = math::Floor(layerTileY1/2.0);
         }
         int64 width = layerTileX1-layerTileX0+1;
         int64 height = layerTileY1-layerTileY0+1;
         if ((width<3 || height<3) && !bBorders)
         {
            std::cout << "Extent is too small for hillshading processing skipping current and following LOD levels\n";
            return ERROR_ELVLAYERSETTINGS;
         }

         // Retrieve dataset extent in mercator coord:
         
         double xmin, ymin, xmax, ymax;
         std::string qc0 = qQuadtree->TileCoordToQuadkey(layerTileX0,layerTileY0,lod);
         std::string qc1 = qQuadtree->TileCoordToQuadkey(layerTileX1,layerTileY1,lod);

         double x00, y00, x10, y10;
         double x01, y01, x11, y11;
         qQuadtree->QuadKeyToMercatorCoord(qc0, x00,y00,x10,y10);
         qQuadtree->QuadKeyToMercatorCoord(qc1, x01,y01,x11,y11);

         xmin = x00;
         ymin = y11;
         xmax = x11;
         ymax = y00;
         if (bVerbose)
         {
            std::cout << "\n[" << sProcessHostName<< "] " << "Extent mercator:";
            std::cout << "   extent = " << xmin << ", " << ymin << ", " << xmax << ", " << ymax << "\n"<< std::flush;
         }

         if (!ProcessingUtils::init_gdal())
         {
            std::cout << "[" << sProcessHostName<< "] " << "Warning: gdal-data directory not found. Ouput may be wrong!\n"<< std::flush;
            return 1;
         }   
      
         int idx = 0;
         std::cout << "[" << sProcessHostName<< "] " << " Generating jobs starting from (z, x, y) " << "(" << lod << ", " << layerTileX0+(bBorders ? 0 : 1) << ", " << layerTileY0+(bBorders ? 0 : 1) << ")\n"<< std::flush;
         for (int64 xx = layerTileX0+(bBorders ? 0 : 1); xx < layerTileX1+(bBorders ? 1 : 0); ++xx)
         {
            for (int64 yy = layerTileY0+(bBorders ? 0 : 1); yy < layerTileY1+(bBorders ? 1 : 0); ++yy)
            {
               QJob job;
               SJob sJob;
               sJob.lod = lod;
               sJob.xx = xx;
               sJob.yy = yy;
               job.data = boost::shared_array<char>(new char[sizeof(SJob)]);
               memcpy(job.data.get(), &sJob, sizeof(SJob));
               job.size = sizeof(SJob);
               _QueueManager.AddToJobQueue(sJobQueueFile, job, (bOverrideQueue && idx == 0)? false : true);
               idx++;
               iX = xx;
               iY = yy;
            }
         }
         _QueueManager.CommitJobQueue(sJobQueueFile);
         std::cout << "[" << sProcessHostName<< "] " << " Generated " << idx << " jobs ending with (z, x, y) " << "(" << lod << ", " << iX << ", " << iY << ")!\n"<< std::flush;
      }
   }
   //---------------------------------------------------------------------------
   // -- Process Jobs Queue
   else
   {
      if(!FileSystem::FileExists(sJobQueueFile))
      {
         std::cout << "[" << sProcessHostName<< "] " << "ERROR: Jobqueue file not found: " << sJobQueueFile << " use --generatejobs first...\n"<< std::flush;
         return ERROR_PARAMS;
      }
      std::vector<SJob> vecConverted;
      std::vector<QJob> jobs;
      std::cout << "[" << sProcessHostName<< "] >>>" << "start processing...\n"<< std::flush;
      do
      {
         jobs.clear();
         vecConverted.clear();
         jobs = _QueueManager.FetchJobList(sJobQueueFile, sizeof(SJob), iAmount,bVerbose);
         if(jobs.size() > 0)
         {
         ConvertJobs(jobs, vecConverted);
         SJob first, last;
         first = vecConverted[0];
         last = vecConverted[vecConverted.size()-1];
         clock_t subT0 = clock();
         clock_t subT1;
         std::cout << "--[" << sProcessHostName<< "] " << "  processing " << vecConverted.size() << " jobs\n       starting from (z, x, y) " << "(" << first.lod << ", " << first.xx << ", " << first.yy << ")\n"<< std::flush;
#ifndef _DEBUG
         std::cout << "..Processing parallel using " << numThreads << "\n";
               #pragma omp parallel shared(vecConverted, sTempTileDir, sTileDir, inputX, inputY, outputX, outputY)
               {
                  #pragma omp for 
#endif
                  for(int index = 0; index < vecConverted.size(); index++)
                  {
                     ProcessJob(vecConverted[index],layermaxlod);
                     tileCount++;
                  }
#ifndef _DEBUG
               }
#endif
            subT1 = clock();
            double subTime=(double(subT1-subT0)/double(CLOCKS_PER_SEC));
            double subTps = vecConverted.size()/subTime;
            std::cout << "--[" << sProcessHostName<< "] " << "  processing average " << subTps << " tiles per second.\n";
            std::cout << "--[" << sProcessHostName<< "] " << "  processed " << vecConverted.size() << " jobs\n       terminating with (z, x, y) " << "(" << last.lod << ", " << last.xx << ", " << last.yy << ")\n"<< std::flush;
         }
      }while(jobs.size() >= iAmount); 
   }
   t_1 = clock();
         double time=(double(t_1-t_0)/double(CLOCKS_PER_SEC));
         double tps = tileCount/time;
         std::cout << "[" << sProcessHostName<< "] <<<" << "finished processing "<< tileCount << " jobs at " << tps << " tiles pers second working for " << time << " seconds.\n"<< std::flush;
   return 0;
}