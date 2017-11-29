#include "Kinematics.h"
#include <math.h>

namespace elastic_scattering{
    //This namespace deals with 2->2 elastic scattering
    
    //Returns recoil angle of particle 2 in the lab frame.
    //E2f: Recoil energy of particle 2
    //E1i: Incident energy of particle 1
    //m1,2: masses of particle 1 and 2
    double Theta_from_E2f(double E2f, double E1i, double m1, double m2){
        return acos(sqrt((E2f-m2)/(E2f+m2))*(E1i+m2)/sqrt(E1i*E1i-m1*m1));
    }
    //This is the inverse of Theta_from_E2f.
    double E2f_from_Theta(double theta2, double E1i, double m1, double m2){
        return m2 ((E1i - m1)*(E1i + m1)*pow(cos(theta),2) + pow((E1i + m2),2))/(pow(E1i + m2,2) + (-pow(E1i,2) + pow(m1,2))*pow(cos(theta),2));
    }
    //Maximum and minimum energy of outgoing recoil particle
    //as function of masses and incoming energy E1i
    double E2fMax(double E1i, double m1, double m2){
        return E2f_from_Theta(0.0, E1o, m1, m2);
    }
    double E2fMin(double E1i, double m1, double m2){
        return E2f_from_Theta(M_PI/2.0, E1o, m1, m2);
    }
}

//
double pAbs (double px, double py, double pz, double p0) {
	double rpAbs;
	rpAbs = sqrt(px*px+py*py+pz*pz);
	return(rpAbs);
}
//
double pt (double px, double py, double pz, double p0) {
	double rpt;
	rpt = sqrt(px*px+py*py);
	return(rpt);
}
//
double calculatetheta (double px, double py, double pz, double p0) {
	double rtheta;
	rtheta = acos(pz/pAbs(px,py,pz,p0));
	return(rtheta);
}
//
double phi (double px, double py, double pz, double p0) {
    if(px==0 && py==0){
        return 0;
    }
	if (px > 0 && py > 0) 
	{
		return atan(fabs(py/px));
	}
	else if (px < 0 && py > 0) 
	{
		return M_PI-atan(fabs(py/px));
	}	
	else if (px < 0 && py < 0) 
	{
		return M_PI+atan(fabs(py/px));
	}	
	else
	{
		return 2*M_PI - atan(fabs(py/px));
	}	
}
//
double invariantmass (double px, double py, double pz, double p0) {
	double rinvariantmass;
	rinvariantmass = sqrt(p0*p0-px*px-py*py-pz*pz);
	return(rinvariantmass);
}
//
double lambda (double a, double b, double c) {
	double rlam;
	rlam = a*a+b*b+c*c-2*a*b-2*a*c-2*b*c;
	return(fabs(rlam));
}
//
double TriangleFunc(double m1, double m2, double m3){
    return 1/(2*m1)*sqrt(pow(m1,4)+pow(m2,4)+pow(m3,4)-2*pow(m1*m2,2)-2*pow(m1*m3,2)-2*pow(m2*m3,2));
}

double TriangleFunc2(double m1, double m2, double m3){
	return pow(m1,4)+pow(m2,4)+pow(m3,4)-2*pow(m1*m2,2)-2*pow(m1*m3,2)-2*pow(m2*m3,2);
}

double beta_gamma(double beta){
    return 1.0/sqrt(1-pow(beta,2));
}

double Angle_Spread(double x1, double y1, double z1, double x2, double y2, double z2){
    return acos((x1*x2+y1*y2+z1*z2)/sqrt(x1*x1+y1*y1+z1*z1)/sqrt(x2*x2+y2*y2+z2*z2));
}

//Kinematics for DM+X->DM+X scattering, with X at rest.

// X Angle as a function of X final energy, DM initial energy, and masses.
//double ThetaEe (double Ee, double EDM, double MDM, MX) {
//	return(acos(sqrt((Ee-MX)/(Ee+MX))*(EDM+MX)/sqrt(EDM*EDM-MDM*MDM)));
//}
