#include <sstream>
#include <iostream>
#include <string>
#include <iostream>
#include "opencv\highgui.h"
#include "opencv\cv.h"

using namespace std;
using namespace cv;

//Parametros
//default capture width and height
const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;
//max number of objects to be detected in frame
const int MAX_NUM_OBJECTS = 50;
//minimum and maximum object area
const int MIN_OBJECT_AREA = 20*20;
const int MAX_OBJECT_AREA = FRAME_HEIGHT*FRAME_WIDTH / 1.5;
//Contadores
int l,x,y,i,j;
int numObjects;
float mayor=0.0,menor=0.0;
///MEDIDA DE CALIBRACION, NUESTRA MONEDA DE CALIBRACION SERA LA MONEDA DE 10 CTVS, ESTA SIEMPRE ESTARA EN LA ESQUINA
/// INFERNIOR IZQUIERDA
float medida_calibracion=17.91; //medida en milimetros de la moneda de 10 ctvs
float factor_conversion=0.0;
//Variables de tratamiento de imagen
VideoCapture cam(1);
Mat src,gray,bluri,th,deli,eri,adap,adapg,mp,final,er,dil;

int main(int argc, char *argv[]){
    int ma=9,mb=3,ga=13,gb=5;
    Mat kernelc=getStructuringElement(MORPH_ELLIPSE,Size(5,5),Point(-1,-1));//Kernel para operaciones morphologicas close y open
    Mat kernel=getStructuringElement(MORPH_ELLIPSE,Size(3,3));//Kernel para operaciones Erode y dilate

    while(1){
        //Carga del video
        cam>>src;
        if(src.empty()){
            cout<<"Error capturando imagen, seriorese del puerto de la camara en la llamada VIDEOCAPTURE"<<endl;
            return -1;
        }
        cvtColor(src, gray, CV_BGR2GRAY);//Conversion a escala de grises
        GaussianBlur(gray, bluri, Size(7,7),2,2,2);
        //Treshold adapatativo para facilitar la deteccion de bordes
        adaptiveThreshold(bluri,adap,255,ADAPTIVE_THRESH_MEAN_C,THRESH_BINARY_INV,ma,mb);
        adaptiveThreshold(bluri,adapg,255,ADAPTIVE_THRESH_GAUSSIAN_C,THRESH_BINARY_INV,ga,gb);
        final=(adapg+adap);
        //Operaciones morfologicas para mejorar la imagen
        morphologyEx( final, mp, MORPH_OPEN, kernelc,Point(-1,-1),2);
        morphologyEx( final, mp, MORPH_CLOSE, kernelc,Point(-1,-1),2);
        erode(mp,er,kernel,Point(-1,-1),2);
        dilate(er,dil,kernel,Point(-1,-1),3);
        erode(dil,dil,kernel,Point(-1,-1),1);

        //Mostrar imagenes, algunas estan desactivadas
        imshow("Original", src);
        //imshow("Gray", gray);
        //imshow("Bluri", bluri);
        //imshow("Adaptativo mean", adap);
        //imshow("Adaptativo gauss",adapg);
        //imshow("Combinado",final);
        //imshow("Closed",mp);
        //imshow("Erode ",er);
        imshow("Dilate ",dil);

        //DETECCION DE BORDES
        Mat canny_output;
        vector<vector<Point> > contours;
        vector<Vec4i> hierarchy;
        // Detectar los bordes con un umbral min = 100 y max = 200
        Canny(dil, canny_output, 150, 255);
        // Mostrar los bordes detectados con Canny
        imshow("Bordes", canny_output);
        // Buscar los contornos de la imagen, se almacenan en contours
        findContours(canny_output, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
        //Ordenar contornos
        sort(contours.begin(),
             contours.end(),
             []( const vector<Point>& a, const vector<Point> & b ){
                    Rect ra(boundingRect(a));
                    Rect rb(boundingRect(b));
                    return ((ra.x > rb.x)&&(ra.y < rb.y));//Ordena tanto en X como en Y
                });

        //Conteo del dinero
        float conteo_dinero=0;
        //Inicio del tratamiento de contornos, todo el programa se basa en el contorno
        for (i=0;i<contours.size();i++){
            //Calculo del Area
            Moments moment = moments((cv::Mat)contours[i]);
            double area = moment.m00;
            if(area>800){
                x = moment.m10 / area;
                y = moment.m01 / area;
                //DIBUJAMOS LOS CONTORNOS
                //Scalar color = CV_RGB(0, 0, 255);
                //drawContours(src, contours, (int)k, color, 2, 8, hierarchy, 0, Point());
                //DIBUJAMOS LOS CONTORNOS
                float r=sqrt(area/3.1416);
                circle(src, Point(x, y), r, Scalar(0,255,0),3);
                //DIBUJAMOS EL CENTRO
                circle(src, Point(x, y), 5, Scalar(0,0,255),-1);
                //Calculo de dimensiones
                if(i==0)
                    factor_conversion=medida_calibracion/r;
                //Calculo de dimensiones
                float dim=r*factor_conversion;
                //Moneda de 1ctv
                if(dim>18.5&&dim<20){
                    conteo_dinero=conteo_dinero+.01;
                    putText(src,"0.01 USD", Point(x,y+r/6), FONT_HERSHEY_COMPLEX_SMALL,0.55, Scalar(255, 255, 255), 1, CV_AA);
                }
                //Moneda de 5 ctvs
                if(dim>20.5&&dim<22){
                    conteo_dinero=conteo_dinero+.05;
                    putText(src,"0.05 USD", Point(x,y+r/6), FONT_HERSHEY_COMPLEX_SMALL,0.55, Scalar(255, 255, 255), 1, CV_AA);
                }
                //Moneda de 10 ctvs
                if(dim>16&&dim<18.5){
                    conteo_dinero=conteo_dinero+.1;
                    putText(src,"0.10 USD", Point(x,y+r/6), FONT_HERSHEY_COMPLEX_SMALL,0.55, Scalar(255, 255, 255), 1, CV_AA);
                }
                //Moneda de 25 ctvs
                if(dim>24&&dim<25){
                    conteo_dinero=conteo_dinero+.25;
                    putText(src,"0.25 USD", Point(x,y+r/6), FONT_HERSHEY_COMPLEX_SMALL,0.55, Scalar(255, 255, 255), 1, CV_AA);
                }
                //Moneda de 1 dolar
                if(dim>26&&dim<27){
                    conteo_dinero=conteo_dinero+1;
                    putText(src,"1 USD", Point(x,y+r/6), FONT_HERSHEY_COMPLEX_SMALL,0.55, Scalar(255, 255, 255), 1, CV_AA);
                }
                //Moneda de 50 ctvs
                if(dim>30){
                    conteo_dinero=conteo_dinero+.5;
                    putText(src,"0.50 USD", Point(x,y+r/6), FONT_HERSHEY_COMPLEX_SMALL,0.55, Scalar(255, 255, 255), 1, CV_AA);
                }

                //Muestra de datos
                if(i!=0)
                    putText(src,"Moneda: "+to_string(i+1), Point(x,y-r), FONT_HERSHEY_COMPLEX_SMALL,0.55, Scalar(255, 255, 255), 1, CV_AA);
                if(i==0){
                    putText(src,"Moneda: "+to_string(i+1), Point(x,y-r), FONT_HERSHEY_COMPLEX_SMALL,0.55, Scalar(255, 255, 255), 1, CV_AA);
                    putText(src,"Moneda BASE", Point(x,y-r/2), FONT_HERSHEY_COMPLEX_SMALL,0.55, Scalar(0, 0, 255), 1, CV_AA);
                }
                //putText(src,"Dim: "+to_string(dim), Point(x,y+r/4), FONT_HERSHEY_COMPLEX_SMALL,0.55, Scalar(255, 255, 255), 1, CV_AA);
            }
        }
        // Mostrar la imagen final
        putText(src,"CONTADOR DE MONEDAS", Point(0, 15),FONT_HERSHEY_COMPLEX_SMALL,0.75, Scalar(255, 0, 0), 1, CV_AA);
        //Conteo de monedas
        putText(src,"Hay:" + to_string(contours.size())+" monedas", Point(0, 30),FONT_HERSHEY_COMPLEX_SMALL,0.75, Scalar(255, 0, 0), 1, CV_AA);
        //Impresion del total del dinero
        putText(src,"Total:" + to_string(conteo_dinero), Point(0,50), FONT_HERSHEY_COMPLEX_SMALL,0.75, Scalar(255, 0, 0), 1, CV_AA);
        imshow("Contours", src);
        int c = waitKey(1);
        if((char)c ==27)
        break;
    }
    return 0;
}
