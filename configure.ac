# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2, as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.



# =============
# Automake init
# =============

AC_INIT([hcell],[0.7.0],[serrajosemi@gmail.com],
[hcell],[http:://hcellapp.blogspot.com/])
AC_PREREQ([2.68])
AC_LANG([C++])

ACLOCAL_FLAGS='--install'


# Get OPENCV library and include locations

#[AC_MSG_ERROR([${OPENCV_PKG_ERRORS}])])
HAVE_OPENCV=false
if test -z $OPENCV_include_path; then 	
	PKG_CHECK_MODULES([OPENCV],[opencv >= 2.3.1],[HAVE_OPENCV=true],[true])
	if test  x$HAVE_OPENCV = xfalse; then   
	AC_MSG_ERROR(["OpenCV libraries could not be found. Use ./configure OPENCV_include_path=/the/location/include OPENCV_include_library=/the/location/lib if you have compiled OPENCV in other directory."])
	fi
else
	# go to path and extract all variables which
	#for i in $(ls ${OPENCV_library_path} | grep "^libopencv_.*so$")
  	#do
     	#	OPENCV_LIBS="$OPENCV_LIBS -l${OPENCV_library_path}/$i"
	#	echo "$i added to the library pathway."
 	#done
	OPENCV_LIBS="-L${OPENCV_library_path} -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_ml -lopencv_video -lopencv_features2d -lopencv_calib3d -lopencv_objdetect -lopencv_contrib -lopencv_legacy -lopencv_flann"
	OPENCV_CFLAGS="-I${OPENCV_include_path}"
	AC_MSG_WARN(["Taking Lib paths from local directories. At your own risk!"])
fi

CPPFLAGS=''
CPPFLAGS="${OPENCV_CPPFLAGS}"

# Checks for programs.
AC_PROG_CXXCPP
AC_PROG_CXX

AC_LANG([C++])

AM_PROG_CC_C_O



# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([string])
AC_CHECK_HEADERS([iostream])
AC_CHECK_HEADERS([sys/time.h])
AC_CHECK_HEADERS([memory.h])


AX_BOOST_BASE([1.40],[],[AC_MSG_ERROR(You need the Boost libraries. Download and install them or use sudo apt-get install libboost-all-dev.)])
AX_BOOST_REGEX
AX_CXX_COMPILE_STDCXX_0X

if test "x$HAVE_OPENCV" = "xtrue"; then
AC_CHECK_HEADERS([opencv2/opencv.hpp])
else
AC_CHECK_FILE(
   [${OPENCV_include_path}/opencv2/opencv.hpp],
   [AC_MSG_NOTICE([Found opencv.hpp])],
   [AC_MSG_NOTICE([Didn't find opencv.hpp])]
   )
fi




# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

CHECKLIBS=true
#
# png 
#
AC_CHECK_LIB([png], [png_sig_cmp],[],[echo "You need libpng!"; CHECKLIBS=false])
#AC_CHECK_LIB([png], [png_sig_cmp])
AC_CHECK_HEADERS([png.h],[pngfound=1],[pngfound=0])

#
# JPEG
#
AC_CHECK_LIB([jpeg], [jpeg_start_decompress],[],[echo "You need libjpeg!"; CHECKLIBS=false])
AC_CHECK_HEADERS([jpeglib.h],[jpegfound=1],[jpegfound=0])

#
# netpbm
#
AC_CHECK_LIB([netpbm], [pnm_init],[],[echo "You could need netpbm!"])
AC_CHECK_HEADERS([pam.h],[netpbmfound=1],[netpbmfound=0])


#
# libtiff
#
AC_CHECK_LIB([tiff], [TIFFOpen], , [echo "You need libtiff!"; CHECKLIBS=false], -lm)

if test "x$CHECKLIBS" = "xfalse"; then
AC_MSG_ERROR(["Libraries libjpeg, libtiff, libjasper, libpng and netpbm are needed. Install any of those with the associated development packages (sudo apt-get install libjpeg-dev libtiff-dev libpng-dev netpbm libjasper-dev on Debian/Ubuntu) if you want to use those stock libraries to read and write images in the respective formats. You also can link them from the versions supplied with OpenCV."])
fi

# distribute additional compiler and linker flags among Makefiles
# --> set and change these variables instead of CXXFLAGS or LDFLAGS (for user only)

AC_SUBST([AM_CXXFLAGS])
AC_SUBST([AM_LDFLAGS])

AC_CONFIG_SRCDIR([src/Hcell.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([subdir-objects -Wall -Werror])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_FILES([Makefile src/Makefile])

AC_OUTPUT
