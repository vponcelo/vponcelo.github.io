#include "cv.h"
#include "cxcore.h"
#include "highgui.h"
#include "cvaux.h"

/* SEGMENTATION CONSTANTS */
#define VMIN 60
#define VMAX 180
#define SMIN 60

int dibujarEscGrises( CvRect, IplImage * );

int main() {
	IplImage *img = 0, *img2 = 0;
	CvRect selection;

	cvNamedWindow( "modelo", 0 );
	cvNamedWindow( "imagen", 0 );
	cvNamedWindow( "gray", 0 );
	cvMoveWindow( "modelo", 100, 200 );
	cvMoveWindow( "imagen", 500, 200 );
	cvMoveWindow( "gray", 900, 200 );

	img = cvLoadImage( "F:/colilla.bmp", CV_LOAD_IMAGE_COLOR );

	if ( !img )
		return -1;

	cvShowImage( "modelo", img );

	selection.x = 0;
	selection.y = 0;
	selection.width = img->width-1;
	selection.height = img->height-1;

	img2 = cvLoadImage( "F:/test/untitled16.bmp", CV_LOAD_IMAGE_COLOR );

	if ( !img2 )
		return -1;

	cvShowImage( "imagen", img2 );

	cvRectangle( img, cvPoint(selection.x, selection.y), cvPoint(selection.x+selection.width, selection.y+selection.height), CV_RGB(0, 255, 0) );
	//cvShowImage( "result", img );

	switch ( dibujarEscGrises( selection, img2 ) ) {
		case 1:
			printf( "No hay imagen a tratar" );
			break;
		case -1:
			printf(" No hay rectangulo de seleccion\n" );
			break;
	}

	cvWaitKey(1000);
	for (;;) {
		if( cvWaitKey( 10 ) >= 0 )
			break;
    }
	cvWaitKey(1000);
	cvDestroyWindow( "modelo" );
	cvDestroyWindow( "imagen" );
	cvDestroyWindow( "gray" );

	return 0;
}

// Funcion que dibuja una imagen en escala de grises de un frame a partir de una selección
int dibujarEscGrises( CvRect selection, IplImage *img ) {
	int hdims = 16, track_object = -1;
	float hranges_arr[] = {0,180}, *hranges = hranges_arr;
	IplImage *image = 0, *hsv, *hue, *mask, *backproject;
	CvHistogram *hist = 0;
	CvBox2D track_box;
	CvConnectedComp track_comp;
	CvRect track_window;

	if ( !img )
		return -1;

	if ( selection.width == NULL )
		return 1;

	if( !image ) {
        //allocate all the buffers
        image = cvCreateImage( cvGetSize(img), 8, 3 );
        image->origin = img->origin;
        hsv = cvCreateImage( cvGetSize(img), 8, 3 );
        hue = cvCreateImage( cvGetSize(img), 8, 1 );
        mask = cvCreateImage( cvGetSize(img), 8, 1 );
        backproject = cvCreateImage( cvGetSize(img), 8, 1 );
		hist = cvCreateHist( 1, &hdims, CV_HIST_ARRAY, &hranges, 1 );
    }
	
	cvCopy( img, image, 0 );
	cvCvtColor( image, hsv, CV_RGB2HSV );

    if( track_object ) {
        int _vmin = VMIN, _vmax = VMAX;
        cvInRangeS( hsv, cvScalar(0,SMIN,MIN(_vmin,_vmax),0), cvScalar(180,256,MAX(_vmin,_vmax),0), mask );
        cvSplit( hsv, hue, 0, 0, 0 );

        if( track_object < 0 ) {
            float max_val = 0.f;
            cvSetImageROI( hue, selection );
            cvSetImageROI( mask, selection );
			cvCalcHist( &hue, hist, 0, mask );
			cvGetMinMaxHistValue( hist, 0, &max_val, 0, 0 );
            cvConvertScale( hist->bins, hist->bins, max_val ? 255. / max_val : 0., 0 );
            cvResetImageROI( hue );
            cvResetImageROI( mask );
			track_window = selection;
            track_object = 1;
        }
		track_window = selection;
		cvCalcBackProject( &hue, backproject, hist );
		cvAnd( backproject, mask, backproject, 0 );
        cvCamShift( backproject, track_window,
                        cvTermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 ),
                        &track_comp, &track_box );
        track_window = track_comp.rect;
	}
	cvCvtColor( backproject, image, CV_GRAY2BGR );

	cvShowImage( "gray", image );

	return 0;
}
