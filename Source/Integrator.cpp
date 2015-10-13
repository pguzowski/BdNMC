#include "Integrator.h"
#include <cmath>
#include <stdexcept>
#include <iostream>

using std::cout; using std::endl;

const double pi = 3.14159265359;
const int DDMAX = 30;

using std::vector;

double ddweight(double k, double h){
    return 1.0/2.0*h*pi*cosh(k*h)/pow(cosh(1.0/2.0*pi*sinh(k*h)),2);
}

long double ddabscissa(int k, double h){
    return tanh(1.0/2.0*pi*sinh(h*k));
}
//rescales an abscissa on integration region [min,max] to [-1,1]
long double res_int(double x, double min, double max){
    return 1.0/2.0*(min+max-min*x+max*x);
}

double func_check(std::function<double(double)> f, double abscissa, double min, double max){
	double fp,fm;
	fp=f(res_int(abscissa,min,max));
	if(isnan(fp))
		fp=0;
	fm=f(res_int(-abscissa,min,max));
	if(isnan(fm))
		fm=0;
	return fm+fp;
}

double GridTrapezoidal(std::function<double(double, double)> f, double xmin, double xmax, double xmin,double xN, double ymin, double ymax, double yN){

}

//h is the width between abscissa.
double DoubleExponential(std::function<double(double)> f, double min, double max, int N, double h){
    double scale = 2.0/(max - min);
    if(min>=max)
        return 0;
    double sum = 0; double abscissa = 0;
    //cout << "scale=" << scale << endl;
	for(int k=0; k<=N; k++){
        if((abscissa=ddabscissa(k,h))>=1)
            break;
		//cout << " k=" << k << " ddweight=" << ddweight(k,h) << " f+=" << f(res_int(abscissa,min,max)) << " res_int+=" << res_int(+abscissa,min,max) << " f-=" << f(res_int(-abscissa,min,max)) <<  " res_int-=" << res_int(-abscissa,min,max) << endl;
        //sum+=ddweight(k,h)*(f(res_int(abscissa,min,max))+f(res_int(-abscissa,min,max)));
        sum+=ddweight(k,h)*func_check(f,abscissa,min,max);
    }
    return sum/scale;
}

//Double exponential routine used for extending range in n.
double DoubleExponential_Nout(std::function<double(double)> f, double min, double max, int N, int Nmin, double h){
    double scale = 2.0/(max - min);
    if(min>=max)
        return 0;
    double sum = 0; double abscissa = 0;
    for(int k=Nmin+1; k<=N; k++){
        if((abscissa=ddabscissa(k,h))>=1)
            break;
        //sum+=ddweight(k,h)*(f(res_int(abscissa,min,max))+f(res_int(-abscissa,min,max)));
        sum+=ddweight(k,h)*func_check(f,abscissa,min,max);
    }
    return sum/scale;
}

//Only sums over odd k. Used to increase resolution.
double DoubleExponential_hdub(std::function<double(double)> f, double min, double max, int N, double h){
    double scale = 2.0/(max - min);
    if(min>=max)
        return 0;
    double sum = 0; double abscissa = 0;
    for(int k=1; k<=N-1; k+=2){
        if((abscissa=ddabscissa(k,h))>=1)
            break;
        //sum+=ddweight(k,h)*(f(res_int(abscissa,min,max))+f(res_int(-abscissa,min,max)));
        sum+=ddweight(k,h)*func_check(f,abscissa,min,max);
    }
    return sum/scale;
}
//slow and inefficient. Still good enough. Could be improved by adding a means of subdividing the integration region.
double DoubleExponential_adapt(std::function<double(double)> f, double min, double max, int N, double h, double precision){
    //cout << "initial estimate" << endl;
	double dd0 = DoubleExponential(f, min, max, N, h);
    double dd1,dd2;
    int attempts;
	if(min>=max)
		return 0;
    //cout << "N=" << N << " h=" << h << " dd0=" << dd0 << endl;
    for(attempts=0; attempts<DDMAX; attempts++){ 
        if(fabs(dd0-(dd1 = dd0+DoubleExponential_Nout(f, min, max, N*2.0, N, h)))>dd0*precision){
            dd0=dd1;
            N*=2.0;
            //cout << "N=" << N << " h=" << h << " dd1=" << dd0 << endl;
        }
        else if(fabs(dd0-(dd2 = 0.5*dd0+DoubleExponential_hdub(f, min, max, 2*N, h/2.0)))>dd0*precision){
            dd0=dd2;
            h*=0.5; N*=2.0;
            //cout << "N=" << N << " h=" << h << " dd1=" << dd0 << endl;
        }
        else
            break;
    }
    if(attempts==DDMAX){
        std::cerr << "Integrator reached max number of iterations=" << DDMAX << endl;	
	}
    return dd0;
}

//Not good enough anymore
double SimpsonsRule(std::function<double(double)> f, double min, double max, int steps){
    if(min>=max)
        return 0.0;
    if(steps%2!=0){
        std::cerr << "Invalid Argument: steps must be a multiple of 2\n";
    }
    double dx = (double)(max-min)/steps;
    double sum1 = 0;
    double sum2 = 0;

    for(int i=1;i<=steps;i+=2){
        sum1 += 4.0 * f(min+i*dx);
    }
    for(int i=2;i<=steps-1;i+=2){
        sum2 += 2.0 * f(min+i*dx);
    }
    return (sum1+sum2+f(min)+f(max)) * dx/3.0;
}

/*
 *  Linear_Interpolation takes a set of data Yvals whose elements map to xvals between Xmin
 *  and Xmax such that Yvals[0] = f(Xmin), Yvals[i] = f(Xmin + i*xres), where xres is defined
 *  in the function body and f(x) is some function approximating the data set Yvals.
 */
Linear_Interpolation::Linear_Interpolation(vector<double> Yvals, double Xmin, double Xmax){
    yvals.resize(Yvals.size());
    std::copy(Yvals.begin(),Yvals.end(),yvals.begin());
    xmin = Xmin;
    xres = (Xmax-Xmin)/(Yvals.size()-1);
    yvals.push_back(0);
}

/*
 * Interpolate returns the value of f(xval) for xmin<=xval<=xmax. Values outside this range will
 * likely lead to a segmenation fault. Should perhaps add error handling, not certain of the 
 * cost at run time to check the domain at every evaluation.
 */
double Linear_Interpolation::Interpolate(double xval){
    double index = (xval-xmin)/xres;
    int lowindex = (int)floor(index);
    return (lowindex+1-index)*yvals[lowindex]+(index-lowindex)*yvals[lowindex+1];
}
