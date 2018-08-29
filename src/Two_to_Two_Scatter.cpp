#include "Scatter.h"
#include "Kinematics.h"
#include "constants.h"

using std::string; using std::vector;
using std::cout; using std::endl;
using std::function; using std::string;

using namespace two_to_two_scattering;
using namespace std::placeholders;

Two_to_Two_Scatter::Two_to_Two_Scatter(){
    chan_number = 0;
    pMax=0;
}

//end_state is the default particle to detect, need to add handling for detecting both at some point. out_state defaults to being invisible. 
void Two_to_Two_Scatter::add_channel(Particle& out_state, Particle& end_state, double targ_mass, function<double(double)>& cross_total, function<double(double)>& cross_max, function<double(double,double)>& dsig, double num_density, const string in_state){
    
    cross_tot.push_back(cross_total);
    cross_maxima.push_back(cross_max);
    dsigma.push_back(dsig);
    number_density.push_back(num_density);
    recoil_particle.push_back(end_state);
    out_particle.push_back(out_state);
    target_mass.push_back(targ_mass);
    in_name.push_back(in_state);

    chan_number++;
}

//This creates Particle objects for the recoiling particles.
bool Two_to_Two_Scatter::probscatter(std::shared_ptr<detector>& det, std::list<Particle>& partlist, std::list<Particle>::iterator& partit){
    //This one is the more important recoil particle!
    Particle recoil(0);
    //This one tends to be invisible!
    Particle recoil2(0);
    //Can probably add more cuts later!
    if(probscatter(det, *partit, recoil, recoil2)&&(min_angle<=0||recoil.Theta()>min_angle)&&(max_angle>2*pi||recoil.Theta()<max_angle)){
        partit->Generate_Position();
        Link_Particles(*partit,recoil);
        partlist.insert(std::next(partit),recoil);
        //recoil2 handled by probscatter, which passes it off to scatterevent!
        Link_Particles(*partit, recoil2);
        partlist.insert(std::next(partit),recoil2);
        return true;
    }
    return false;
}

//This samples the scattering cross-section, useful for burn-in, where
//we do not care what the final state particles are.
bool Two_to_Two_Scatter::probscatter(std::shared_ptr<detector>& det, Particle& part){
 //   cout << "Hi there!" << endl;

    double LXDet = m_to_cm*(det->Ldet(part));
    double total=0; vector<double> prob;
//    part.report(cout);
//    cout << "LXDet = " << LXDet << endl;
    for(int i = 0; i < chan_number; i++){
        //Check if this particle is a valid target for this channel!
    //    cout << "i = " << i << " in_name[i] = " << in_name[i] << endl;
        if(part.name==in_name[i]){
/*
            cout << "pushing back probability\n";
            cout << number_density[i] << endl;
            cout << part.E << endl;
            cout << cross_tot[i](part.E) << endl;
*/
            prob.push_back(LXDet*convGeV2cm2*(cross_tot[i](part.E))*number_density[i]);
        }//If not a valid target, the cross section is zero.
        else{
            prob.push_back(0);
        }
        //cout << "cross_section =" << cross_tot[i](part.E) << " num_dens=" << number_density[i] << endl;
        //cout << prob.back() << endl;
        total+=prob.back();
    }
    if(total > pMax*Random::Flat(0,1)){
        if(total > pMax){
            pMax = total;
        }
        return true;
    }
    return false;
}

//part is the incoming particle, recoil is the target particle after scattering, out is the other outgoing particle, which may or may not be the same as 
//part.
bool Two_to_Two_Scatter::probscatter(std::shared_ptr<detector>& det, Particle &part, Particle &recoil, Particle &recoil2){
    double LXDet = m_to_cm*(det->Ldet(part));
    //cout << LXDet << endl;
    double total=0; vector<double> prob;
    for(int i = 0; i < chan_number; i++){
        if(part.name==in_name[i]){
            prob.push_back(LXDet*convGeV2cm2*(cross_tot[i](part.E))*number_density[i]);
        }
        else{
            prob.push_back(0);
        }
        total+=prob.back();
    }
    if(total > pMax*Random::Flat(0,1)){
        if(total > pMax){
            pMax = total;
        }
        
        double u=Random::Flat(0,1);
        double p=0; 
        for(int i = 0; i < chan_number; i++){
            p+=prob[i];
            if(p/total > u){
                std::function<double(double)> Xsec = std::bind(dsigma[i],part.E,_1);
                //This is where we set the outgoing particles!
                recoil = recoil_particle[i];
                recoil2 = out_particle[i];
                if(scatmax(part.E, part.m, target_mass[i], recoil2.m, recoil.m) < scatmin(part.E, part.m, target_mass[i], recoil2.m, recoil.m)){
                    return false;
                }
                scatterevent(part, target_mass[i], recoil,recoil2,Xsec,cross_maxima[i]);
            }
        }
        return true;
    }
    return false;
}

void Two_to_Two_Scatter::scatterevent (Particle &part, double targ_mass, Particle &recoil, Particle &recoil2, std::function<double(double)> Xsec,std::function<double(double)> Xmax){
    double Efmax = scatmax(part.E, part.m, targ_mass, recoil.m, recoil2.m);
    double Efmin = scatmin(part.E, part.m, targ_mass, recoil.m, recoil2.m); 
    double dsigmax = std::max(Xsec(Efmax),Xmax(part.E));
    double xe,thetaN,phiN,pN;
    while(true){
        xe = Random::Flat(Efmin,Efmax);//Recoil energy
        if(Xsec(xe)/dsigmax > Random::Flat(0,1)){
            thetaN =  Theta_from_E4(part.E, xe, part.m, targ_mass, recoil2.m, recoil.m);
            phiN = Random::Flat(0,1)*2*pi;
            pN = sqrt(pow(xe,2)-pow(recoil.m,2));
            recoil.ThreeMomentum(pN*sin(thetaN)*cos(phiN),pN*sin(thetaN)*sin(phiN),cos(thetaN)*pN);
            recoil.Rotate_y(part.Theta());
            recoil.Rotate_z(part.Phi());
            recoil2.ThreeMomentum(part.px-recoil.px,part.py-recoil.py,part.pz-recoil.pz);
            break;
        }
    }
}

//I'm going to need to break these down by channel eventually.
double Two_to_Two_Scatter::scatmax(double E1, double m1, double m2, double m3, double m4){
    return(std::min(Escatmax,E4max(E1,m1,m2,m3,m4)));
}

double Two_to_Two_Scatter::scatmin(double E1, double m1, double m2, double m3, double m4){
    return(std::max(Escatmin,E4min(E1,m1,m2,m3,m4)));
}