#include "MuonTree.h"
#include "MuonPtBDT.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TProfile.h"
#include "TFile.h"
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <memory>
// #include <boost/Program_options.hpp>
#include <boost/program_options.hpp>
#include <boost/iostreams/tee.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/algorithm/string/replace.hpp>
namespace po = boost::program_options;

namespace bio = boost::iostreams;
using bio::tee_device;
using bio::stream;

using namespace std;

#define DEBUG false

// c++ -lm -o reapplyBDT reapplyBDT.cpp `root-config --glibs --cflags` -lboost_program_options
// ./reapplyBDT --filelist ../../Analyzers/batch/flatNtuples_SingleNeutrino_PU50_01Dic2017_bugFix.txt
// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- -
// open input txt file and append all the files it contains to TChain
void appendFromFileList (TChain* chain, TString filename)
{
    //cout << "=== inizio parser ===" << endl;
    std::ifstream infile(filename.Data());
    std::string line;
    while (std::getline(infile, line))
    {
        line = line.substr(0, line.find("#", 0)); // remove comments introduced by #
        while (line.find(" ") != std::string::npos) line = line.erase(line.find(" "), 1); // remove white spaces
        while (line.find("\n") != std::string::npos) line = line.erase(line.find("\n"), 1); // remove new line characters
        while (line.find("\r") != std::string::npos) line = line.erase(line.find("\r"), 1); // remove carriage return characters
        if (!line.empty()) // skip empty lines
            chain->Add(line.c_str());
    }
    return;
}


int main(int argc, char** argv)
{

    ////////// read opts from command line


    // Declare the supported options.
    po::options_description desc("Allowed options");
    // desc.add_options()
    //     ("help", "produce help message")
    //     ("compression", po::value<int>()->default_value(10), "set compression level")
    //     ("astring", po::value<string>(), "a random string")
    //     ("apair", po::value<vector<float> >()->multitoken(), "a random vector")
    // ;
    desc.add_options()
        ("help", "Available options")
        ("filelist", po::value<string>()->default_value("testFileList.txt"), "input file list")
        ("output",   po::value<string>()->default_value("reapply_BDT.root"), "output file name")
        ("maxEvts",  po::value<int>()->default_value(-1), "max events")
        ("skip",     po::value<int>()->default_value(0), "skip (initial offset)")
    ;

    po::variables_map args;
    // po::store(po::parse_command_line(argc, argv, desc), vm);
    po::store(parse_command_line(argc, argv, desc, po::command_line_style::unix_style ^ po::command_line_style::allow_short), args);
    po::notify(args);    

    if (args.count("help")) {
        cout << desc << "\n";
        return 1;
    }

    // example to check if ops exists
    // if (vm.count("filelist")) {
    //     cout << "a random string is " 
    //  << vm["astring"].as<string>() << ".\n";
    // } //else {
    //     cout << "Compression level was not set.\n";
    // }

    string filelist   = args["filelist"].as<string>();
    string outputname = args["output"].as<string>();
    int maxEvts       = args["maxEvts"].as<int>();
    int offset        = args["skip"].as<int>();

    typedef tee_device<ostream, ofstream> TeeDevice;
    typedef stream<TeeDevice> TeeStream;
    std::ofstream ofs( boost::replace_all_copy(outputname, ".root", ".txt"));
    TeeDevice my_tee(cout, ofs); 
    TeeStream my_split(my_tee);

    my_split << "** INFO: using file list " << filelist << endl;
    my_split << "** INFO: opening files and creating TChain" << endl;
    TChain * bigChain = new TChain ("ntupler/tree") ;
    appendFromFileList (bigChain, filelist);
    std::unique_ptr<MuonTree> mt (new MuonTree(bigChain));

 
    my_split << "** INFO: starting loop over events" << endl;
    if (offset > 0)
        my_split << "** INFO: skipping first " << offset << " events" << endl;
    if (maxEvts >= 0)
        my_split << "** INFO: limiting to up to " << maxEvts << " events" << endl;


    for (uint iEv = offset; true; ++iEv)
    {
        if (iEv % 10000 == 0 || DEBUG)
            my_split << "... processing " << iEv << endl;
        
        if (maxEvts >= 0 && iEv >= (maxEvts-offset))
            break;

        int got = mt->GetEntry(iEv);
        if (!got) break;

        for (uint iTrack = 0; iTrack < mt->vt_size; ++iTrack)
        {
            uint64_t addr = EMTFBDTAdapter::calculate_address(mt.get(), iTrack);
            cout << addr << endl;
        }

    }
}

