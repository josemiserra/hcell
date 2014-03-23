#include "Action.h"
#include "MType.h"

 Action::TypesMap Action::_typesmap;

/*
 In the Action class all parameters are mapped to its type.
 The reason of place the input-types map here is because only Action makes use of this map
 during the setAction operation.
 objects that need to be withdrawn or stored in the pool are from the type idT (Identifiers)

*/
Action::TypesMap Action::initialize(){
	 TypesMap amap;
/**FILEPROCESSING MODULE*************************/
/************************************************/
/*** LOAD FILE ***/
	 amap["REGEXP"]=stringT;
	 amap["WINDOWSNAME"]=stringT;
     amap["FILENAME"]=stringT;
	 amap["OUTPUT"]=idT;
	 amap["ASHOW"]=boolT;
	 amap["DETECT_EMPTY"]=boolT;
/** WRITE FILE **/	 
	 amap["INPUT"]=idT;
	 amap["FILEEXT"]=stringT;
	 amap["P_COMPRESSIONLEVEL"]=intT;
	 amap["FOUTPUT"]=stringT;
	 amap["APPEND_DATE"]=boolT;
	 amap["APPEND_ORIGINAL_NAME"]=boolT;
/**NORMALIZE**/
	 amap["TYPE"]=stringT;
	 amap["MAXINT"]=doubleT;
	 amap["MININT"]=doubleT;
/**CROP_FILE**/
	amap["COORDINATES_X"]=intT;
	amap["COORDINATES_Y"]=intT;
	amap["WINDOW_HEIGHT"]=intT;
	amap["WINDOW_WIDTH"]=intT;

/**GAUSSIAN BLUR**/
	amap["HEIGHT"]=intT;
	amap["WIDTH"]=intT;
	amap["SIGMA_X"]=doubleT;
	amap["SIGMA_Y"]=doubleT;

/***DERIVATIVE FILTERS***/
	// SOBEL
	amap["X_ORDER"]=intT;
	amap["Y_ORDER"]=intT;
	amap["SIZE"]=intT;
	amap["OPERATION"]=stringT;
	//CANNY
	amap["HIGH_THRESHOLD"]=doubleT;
	amap["LOW_THRESHOLD"]=doubleT;
	amap["KERNEL_SIZE"]=intT;
	amap["L2_GRADIENT"]=boolT;

/***THRESHOLDING***/
	amap["THRESHOLD_TYPE"]=stringT;
	amap["THRESHOLD"]=doubleT;
	amap["MAX_THRESHOLD"]=doubleT;

/***MaKE bRUsH***/
	amap["BRUSH_SIZE_X"]=intT;
	amap["BRUSH_SIZE_Y"]=intT;
	amap["BRUSH_SIZE"]=intT;
	amap["BRUSH_TYPE"]=stringT;
	amap["SIGMA"]=doubleT;

/***MORPHOLOGICAL**/
	amap["BRUSH"]=idT;
	amap["OPERATION"]=stringT;
	amap["ITERATIONS"]=intT;

/**Find Contours***/
	amap["COLOUR"]=stringT;
	amap["THICKNESS"]=intT;
    amap["RETRIEVAL_MODE"]=stringT;
	amap["APPROXIMATION"]=stringT;

 /**KERNELs and FILTERS***/
	amap["DFACTOR"]=doubleT;
	amap["MATRIX"]=stringT;
	amap["KERNEL_NAME"]=stringT;
 /**ADAPTATIVE THRESHOLDING**/
	amap["BLOCKSIZE"]=intT;
	amap["CONSTANT"]=doubleT;
	amap["METHOD"]=stringT;
/** COMPOSITIONS****/
	amap["RED_CHANNEL"]=idT;
	amap["GREEN_CHANNEL"]=idT;
	amap["BLUE_CHANNEL"]=idT;
	amap["INPUT_1"]=idT;
	amap["INPUT_2"]=idT;
	amap["ALPHA_1"]=doubleT;
	amap["ALPHA_2"]=doubleT;
	amap["ALPHA"]=doubleT;
	/** FLOODFILL**/
    amap["SEEDS"]=idT;
	amap["NEW_VALUE"]=stringT;
	amap["LOW_DIFF"]=doubleT;
	amap["UP_DIFF"]=doubleT;
	amap["SAVE_OBJECTS"]=idT;
	amap["SAVE_OBJ"]=idT;
	amap["LOAD_OBJECTS"]=idT;
	amap["LAMBDA"]=doubleT;
	amap["PAINT_CONTOURS"]=boolT;

	/**FEATURES**/
	amap["BASIC"]=boolT;
	amap["SHAPE"]=boolT;
	amap["HARALICK"]=boolT;
	amap["MOMENT"]=boolT;


	amap["MEAN"]=boolT;
	amap["SD"]=boolT;
	amap["MAD"]=boolT;
	amap["QUANTILES"]=boolT;
	amap["SAVE_INDIVIDUAL"]=boolT;
	amap["REFERENCE"]=idT;
	
	amap["AREA"]=boolT;
	amap["PERIMETER"]=boolT;
	amap["RADIUS"]=boolT;
	amap["ROUNDNESS"]=boolT;

	amap["CENTROID"]=boolT;
	amap["AXIS"]=boolT;
	amap["ECCENTRICITY"]=boolT;
	amap["THETA"]=boolT;

	// Haralick features
	amap["BINS"]=intT;
	amap["SCALES"]=stringT;
	amap["SAVE_COOCURRENCE_MATRIX"]=boolT;

	/**SPLIT NUCLEI **/
	amap["EXT"]=intT;
	amap["TOL"]= doubleT;
	amap["REFERENCE"]=idT;
	amap["METHOD"] = stringT;
	amap["NUCLEI"] = idT;
	amap["LOAD_OBJECTS"]= idT;
	amap["LOAD_OBJECTS_1"]= idT;
	amap["LOAD_OBJECTS_2"]= idT;
	amap["COEFFICIENT"]= doubleT;

	/**FILTER OPTS**/
	amap["MINIMUM"]=intT;
	amap["MAXIMUM"]=intT;
	amap["BY"]=stringT;
	/**OPERATORS **/
	amap["FACTOR"]=doubleT;   
	amap["OUTPUT_FACTOR"]=idT;

	/**PROPAGATE, WATERSHED**/
	amap["MASK"]=idT;
	amap["BACKGROUND"]=idT;
	amap["FOREGROUND"]=idT;
	amap["BIG_OBJECTS"]=idT;
	amap["FRAG_OBJECTS"]=idT;
/**GBLOB options**/
	amap["SEQUENCE"]=intT;
	amap["BETA"]=doubleT;

	/** Convert **/
	amap["DEPTH"]=intT;
	amap["NORMALIZE"]=boolT;
	/**HOUGH**/
	amap["LINES"]=idT;
	amap["RHO"]=doubleT;
	amap["DTHETA"]=doubleT;
	amap["THRESHOLD_INTERSECTIONS"]=intT;
	amap["PROBABILISTIC"]=boolT;
	amap["MIN_LINE_LENGTH"]=doubleT;
	amap["MAX_LINE_GAP"]=doubleT;
	return amap;
}