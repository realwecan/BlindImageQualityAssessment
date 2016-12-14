#include "brisque.h"


//function definitions
void ComputeBrisqueFeature(IplImage *orig, vector<double>& featurevector)
{
    IplImage *orig_bw_int = cvCreateImage(cvGetSize(orig), orig->depth, 1); 
    cvCvtColor(orig, orig_bw_int, CV_RGB2GRAY);
    IplImage *orig_bw = cvCreateImage(cvGetSize(orig_bw_int), IPL_DEPTH_64F, 1);
    cvConvertScale(orig_bw_int, orig_bw, 1.0/255);
    cvReleaseImage(&orig_bw_int);
    
    //orig_bw now contains the grayscale image normalized to the range 0,1
    
    int scalenum = 2;
    for (int itr_scale = 1; itr_scale<=scalenum; itr_scale++)
	{
		IplImage *imdist_scaled = cvCreateImage(cvSize(orig_bw->width/pow((double)2,itr_scale-1), orig_bw->height/pow((double)2,itr_scale-1)), IPL_DEPTH_64F, 1);
		cvResize(orig_bw, imdist_scaled,CV_INTER_CUBIC); 
		
		//compute mu and mu squared
		IplImage* mu = cvCreateImage(cvGetSize(imdist_scaled), IPL_DEPTH_64F, 1);
		cvSmooth( imdist_scaled, mu, CV_GAUSSIAN, 7, 7, 1.16666 );
		IplImage* mu_sq = cvCreateImage(cvGetSize(imdist_scaled), IPL_DEPTH_64F, 1);
		cvMul(mu, mu, mu_sq);

		//compute sigma
		IplImage* sigma = cvCreateImage(cvGetSize(imdist_scaled), IPL_DEPTH_64F, 1);
		cvMul(imdist_scaled, imdist_scaled, sigma);
		cvSmooth(sigma, sigma, CV_GAUSSIAN, 7, 7, 1.16666 );
		cvSub(sigma, mu_sq, sigma);
		cvPow(sigma, sigma, 0.5);

		//compute structdis = (x-mu)/sigma
		cvAddS(sigma, cvScalar(1.0/255), sigma);
		IplImage* structdis = cvCreateImage(cvGetSize(imdist_scaled), IPL_DEPTH_64F, 1);
		cvSub(imdist_scaled, mu, structdis);
		cvDiv(structdis, sigma, structdis);

		//Compute AGGD fit
                double lsigma_best, rsigma_best, gamma_best;
                AGGDfit(structdis, lsigma_best, rsigma_best, gamma_best);
		featurevector.push_back(gamma_best);
		featurevector.push_back((lsigma_best*lsigma_best + rsigma_best*rsigma_best)/2);
		
		//Compute paired product images
		int shifts[4][2]={{0,1},{1,0},{1,1},{-1,1}};
		for(int itr_shift=1; itr_shift<=4; itr_shift++)
		{
			int* reqshift = shifts[itr_shift-1];

                        IplImage* shifted_structdis = cvCreateImage(cvGetSize(imdist_scaled), IPL_DEPTH_64F, 1);
			BwImage OrigArr(structdis);
			BwImage ShiftArr(shifted_structdis);
			for(int i=0; i<structdis->height; i++)
			{
				for(int j=0; j<structdis->width; j++)
				{
					if(i+reqshift[0]>=0 && i+reqshift[0]<structdis->height && j+reqshift[1]>=0 && j+reqshift[1]<structdis->width)
					{
						ShiftArr[i][j]=OrigArr[i+reqshift[0]][j+reqshift[1]];
					}
					else
					{
						ShiftArr[i][j]=0;
					}
				}
			}
		
			//computing correlation
			cvMul(structdis, shifted_structdis, shifted_structdis);
			AGGDfit(shifted_structdis, lsigma_best, rsigma_best, gamma_best);
		
			double constant = sqrt(tgamma(1/gamma_best))/sqrt(tgamma(3/gamma_best));
			double meanparam = (rsigma_best-lsigma_best)*(tgamma(2/gamma_best)/tgamma(1/gamma_best))*constant;
			
			featurevector.push_back(gamma_best);
			featurevector.push_back(meanparam);
			featurevector.push_back(pow(lsigma_best,2));
			featurevector.push_back(pow(rsigma_best,2));

                        cvReleaseImage(&shifted_structdis);	
		}

                cvReleaseImage(&mu);
		cvReleaseImage(&mu_sq);
		cvReleaseImage(&sigma);
		cvReleaseImage(&structdis);
		cvReleaseImage(&imdist_scaled);
	}
	
}
    
//function definitions
void AGGDfit(IplImage* structdis, double& lsigma_best, double& rsigma_best, double& gamma_best)
{
	BwImage ImArr(structdis);
	
	//int width = structdis->width;
	//int height = structdis->height;
	long int poscount=0, negcount=0;
	double possqsum=0, negsqsum=0, abssum=0;
	for(int i=0;i<structdis->height;i++)
	{
		for (int j =0; j<structdis->width; j++)
		{
			double pt = ImArr[i][j];
			if(pt>0)
			{
				poscount++;
				possqsum += pt*pt; 
				abssum += pt;
			}
			else if(pt<0)
			{
				negcount++;
				negsqsum += pt*pt;
				abssum -= pt;
			}
		}
	}
	lsigma_best = pow(negsqsum/negcount, 0.5); //1st output parameter set
	rsigma_best = pow(possqsum/poscount, 0.5); //2nd output parameter set
	 
	double gammahat = lsigma_best/rsigma_best;
	long int totalcount = structdis->width*structdis->height;
	double rhat = pow(abssum/totalcount, static_cast<double>(2))/((negsqsum + possqsum)/totalcount);
	double rhatnorm = rhat*(pow(gammahat,3) +1)*(gammahat+1)/pow(pow(gammahat,2)+1,2);
	
	double prevgamma = 0;
	double prevdiff = 1e10;	
        float sampling = 0.001;
	for (float gam=0.2; gam<10; gam+=sampling) //possible to coarsen sampling to quicken the code, with some loss of accuracy
	{
		double r_gam = tgamma(2/gam)*tgamma(2/gam)/(tgamma(1/gam)*tgamma(3/gam));
		double diff = abs(r_gam-rhatnorm);
		if(diff> prevdiff) break;
		prevdiff = diff;
		prevgamma = gam;
	}
	gamma_best = prevgamma;
}

