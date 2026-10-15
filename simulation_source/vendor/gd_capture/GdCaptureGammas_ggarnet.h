
///////////////////////////////////////////////////////////////////////////////
//                   Spectrum of radiative neutron capture by Gadolinium           
//                                    version 1.0.0                                
//                                    (Sep.09.2005)                               

//                Author : karim.zbiri@subatech.in2p3.fr                  

//This file contains the gammas spectrum produced in radiative capture of 
//neutrons by gadolinium.
//This work is adapted from earlier work in geant3 for chooz 1.

//First version by Karim Zbiri, April, 2005
///////////////////////////////////////////////////////////////////////////////



#ifndef GdCaptureGammas_hh
#define GdCaptureGammas_hh

#include "G4ReactionProductVector.hh"

using namespace std;

class GdCaptureGammas_ggarnet
{
	public:

		GdCaptureGammas_ggarnet();
		~GdCaptureGammas_ggarnet();
		G4ReactionProductVector * GetGammas ();
		vector<double>  Initialize ();
		vector<double> SpGad157();
		vector<double> CapGad155();
		vector<double> CapGad157();
		vector<double> casca1Gd155();
		vector<double> casca2Gd155();
		vector<double> casca3Gd155();
		vector<double> casca4Gd155();
		vector<double> casca5Gd155();

		vector<double> casca1Gd157();
		vector<double> casca2Gd157();
		vector<double> casca3Gd157();
		vector<double> casca4Gd157();
		vector<double> casca5Gd157();
		vector<double> casca6Gd157();
		vector<double> casca7Gd157();
		vector<double> casca8Gd157();
		vector<double> casca9Gd157();
		vector<double> casca10Gd157();
		vector<double> casca11Gd157();
		vector<double> casca12Gd157();
		vector<double> casca13Gd157();
		vector<double> casca14Gd157();
		vector<double> casca15Gd157();
		
		vector<double> continuum155();
		vector<double> continuum157();
		void intc();
		static void SetDataFiles(const G4String& gd155, const G4String& gd157);  // EricaNext


	public:
		double  kopecky155_int[200][200];
		double  kopecky157_int[200][200];
};
#endif
