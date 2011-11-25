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
#                           martin.christen@fhnw.ch                            #
********************************************************************************
*     Licensed under MIT License. Read the file LICENSE for more information   *
*******************************************************************************/

#ifndef _IMAGEWRITER_H
#define _IMAGEWRITER_H

#include "og.h"
#include "image/ImageHandler.h"
#include <string>

class OPENGLOBE_API ImageWriter
{
public:
   ImageWriter();
   virtual ~ImageWriter();

   // write rgba buffer to PNG
   static bool WritePNG(const std::string& sFilename, unsigned char* buffer_rbga, int width, int height);

   // write imageobject to PNG (currently only RGBA images are supported)
   static bool WritePNG(const std::string& sFilename, ImageObject& image);

   // writes JPG image. Note: Image is converted to RGB.
   static bool WriteJPG(const std::string& sFilename, ImageObject& image, int quality);

   // writes Raw data format 32 bit channeldepth.
   static bool WriteRaw32(const std::string& sFilename, Raw32ImageObject& image);
   static bool WriteRaw32(const std::string& sFilename, int w, int h, float* data);
 
};


#endif

