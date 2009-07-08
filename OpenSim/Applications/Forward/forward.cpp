// forward.cpp
// Author: Frank C. Anderson
/*
* Copyright (c)  2005, Stanford University. All rights reserved. 
* Use of the OpenSim software in source form is permitted provided that the following
* conditions are met:
* 	1. The software is used only for non-commercial research and education. It may not
*     be used in relation to any commercial activity.
* 	2. The software is not distributed or redistributed.  Software distribution is allowed 
*     only through https://simtk.org/home/opensim.
* 	3. Use of the OpenSim software or derivatives must be acknowledged in all publications,
*      presentations, or documents describing work in which OpenSim or derivatives are used.
* 	4. Credits to developers may not be removed from executables
*     created from modifications of the source.
* 	5. Modifications of source code must retain the above copyright notice, this list of
*     conditions and the following disclaimer. 
* 
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
*  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
*  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
*  SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
*  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
*  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
*  OR BUSINESS INTERRUPTION) OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
*  WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// INCLUDE
#include <string>
#include <iostream>
#include <OpenSim/version.h>
#include <OpenSim/Common/IO.h>
#include <OpenSim/Common/LoadOpenSimLibrary.h>
#include <OpenSim/Simulation/Model/Model.h>
#include <OpenSim/Simulation/Model/BodySet.h>
#include <OpenSim/Tools/ForwardTool.h>
#include <OpenSim/Actuators/GeneralizedForceAtv.h>


using namespace OpenSim;
using namespace std;

static void PrintUsage(const char *aProgName, ostream &aOStream);

//_____________________________________________________________________________
/**
 * Main routine for executing a forward integration and running a set of
 * analyses during the forward integration.
 */
int main(int argc,char **argv)
{
	//----------------------
	// Surrounding try block
	//----------------------
	try {
	//----------------------

#ifndef STATIC_OSIM_LIBS
	//LoadOpenSimLibrary("osimSdfastEngine");
	LoadOpenSimLibrary("osimSimbodyEngine");
#endif

	// PARSE COMMAND LINE
	int i;
	string option = "";
	string setupFileName = "";
	if(argc<2) {
		PrintUsage(argv[0], cout);
		return(-1);
	}
	// Load libraries first
	LoadOpenSimLibraries(argc,argv);
	for(i=1;i<argc;i++) {
		option = argv[i];

		// PRINT THE USAGE OPTIONS
		if((option=="-help")||(option=="-h")||(option=="-Help")||(option=="-H")||
		(option=="-usage")||(option=="-u")||(option=="-Usage")||(option=="-U")) {

			PrintUsage(argv[0], cout);
			return(0);
 
		// PRINT A DEFAULT SETUP FILE FOR THIS INVESTIGATION
		} else if((option=="-PrintSetup")||(option=="-PS")) {
			ForwardTool *tool = new ForwardTool();
			tool->setName("default");
			Object::setSerializeAllDefaults(true);
			tool->print("default_Setup_Forward.xml");
			Object::setSerializeAllDefaults(false);
			cout << "Created file default_Setup_Forward.xml with default setup" << endl;
			return(0);

		// IDENTIFY SETUP FILE
		} else if((option=="-Setup")||(option=="-S")) {
			if((i+1)<argc) setupFileName = argv[i+1];
			break;

		// PRINT PROPERTY INFO
		} else if((option=="-PropertyInfo")||(option=="-PI")) {
			if((i+1)>=argc) {
				Object::PrintPropertyInfo(cout,"");

			} else {
				char *compoundName = argv[i+1];
				if(compoundName[0]=='-') {
					Object::PrintPropertyInfo(cout,"");
				} else {
					Object::PrintPropertyInfo(cout,compoundName);
				}
			}
			return(0);
		}


	}

	// ERROR CHECK
	if(setupFileName=="") {
		cout<<"\n\nforward.exe: ERROR- A setup file must be specified.\n";
		PrintUsage(argv[0], cout);
		return(-1);
	}

	/*
	  ISSUES:
	  1. need to make an actuator in order to pull in DLL and register actuator objects.
     2. ActuatorSet, SdfastBodySet, AnalysisSet must have specific names in hopper.xml.
    */
	GeneralizedForceAtv *atv = new GeneralizedForceAtv();
	delete atv;

	// CONSTRUCT
	cout<<"Constructing tool from setup file "<<setupFileName<<".\n\n";
	ForwardTool forward(setupFileName);
	//forward.print("check.xml");

	// PRINT MODEL INFORMATION
	Model *model = forward.getModel();
	if(model==NULL) {
		cout<<"\nforward:  ERROR- failed to load model.\n";
		exit(-1);
	}
	cout<<"-----------------------------------------------------------------------"<<endl;
	cout<<"Loaded library\n";
	cout<<"-----------------------------------------------------------------------"<<endl;
	model->printDetailedInfo(cout);
	cout<<"-----------------------------------------------------------------------"<<endl<<endl;

	// RUN
	forward.run();

	//----------------------------
	// Catch any thrown exceptions
	//----------------------------
	} catch(Exception x) {
		x.print(cout);
		return(-1);
	}
	//----------------------------

	return(0);
}


//_____________________________________________________________________________
/**
 * Print the usage for this application
 */
void PrintUsage(const char *aProgName, ostream &aOStream)
{
	string progName=IO::GetFileNameFromURI(aProgName);
	aOStream<<"\n\n"<<progName<<":\n"<<GetVersionAndDate()<<"\n\n";
	aOStream<<"Option              Argument         Action / Notes\n";
	aOStream<<"------              --------         --------------\n";
	aOStream<<"-Help, -H                            Print the command-line options for forward.exe.\n";
	aOStream<<"-PrintSetup, -PS                     Print a default setup file for forward.exe (default_forward.xml).\n";
	aOStream<<"-Setup, -S          SetupFileName    Specify the name of the XML setup file to use for this forward tool.\n";
	aOStream<<"-PropertyInfo, -PI                   Print help information for properties in setup files.\n";


	//aOStream<<"\nThe input to the -PropertyInfo option is the name of the class to which a property\n";
	//aOStream<<"belongs, followed by a '.', followed by the name of the property.  If a class name\n";
	//aOStream<<"is not specified, a list of all registered classes is printed. If a class name is\n";
	//aOStream<<"specified, but a property is not, a list of all properties in that class is printed.\n";
}

