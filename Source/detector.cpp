#include "detector.h"
#include <cmath>
#include <vector>

using std::vector;

//inner product for 3-vectors
double ip(double vec1[],double vec2[]){
    return vec1[0]*vec2[0]+vec1[1]*vec2[1]+vec1[2]*vec2[2];
}

void detector::add_material(double nd, double np, double nn, double ne, double m, std::string matname){
    p_num_tot+=np*nd;
    n_num_tot+=nn*nd;
    e_num_tot+=ne*nd;
    matvec.push_back(Material(nd, np, nn, ne, m, matname));
}


/*********************
 * detector_sphere
 * ******************/

detector_sphere::detector_sphere (double x, double y, double z, double radius){
    r[0] = x; r[1] = y; r[2] = z; Rdet = radius;
}

// distance DM travels through detector
double detector_sphere::Ldet (const Particle &DM) {
	double Ldetenter, Ldetexit;
    double A, B, C;
   
    b[0]=DM.px;b[1]=DM.py;b[2]=DM.pz;

    A = ip(b,b);
    B = -2*ip(r,b);
    C = ip(r,r)-pow(Rdet,2);

    if((pow(B,2)-4*A*C)<0||A==0){
        return 0.0;
    }

    Ldetenter = (-B-sqrt(pow(B,2)-4*A*C))/(2*A); 
	Ldetexit = (-B+sqrt(pow(B,2)-4*A*C))/(2*A);

    if(Ldetenter<Ldetexit){
        cross_point[0] = Ldetenter;
        cross_point[1] = Ldetexit;
        return (Ldetexit-Ldetenter)*sqrt(A);
    }
    else
        return 0;
}

/*******************
 *detector_cylinder*
 *******************/


//May want to add an extra constructure that instead asks for the coordinates of the two faces.
detector_cylinder::detector_cylinder (double x, double y, double z, double detlength, double radius, double detTheta, double detPhi){
    //r points to the center of the detector
    r[0]=x;
    r[1]=y;
    r[2]=z;
    Rdet=radius;
    Ldetector=detlength;

    //l points from the center of the detector to the center of the one of the circular detector faces.
    l[0] = detlength*cos(detPhi)*sin(detTheta)/2;
    l[1] = detlength*sin(detPhi)*sin(detTheta)/2;
    l[2] = detlength*cos(detTheta)/2;
    
}
//This is a temporary setup.
double detector_cylinder::Ldeto (const Particle &DM, const double offset[3]){
    b[0]=DM.px;b[1]=DM.py;b[2]=DM.pz;
    double ro[3];
    for(int i=0; i<3; i++){
        ro[i] = r[i] - offset[i];
    } 
    vector<double> crossings;
    double B1,B2;
    

    //Checking crossing point of circular faces.
    if(ip(b,l)!=0){//if b.l==0, beam is parallel to the circular faces.
        

        B1 = (ip(ro,l)+ip(l,l))/ip(b,l);
        double Rhold[3];//Need to hold this value for use in calculation.
        if(B1>0){
            for(int iter=0; iter <3; iter++){
                Rhold[iter] = B1*b[iter]-l[iter]-ro[iter];
            }
            if(sqrt(ip(Rhold,Rhold))<=Rdet){
                crossings.push_back(B1);
            }
        }
            

        B2 = (ip(ro,l)-ip(l,l))/ip(b,l);//For B2 l->-l.
        if(B2>0){ 
            
            for(int iter=0; iter <3; iter++){
                Rhold[iter] = B2*b[iter]+l[iter]-ro[iter];
            }
            
            if(sqrt(ip(Rhold,Rhold))<=Rdet){
                crossings.push_back(B2);
            }
        }
    }
    

    double B3, B4, A3, A4;

    if(crossings.size()<2){
        double X = -pow(ip(b,l),2) + ip(b,b)*ip(l,l);
        double Y = -2*ip(ro,b)*ip(l,l)+2*ip(b,l)*ip(ro,l);
        double Z = -pow(ip(ro,l),2)-pow(Rdet,2)*ip(l,l)+ip(ro,ro)*ip(l,l);
        double sqr_arg=Y*Y-4*X*Z; //argument in the quadratic equation square root.
        if(sqr_arg > 0){
            B3 = (-Y+sqrt(sqr_arg))/(2*X);
            B4 = (-Y-sqrt(sqr_arg))/(2*X);

            
            if(B3 > 0 && (A3 = (B3*ip(b,l)-ip(ro,l))/ip(l,l) ) > -1 && A3<1){
                crossings.push_back(B3);
            }
             
            if(B4 > 0 && (A4 = (B4*ip(b,l)-ip(ro,l))/ip(l,l) )>-1 && A4<1){
                crossings.push_back(B4);
            }
        }
    }

    
    if(crossings.size()==0)
        return 0.0;
    else if(crossings.size()==2){
        cross_point[0]=crossings[0];
        cross_point[1]=crossings[1];
        return (crossings[1]>crossings[0] ? (crossings[1]-crossings[0])*sqrt(ip(b,b)) :\
                (crossings[0]-crossings[1])*sqrt(ip(b,b)));
    }
    else if(crossings.size()==1)
        //return crossings[0]*sqrt(ip(b,b));
        //needs to be written better for inside of detector case.
        return 0.0;
    else
        throw crossings.size();
}

double detector_cylinder::Ldet (const Particle &DM){
    b[0]=DM.px;b[1]=DM.py;b[2]=DM.pz;
    
    vector<double> crossings;
    double B1,B2;
    

    //Checking crossing point of circular faces.
    if(ip(b,l)!=0){//if b.l==0, beam is parallel to the circular faces.
        

        B1 = (ip(r,l)+ip(l,l))/ip(b,l);
        double Rhold[3];//Need to hold this value for use in calculation.
        if(B1>0){
            for(int iter=0; iter <3; iter++){
                Rhold[iter] = B1*b[iter]-l[iter]-r[iter];
            }
            if(sqrt(ip(Rhold,Rhold))<=Rdet){
                crossings.push_back(B1);
            }
        }
            

        B2 = (ip(r,l)-ip(l,l))/ip(b,l);//For B2 l->-l.
        if(B2>0){ 
            
            for(int iter=0; iter <3; iter++){
                Rhold[iter] = B2*b[iter]+l[iter]-r[iter];
            }
            
            if(sqrt(ip(Rhold,Rhold))<=Rdet){
                crossings.push_back(B2);
            }
        }
    }
    

    double B3, B4, A3, A4;

    if(crossings.size()<2){
        double X = -pow(ip(b,l),2) + ip(b,b)*ip(l,l);
        double Y = -2*ip(r,b)*ip(l,l)+2*ip(b,l)*ip(r,l);
        double Z = -pow(ip(r,l),2)-pow(Rdet,2)*ip(l,l)+ip(r,r)*ip(l,l);
        double sqr_arg=Y*Y-4*X*Z; //argument in the quadratic equation square root.
        if(sqr_arg > 0){
            B3 = (-Y+sqrt(sqr_arg))/(2*X);
            B4 = (-Y-sqrt(sqr_arg))/(2*X);

            
            if(B3 > 0 && (A3 = (B3*ip(b,l)-ip(r,l))/ip(l,l) ) > -1 && A3<1){
                crossings.push_back(B3);
            }
             
            if(B4 > 0 && (A4 = (B4*ip(b,l)-ip(r,l))/ip(l,l) )>-1 && A4<1){
                crossings.push_back(B4);
            }
        }
    }

    
    if(crossings.size()==0)
        return 0.0;
    else if(crossings.size()==2){
        cross_point[0]=crossings[0];
        cross_point[1]=crossings[1];
        return (crossings[1]>crossings[0] ? (crossings[1]-crossings[0])*sqrt(ip(b,b)) :\
                (crossings[0]-crossings[1])*sqrt(ip(b,b)));
    }
    else if(crossings.size()==1)
        //return crossings[0]*sqrt(ip(b,b));
        //needs to be written better for inside of detector case.
        return 0.0;
    else
        throw crossings.size();
}
