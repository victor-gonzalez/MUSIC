#include <mpi.h>
#include <stdio.h>
#include "util.h"
#include "grid.h"
#include "data.h"
#include "init.h"
#include "eos.h"
#include "freeze.h"
#include "evolve.h"

using namespace std;
void ReadInData(InitData *DATA, char *file);


// main program
main(int argc, char *argv[])
{
  // initialize MPI
  //  MPI::Init(argc, argv);
  //int rank = MPI::COMM_WORLD.Get_rank(); //number of current processor
  //int size = MPI::COMM_WORLD.Get_size(); //total number of processors
  //fprintf(stderr, "This is processor %i/%i: READY.\n", rank,size);

  int rank=0;
  int size=1;

  Grid ***arena; 
  Grid ***Lneighbor; 
  Grid ***Rneighbor;
 
  char *input_file;
  static InitData DATA;
  int ieta, ix, iy, flag;
  double f, tau;


  // you have the option to give a second command line option, which is an integer to be added to the random seed from the current time
  // because on the cluster it will happen that two jobs start at exactly the same time, this makes sure that they dont run with excatly the same seed
  string sseed;
  int seed = 0;
  if (argc>2)
    {
      sseed = argv[2];
      seed = atoi(sseed.c_str()); 
    }
  seed *= 10000;

  DATA.seed = seed;

  input_file = *(argv+1);
  ReadInData(&DATA, input_file);
  
  if (DATA.neta%size!=0 && DATA.mode < 3)
    {
      cerr << " Number of cells in eta direction " << DATA.neta << " is not a multiple of the number of processors " << size 
	   << ". Exiting." << endl;
      MPI::Finalize();
      exit(1);
    }

  if (DATA.neta/size<4 && DATA.mode < 3)
    {
      cerr << " Number of cells in eta direction per processor " << DATA.neta/size << " is less than 4. Exiting." << endl;
      MPI::Finalize();
      exit(1);
    }

  // reduce the lattice size on each processor (this is slicing it in 'size' pieces)
  if (size>0)
    DATA.neta=DATA.neta/size;
  
  EOS *eos;
  eos = new EOS;
  
  if (DATA.whichEOS==0)
    {
      if (rank == 0) 
        fprintf(stderr,"Using the ideal gas EOS \n");
    }
  else if (DATA.whichEOS==1)
    {
      if (rank == 0)
	fprintf(stderr,"Using EOS-Q from AZHYDRO \n");
      eos->init_eos();
    }
  else if (DATA.whichEOS==2)
    {
      if (rank == 0)
	fprintf(stderr,"Using lattice EOS from Huovinen/Petreczky \n");
      eos->init_eos2();
    }
  else 
    {
      if (rank == 0)
	fprintf(stderr,"No EOS for whichEOS = %d, use EOS_to_use = 1 or 2 \n",DATA.whichEOS);
      exit(1);
    }
      
  if (DATA.mode == 1 || DATA.mode == 2)
    {
      Glauber *glauber;
      glauber = new Glauber;
      
      Init *init;
      init = new Init(eos,glauber);
      
      Evolve *evolve;
      evolve = new Evolve(eos);
      
      glauber->initGlauber(DATA.SigmaNN, DATA.Target, DATA.Projectile, DATA.b, DATA.LexusImax, size, rank);
      
      init->InitArena(&DATA, &arena, &Lneighbor, &Rneighbor, size, rank);
      
      flag =  evolve->EvolveIt(&DATA, arena, Lneighbor, Rneighbor, size, rank); 
      
      tau = DATA.tau0 + DATA.tau_size; 
      
      //    delete evolve;
      //delete glauber;
      //delete init;
    }
  
  if (DATA.mode == 1 || DATA.mode == 3 || DATA.mode == 4 || DATA.mode >= 5)
    {
      //if (rank == 0) int ret = system("rm yptphiSpectra*");
      //  freeze-out
      Freeze *freeze;
      freeze = new Freeze();
      freeze->CooperFrye(DATA.particleSpectrumNumber, DATA.mode, &DATA, size, rank);
      delete freeze;
    }
  
   MPI::Finalize();
}/* main */
  

void ReadInData(InitData *DATA, char *file)
{
  int m, n;
  double tempf;
  Util *util;
  util = new Util();
  
 DATA->LexusImax = util->IFind(file, "Lexus_Imax");
 DATA->b = util->DFind(file, "Impact_parameter");
 DATA->Target = util->StringFind(file, "Target");
 DATA->Projectile = util->StringFind(file, "Projectile");
 DATA->SigmaNN = util->DFind(file, "SigmaNN");
 DATA->Initial_profile = util->IFind(file, "Initial_profile");
 DATA->initializeEntropy = util->IFind(file, "initialize_with_entropy");
 DATA->epsilonFreeze = util->DFind(file, "epsilon_freeze");
 DATA->particleSpectrumNumber = util->IFind(file, "particle_spectrum_to_compute");
 DATA->mode = util->IFind(file, "mode"); // 1: do everything; 2: do hydro evolution only; 3: do calculation of thermal spectra only;
 // 4: do resonance decays only
 DATA->whichEOS = util->IFind(file, "EOS_to_use");
 DATA->NumberOfParticlesToInclude = util->IFind(file, "number_of_particles_to_include");
 DATA->freezeOutMethod = util->IFind(file, "freeze_out_method");
 DATA->rhoB0 = util->DFind(file, "rho_b_max");
 DATA->hard = util->DFind(file, "binary_collision_scaling_fraction");
 DATA->facTau = util->IFind(file, "average_surface_over_this_many_time_steps"); 

 DATA->nx = util->IFind(file, "Grid_size_in_x");
 DATA->ny = util->IFind(file, "Grid_size_in_y");
 DATA->neta = util->IFind(file, "Grid_size_in_eta");
 
 DATA->x_size = util->DFind(file, "X_grid_size_in_fm");
 DATA->y_size = util->DFind(file, "Y_grid_size_in_fm");
 DATA->eta_size = util->DFind(file, "Eta_grid_size");
 DATA->tau_size = util->DFind(file, "Total_evolution_time_tau");
 DATA->tau0 = util->DFind(file, "Initial_time_tau_0");

 /* x-grid, for instance, runs from 0 to nx */
 DATA->delta_x = (DATA->x_size)/( ((double) DATA->nx) ); 
 DATA->delta_y = (DATA->y_size)/( ((double) DATA->ny) ); 
 DATA->delta_eta = (DATA->eta_size)/( ((double) DATA->neta) ); 

 cerr << " DeltaX=" << DATA->delta_x << endl;
 cerr << " DeltaY=" << DATA->delta_y << endl;
 cerr << " DeltaETA=" << DATA->delta_eta << endl;
 
 /* CFL condition : delta_tau < min(delta_x/10, tau0 delta_eta/10) */
 
 DATA->delta_tau = util->DFind(file, "Delta_Tau");
 tempf = mini(DATA->delta_x/10.0, (DATA->tau0)*(DATA->delta_eta/10.0));
 if(tempf < DATA->delta_tau) DATA->delta_tau = tempf;
 
 DATA->rotateBy45degrees = util->IFind(file, "rotate_by_45_degrees");
 DATA->outputEvolutionData = util->IFind(file, "output_evolution_data");
 
 DATA->nt = (int) (floor(DATA->tau_size/(DATA->delta_tau) + 0.5));
 fprintf(stderr, "ReadInData: Time step size = %e\n", DATA->delta_tau);
 fprintf(stderr, "ReadInData: Number of time steps required = %d\n", DATA->nt);
 
 DATA->epsilon0 = util->DFind(file, "Maximum_energy_density");
 DATA->eta_fall_off = util->DFind(file, "Eta_fall_off");
 DATA->eta_flat = util->DFind(file, "Eta_plateau_size");
 DATA->R_A = util->DFind(file, "Initial_radius_size_in_fm");
 DATA->a_A = util->DFind(file, "Initial_radial_fall_off_in_fm");
 
 DATA->a_short = util->DFind(file, "Initial_asym_short_axis_in_fm");
 DATA->a_long  = util->DFind(file, "Initial_asym_long_axis_in_fm");
 
 // for calculation of spectra:
 DATA->ymax = util->DFind(file, "maximal_rapidity");
 DATA->deltaY = util->DFind(file, "delta_y");
 
 if(DATA->turn_on_rhob == 1)
  {
   DATA->alpha_max = 5;
  }
 else
  {
   DATA->alpha_max = 4;
  }
 
 DATA->rk_order = util->IFind(file, "Runge_Kutta_order");
 
 if( (DATA->rk_order != 1) )
  {
   fprintf(stderr, "Runge-Kutta order = %d.\n", DATA->rk_order);
  }

 // in case of using Initial_Distribution 3, these are the limits between which to sample the impact parameter
 DATA->bmin = util->DFind(file, "bmin");
 DATA->bmax = util->DFind(file, "bmax");

 DATA->includeJet = util->IFind(file,"Include_Jet");
 DATA->includeTrigger = util->IFind(file,"Include_Trigger");
 DATA->minmod_theta = util->DFind(file, "Minmod_Theta");

 DATA->local_y_max = util->DFind(file, "Maximum_Local_Rapidity");

 DATA->viscosity_flag = util->IFind(file, "Viscosity_Flag_Yes_1_No_0");

 DATA->verbose_flag = util->IFind(file, "Verbose_Flag_Yes_1_No_0");
 
 DATA->eps_limit  = util->DFind(file, "Minimum_Epsilon_Frac_For_Visc");

 DATA->turn_on_rhob = util->IFind(file, "Include_Rhob_Yes_1_No_0");
 
 DATA->turn_on_shear = util->IFind(file, "Include_Shear_Visc_Yes_1_No_0");
 
 DATA->turn_on_bulk = util->IFind(file, "Include_Bulk_Visc_Yes_1_No_0");
 
 DATA->zero_or_stop = util->IFind(file, "Vicous_Trouble_Zero=0_or_Stop=1:");

 DATA->tau_pi = util->DFind(file, "Shear_relaxation_time_tau_pi");
 DATA->tau_b_pi = util->DFind(file, "Bulk_relaxation_time_tau_b_pi");
 DATA->shear_to_s = util->DFind(file, "Shear_to_S_ratio");
 DATA->bulk_to_s = util->DFind(file, "Bulk_to_S_ratio");

 DATA->include_deltaf = util->IFind(file, "Include_deltaf");
 
 DATA->doFreezeOut = util->IFind(file, "Do_FreezeOut_Yes_1_No_0");

/* initialize the metric, mostly plus */



 DATA->gmunu = util->mtx_malloc(4,4);
 for(m=0; m<4; m++)
  {
   for(n=0; n<4; n++)
    {
     if(m == n) (DATA->gmunu)[m][n] = 1.0;
     else (DATA->gmunu)[m][n] = 0.0;
     
     if(m==0 && n==0) (DATA->gmunu)[m][n] *= -1.0;
    }
  }/* m */

 fprintf(stderr, "Done ReadInData.\n");
 delete util;
}/* ReadInData */

