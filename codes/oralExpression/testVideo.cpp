//#include "ipl.h"
#include "cv.h"
#include "cxcore.h"
#include "highgui.h"
#include "cvaux.h"
#include "BlobResult.h"
//#include "cvhaartraining.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <sstream>

#define minimF 3
#define minimH 4
#define INTERS 1
#define minimR 20
#define maxR 80
#define VMIN 60
#define VMAX 180
#define SMIN 60
#define PORCENT_SELEC 0.4
#define CONST_YREGION 0.5
#define CONST_WREGION 7
#define CONST_HREGION 6
#define MAX_WIDTH 600
#define MAX_HEIGHT 500
#define DIFPIX_REGIONES 100
#define DIFPIX_CARA 50
#define MANGA_CORTA_H 70
#define MANGA_CORTA_W 70
#define MIN_BLOB_W 15
#define MIN_BLOB_H 15
#define MAX_BLOB_W 250
#define MAX_BLOB_H 250
#define MAX_DIST_BLOBS 300
#define MAX_W_CARA 130
#define MAX_H_CARA 130
#define CAPT_POR_INTERVALO 30
#define PIX_QUIETUD 2
#define FRAMES_POR_MOVIMIENTO 5
#define MAX_INTERV 250
#define MAX_CHARS 300
#define MAX_AUDIO_FRAMES 15000
#define MAX_VIDEO_FRAMES 7200
#define PIX_MUCHA_AGITACION 4
#define LIM_Y_HIST_MAX 15
#define LIM_Y_HIST_MIN 5
#define MAX_DIST_JUNTOS 5
#define MAX_JUNTOS_SEGUIDOS 100
#define FRAMES_BARRAS 40
#define AGI_ALTA 20

CBlobResult blobs;
int frontal = 0, frontalTiempoReal = 0, detectado = 0, fallos = minimF+1, hits = 0, radius_ant = 0, juntos = 0, countMarcar = 0, pocosBlobs = 0;
int track_object = -1, primeraSeleccion = 1, count = 0, framesAudio = 0, framesVideoMostrados = 0, habla_frame[MAX_AUDIO_FRAMES];
int totalIzq = 0, totalDer = 0, totalCara = 0, marcarRegion = 1, captInterv = CAPT_POR_INTERVALO, interv = 0;
int captDer = 0, captIzq = 0, captCentro = 0, captQuieto = 0, habla = 0, noHabla = 0, derecha = 0, izquierda = 0;
int derSeparada = 0, izqSeparada = 0, caraSeparada = 0, captBlob = 0, countJuntos = 0, totalDirec = 0;
int totalIzqTR = 0, totalDerTR = 0, totalCaraTR = 0;
IplImage *image = 0, *hsv = 0, *hue = 0, *mask = 0, *backproject = 0, *displayedImage;
IplConvKernel *element;
CvHistogram *hist = 0;
CvPoint center_ant, centIzq_ant, centCara_ant, centDer_ant;
CvRect selection, track_window, regionCuerpo, *rface, *r, rect_ant, rblobIzq_ant, rblobDer_ant, rblobCara_ant;
CvBox2D track_box;
CvConnectedComp track_comp;
CvVideoWriter *writer;
float hranges_arr[] = {0,180}, *hranges = hranges_arr;
double interseccion = INTERS, promAgiIzq = 0, promAgiCab = 0, promAgiDer = 0, promAgiTotal = 0, prom_arr[MAX_INTERV];
double prom_agi_frame[MAX_VIDEO_FRAMES];
double promAgiIzqTR = 0, promAgiCabTR = 0, promAgiDerTR = 0, promAgiTotalTR = 0;
static CvMemStorage* storage = 0;
static CvHaarClassifierCascade* cascade = 0;
const char *cascade_name = "haarcascade_frontalface_alt.xml";

void detect_and_draw( IplImage* );
int dibujarEscGrises( IplImage*, double );
int seleccionarPorcion( int, double );
int dibujarRegiones(IplImage *, IplImage *, double );
CvRect buscarRegionPrincipal( IplImage *, double );
int calcularAgitacion( int, int, int );
int calcularPosicionIndividuo( int , int, int, int );
int guardarPromedios();
CvScalar hsv2rgb( float );

int main( int argc, char **argv ) {
	IplImage *frame, *frame_copy=0, *histimg, *status;
	CvCapture *capture;
	CvHistogram *histograma = 0;
	int optlen = strlen("--cascade="), i, bin_w, hablar = 0, haHablado = 0, framesEnPausa = 0;
	int habla_agi = 0, habla_noAgi = 0, noHabla_agi = 0, noHabla_noAgi = 0, pausas10sec = 0, pausas8sec = 0, pausas6sec = 0;
	int pausas4sec = 0, pausas2sec = 0, pausas12sec = 0, pausas14sec = 0, pausas16sec = 0, pausas18sec = 0, pausas20sec = 0;
	double relFps, fpsAudio = 0;
	FILE *fichero;
	int framesTiempoReal = 0, f_100 = 0, f_75 = 0, f_50 = 0, f_25 = 0, f_0 = 0, f_total = 0, a_total = 0, a_100 = 0, a_75 = 0, a_50 = 0, a_25 = 0, a_0 = 0;
	double porcentFrontReal = 0;
	std::ostringstream str;
	
	cascade_name = "haarcascade_frontalface_alt2.xml";
	//xml con las características de la cara
	cascade = (CvHaarClassifierCascade*)cvLoad( cascade_name, 0, 0, 0 );
	element = cvCreateStructuringElementEx(15, 15, 10, 10, CV_SHAPE_RECT);

	if( !cascade ) {
        fprintf( stderr, "ERROR: Could not load classifier cascade\n" );
        fprintf( stderr, "Usage: facedetect --cascade=\"<cascade_path>\" [filename|camera_index]\n" );
        return -1;
    }
	if ( !argv[1] ) {
		perror( "This program need 1 argument\n");
		return -1;
	}

	printf( "%s\n", argv[1] );
	capture = cvCaptureFromFile( argv[1] ); 

	// Cambiamos el nombre del fichero de entrada 
	for ( i = 0; i < MAX_CHARS; i++ ) {
		if ( argv[1][i] == '.' ) {
			argv[1][i+1] = 't';
			argv[1][i+2] = 'x';
			argv[1][i+3] = 't';
			break;
		}
	}
		
	// Abrimos el fichero que contiene informacion sobre el audio
    if ( (fichero = fopen( argv[1], "r" )) == NULL ) {
		printf( "No audio\n" );
	} else {
		while ( !feof(fichero) ) {
			fscanf( fichero, "%i", &hablar );
			fscanf( fichero, "%i", &hablar );
			framesAudio++;
		}
		fpsAudio = (double)(framesAudio/4)/60;
		rewind( fichero );
		// Se recorre el fichero y se guarda la informacion sobre el habla
		while ( !feof(fichero) ) {
			fscanf( fichero, "%i", &hablar );
			// Si se ha detectado que esta hablando, se incrementa el total de habla
			if ( hablar == 2 ) {
				habla++;
				haHablado = 1;
			}
			fscanf( fichero, "%i", &hablar );
			// Si previamente ya se ha detectado que habla, no se incrementa
			if ( hablar == 2 && !haHablado ) {
				habla++;
				haHablado = 1;
			}
			// Se calculan los frames en que no se esta hablando y se guarda en un vector si habla o no en el frame actual
			if ( !haHablado ) {
				habla_frame[framesAudio] = 0;
				framesEnPausa++;
				// Consideraremos una pausa cuando pasen mas de 2 segundos sin decir nada en adelante
				if ( framesEnPausa >= 2*fpsAudio ) {
					if ( framesEnPausa >= 4*fpsAudio ) {
						if ( framesEnPausa >= 6*fpsAudio ) {
							if ( framesEnPausa >= 8*fpsAudio ) {
								if ( framesEnPausa >= 10*fpsAudio ) {
									if ( framesEnPausa >= 12*fpsAudio ) {
										if ( framesEnPausa >= 14*fpsAudio ) {
											if ( framesEnPausa >= 16*fpsAudio ) {
												if ( framesEnPausa >= 18*fpsAudio ) {
													if ( framesEnPausa >= 20*fpsAudio ) {
														pausas20sec++;
														framesEnPausa = 0;
													} else {
														pausas18sec++;
														framesEnPausa = 0;
													}
												} else {
													pausas16sec++;
													framesEnPausa = 0;
												}
											} else {
												pausas14sec++;
												framesEnPausa = 0;
											}
										} else {
											pausas12sec++;
											framesEnPausa = 0;
										}
									} else {
										pausas10sec++;
										framesEnPausa = 0;
									}
								} else {
									pausas8sec++;
									framesEnPausa = 0;
								}
							} else {
								pausas6sec++;
								framesEnPausa = 0;
							}
						} else {
							pausas4sec++;
							framesEnPausa = 0;
						}
					} else {
						pausas2sec++;
						framesEnPausa = 0;
					}
				}
			} else {
				habla_frame[framesAudio] = 1;
				framesEnPausa = 0;
			}
				
			haHablado = 0;
			hablar = 0;
		}
		fclose(fichero);
	}

	rblobIzq_ant.x = 1;
	rblobDer_ant.x = 2;

	/* Caso que se quiera partir de un modelo de color
	modelo = cvLoadImage( "modelo_color.bmp", CV_LOAD_IMAGE_COLOR );

	if ( !modelo )
		return -1;

	cvShowImage( "modelo", modelo );

	selection.x = 0;
	selection.y = 0;
	selection.width = modelo->width-1;
	selection.height = modelo->height-1;
	*/
	
	storage = cvCreateMemStorage(0);
	cvNamedWindow( "result", 0 );
	cvNamedWindow( "Blobs", 0 );
	cvNamedWindow( "barras", 0 );
	cvMoveWindow( "result", 135, 50 );
	cvResizeWindow( "result", 500, 400 );
	cvMoveWindow( "Blobs", 665, 50 );
	cvResizeWindow( "Blobs", 500, 400 );
	cvMoveWindow( "barras", 375, 501 );
	cvResizeWindow( "barras", 500, 200 );
	//cvNamedWindow( "modelo", 0 );

	//cvShowImage( "modelo", modelo );

	double fps = cvGetCaptureProperty(capture, CV_CAP_PROP_FPS);
	// Calculamos los frames que trabajaremos en 4 minutos
	int frames = fps*60*4;

	if( capture ) {								//Ejecuta frame a frame el video
        for(;;) {
            if( !cvGrabFrame( capture ))		//Coje una imagen de la captura
                break;
            frame = cvRetrieveFrame( capture );	//Utiliza la imagen cogida en el Grab para guardarla en el puntero
			//frame = cvQueryFrame( capture );	//Otra forma de capturar la imagen sin necesidad de usar grab
            if( !frame )
                break;
            if( !frame_copy )
                frame_copy = cvCreateImage( cvSize(frame->width,frame->height), IPL_DEPTH_8U, frame->nChannels );
				//Crea una imagen con la informacion de la captura
            if( frame->origin == IPL_ORIGIN_TL )
                cvCopy( frame, frame_copy, 0 );
				//Crea una copia identica de la imagen
            else
                cvFlip( frame, frame_copy, 0 ); 
			if ( !writer )
				writer = cvCreateVideoWriter( "D:/proyecto/videosGrabados/video.avi", CV_FOURCC('D','I','V','X'), 29, cvGetSize(frame));
			detect_and_draw( frame_copy );

			porcentFrontReal = (double)frontalTiempoReal*100/framesTiempoReal;

			// Cada 20 frames se actualizan las barras de estado
			if ( framesTiempoReal >= FRAMES_BARRAS ) {
				status = cvCreateImage( cvSize(500,200), IPL_DEPTH_8U, 3 );
				cvZero( status );
				//cvRectangle( status, cvPoint(0,0), cvPoint(500,200), CV_RGB(0, 0, 0), CV_FILLED );cvReleaseImage( &status );
				CvFont font;
				cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.5, 0.5, 0, 1, CV_AA);
				cvPutText(status, "agitacion", cvPoint(65, 195), &font, cvScalar(255, 255, 255, 0));
				cvPutText(status, "frontal", cvPoint(320, 195), &font, cvScalar(255, 255, 255, 0));
				cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.35, 0.35, 0, 1, CV_AA);
				
				// Dibujo de la barra del promedio de agilidad total
				if ( promAgiTotalTR >= AGI_ALTA ) {
					cvRectangle( status, cvPoint(40,180), cvPoint(160,0), CV_RGB(255, 0, 0), CV_FILLED );
					a_100++;
				} else if ( promAgiTotalTR >=  (AGI_ALTA/2)+AGI_ALTA/4) {
					cvRectangle( status, cvPoint(40,180), cvPoint(160,50), CV_RGB(175, 50, 0), CV_FILLED );
					a_75++;
				} else if ( promAgiTotalTR >= AGI_ALTA/2 ) {
					cvRectangle( status, cvPoint(40,180), cvPoint(160,95), CV_RGB(100, 100, 0), CV_FILLED );
					a_50++;
				} else if ( promAgiTotalTR >= (AGI_ALTA/2)-AGI_ALTA/4) {
					cvRectangle( status, cvPoint(40,180), cvPoint(160,140), CV_RGB(50, 175, 0), CV_FILLED );
					a_25++;
				} else if ( promAgiTotalTR >= 0 ) {
					cvRectangle( status, cvPoint(40,180), cvPoint(160,178), CV_RGB(0, 255, 0), CV_FILLED );
					a_0++;
				}
				a_total = a_100+a_75+a_50+a_25+a_0;
				str.str(std::string());
				str << (float)a_100*100/a_total;
				cvPutText(status, strcat((char *)str.str().c_str(), "%"), cvPoint(165, 10), &font, cvScalar(255, 255, 255, 0));
				str.str(std::string());
				str << (float)a_75*100/a_total;
				cvPutText(status, strcat((char *)str.str().c_str(), "%"), cvPoint(165, 50), &font, cvScalar(255, 255, 255, 0));
				str.str(std::string());
				str << (float)a_50*100/a_total;
				cvPutText(status, strcat((char *)str.str().c_str(), "%"), cvPoint(165, 95), &font, cvScalar(255, 255, 255, 0));
				str.str(std::string());
				str << (float)a_25*100/a_total;
				cvPutText(status, strcat((char *)str.str().c_str(), "%"), cvPoint(165, 140), &font, cvScalar(255, 255, 255, 0));
				str.str(std::string());
				str << (float)a_0*100/a_total;
				cvPutText(status, strcat((char *)str.str().c_str(), "%"), cvPoint(165, 180), &font, cvScalar(255, 255, 255, 0));

				// Dibujo de la barra del porcentaje de miradas frontales
				if ( porcentFrontReal >= 87.5 ) {
					cvRectangle( status, cvPoint(285,180), cvPoint(405,0), CV_RGB(0, 255, 0), CV_FILLED );
					f_100++;
				} else if ( porcentFrontReal >= 67.5 ) {
					cvRectangle( status, cvPoint(285,180), cvPoint(405,50), CV_RGB(50, 175, 0), CV_FILLED );
					f_75++;
				} else if ( porcentFrontReal >= 37.5 ) {
					cvRectangle( status, cvPoint(285,180), cvPoint(405,95), CV_RGB(100, 100, 0), CV_FILLED );
					f_50++;
				} else if ( porcentFrontReal >= 12.5 ) {
					cvRectangle( status, cvPoint(285,180), cvPoint(405,140), CV_RGB(175, 50, 0), CV_FILLED );
					f_25++;
				} else if ( porcentFrontReal >= 0 ) {
					cvRectangle( status, cvPoint(285,180), cvPoint(405,178), CV_RGB(255, 0, 0), CV_FILLED );
					f_0++;
				}
				f_total = f_100+f_75+f_50+f_25+f_0;
				str.str(std::string());
				str << (float)f_100*100/f_total;
				cvPutText(status, strcat((char *)str.str().c_str(), "%"), cvPoint(410, 10), &font, cvScalar(255, 255, 255, 0));
				str.str(std::string());
				str << (float)f_75*100/f_total;
				cvPutText(status, strcat((char *)str.str().c_str(), "%"), cvPoint(410, 50), &font, cvScalar(255, 255, 255, 0));
				str.str(std::string());
				str << (float)f_50*100/f_total;
				cvPutText(status, strcat((char *)str.str().c_str(), "%"), cvPoint(410, 95), &font, cvScalar(255, 255, 255, 0));
				str.str(std::string());
				str << (float)f_25*100/f_total;
				cvPutText(status, strcat((char *)str.str().c_str(), "%"), cvPoint(410, 140), &font, cvScalar(255, 255, 255, 0));
				str.str(std::string());
				str << (float)f_0*100/f_total;
				cvPutText(status, strcat((char *)str.str().c_str(), "%"), cvPoint(410, 180), &font, cvScalar(255, 255, 255, 0));
				str.str(std::string());

				framesTiempoReal = 0;
				frontalTiempoReal = 0;
				totalIzqTR = 0;
				totalDerTR = 0;
				totalCaraTR = 0;
				promAgiIzqTR = 0;
				promAgiCabTR = 0;
				promAgiDerTR = 0;
				promAgiTotalTR = 0;
				cvShowImage( "barras", status );
				cvReleaseImage( &status );
			}

			cvReleaseImage( &frame_copy );
			framesVideoMostrados++;
			framesTiempoReal++;

            if( cvWaitKey( 10 ) >= 0 || framesVideoMostrados >= frames )
                break;
        }

		/* Caracteristicas */

		for ( i = 0; i < MAX_CHARS; i++ ) {
			if ( argv[1][i] == '.' ) {
				argv[1][i+1] = 'd';
				argv[1][i+2] = 'a';
				argv[1][i+3] = 't';
				break;
			}
		}
		fichero = fopen( argv[1], "w");

		// Propiedades del video
		printf( "El video tiene un total de %i frames\n", frames );
		printf( "El video es de %lf fps\n", fps );

		// Total de frames del video mostrados
		printf( "Total de frames mostrados: %i\n", framesVideoMostrados );

		// En este caso tenemos informacion sobre el audio
		if ( fichero != NULL ) {
			// Propiedades del audio teniendo en cuenta que dura 4 minutos
			relFps = fpsAudio/fps;
			printf( "El audio tiene %i frames\n", framesAudio );
			printf( "El audio es de %lf fps\n", fpsAudio );
			printf( "Relacion fps audio respecto a video: %lf\n", relFps );

			// Porcentaje de habla del individuo
			printf( "Porcentaje de habla del individuo: %lf\n", (double)(habla*100)/framesAudio );
			printf( "Porcentaje de no habla del individuo: %lf\n", (double)((framesAudio-habla)*100)/framesAudio );
			printf( "Numero de pausas de 2 segundos: %i\n", pausas2sec );
			printf( "Numero de pausas de 4 segundos: %i\n", pausas4sec );
			printf( "Numero de pausas de 6 segundos: %i\n", pausas6sec );
			printf( "Numero de pausas de 8 segundos: %i\n", pausas8sec );
			printf( "Numero de pausas de 10 segundos: %i\n", pausas10sec );
			printf( "Numero de pausas de 12 segundos: %i\n", pausas12sec );
			printf( "Numero de pausas de 14 segundos: %i\n", pausas14sec );
			printf( "Numero de pausas de 16 segundos: %i\n", pausas16sec );
			printf( "Numero de pausas de 18 segundos: %i\n", pausas18sec );
			printf( "Numero de pausas de 20 segundos: %i\n", pausas20sec );
		}

		// Escritura de los tags de las columnas del fichero caracteristicas.txt
		fprintf( fichero, "Video\tHabla\tNo_habla\tPausas2\tPausas4\tPausas6\tPausas8\tPausas10\tPausas12\tPausas14\tPausas16\tPausas18\tPausas20\t" );
		fprintf( fichero, "RelacionF_NOF\t" );
		fprintf( fichero, "Frontal\tNo_Frontal\t" );
		fprintf( fichero, "Agit_der\tAgit_cab\tAgit_izq\tAgit_general\tAgit_direc_der\tAgit_direc_izq\t" );		
		fprintf( fichero, "Habla_agit\tNo_habla_agit\tHabla_no_agit\tNo_Habla_No_Agit\t" );
		fprintf( fichero, "PosIzq\tPosDer\tPosCent\tQuieto\n" );

		// Empezamos la escritura de los datos
		fprintf( fichero, "%s\t%lf\t%lf\t", argv[1], (double)(habla*100)/framesAudio, (double)((framesAudio-habla)*100)/framesAudio );
		fprintf( fichero, "%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t", pausas2sec, pausas4sec, pausas6sec, pausas8sec,
			pausas10sec, pausas12sec, pausas14sec, pausas16sec, pausas18sec, pausas20sec );

		// Capturas frontales y no frontales
		printf( "Porcentaje capturas frontales: %lf\n", (double)(frontal*100)/framesVideoMostrados );
		printf( "Porcentaje de capturas no frontales: %lf\n", (double)((framesVideoMostrados-frontal)*100)/framesVideoMostrados );

		if ( (framesVideoMostrados-frontal) > 0 )
			printf( "Relacion capturas frontales respecto no frontales: %lf\n", (double)frontal/(framesVideoMostrados-frontal) );

		fprintf( fichero, "%lf\t", (double)frontal/(framesVideoMostrados-frontal) );
		fprintf( fichero, "%lf\t%lf\t", (double)(frontal*100)/framesVideoMostrados, (double)((framesVideoMostrados-frontal)*100)/framesVideoMostrados );

		// Agitacion de brazos y cabeza
		printf( "Promedio agitacion brazo derecho: %lf\n", promAgiIzq );
		printf( "Promedio agitacion cabeza: %lf\n", promAgiCab );
		printf( "Promedio agitacion brazo izquierdo: %lf\n", promAgiDer );
		printf( "Promedio agitacion general: %lf\n", promAgiTotal );
		printf( "Porcentaje de agitacion hacia la derecha: %lf\n", (double)(derecha*100)/totalDirec ); 
		printf( "Porcentaje de agitacion hacia la izquierda: %lf\n", (double)(izquierda*100)/totalDirec );

		fprintf( fichero, "%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t", promAgiIzq, promAgiCab, promAgiDer, promAgiTotal,
			(double)(derecha*100)/count, (double)(izquierda*100)/count );

		// Porcentaje de agitacion segun si habla o no
		for ( i = 0; i < framesVideoMostrados; i++ ) {
			int frameEquiv = i*relFps;
			//printf ( "%i - frameEquiv: %i , PROMEDIO_AGI: %lf , HABLA: %i\n", i, frameEquiv, prom_agi_frame[i], habla_frame[frameEquiv] );
			if ( prom_agi_frame[i] != NULL && prom_agi_frame[i] >= PIX_MUCHA_AGITACION ) {
				// Se comprueba la agitacion y habla en los frames equivalentes de video y audio
				if (  habla_frame[frameEquiv] ) {
					habla_agi++;
				} else 
					noHabla_agi++;
			} else if ( prom_agi_frame[i] == NULL || prom_agi_frame[i] < PIX_MUCHA_AGITACION ) {
				if ( habla_frame[frameEquiv] ) {
					habla_noAgi++;
				} else 
					noHabla_noAgi++;
			}
		}
		printf( "El porcentaje de habla mientras hay bastante agitacion es de %lf\n", (double)(habla_agi*100)/framesVideoMostrados );
		printf( "El porcentaje de no habla mientras hay bastante agitacion es de %lf\n", (double)(noHabla_agi*100)/framesVideoMostrados );
		printf( "El porcentaje de habla mientras no hay agitacion es de %lf\n", (double)(habla_noAgi*100)/framesVideoMostrados );
		printf( "El porcentaje de no habla mientras no hay agitacion es de %lf\n", (double)(noHabla_noAgi*100)/framesVideoMostrados );

		fprintf( fichero, "%lf\t%lf\t%lf\t%lf\t", (double)(habla_agi*100)/framesVideoMostrados, (double)(noHabla_agi*100)/framesVideoMostrados,
			(double)(habla_noAgi*100)/framesVideoMostrados, (double)(noHabla_noAgi*100)/framesVideoMostrados );

		// Posicion del individuo
		printf( "Porcentaje con el individuo situado a la izquierda: %lf\n", (double)(captIzq*100)/captBlob);
		printf( "Porcentaje con el individuo situado a la derecha: %lf\n", (double)(captDer*100)/captBlob);
		printf( "Porcentaje con el individuo situado en el centro: %lf\n", (double)(captCentro*100)/captBlob);
		printf( "Porcentaje con el individuo quieto: %lf\n", (double)((captQuieto/FRAMES_POR_MOVIMIENTO)*100)/captBlob);

		fprintf( fichero, "%lf\t%lf\t%lf\t%lf\n", (double)(captIzq*100)/framesVideoMostrados, (double)(captDer*100)/framesVideoMostrados, 
			(double)(captCentro*100)/framesVideoMostrados, (double)((captQuieto/FRAMES_POR_MOVIMIENTO)*100)/framesVideoMostrados );

		// Histograma de movimientos
		cvNamedWindow( "Histograma", 0 );
		// Se mira si se puede dibujar el histograma mirando si se han definido intervalos
		if ( interv ) {
			histograma = cvCreateHist( 1, &interv, CV_HIST_ARRAY, &hranges, 1 );
			histimg = cvCreateImage( cvSize(320,200), 8, 3 );
			cvZero( histimg );
			bin_w = histimg->width / interv;
			float max = 0;
			for ( i = 0; i < interv; i++ ) {
				if ( max < prom_arr[i] )
					max = prom_arr[i];
			}
			if ( max > 0 ) {
				while ( max > LIM_Y_HIST_MAX )
					max /= 2;
				while ( max < LIM_Y_HIST_MIN )
					max *= 2;
				// Calculamos el valor de y en el histograma, que son los promedios
				for ( i = 0; i < interv; i++ ) {
					float y = (int)prom_arr[i];
					int val = cvRound( cvGetReal1D(histograma->bins,i)*histimg->height/255 );
					CvScalar color = hsv2rgb(i*154.f/interv);
					cvRectangle( histimg, cvPoint(i*bin_w,histimg->height), cvPoint((i+1)*bin_w,histimg->height - y*max), color, -1, 8, 0 );
				}
				cvShowImage( "Histograma", histimg );
			}
			cvReleaseImage( &histimg );
		} else 
			printf( "No hay suficientes datos para dibujar el histograma\n" );
    } else 
		printf("Capture Error\n");

	cvWaitKey(1000);
	cvReleaseCapture( &capture );
	cvReleaseVideoWriter( &writer );
	cvDestroyWindow( "result" );
	cvDestroyWindow( "Blobs" );
	cvDestroyWindow( "barras" );
	for (;;) {
		if( cvWaitKey( 10 ) >= 0 )
			break;
    }
	cvWaitKey(1000);
	//cvReleaseImage( &frame );
	cvDestroyWindow( "Histogram" );

	return 0;
}

/* Funcion que detecta la cara en cada frame */
void detect_and_draw( IplImage* img ) {
    static CvScalar colors[] = 
    {
        {{0,0,255}},
        {{0,128,255}},
        {{0,255,255}},
        {{0,255,0}},
        {{255,128,0}},
        {{255,255,0}},
        {{255,0,0}},
        {{255,0,255}}
    };

    double scale = 3.5, t = (double)cvGetTickCount();
	int i = 0, min, max, radius = 0, escGrises;
    IplImage* gray = cvCreateImage( cvSize(img->width,img->height), 8, 1 );
    IplImage* small_img = cvCreateImage( cvSize( cvRound (img->width/scale),
                         cvRound (img->height/scale)), 8, 1 );
    CvSeq* faces;
	CvPoint center;

    cvCvtColor( img, gray, CV_BGR2GRAY );
    cvResize( gray, small_img, CV_INTER_LINEAR );
    cvEqualizeHist( small_img, small_img );
    cvClearMemStorage( storage );
	
	faces = cvHaarDetectObjects( small_img, cascade, storage, 
		1.1, 2, 0/*CV_HAAR_DO_CANNY_PRUNING*/, cvSize(20, 20) );			//Mirar secuencia cara
    if( cascade ) {
        t = (double)cvGetTickCount() - t;		
		// Se recorren las caras encontradas
		for ( i = 0; i < (faces ? faces->total : 0); i++ ) {			
			rface = (CvRect*)cvGetSeqElem( faces, i );
			double dist_centros;
			// Esta sera la primera region (cara)
			center.x = cvRound((rface->x + rface->width*0.5)*scale);
			center.y = cvRound((rface->y + rface->height*0.5)*scale);
			radius = cvRound((rface->width + rface->height)*0.25*scale);
			// Se usaran estos valores para el calculo de la interseccion
			if ( radius >= radius_ant ) {
				min = radius_ant;
				max = radius;
			} else {
				max = radius_ant;
				min = radius;
			}
			dist_centros = sqrt((center.x-center_ant.x)*(center.x-center_ant.x)+(center.y-center_ant.y)*(center.y-center_ant.y));
			interseccion = (min - dist_centros)/max;

			// Si hay mas de una cara nos quedaremos con la que nos interesa
			if ( faces->total > 1 ) {
				if ( interseccion <= INTERS && interseccion > 0 && radius >= minimR && radius <= maxR ) {
					r = rface;
					hits++;
					center_ant.x = center.x;
					center_ant.y = center.y;
					radius_ant = radius;
					dist_centros = sqrt((center.x-center_ant.x)*(center.x-center_ant.x)+(center.y-center_ant.y)*(center.y-center_ant.y));
					interseccion = (min - dist_centros)/max;
				} //else 
					//printf( "detection out of range\n" );
			} else if ( faces->total == 1 )
				r = rface;
		}
		
		// Caso en que no se ha detectado cara
		if ( faces->total == 0 ) {
			/* Comprobamos los fallos que han habido y si son menores que el minimo se dibujara 
			 * (inicialmente fallo=minimF+1 para evitar que se dibuje la primera vez si no se detecta cara).
			 * Si hay mas de minimF fallos no se dibuja, se reinician los hits y se pone la variable detectado a 0
			 */
			if ( fallos <= minimF ) {
				frontal++;
				frontalTiempoReal++;
				fallos++;
				center = center_ant;
				radius = radius_ant;
				cvCircle( img, center, radius, colors[i%8], 3, 8, 0 );		//dibujar circulos de distintos colores
				//printf( "Drawing face, no face detected\n" );
				if ( seleccionarPorcion( radius, scale ) == -1 )
					perror( "No hay rectangulo de seleccion\n" );
			} else {
				hits = 0;
				detectado = 0;
				//printf( "Face not detected\n" );
				//printf( "Not drawing\n" );
			}

		// Caso en que se han detectado 1 o mas caras
		} else {
			/* Se dibujara si se ha detectado una cara de tamano correcto y hay interseccion, 
			 * una vez superados los hits minimos
			 */
			if ( detectado && interseccion <= INTERS && interseccion > 0 && radius >= minimR && radius <= maxR ) {
				frontal++;
				frontalTiempoReal++;
				fallos = 0;
				cvCircle( img, center, radius, colors[i%8], 3, 8, 0 );		//dibujar circulos de distintos colores
				//printf( "Drawing face\n" );
				if ( seleccionarPorcion( radius, scale ) == -1 )
					perror( "No hay rectangulo de seleccion\n" );
			/* Se dibujará en la posición detectada anteriormente en caso de una detección fuera de rango o de 
			 * tamano incorrecto si aun no ha habido mas de minimF fallos
			 */
			} else if ( detectado && (interseccion > INTERS || interseccion < 0 || radius <= minimR || radius >= maxR) && fallos <= minimF ) {
				frontal++;
				frontalTiempoReal++;
				fallos++;
				center = center_ant;
				radius = radius_ant;
				cvCircle( img, center, radius, colors[i%8], 3, 8, 0 );		//dibujar circulos de distintos colores
				//printf( "Drawing face, but out of range\n" );
				if ( seleccionarPorcion( radius, scale ) == -1 )
					perror( "No hay rectangulo de seleccion\n" );
			/* Este es el caso en que aun no tenemos suficientes detecciones correctas para considerar
			 * que las detecciones se corresponden a una cara
			 */
			} else if ( !detectado && hits <= minimH ) {
				if ( interseccion <= INTERS && interseccion > 0 && radius >= minimR && radius <= maxR )
					hits++;
			/* Este es el caso en que ya tenemos suficientes detecciones correctas para considerar que las 
			 * detecciones corresponden a una cara, por lo que comprobamos que la deteccion actual cumpla tambien
			 * las caracteristicas similares a las de una cara y en ese caso dibujaremos y canviaremos la variable 
			 * detectado a 1
			 */
			} else if ( !detectado && hits > minimH ) {
				if ( interseccion <= INTERS && interseccion > 0 && radius >= minimR && radius <= maxR ) {
					frontal++;
					frontalTiempoReal++;
					hits = 0;
					fallos = 0;
					detectado = 1;
					cvCircle( img, center, radius, colors[i%8], 3, 8, 0 );		//dibujar circulos de distintos colores
					//printf( "Drawing face\n" );
					if ( seleccionarPorcion( radius, scale ) == -1 )
						perror( "No hay rectangulo de seleccion\n" );
				} else 
					printf( "detection out of range\n" );
			}
			// Se guardan los valores actuales para posibles errores en las detecciones futuras
			center_ant.x = center.x;
			center_ant.y = center.y;
			radius_ant = radius;
			if ( radius >= minimR && radius <= maxR ) {
				printf( "intersection = %lf , radio OK\n", interseccion );
			} else 
				printf( "intersection = %lf , radio malo\n", interseccion );
		}
    }

	// Se llama a la funcion que transforma el frame a escala de grises
	escGrises = dibujarEscGrises( img , scale );
	switch ( escGrises ) {
		case -1:
			perror( "Error de frame \n");
			break;
		case 1:
			printf( "No hay seleccion\n" );
			break;
	}	
	printf( "\n" );

	// Se muestra el frame resultante y se libera la memoria
    cvShowImage( "result", img );
    cvReleaseImage( &gray );
    cvReleaseImage( &small_img );
}

// Funcion que selecciona la porcion de la cara a partir del cual se hará la segmentacion del color
int seleccionarPorcion( int rad, double scale ) {
	
	// Se comprueba que el rectangulo de seleccion de la cara existe
	if ( !r )
		return -1;

	// Se anade una nueva seleccion si esta esta en sus valores correctos
	if ( interseccion <= INTERS && interseccion >= 0 && rad < radius_ant && rad >= minimR && rad <= maxR || primeraSeleccion ) {
		selection.x = (int)r->x*scale + r->width*PORCENT_SELEC;
		selection.y = (int)r->y*scale + r->height*PORCENT_SELEC;
		selection.width = (int)r->width*scale * (1-PORCENT_SELEC);
		selection.height = (int)r->height*scale * (1-PORCENT_SELEC);
		//printf( "Seleccion nueva: %i %i\n", selection.width, selection.height);
		primeraSeleccion = 0;
	}

	return 0;
}

// Funcion que dibuja una imagen en escala de grises de un frame a partir de una selección
int dibujarEscGrises( IplImage* frame, double scale ) {
	int hdims = 16, regiones;

	if ( !frame ) 
		return -1;
	if ( selection.width == NULL )
		return 1;

	if( !image ) {
        //allocate all the buffers
        image = cvCreateImage( cvGetSize(frame), 8, 3 );
        image->origin = frame->origin;
        hsv = cvCreateImage( cvGetSize(frame), 8, 3 );
        hue = cvCreateImage( cvGetSize(frame), 8, 1 );
        mask = cvCreateImage( cvGetSize(frame), 8, 1 );
        backproject = cvCreateImage( cvGetSize(frame), 8, 1 );
		hist = cvCreateHist( 1, &hdims, CV_HIST_ARRAY, &hranges, 1 );
		cvZero( image );
		cvZero( hsv );
		cvZero( mask );
		cvZero( backproject );
    }
	
	cvCopy( frame, image, 0 );
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
        //cvCvtColor( backproject, image, CV_GRAY2BGR );
	}

	// Llamamos a la funcion que dibuja las regiones de los brazos y hace el seguimiento
	regiones = dibujarRegiones( frame, backproject, scale );
	switch ( regiones ) {
		case -1:
			perror( "Error de imagen o deteccion\n" );
			break;
		case 1:
			printf( "No se puede dibujar la region\n" );
			break;
		case 2:
			marcarRegion = 1;
			printf( "REGION PERDIDA\n" );
			break;
	}

	// Grabamos la imagen que se va a mostrar
	cvWriteFrame( writer, frame );
	// Mostramos los frames habiendo aplicado erosion y morfology
	cvShowImage( "dibujoGray", displayedImage );
	cvReleaseImage( &displayedImage );
	
	return 0;
}

/* Funcion que dibuja las regiones de los brazos y hace su seguimiento a partir de una imagen en escala de grises,
 * y que pinta el resultado en el frame original
 */
int dibujarRegiones( IplImage *frame, IplImage *imageGray, double scale ) {
	IplImage *tmp = 0, *erosioned;
	int i, detRegIzq = 0, detRegDer = 0, detRegCara = 0;

	// r es el ultimo rectangulo del face detect. Si no se detecta cara hay que trabajar con los blobs
	if ( !imageGray )
		return -1;

	// creamos la imagen que se mostrara finalmente
	displayedImage = cvCreateImage(cvGetSize(frame), frame->depth, 3);
	cvZero( displayedImage );

	// discard the blobs with less area than 5000 pixels
	// exclude the ones smaller than param2 value
	blobs.Filter( blobs, B_EXCLUDE, CBlobGetArea(), B_LESS, 5000 );

	// creamos una imagen copia de la imagen en gris, donde aplicaremos erosion y morfology si es necesario
	erosioned = cvCreateImage(cvGetSize(frame), frame->depth, 1);
	cvCopy( imageGray, erosioned, 0 );
	
	// create a file with some of the extracted features
	blobs = CBlobResult( imageGray, NULL, 0 );
	blobs.PrintBlobs( "d:/proyecto/blobsOrig.txt" );
	
	// Aplicamos erosion y morphology hasta obtener un numero de blobs de la imagen validos
	cvMorphologyEx( imageGray, erosioned, tmp, element, CV_MOP_CLOSE );
	blobs = CBlobResult( erosioned, NULL, 0 );
	
	// create a file with some of the extracted features
	blobs.PrintBlobs( "d:/proyecto/blobsEro.txt" );

	cvMerge( erosioned, erosioned, erosioned, NULL, displayedImage );

	// Se comprueba si hay blob en las regiones o si hay orden de marcar la region principal
	if ( marcarRegion ) {
		rect_ant = buscarRegionPrincipal( imageGray, scale );
		// Si se ha recibido el rectangulo de region principal lo dibujaremos
		if ( rect_ant.x != NULL ) {
			//cvRectangle( frame, cvPoint(rect_ant.x, rect_ant.y), cvPoint(rect_ant.x + rect_ant.width, rect_ant.y + rect_ant.height), CV_RGB(0, 255, 0), 2 );
			//printf(" %i %i %i %i\n", rect_ant.x, rect_ant.y, rect_ant.width, rect_ant.height );
		} else 
			marcarRegion = 1;
	}

	// Se recorren los blobs y se buscan los brazos dentro de las regiones. Se sale cuando se han encontrado
	// Se comprueba si hay blob en las regiones o si hay orden de marcar la region principal
	if ( marcarRegion ) {
		rect_ant = buscarRegionPrincipal( imageGray, scale );
		// Si se ha recibido el rectangulo de region principal lo dibujaremos
		if ( rect_ant.x != NULL ) {
			//cvRectangle( frame, cvPoint(rect_ant.x, rect_ant.y), cvPoint(rect_ant.x + rect_ant.width, rect_ant.y + rect_ant.height), CV_RGB(0, 255, 0), 2 );
			//cvLine( frame, cvPoint(rect_ant.x, rect_ant.y+2*r->height*scale), cvPoint(rect_ant.x + rect_ant.width, rect_ant.y+2*r->height*scale), CV_RGB(0, 255, 0), 2 );
			//printf(" %i %i %i %i\n", rect_ant.x, rect_ant.y, rect_ant.width, rect_ant.height );
		} else 
			marcarRegion = 0;			// Si el rectangulo es null, ponemos a 0 para hacer seguimiento normal
	}

	// Se recorren los blobs y nos quedamos con los 3 blobs más grandes
	for (i = 0; i < blobs.GetNumBlobs(); i++ ) {
		CBlob currentBlob = blobs.GetBlob(i);
		CvRect rblob;
		rblob.x = currentBlob.MinX();
		rblob.y = currentBlob.MinY();
		rblob.width = currentBlob.MaxX()-rblob.x;
		rblob.height = currentBlob.MaxY()-rblob.y;

		// Caso que aun no se ha detectado la region del medio, donde se encuentra la cara
		if ( !detRegCara ) {
			// Si ha habido deteccion de cara, se define como region el rectangulo del face detect
			if ( detectado && marcarRegion ) {
				//cvRectangle( frame, cvPoint( r->x*scale, r->y*scale ), cvPoint( r->x*scale+r->width*scale, r->y*scale+r->height*scale ), CV_RGB( 0, 0, 255 ) );
				if ( currentBlob.MinX() < center_ant.x+DIFPIX_REGIONES && currentBlob.MinX() > center_ant.x-DIFPIX_REGIONES && currentBlob.MinY() < center_ant.y+DIFPIX_REGIONES && currentBlob.MinY() > center_ant.y-DIFPIX_REGIONES ) {
					rblobCara_ant = rblob;
					detRegCara = 1;
				}
			// Si no ha habido deteccion de cara se usa el seguimiento de los blobs de la region de la cara
			// Se comprueba que la distancia entre el blob anterior y el actual son cortas para dar por buena esta region
			} else if ( rblobCara_ant.x != NULL && currentBlob.MinX() < rblobCara_ant.x+DIFPIX_REGIONES && currentBlob.MinX() > rblobCara_ant.x-DIFPIX_REGIONES && currentBlob.MinY() < rblobCara_ant.y+DIFPIX_REGIONES && currentBlob.MinY() > rblobCara_ant.y-DIFPIX_REGIONES ) {
				if ( rblob.x != rblobIzq_ant.x && rblob.x != rblobDer_ant.x ) {
					rblobCara_ant = rblob;
					detRegCara = 1;
				}
			}
		}

		// Caso en que aun no se ha detectado la region ìzquierda, donde se encuentra el brazo derecho
		if ( !detRegIzq ) {
			// Caso en el que aun no se ha encontrado nunca la region izquierda o se ha perdido, donde esta el brazo derecho
			if ( marcarRegion ) {
				// Se mira si el blob esta dentro de esta region y se guarda en memoria
				if ( currentBlob.MinX() >= rect_ant.x && currentBlob.MinY() >= rect_ant.y+2*r->height*scale && currentBlob.MinY() < rect_ant.y+rect_ant.height ) {
					// Se comprueba que el blob esta dentro de los limites por la derecha del eje x
					if ( currentBlob.MinX() <= rect_ant.x+rect_ant.width/2 ) {
						detRegIzq = 1;
						rblobIzq_ant = rblob;
					// Si no esta dentro de los limites respecto el eje x por la derecha, es posible que el blob acabe en la parte derecha
					} else if ( currentBlob.MinX() > rect_ant.x+rect_ant.width/2 && currentBlob.MaxX() <= rect_ant.x+rect_ant.width ) {
						// Se mira que no sea el mismo blob que el de la derecha
						if ( rblobDer_ant.x-currentBlob.MinX() > 0 ) {
							detRegDer = 1;
							rblobDer_ant = rblob;
						}
					}
				}
			// Caso en el que ya se ha encontrado una region izquierda, donde esta el brazo derecho (seguimiento)
			// Se comprueba que la distancia entre el blob anterior y el actual son cortas para dar por buena esta region
			} else if ( rblobIzq_ant.x != NULL && currentBlob.MinX() < rblobIzq_ant.x+DIFPIX_REGIONES && currentBlob.MinX() > rblobIzq_ant.x-DIFPIX_REGIONES && currentBlob.MinY() < rblobIzq_ant.y+DIFPIX_REGIONES && currentBlob.MinY() > rblobIzq_ant.y-DIFPIX_REGIONES ) {
				//printf( "Estoy en IZQ\n" );
				// Si la region izquierda es independiente la asignamos
				if ( rblob.x != rblobCara_ant.x && rblob.x != rblobDer_ant.x ) {
					rblobIzq_ant = rblob;
					detRegIzq = 1;
				}
			}
		}
		
		// Caso en que aun no se ha detectado la region derecha, donde se encuentra el brazo izquierdo
		if ( !detRegDer ) {
			// Caso en el que aun no se ha encontrado nunca la region derecha o se ha perdido, donde esta el brazo izquierdo
			if ( marcarRegion ) {
				// Se mira si el blob esta dentro de esta region y se guarda en memoria
				if ( currentBlob.MaxX() <= rect_ant.x+rect_ant.width && currentBlob.MinY() >= rect_ant.y+2*r->height*scale && currentBlob.MinY() < rect_ant.y+rect_ant.height ) {
					// Se comprueba que el blob esta dentro de los limites por la izquierda del eje x
					if ( currentBlob.MinX() >= rect_ant.x+rect_ant.width/2 ) {
						detRegDer = 1;
						rblobDer_ant = rblob;
					// Si no esta dentro de los limites respecto el eje x por la izquierda, es posible que el blob empiece en la parte izquierda
					} else if ( currentBlob.MinX() < rect_ant.x-rect_ant.width/2 && currentBlob.MinX() >= rect_ant.x ) {
						// Se mira si no es el mismo blob que el de la izquierda
						if ( currentBlob.MinX()-rblobIzq_ant.x > 0 ) {
							detRegIzq = 1;
							rblobIzq_ant = rblob;
						}
					}
				}
			// Caso en el que ya se ha encontrado una region derecha, donde esta el brazo izquierdo (seguimiento)
			// Se comprueba que la distancia entre el blob anterior y el actual son cortas para dar por buena esta region
			} else if ( rblobDer_ant.x != NULL && currentBlob.MinX() < rblobDer_ant.x+DIFPIX_REGIONES && currentBlob.MinX() > rblobDer_ant.x-DIFPIX_REGIONES && currentBlob.MinY() < rblobDer_ant.y+DIFPIX_REGIONES && currentBlob.MinY() > rblobDer_ant.y-DIFPIX_REGIONES ) {
				//printf( "Estoy en DER\n" );
				// Si la region derecha es independiente la asignamos
				if ( rblob.x != rblobCara_ant.x && rblob.x != rblobIzq_ant.x ) {
					detRegDer = 1;
					rblobDer_ant = rblob;
				}
			}
		}
	}
	
	blobs.ClearBlobs();
	printf( "%i\t%i\t%i\n", detRegIzq, detRegCara, detRegDer );
	

	// Caso en el que hay que dibujar la region de la izquierda
	if ( detRegIzq ) 
		cvRectangle( frame, cvPoint(rblobIzq_ant.x, rblobIzq_ant.y), cvPoint(rblobIzq_ant.x + rblobIzq_ant.width, rblobIzq_ant.y + rblobIzq_ant.height), CV_RGB(0, 255, 255), 2 );
	// Caso en el que hay que dibujar la region de la cara
	if ( detRegCara ) 
		cvRectangle( frame, cvPoint(rblobCara_ant.x, rblobCara_ant.y), cvPoint(rblobCara_ant.x + rblobCara_ant.width, rblobCara_ant.y + rblobCara_ant.height), CV_RGB(0, 255, 255), 2 );
	// Caso en el que hay que dibujar la region derecha
	if ( detRegDer ) 
		cvRectangle( frame, cvPoint(rblobDer_ant.x, rblobDer_ant.y), cvPoint(rblobDer_ant.x + rblobDer_ant.width, rblobDer_ant.y + rblobDer_ant.height), CV_RGB(0, 255, 255), 2 );

	calcularAgitacion( detRegIzq, detRegDer, detRegCara );
	calcularPosicionIndividuo( frame->width/2, detRegIzq, detRegDer, detRegCara );
	guardarPromedios();

	cvReleaseImage( &erosioned );
	cvReleaseImage( &tmp );

	// Si se han detectado todas las regiones, reiniciaremos el contador de pérdidas de región
	if ( detRegIzq && detRegCara && detRegDer ) {
		countMarcar = 0;
		marcarRegion = 0;
		pocosBlobs = 0;
	// Si hemos perdido algunas de las regiones, retornaremos un 3 que indica volver incrementar el contador de pérdidas de región
	} else if ( !detRegIzq || !detRegCara || !detRegDer )
		return 3;

	return 0;
}

/* Funcion que devuelve una region que abarca los brazos y la cara, a partir del tamano de la cara */
CvRect buscarRegionPrincipal( IplImage *imageGray, double scale ) {
	CvRect rect;
	rect.x = NULL;
	int regionPosible = 1;

	if ( detectado ) {
		if ( !r )
			return rect;
		// Se calcula la nueva region
		rect.width = (CONST_WREGION*r->width)*scale;
		rect.height = (CONST_HREGION*r->height)*scale;
		rect.x = (r->x*scale+(r->width/2)*scale)-rect.width/2;
		rect.y = (r->y - CONST_YREGION*r->height)*scale;
		
		
		// Casos en que el rectangulo de la region no es valido y hay que modificarlo

		if ( rect.x < 0 ) {
			rect.x = (r->x*scale+(r->width/2)*scale)-rect.width/4;
			if ( rect.x < 0 )
				regionPosible = 0;
		}
		if ( rect.y < 0 ) {
			rect.y = (r->y - CONST_YREGION*(r->height/2))*scale;
			if ( rect.y < 0 )
				regionPosible = 0;
		}
		if ( rect.width > imageGray->width ) {
			rect.width = (CONST_WREGION*(r->width/2))*scale;
			if ( rect.width > imageGray->width )
				regionPosible = 0;
		}
		if ( rect.height > imageGray->height ) {
			rect.height = (CONST_HREGION*(r->height/2))*scale;
			if ( rect.height > imageGray->height )
				regionPosible = 0;
		}
		

		//printf(" %i %i %i %i\n", rect.x, rect.y, rect.width, rect.height );

		if ( regionPosible ) {
			// Se asigna un nuevo rectangulo si la region es valida y se guarda
			if ( rect.x > 0 && rect.y > 0 && rect.width <= MAX_WIDTH && rect.height <= MAX_HEIGHT ) {
				return rect;
			} else 
				rect.x = NULL;
		} else
			rect.x = NULL;
	}
	
	return rect;
}

/* Funcion que calcula los promedios de la agitacion de cada region, el general, y la direccion de la agitacion */
int calcularAgitacion( int detRegIzq, int detRegCara, int detRegDer ) {
	CvPoint centIzq, centCara, centDer;

	// Calculo de los promedios de agitacion de cada region

	if ( detRegIzq ) {
		centIzq.x = cvRound(rblobIzq_ant.x + rblobIzq_ant.width*0.5);
		centIzq.y = cvRound(rblobIzq_ant.y + rblobIzq_ant.height*0.5);
		totalIzq++;
		if ( centIzq_ant.x != NULL && totalIzq > 1 ) 
			promAgiIzq = (promAgiIzq*(totalIzq-1)+sqrt((centIzq.x-centIzq_ant.x)*(centIzq.x-centIzq_ant.x)+(centIzq.y-centIzq_ant.y)*(centIzq.y-centIzq_ant.y)))/totalIzq;
	}
	if ( detRegCara ) {
		centCara.x = cvRound(rblobCara_ant.x + rblobCara_ant.width*0.5);
		centCara.y = cvRound(rblobCara_ant.y + rblobCara_ant.height*0.5);
		totalCara++;
		if ( centCara_ant.x != NULL && totalCara > 1 ) 
			promAgiCab = (promAgiCab*(totalCara-1)+sqrt((centCara.x-centCara_ant.x)*(centCara.x-centCara_ant.x)+(centCara.y-centCara_ant.y)*(centCara.y-centCara_ant.y)))/totalCara;
	}
	if ( detRegDer ) {
		centDer.x = cvRound(rblobDer_ant.x + rblobDer_ant.width*0.5);
		centDer.y = cvRound(rblobDer_ant.y + rblobDer_ant.height*0.5);
		totalDer++;
		if ( centDer_ant.x != NULL && totalDer > 1 ) 
			promAgiDer = (promAgiDer*(totalDer-1)+sqrt((centDer.x-centDer_ant.x)*(centDer.x-centDer_ant.x)+(centDer.y-centDer_ant.y)*(centDer.y-centDer_ant.y)))/totalDer;
	}

	// Se comprueba si hay agitacion hacia la derecha o hacia la izquierda, aunque existen muchos otros casos:
	// Combinacion de ambos, hacia adelante, hacia atras, combinacion de estos, etc
	if ( detRegIzq && detRegCara && detRegDer ) {
		if ( centIzq_ant.x != NULL && centCara_ant.x != NULL && centDer_ant.x != NULL ) {
			if ( centIzq.x-centIzq_ant.x > 0 && centCara.x-centCara_ant.x > 0 && centDer.x-centDer_ant.x > 0 ) {
				derecha++;
				totalDirec++;
			} else if ( centIzq.x-centIzq_ant.x < 0 && centCara.x-centCara_ant.x < 0 && centDer.x-centDer_ant.x < 0 ) {
				izquierda++;
				totalDirec++;
			}
		}
	}

	// Se guardan los valores de los centros para su futuro uso
	if ( centIzq.x != NULL && detRegIzq )
		centIzq_ant = centIzq;
	if ( centCara.x != NULL && detRegCara )
		centCara_ant = centCara;
	if ( centDer.x != NULL && detRegDer )
		centDer_ant = centDer;


	return 0;
}

/* Funcion que guarda los promedios en una tabla cada ciertas capturas para su uso a la hora de dibujar el histograma */
int guardarPromedios() {
	count++;		// Varible que contiene el total de veces que se han detectado blobs
	// Se guarda el promedio cuando se llega al final de un intervalo
	promAgiTotal = (promAgiIzq+promAgiCab+promAgiDer)/3;
	prom_agi_frame[framesVideoMostrados] = promAgiTotal;
	if ( captInterv == count && interv < MAX_INTERV ) {
		captInterv += CAPT_POR_INTERVALO;
		prom_arr[interv] = promAgiTotal;
		//printf( "promAgiTotal = %lf == %lf !!!!\npromAgiIzq = %lf\npromAgiCab = %lf\npromAgiDer = %lf\n", promAgiTotal, prom_arr[interv], promAgiIzq, promAgiCab, promAgiDer );
		interv++;
	}
	return 0;
}

/* Funcion que cuenta las veces que un individuo esta situado en alguna parte o de alguna cierta manera */
int calcularPosicionIndividuo( int middle , int detRegIzq, int detRegCara, int detRegDer ) {
	CvPoint centIzq, centCara, centDer;

	centIzq.x = cvRound(rblobIzq_ant.x + rblobIzq_ant.width*0.5);
	centIzq.y = cvRound(rblobIzq_ant.y + rblobIzq_ant.height*0.5);
	centCara.x = cvRound(rblobCara_ant.x + rblobCara_ant.width*0.5);
	centCara.y = cvRound(rblobCara_ant.y + rblobCara_ant.height*0.5);
	centDer.x = cvRound(rblobDer_ant.x + rblobDer_ant.width*0.5);
	centDer.y = cvRound(rblobDer_ant.y + rblobDer_ant.height*0.5);

	// Se comprueba la posicion del individuo respecto la pantalla
	if ( centIzq.x < middle && centDer.x < middle ) {
		captIzq++;
		captBlob++;
	} else if ( centIzq.x > middle && centDer.x > middle ) {
		captDer++;
		captBlob++;
	} else if ( centIzq.x < middle && centDer.x > middle ) {
		captCentro++;
		captBlob++;
	}

	// Se comprueba que se hayan dibujado las 3 regiones 
	if ( detRegIzq && detRegCara && detRegDer ) {	
		if ( sqrt((centIzq.x-centIzq_ant.x)*(centIzq.x-centIzq_ant.x)+(centIzq.y-centIzq_ant.y)*(centIzq.y-centIzq_ant.y)) < PIX_QUIETUD &&
			sqrt((centCara.x-centCara_ant.x)*(centCara.x-centCara_ant.x)+(centCara.y-centCara_ant.y)*(centCara.y-centCara_ant.y)) < PIX_QUIETUD &&
			sqrt((centDer.x-centDer_ant.x)*(centDer.x-centDer_ant.x)+(centDer.y-centDer_ant.y)*(centDer.y-centDer_ant.y)) < PIX_QUIETUD ) {
			captQuieto++;
			captBlob++;
		}
	}

	return 0;
}

CvScalar hsv2rgb( float hue ) {
    int rgb[3], p, sector;
    static const int sector_data[][3]=
        {{0,2,1}, {1,2,0}, {1,0,2}, {2,0,1}, {2,1,0}, {0,1,2}};
    hue *= 0.033333333333333333333333333333333f;
    sector = cvFloor(hue);
    p = cvRound(255*(hue - sector));
    p ^= sector & 1 ? 255 : 0;

    rgb[sector_data[sector][0]] = 255;
    rgb[sector_data[sector][1]] = 0;
    rgb[sector_data[sector][2]] = p;

    return cvScalar(rgb[2], rgb[1], rgb[0],0);
}
