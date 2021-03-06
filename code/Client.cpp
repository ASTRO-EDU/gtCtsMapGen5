/*
 * Client.cpp
 *
 *  Created on: Dec 21, 2013
 *      Author: Alberto Paganelli
 */

#include <Ice/Ice.h>
#include <Freeze/Freeze.h>

#include <GenmapParams.h>

#include <Astro.h>
#include <DBAstro.h>

using namespace std;
using namespace Astro;

//#define SIMPLE_KEY
//#define COMPOSITE_KEY

class AstroClient : public Ice::Application {
public:

	AstroClient();
	virtual int run(int, char*[]);

private:

	void menu();
};

int main(int argc, char **argv) {
	AstroClient app;
	return app.main(argc, argv, "config.client");
}

AstroClient::AstroClient() : Ice::Application(Ice::NoSignalHandling) {}

int AstroClient::run(int argc, char* argv[]){

//	if (argc > 1 ) {
//		cerr << appName() << ": too many arguments" << endl;
//		return EXIT_FAILURE;
//	}

	CtsGenParams params;
	if (!params.Load(argc, argv)) {
		return -1;
	}

	Astro::AGILECtsMapGenParams myParams;
	myParams.albrad = params.albrad;
	myParams.ba = params.ba;
	myParams.emax = params.emax;
	myParams.emin = params.emin;
	myParams.filtercode = params.filtercode;
	myParams.fovradmax = params.fovradmax;
	myParams.fovradmin = params.fovradmin;
	myParams.la = params.la;
	myParams.lonpole = params.lonpole;
	myParams.mdim = params.mdim;
	myParams.mres = params.mres;
	myParams.mxdim = params.mxdim;
	myParams.phasecode = params.phasecode;
	for (int i = 0;  i < params.intervals.Count(); ++ i) {
		Astro::IntervalTime intvt;
		intvt.tstart = params.intervals[i].Start();
		intvt.tstop = params.intervals[i].Stop();
		myParams.paramIntervals.push_back(intvt);
	}
	if (params.projection == ARC) {
		myParams.projection = "ARC";
	} else if (params.projection == AIT) {
		myParams.projection = "AIT";
	}

	AgileCtsMapGenPrx twoway = AgileCtsMapGenPrx::checkedCast(communicator()->propertyToProxy("Astro.Proxy")->ice_twoway()->ice_timeout(-1)->ice_secure(false));

	if (!twoway)
	{
		cerr << argv[0] << ": invalid proxy" << endl;
		return EXIT_FAILURE;
	}
	AgileCtsMapGenPrx oneway = twoway->ice_oneway();
	AgileCtsMapGenPrx batchOneway = twoway->ice_batchOneway();
	AgileCtsMapGenPrx datagram = twoway->ice_datagram();
	AgileCtsMapGenPrx batchDatagram = twoway->ice_batchDatagram();

//	Ice::InitializationData initData;
//	initData.properties = Ice::createProperties();
//	initData.properties->load("config");
//
//	// Initialize the Communicator.
//	Ice::CommunicatorPtr communicator = Ice::initialize(initData);
//
//	// Create a Freeze database connection.
//	Freeze::ConnectionPtr connection = Freeze::createConnection(communicator, "db");
//
//
//	//The map
//	DBAstro DBEvt(connection,"AgileEvtMap");
//	//The iterator
//	DBAstro::iterator it;

//#ifdef SIMPLE_KEY
//	//The evt vector
//	Astro::agileEvt agileEvt;
//
////	vector<double> ra, dec;
//	Astro::Ra ra;
//	Astro::Dec dec;
//
//	for(it=DBEvt.begin(); it != DBEvt.end(); ++it){
//		agileEvt = it->second;
//		ra.push_back(agileEvt[6]);
//		dec.push_back(agileEvt[5]);
//	}
//#endif
//
//#ifdef COMPOSITE_KEY
//
//	Astro::SeqEvtKey evtKeys;
//	Astro::AgileEvtKey key;
//
//	for(it=DBEvt.begin(); it != DBEvt.end(); ++it){
//		key = it->first;
//		evtKeys.push_back(key);
//	}
//
//#endif

	menu();

	char c;

	bool secure = false;
	int timeout = -1;

	do {
		try {
			cout << "==> ";
			cin >> c;
			if(c == 't')
			{

				Astro::Matrix retv = twoway->calculateMapKey(myParams);

				unsigned short A[params.mxdim][params.mxdim]; //counts map
				 for (int i = 0; i < params.mxdim; i++)
				{   for (int ii = 0; ii < params.mxdim; ii++)
					{
						A[ii][i] = retv[ii][i];
					}
				}

				fitsfile * mapFits;
				int status = 0;
				const double obtlimit = 104407200.;
				    if (params.tmin<obtlimit)
				        status = 1005;
//				    else
//				        status = addfile(evtFits, params);

				if ( fits_create_file(&mapFits, params.outfile, &status) != 0 ) {
					printf("Errore in apertura file '%s'\n",params.outfile);
					return status;
				}

				int bitpix   =  USHORT_IMG; /* 16-bit unsigned short pixel values       */
				long naxis    =   2;  /* 2-dimensional image                            */
				long naxes[2] = { params.mxdim, params.mxdim };   /* image is 300 pixels wide by 200 rows */
				long nelement =  naxes[0] * naxes[1];
				std::cout<< "creating Counts Map...................................." << std::endl;
				fits_create_img(mapFits, bitpix, naxis, naxes, &status);
//				std::cout<< "writinig Counts Map with " << selectedEvents << " events" << std::endl;
				fits_write_img(mapFits, bitpix, 1, nelement, A, &status);
				std::cout<< "writing header........................................" << std::endl<< std::endl;

				params.write_fits_header(mapFits, params.projection, status);

				//fits_delete_file(evtFits, &status);
				fits_close_file(mapFits, &status);


//#ifdef SIMPLE_KEY
//				Astro::Matrix retv = twoway->calculateMapVector(ra, dec);
//#endif
//
//#ifdef COMPOSITE_KEY
//				Astro::Matrix retv = twoway->calculateMapKey(evtKeys);
//#endif

				cout << "Received back the matrix" << endl;

				//Demo::FloatSeq retv = twoway->update(1, 2.0, "ciao", v);
//				cout << "Received back the vector [ ";
//				for(unsigned int i=0; i<retv.size(); i++)
//					cout << retv[i] << " ";
//				cout << "]" << endl;
			}
			else if(c == 'o')
			{
				oneway->calculateMapKey(myParams);
			}
			else if(c == 'O')
			{
				batchOneway->calculateMapKey(myParams);
			}
			else if(c == 'd')
			{
				if(secure)
				{
					cout << "secure datagrams are not supported" << endl;
				}
				else
				{
					datagram->calculateMapKey(myParams);
				}
			}
			else if(c == 'D')
			{
				if(secure)
				{
					cout << "secure datagrams are not supported" << endl;
				}
				else
				{
					batchDatagram->calculateMapKey(myParams);
				}
			}
			else if(c == 'f')
			{
				Ice::Application::communicator()->flushBatchRequests();
			}
			else if(c == 'T')
			{
				if(timeout == -1)
				{
					timeout = 2000;
				}
				else
				{
					timeout = -1;
				}

				twoway = twoway->ice_timeout(timeout);
				oneway = oneway->ice_timeout(timeout);
				batchOneway = batchOneway->ice_timeout(timeout);

				if(timeout == -1)
				{
					cout << "timeout is now switched off" << endl;
				}
				else
				{
					cout << "timeout is now set to 2000ms" << endl;
				}
			}
			else if(c == 'S')
			{
				secure = !secure;

				twoway = twoway->ice_secure(secure);
				oneway = oneway->ice_secure(secure);
				batchOneway = batchOneway->ice_secure(secure);
				datagram = datagram->ice_secure(secure);
				batchDatagram = batchDatagram->ice_secure(secure);

				if(secure)
				{
					cout << "secure mode is now on" << endl;
				}
				else
				{
					cout << "secure mode is now off" << endl;
				}
			}
			else if(c == 's')
			{
				twoway->shutdown();
			}
			else if(c == 'x')
			{
				// Nothing to do
			}
			else if(c == '?')
			{
				menu();
			}
			else
			{
				cout << "unknown command `" << c << "'" << endl;
				menu();
			}
		} catch (const Ice::Exception& e) {
			cerr << e << endl;
		}
	} while (cin.good() && c != 'x');


}

void
AstroClient::menu()
{
    cout <<
        "usage:\n"
        "t: send a vector as twoway\n"
        "o: send a vector one way oneway\n"
        "O: send greeting as batch oneway\n"
        "d: send greeting as datagram\n"
        "D: send greeting as batch datagram\n"
        "f: flush all batch requests\n"
        "T: set a timeout\n"
        "S: switch secure mode on/off\n"
        "s: shutdown server\n"
        "x: exit\n"
        "?: help\n";
}
