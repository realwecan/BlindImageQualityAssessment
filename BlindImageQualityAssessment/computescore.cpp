#include "brisque.h"
#include "libsvm/svm.h"
    
/*
float computescore(char* imagename){
    
    
    float qualityscore;

    char* filename = "test.txt";
    FILE* fid = fopen(filename,"w");
    fclose(fid);
 
         
    IplImage* orig = cvLoadImage(imagename);
    vector<double> brisqueFeatures;
    ComputeBrisqueFeature(orig, brisqueFeatures);
    printVectortoFile(filename,brisqueFeatures,0);
      
    remove("output.txt");
    remove("test_ind_scaled");
    system("libsvm/svm-scale -r allrange test.txt >> test_ind_scaled");
    system("libsvm/svm-predict -b 1 test_ind_scaled allmodel output.txt >>dump");

    fid = fopen("output.txt","r");
    fscanf(fid,"%f",&qualityscore);
    fclose(fid);
    remove("test.txt");
    remove("dump");
    remove("output.txt");
    remove("test_ind_scaled");
             
    return qualityscore;
}
*/

float computescore(string prefix, char* imagename) {
	double qualityscore;
	int i;
    struct svm_model* model;
	IplImage* orig = cvLoadImage(imagename);
    vector<double> brisqueFeatures;
    ComputeBrisqueFeature(orig, brisqueFeatures);

	//use the allmodel file
	string modelfile = prefix + "allmodel";
	if((model=svm_load_model(modelfile.c_str()))==0) {
		fprintf(stderr,"can't open model file allmodel\n");
		exit(1);
	}

	struct svm_node x[37];
	//rescale the brisqueFeatures vector from -1 to 1
	for(i = 0; i < 36; ++i) {
		float min = rescale_vector[i][0];
		float max = rescale_vector[i][1];
		x[i].value = -1 + (2.0/(max - min) * (brisqueFeatures[i] - min));
		x[i].index = i + 1;
	}
	x[36].index = -1;
	
	int nr_class=svm_get_nr_class(model);
	double *prob_estimates = (double *) malloc(nr_class*sizeof(double));

	qualityscore = svm_predict_probability(model,x,prob_estimates);

	free(prob_estimates);
	svm_free_and_destroy_model(&model);
	cvReleaseImage(&orig);

	return qualityscore;
}
