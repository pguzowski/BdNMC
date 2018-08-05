#include "Model.h" 
#include "constants.h"
#include "Position_Distributions.h"

#include <list>

using std::string; using std::list;
using std::vector; using std::cout;
using std::endl; using std::cerr;

void Model::Prepare_Model(Parameter& par){
    if(!this->Set_Model_Parameters(par)){
        std::cerr << "Model " << Model_Name << " is missing required model parameters\n";
        throw -1;
    }
    std::cout << "Preparing signal channel " << par.Signal_Channel() << std::endl;
    scat_max = par.Max_Scatter_Energy();
    scat_min = par.Min_Scatter_Energy();
    if(!this->Prepare_Signal_Channel(par)){
        std::cerr << "Something is wrong with signal channel declaration\n";
        throw -1;
    }

    std::shared_ptr<list<production_channel> > prodlist = par.Get_Production_List();   
    //int chan_count = prodlist->size(); 

    for(list<production_channel>::iterator proditer = prodlist->begin(); proditer!=prodlist->end(); proditer++){
        string prodchoice = proditer->Production_Channel(); 
        string proddist = proditer->Prod_Dist();
        std::shared_ptr<DMGenerator> DMGen;
        std::shared_ptr<Distribution> PartDist;
        double Vnum;
        cout << "Setting up distribution " << proddist << " for channel " << prodchoice << endl;
        //Model will handle some general production distribution stuff here.
        if(!Prepare_Production_Distribution(prodchoice,proddist,*proditer,PartDist,par)){
            cerr << "Something wrong with distribution!" << endl;
            throw -1;
        }
        //Specific model handles the production channel
        if(!this->Prepare_Production_Channel(prodchoice, proddist, *proditer, DMGen,PartDist, Vnum, par)){
            cerr << "Something wrong with prod_chan!" << endl;
            throw -1;
        };

        cout << "Production Channel prepared successfully." << endl;
        Gen_list.push_back(DMGen);
        Dist_list.push_back(PartDist);
        Vnum_list.push_back(Vnum);
        Vnumtot+=Vnum;
    };
    //cout << "Gen_list size = " << Gen_list.size() << endl;
    //cout << Gen_list[0]->Channel_Name() << endl;
}

bool Model::Prepare_Production_Distribution(std::string prodchoice, std::string proddist, production_channel& prodchan, std::shared_ptr<Distribution>& Dist, Parameter& par){
    //Eventually all of the main distributions will be handled here.
    if(proddist=="proton_beam"){
        Dist = std::shared_ptr<Distribution>(new BeamDistribution(par.Beam_Energy(),MASS_PROTON));
    }
    else{
        return false;
    }
    Dist->set_name(proddist);
    list<production_distribution> distmodlist = *(prodchan.Get_Dist_Mods_List());
    for(list<production_distribution>::iterator distiter = distmodlist.begin(); distiter!=distmodlist.end();distiter++){
        //list<std::shared_ptr<Distribution> > distlist;
        if(distiter->name()=="position_offset"){
            std::shared_ptr<Distribution> tmpdist (new Position_Offset(distiter->get_offset(0),distiter->get_offset(1),distiter->get_offset(2),distiter->get_offset(3)));
            Dist->Add_Dist(tmpdist);
        }
    }

    if(par.Output_Mode()=="particle_list"){
        Particle part(0);
        double samplesize = par.Sample_Size();
        std::ofstream parstream(prodchan.Part_List_File(),std::ios::out);
        cout << "--------------------" << endl;
        cout << "Run parameters:" << endl;
        cout << "--------------------" << endl; 
        cout << "Number of events to be generated = " << samplesize  << endl;
        cout << "Production Distribution = " << proddist << endl;
        cout << "Writing to " << prodchan.Part_List_File() << endl;
        for(int num = 0; num < samplesize; num++){
            Dist->Sample_Particle(part);
            parstream << part.px << " " << part.py << " " << part.pz << " " << part.E << " " << part.end_coords[0] << " " << part.end_coords[1] << " " << part.end_coords[2] << " " << part.end_coords[3] << endl;
        }
        parstream.close();
        return 0;
    }

    return true;
}