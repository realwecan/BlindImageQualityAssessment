#ifndef JD_BRISQUE
#define JD_BRISQUE

#include<stdio.h>
#include<iostream>
#include <cstdlib>
#include <math.h>
#include "opencv/cv.h"
#include "opencv/cxcore.h"
#include "opencv/highgui.h"
#include <vector>
#include <string.h>
using namespace std;
#define M_PI 3.14159265358979323846

int find_option( int argc, char **argv, const char *option );
int read_int( int argc, char **argv, const char *option, int default_value );
char *read_string( int argc, char **argv, const char *option, char *default_value);
extern float rescale_vector[36][2];


template<class T> class Image
{
  private:

  IplImage* imgp;


  public:

  Image(IplImage* img=0)
  {
   imgp=img;
  }
  ~Image()
  {
   imgp=0;
  }
  void operator=(IplImage* img) 
  {
    imgp=img;
  }
  inline T* operator[](const int rowIndx)
  {
     return ((T *)(imgp->imageData + rowIndx*imgp->widthStep));
  }
};

typedef Image<double> BwImage;

//function declarations
void AGGDfit(IplImage* structdis, double& lsigma_best, double& rsigma_best, double& gamma_best);
void ComputeBrisqueFeature(IplImage *orig, vector<double>& featurevector);
void trainModel();
float computescore(string prefix, char* imname);

template <typename Type>
void  printVector(vector<Type> vec)
{
    for(int i=0; i<vec.size(); i++)
    {
        cout<<i+1<<":"<<vec[i]<<endl;
    }
}

template <typename Type>
void printVectortoFile(char*filename , vector<Type> vec,float score)
{
  FILE* fid = fopen(filename,"a");
  //cout<<"file opened"<<endl;
  fprintf(fid,"%f ",score);
  for(int itr_param = 0; itr_param < vec.size();itr_param++)
    fprintf(fid,"%d:%f ",itr_param+1,vec[itr_param]);
  fprintf(fid,"\n");
  fclose(fid);
}


#endif
