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

#ifndef _ERRORS_H
#define _ERRORS_H


// App Specific:
#define ERROR_GDAL               2     // gdal-data directory not found
#define ERROR_CONFIG             3     // wrong configuration (setup.xml) (processing path or log-path is wrong)
#define ERROR_PARAMS             4     // wrong parameters
#define ERROR_IMAGELAYERSETTINGS 5     // can't load imagelayersettings. (image layer probably doesn't exist)
#define ERROR_ELVLAYERSETTINGS   6     // can't load elevsation layer settings
#define ERROR_LOADELEVATION      10    // can't load elevation
#define ERROR_AREA               20    // area is too small to be processed

// General Errors:
#define ERROR_OUTOFMEMORY        101;  // not enough memory

#endif

