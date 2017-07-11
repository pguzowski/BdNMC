import subprocess as subp
from contextlib import contextmanager
import os
import pandas as pd
import numpy as np
from multiprocessing import Pool

import Parsing_inelastic
import math

@contextmanager
def cd(newdir):
    prevdir : os.getcwd()
    os.chdir(os.path.expanduser(newdir))
    try:
        yield
    finally:
        os.chdir(prevdir)

mb:1e-31
m_proton:0.938
mpi0:0.134
meta:0.547862
mrho:0.77549
momega:0.782
mphi:1.020
pi:3.14159

meson_per_pi0_miniboone : {'pi0_decay' : '1.0', 'eta_decay' : str(1.0/30.0), 'rho_decay' : str(1.0/20.0), 'omega_decay' : '0.046', 'phi_decay' : str(1.0/150.0), 'pi0_decay_baryonic' : '1.0', 'eta_decay_baryonic' : str(1.0/30.0), 'rho_decay_baryonic' : str(1.0/20.0), 'omega_decay_baryonic' : '0.046', 'phi_decay_baryonic' : str(1.0/150.0)}

meson_per_pi0_lsnd : {'pi0_decay' : '1.0'}

meson_per_pi0_coherent : {'pi0_decay' : 1.0, 'piminus_capture' : '0.63'}

meson_per_pi0_ship : {'pi0_decay' : '1.0', 'eta_decay' : str(0.078), 'rho_decay' : str(0.11), 'omega_decay' : '0.11', 'phi_decay' : str(0.02)}

Hydrogen_string : "material Hydrogen\nnumber_density 7.26942e22\nproton_number 1\nneutron_number 0\nelectron_number 1\nmass 0.945778\n"

Water_string : "material Oxygen\nnumber_density 3.34184e22\nproton_number 8\nneutron_number 8\nelectron_number 8\nmass 0.94578\nmaterial Hydrogen\nnumber_density 6.68368e22\nproton_number 1\nneutron_number 0\nelectron_number 1\nmass 0.945778\n"

Carbon_string : "material Carbon\nnumber_density 3.63471e22\nproton_number 6\nneutron_number 6\nelectron_number 6\nmass 11.2593\n"

Argon_string : "material Argon\nnumber_density 2.11e22\nproton_number 18\nneutron_number 22\nelectron_number 18\nmass {0}\n".format(str(39.948*0.938))

ND280_string : "material nd280stuff\nnumber_density 3.7e23\nproton_number 1\nneutron_number 1\nelectron_number 1\nmass 0.945778\n"

Sodium_Iodide_string : "material Sodium\nnumber_density 1.58e22\nproton_number 11\n neutron_number 23\nelectron_number 11\nmass 21.61\nmaterial Iodine\nnumber_density 1.58e22\nproton_number 53\nneutron_number 72\nelectron_number 53\nmass 119.03\n"

#Don't know what the atomic makeup of the MINOS detector is. Not using this for event generation, so it should be okay.
MINOS_string : "material Steel\nnumber_density 5e24\nproton_number 1\nneutron_number 1\nelectron_number 1\nmass 0\n"

default = {"eps" : 1e-3, "mdm" : 0.03, "mv" : 0.1, "alpha_D" : 0.1, "prod_chan" : ["pi0_decay"], "signal_chan" : "NCE_nucleon", "outfile" : "parameter_run.dat", "proddist" : [""], "partlistfile" : ["Source/particle_list.dat"], "sumlog" : "Events/miniboone.dat", "outlog" : "Events/miniboone_events.dat", "output_mode" :"summary", "samplesize" : 5000, "min_scatter_energy" : 0.035, "max_scatter_energy" : 1.0, "dm_energy_resolution" : 0.01, "efficiency" : 0.35, "beam_energy" : 8.9,
        "n_num_target" : 4, "p_num_target" : 4, "max_trials" : 80e6, "ptmax" : 0.2, "zmin" : 0.3, "zmax" : 0.7, "run" : -1, "POT" : 2e20, "pi0_per_POT" : 0.9, "p_cross" : 25*mb, "meson_per_pi0" : meson_per_pi0_miniboone, "min_scatter_angle" : 0.0, "max_scatter_angle" : 2.1*pi, "repeat" : 1, "timing" : 0.0, "burn_max" : -1,"inelastic_dist" : "data/DIS.dat", "coherent" : 'false', "model" : "Dark_Photon_DM", "gagg" : 0, "gagpg" : 0, "gagpgp" : 0)

def write_experiment(write_detector,user):
    context = defaults.copy()
    context.update(user)

    prod_chan = context["prod_chan"]; proddist = context["proddist"]; partlistfile = context["partlistfile"]
    eps = context["eps"]; mdm = context["mdm"]; mv = context["mv"]; alpha_D = context["alpha_D"]; outfile = context["outfile"];
    sumlog = context["sumlog"]; outlog = context["outlog"]; output_mode = context["output_mode"]; samplesize = context["samplesize"];
    min_scatter_energy = context["min_scatter_energy"]; max_scatter_energy=context["max_scatter_energy"]; dm_energy_resolution = context["dm_energy_resolution"];
    efficiency = context["efficiency"]; beam_energy = context["beam_energy"]; n_num_target = context["n_num_target"]; p_num_target = context[p_num_target];
    max_trials = context["max_trials"]; ptmax = context["ptmax"]; zmin = context["zmin"]; zmax = context["zmax"]; run = context["run"];
    pi0_per_POT=context["pi0_per_POT"]; p_cross = context["p_cross"]; meson_per_pi0 = context["meson_per_pi0"]; min_scatter_angle=context["min_scatter_angle"];
    max_scatter_angle=context["max_scatter_angle"]; repeat = context["repeat"]; timing = context["timing"]; burn_max = context["burn_max"];
    inelastic_dist = context["inelastic_dist"]; coherent = context["coherent"]; model = context["model"]; gagg= context["gagg"]; gagpg = context["gagpg"];
    gagpgp = context["gagpgp"];
    with open(context["outfile"],'w') as f:
        if run>=0:
            f.write('run {}\n'.format(context["run"]))
        for i in range(len(prod_chan)):
	    f.write('production_channel {}\n'.format(prod_chan[i]))
	    if(prod_chan[i]=="parton_production"):
            	prepare_parton(mA=mv,energy=beam_energy,file_path=context["v_parton_kinetic"])
                f.write('parton_V_neutron_file {}\n'.format("data/parton_V_n.dat"))
                f.write('parton_V_proton_file {}\n'.format("data/parton_V_p.dat"))
            elif(prod_chan[i]=="parton_production_baryonic"):
            	prepare_parton(mA=mv,energy=beam_energy,file_path=context["v_parton_baryonic"])
                f.write('parton_V_neutron_file {}\n'.format("data/parton_V_n.dat"))
                f.write('parton_V_proton_file {}\n'.format("data/parton_V_p.dat"))
            if(proddist[i]!=""):
            	f.write("production_distribution {}\n".format(proddist[i]))
	    if(partlistfile[i]!=""):
		f.write("particle_list_file {}\n".format(partlistfile[i]))
	    if prod_chan[i] in meson_per_pi0:
                f.write('meson_per_pi0 {}\n'.format(str(meson_per_pi0[prod_chan[i]])))
	    if proddist[i]=='proton_brem' or proddist[i]=='proton_brem_baryonic':
                f.write('zmax {}\n'.format(str(zmax)))
                f.write('zmin {}\n'.format(str(zmin)))
                f.write('ptmax {}\n'.format(str(ptmax)))
                f.write('\n')
        if repeat!=1:
            f.write("repeat {}\n".format(str(repeat)))
        if timing!=0.0:
            f.write("timing_cut {}\n".format(str(timing)))
        if burn_max!=-1:
            f.write("burn_max {}\n".format(str(burn_max)))
        f.write('proton_target_cross_section {}\n'.format(str(p_cross)))
        f.write('max_trials {}\n'.format(str(max_trials)))
        f.write("efficiency {}\n".format(str(efficiency)))
        f.write('min_scatter_energy {}\n'.format(str(min_scatter_energy)))
        f.write('max_scatter_energy {}\n'.format(str(max_scatter_energy)))
        f.write('min_scatter_angle {}\n'.format(str(min_scatter_angle)))
        f.write('max_scatter_angle {}\n'.format(str(max_scatter_angle)))
        f.write('dm_energy_resolution {}\n'.format(str(dm_energy_resolution)))
        f.write('epsilon {}\n'.format(str(eps)))
        f.write('n_num_target {}\n'.format(str(n_num_target)))
        f.write('p_num_target {}\n'.format(str(p_num_target)))
        f.write('beam_energy {}\n'.format(str(beam_energy)))
        f.write('dark_matter_mass {}\n'.format(str(mdm)))
        f.write('dark_photon_mass {}\n'.format(str(mv)))
        f.write('alpha_D {}\n'.format(str(alpha_D)))
        if(model=="Axion_Dark_Photon"):
            f.write('gagg {}\n'.format(str(gagg)))
            f.write('gagpg {}\n'.format(str(gagpg)))
            f.write('gagpgp {}\n'.format(str(gagpgp)))
        f.write('model {}\n'.format(model));
        f.write('POT {}\n'.format(str(POT)))
        f.write('signal_channel {}\n'.format(signal_chan))
        if signal_chan=="Inelastic_Nucleon_Scattering" or signal_chan == "Inelastic_Nucleon_Scattering_Baryonic":
            Parsing_inelastic.gen_cross_section(mv,eps=eps,alpha_d=alpha_D,model=signal_chan,output=inelastic_dist)
            f.write('scatter_dist_filename {}\n'.format(inelastic_dist))
        f.write('output_file {}\n'.format(outlog))
        f.write('summary_file {}\n'.format(sumlog))
        f.write('output_mode {}\n'.format(output_mode))
        f.write('samplesize {}\n'.format(str(samplesize)))
        f.write('coherent {}\n'.format(coherent))
        f.write('pi0_per_POT {}\n\n'.format(str(pi0_per_POT)))
        write_detector(f)
        f.close()

def miniboone_detector(f,xpos=0.0,ypos=-1.9,zpos=491.0,radius=5.0):
    f.write("\ndetector sphere\n")
    f.write("x-position {0}\ny-position {1}\nz-position {2}\nradius {3}\n".format(str(xpos),str(ypos),str(zpos),str(radius)))
    f.write('\n')
    f.write(Hydrogen_string)
    f.write('\n')
    f.write(Carbon_string)

def miniboone_detector_numi(f,xpos=0.0,ypos=0.0,zpos=100.0,radius=5.0):
    f.write("\ndetector sphere\n")
    f.write("x-position {0}\ny-position {1}\nz-position {2}\nradius {3}\n".format(str(xpos),str(ypos),str(zpos),str(radius)))
    f.write('\n')
    f.write(Hydrogen_string)
    f.write('\n')
    f.write(Carbon_string)

def miniboone_detector_full(f,xpos=0.0,ypos=-1.9,zpos=491.0,radius=6.106):
    f.write("\ndetector sphere\n")
    f.write("x-position {0}\ny-position {1}\nz-position {2}\nradius {3}\n".format(str(xpos),str(ypos),str(zpos),str(radius)))
    f.write('\n')
    f.write(Hydrogen_string)
    f.write('\n')
    f.write(Carbon_string)

MINOS_absorber_z=270
MINOS_target_z=950
#Don't use this for actual event generation!
def MINOS_detector(f,xpos=0.0,ypos=0.0,zpos=MINOS_target_z,radius=2.2,length=1.7,theta=0,phi=0):
    print("This detector should not be used for event generation!")
    f.write("\ndetector cylinder\n");
    f.write("x-position {0}\ny-position {1}\nz-position {2}\nradius {3}\nlength {4}\ndet-theta {5}\ndet-phi {6}\n".format(str(xpos),str(ypos),str(zpos),str(radius),str(length),str(theta),str(phi)))
    f.write('\n')
    f.write(MINOS_string)

NOvA_absorber_d=240
NOvA_target_d=920
NOvA_angle=0.0575959#3.3 degree

def NOvA_detector(f,xpos=0.0,ypos=NOvA_target_d*math.sin(NOvA_angle),zpos=NOvA_target_d*math.cos(NOvA_angle),radius=2,length=14,theta=-0.0575959,phi=0):
    print("This detector should not be used for event generation!")
    f.write("\ndetector cylinder\n");
    f.write("x-position {0}\ny-position {1}\nz-position {2}\nradius {3}\nlength {4}\ndet-theta {5}\ndet-phi {6}\n".format(str(xpos),str(ypos),str(zpos),str(radius),str(length),str(theta),str(phi)))
    f.write('\n')
    f.write(MINOS_string)

def SBND_detector(f,xpos=0.0,ypos=0,zpos=112.0,radius=2.38,length=4.76,theta=0,phi=0):
    f.write("\ndetector cylinder\n");
    f.write("x-position {0}\ny-position {1}\nz-position {2}\nradius {3}\nlength {4}\ndet-theta {5}\ndet-phi {6}\n".format(str(xpos),str(ypos),str(zpos),str(radius),str(length),str(theta),str(phi)))
    f.write('\n')
    f.write(Argon_string)

def SBND_detector_old(f,xpos=0.0,ypos=0,zpos=62.0,radius=2.38,length=4.76,theta=0,phi=0):
    f.write("\ndetector cylinder\n");
    f.write("x-position {0}\ny-position {1}\nz-position {2}\nradius {3}\nlength {4}\ndet-theta {5}\ndet-phi {6}\n".format(str(xpos),str(ypos),str(zpos),str(radius),str(length),str(theta),str(phi)))
    f.write('\n')
    f.write(Argon_string)

#temp detector until I implement proper geometry handling
#This is actually the P0D, with only the proper number of neutrons and protons. NO ATOMS IMPLEMENTED
#Double check the fiducial mass on the pod. Is it 3 tons of water? 13 tons of stuff?
def t2k_ND280(f):
    xpos=11;ypos=0;zpos=280;detphi=0;radius=0.9413;dettheta=0.0436332;length=1.7;
    f.write("\ndetector cylinder\n")
    f.write("x-position {0}\ny-position {1}\nz-position {2}\nradius {3}\ndet-theta {4}\ndet-phi {5}\nlength {6}\n".format(str(xpos),str(ypos),str(zpos),str(radius),str(dettheta),str(detphi),str(length),str(length)))
    f.write('\n')
    f.write(ND280_string)

def t2k_superK(f):
    xpos=12867.7;ypos=0;zpos=294719;detphi=0;radius=190.5;dettheta=1.5708;length=410;
    f.write("\ndetector cylinder\n")
    f.write("x-position {0}\ny-position {1}\nz-position {2}\nradius {3}\ndet-theta {4}\ndet-phi {5}\nlength {6}\n".format(str(xpos),str(ypos),str(zpos),str(radius),str(dettheta),str(detphi),str(length),str(length)))
    f.write('\n')
    f.write(Water_string)

miniboone_default = {}

def write_miniboone(d={},det=miniboone_detector):
    context = miniboone_default.copy()
    context.update(d)
    write_experiment(det,d)

t2k_default = {"proddist" : ["bmpt"], "partlistfile" : ["data/particle_list_t2k.dat"], "sumlog" : "Events/t2k.dat", "outlog" : "Events/t2k_events.dat", "beam_energy" : 30, "n_num_target" : 6, "p_num_target" : 6, "ptmax" : 1, "zmin" : 0.2, "zmax" : 0.8}

def write_t2k(d={}, det=t2k_nd280):
    contest = t2k_default.copy()
    contest.update(d)
    write_experiment(det,d)


