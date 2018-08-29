#include "decay.h"
#include "DMgenerator.h"
#include <cstdlib>
#include <iostream>
#include <fstream>
#include "constants.h"
#include "Integrator.h"


using std::shared_ptr; using std::function;
using std::bind; using std::cout;
using std::endl;

using namespace std::placeholders;

using namespace Three_Body_Decay_Space;
// std::shared_ptr<DMGenerator> decaygen1, std::shared_ptr<DMGenerator> decaygen2, std::shared_ptr<DMGenerator> decaygen3


Three_Body_Decay_Gen::Three_Body_Decay_Gen(Particle& Parent, Particle& Daughter1, Particle& Daughter2, Particle& Daughter3, std::string prodstring, double lifetime, function<double(double, double, double, double, double, double)> &amplitude){
    if(Parent.m < Daughter1.m+Daughter2.m+Daughter3.m){
        std::cerr << "Parent_Mass is smaller than combined daughter masses, invalid decay!\n";
        throw -30;
    }
    mother = Particle(Parent);
    daughter1 = Particle(Daughter1);
    daughter2 = Particle(Daughter2);
    daughter3 = Particle(Daughter3);
    
    //Double differential width/dm12/dcost
    function<double(double, double)> d2width = bind(d_decay_width_2,amplitude,_1,_2,mother.m,daughter1.m,daughter2.m,daughter3.m);
//  cout << mother.width << endl;

    int n=800;
    double resx = ((mother.m-daughter3.m) - (daughter1.m+daughter2.m))/n;

    double resy = 2.0/n;
    double tot=0.0;
    for(int nx=0; nx<n; nx++){
        for(int ny=0; ny<n; ny++){
            tot+=d2width((daughter1.m+daughter2.m)+nx*resx,-1+ny*resy);
        }
    }

//    cout << tot*resx*resy*8*pi*pi << endl;
    //commented temporarily until I can determine the issue.
    //branchingratio=8*pi*pi*SimpsonCubature(d2width,daughter1.m+daughter2.m,mother.m-daughter3.m,100,-1,1,100)/mother.width;
    branchingratio=tot*resx*resy*8*pi*pi/mother.width;
    std::cout << "Branching Ratio calculated to be " << branchingratio << endl;
    tau = lifetime;
    //amp = function<double(double,double,double,double,double,double)>(amplitude);
    //This might not work, I may need a pointer to store this. Memory allocation etcetc.
    amp = amplitude;
    prodchoice=prodstring;
    pmax=0;

    //burn-in
    for(int i=0; i<1000; i++){
        Three_Body_Decay(mother, daughter1, daughter2, daughter3, pmax, amplitude);
    }
}

//this might not work, watch for segfaults.
void Three_Body_Decay_Gen::Toggle_Daughter_Decay(int index, std::shared_ptr<DMGenerator> daughter_decay_gen){
    if(index==1){
        d1_unstable=true;
        d1_decay=daughter_decay_gen;
    }
    else if(index==2){
        d2_unstable=true;
        d2_decay=daughter_decay_gen;
    }
    else if(index==3){
        d3_unstable=true;
        d3_decay=daughter_decay_gen;   
    }
    else{
        std::cerr << "Toggle_Daughter_Decay index must be 1, 2 or 3. Index supplied was " << index << endl;
    }
}


bool Three_Body_Decay_Gen::GenDM(std::list<Particle>& vec, std::function<double(Particle&)> det_int, Particle& part){
    Particle parent = Particle(part);
    parent.name = mother.name;

/*
    cout << "Beginning GenDM debug\n";

    parent.report(cout);
*/
    double decay_time=tau*log(1/(1-Random::Flat()));
    double boost = 1/sqrt(1-pow(parent.Speed(),2));
/*
    cout << "decay_time = " << decay_time << endl;
    cout << "parent.Speed() = " << parent.Speed() << endl;
    cout << "parent.Momentum() = " << parent.Momentum() << endl;
    cout << "parent.E = " << parent.E << endl;
    cout << "parent.m = " << parent.m << endl; 
    cout << "boost = " << boost << endl;

    //cout << decay_time << " " << boost << endl;
 */
    parent.END_SET=true;
    parent.Set_Time(decay_time*boost);

    if(record_parent){
        vec.push_back(parent);
    }

    Three_Body_Decay(parent, daughter1, daughter2, daughter3, pmax, amp);
/*
    std::list<Particle>::iterator bookmark = vec.end();
    if(d1_unstable&&(d1_decay->GenDM(vec, det_int, daughter1))){
        vec.insert(bookmark,daughter1);
        bookmark = vec.end();
    }
    if(d2_unstable&&(d2_decay->GenDM(vec, det_int, daughter2))){
        vec.insert(bookmark,daughter2);
        bookmark = vec.end();
    }
    if(d3_unstable&&(d3_decay->GenDM(vec, det_int, daughter3))){
        vec.insert(bookmark,daughter3);
        bookmark = vec.end();
    }
*/
/*
    cout << "Report on THREE_BODY_DECAY\n";

    daughter1.report(cout);
    daughter2.report(cout);
    daughter3.report(cout);
*/
    bool intersect=false;
    //Need to decay these daughter particles!
    if(d1&&(det_int(daughter1)>0)){
        //cout << "daughter1 hit!\n";
        intersect=true;
        vec.push_back(daughter1);
    }
    if(d2&&(det_int(daughter2)>0)){
        //cout << "daughter2 hit!\n";
        intersect=true;
        vec.push_back(daughter2);
    }
    if(d3&&(det_int(daughter3)>0)){
        //cout << "daughter3 hit!\n";
        intersect=true;
        vec.push_back(daughter3);
        
    }
/*
    cout << "EVENT GEN\n";
    parent.report(cout);
    daughter1.report(cout);
    daughter2.report(cout);
    daughter3.report(cout);
*/
    return intersect;
}
