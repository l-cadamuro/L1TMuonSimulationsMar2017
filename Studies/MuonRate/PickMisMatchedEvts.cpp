#include "MuonTree.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TProfile.h"
#include "TFile.h"
#include <vector>
#include <string>
#include <fstream>
#include <utility>
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

// for each element, first index is entry in TTree, second is list of all mismatched tracks
typedef std::pair<int, std::vector<int>> evtAndTracks ;
typedef std::vector<evtAndTracks> evtAndTracksCollection;

typedef tee_device<ostream, ofstream> TeeDevice;
typedef stream<TeeDevice> TeeStream;

// c++ -lm -o PickMisMatchedEvts PickMisMatchedEvts.cpp `root-config --glibs --cflags` -lboost_program_options
// ./PickMisMatchedEvts --filelist ../../Analyzers/batch/flatNtuples_SingleNeutrino_PU50_01Dic2017_bugFix.txt --maxEvts 10 --pt_tolerance 40 --output output_rate_files_PU50.root 
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

void CopyStyles(TH1* from, TH1* to)
{
    to->SetLineColor(from->GetLineColor());
    to->SetLineWidth(from->GetLineWidth());
    to->SetMarkerColor(from->GetMarkerColor());
    to->SetMarkerStyle(from->GetMarkerStyle());
}

TH1D* makeRatePlot(TH1D* h_pt)
{
    TH1D* rate_plot = new TH1D(
        (string("rate_")+string(h_pt->GetName())).c_str(),
        (string("Rate ")+string(h_pt->GetTitle())).c_str(),
        h_pt->GetNbinsX(),
        h_pt->GetBinLowEdge(1),
        h_pt->GetBinLowEdge(h_pt->GetNbinsX()+1)
    );
    CopyStyles(h_pt, rate_plot);

    double tot_entries = h_pt->Integral(-1, -1);
    if (tot_entries != 0)
    {
        for (uint ibin = 1; ibin < h_pt->GetNbinsX()+1; ++ibin)
        {
            double this_integral = h_pt->Integral(ibin, -1);
            double rate_red = this_integral/tot_entries;
            rate_plot->SetBinContent(ibin, rate_red);
        }
    }
    return rate_plot;
}

void dumpTracks (MuonTree* mt, TeeStream& my_split)
{
    my_split << "** TRACKS INFO ** " << " .. tot hits " << mt->vh_size << " .. tot tracks " << mt->vt_size << endl;
    for (uint itrack = 0; itrack < mt->vt_size; ++itrack)
    {
        short mode = mt->vt_mode->at(itrack);
        bool passSingleMu = (mode == 11 || mode == 13 || mode == 14 || mode == 15);

        my_split << " Trk " << itrack << " - single mu qual? " << std::boolalpha << passSingleMu << " " ;
        
        // get the hits associated to this track
        std::vector<int> idxHits = {-1, -1, -1, -1};
        idxHits.at(0) = mt->vt_hitref1->at(itrack); // S1
        idxHits.at(1) = mt->vt_hitref2->at(itrack); // S2
        idxHits.at(2) = mt->vt_hitref3->at(itrack); // S3
        idxHits.at(3) = mt->vt_hitref4->at(itrack); // S4

        std::vector<int> idxTracks_tp1 = {-1, -1, -1, -1};
        std::vector<int> idxTracks_tp2 = {-1, -1, -1, -1};
        for (uint itr = 0; itr < idxHits.size(); ++itr)
        {
            int ihit = idxHits.at(itr);
            idxTracks_tp1.at(itr) = (ihit == -1 ? -999 : mt->vh_sim_tp1->at(ihit));
            idxTracks_tp2.at(itr) = (ihit == -1 ? -999 : mt->vh_sim_tp2->at(ihit));
            string print1 = (idxTracks_tp1.at(itr) == -999 ? "X" : to_string(idxTracks_tp1.at(itr)));
            string print2 = (idxTracks_tp2.at(itr) == -999 ? "X" : to_string(idxTracks_tp2.at(itr)));
            my_split << " S" << itr+1 << " (" << print1 << "/" << print2 << ")";
        }
        my_split << endl;
    }
    my_split << endl;
}

bool allFromSameParticle (int itrack, MuonTree* mt, int& nMissingInfoHits, int& nTotalHits, int& matchedGenTrk)
{
    // get the hits associated to this track
    std::vector<int> idxHits = {-1, -1, -1, -1};
    idxHits.at(0) = mt->vt_hitref1->at(itrack); // S1
    idxHits.at(1) = mt->vt_hitref2->at(itrack); // S2
    idxHits.at(2) = mt->vt_hitref3->at(itrack); // S3
    idxHits.at(3) = mt->vt_hitref4->at(itrack); // S4

    if (DEBUG) cout << "Listing trk num " << itrack << " hits " << endl;
    std::vector<int> idxTracks_tp1 = {-1, -1, -1, -1};
    std::vector<int> idxTracks_tp2 = {-1, -1, -1, -1};
    nTotalHits = 0;
    for (uint itr = 0; itr < idxHits.size(); ++itr)
    {
        int ihit = idxHits.at(itr);
        idxTracks_tp1.at(itr) = (ihit == -1 ? -999 : mt->vh_sim_tp1->at(ihit));
        idxTracks_tp2.at(itr) = (ihit == -1 ? -999 : mt->vh_sim_tp2->at(ihit));
        if (ihit != -1) ++nTotalHits;
        if (DEBUG) cout << "hit " << itr << " -- ihit " << ihit << " -- tp1 " << idxTracks_tp1.at(itr) << " -- tp2 " << idxTracks_tp2.at(itr) << endl;
    }
    // if idxTrack >= 0 : I have the link to the tracking particle
    // if idxTrack == -1 : there was no tracking particle linked
    // if idxTrack == -999 : there was no hit at all

    // only cases in which there are two idxTracks >= 0 that are different must be considered as "wrongly picked hit"
    // -999 ahould not be considered (there was no hit)
    // -1 to be regarded as "good" match (conservative, to avoid bias from MC matching) + output corresponding information in "nMissingInfoHits"

    std::vector<int> v_allPositive;
    nMissingInfoHits = 0; // how many existing hits in this track do not have a tracking particle linked
    matchedGenTrk = -1;   // the idx of the gen trk matched, in case all hits with some linking info come from the same tracking particle

    for (uint idx = 0; idx < idxTracks_tp1.size(); ++idx)
    {
        if (idxTracks_tp1.at(idx) >= 0) v_allPositive.push_back(idxTracks_tp1.at(idx));
        if (idxTracks_tp2.at(idx) >= 0) v_allPositive.push_back(idxTracks_tp2.at(idx));
        if (idxTracks_tp1.at(idx) == -1 || idxTracks_tp2.at(idx) == -1) nMissingInfoHits += 1;
    }
    
    // I could not find any existing hit linked to MC --> be conservative and assume that all hits are from the same particle, return true
    if (v_allPositive.size() == 0){
        if (DEBUG) cout << "Returning " << std::boolalpha << true << " -- nMissingInfoHits = " << nMissingInfoHits << " -- matchedGenTrk = " << matchedGenTrk << endl; 
        return true; // limited infr from MC...
    }

    // reduce to a set of unique elements
    sort(v_allPositive.begin(), v_allPositive.end());
    v_allPositive.erase(unique(v_allPositive.begin(), v_allPositive.end()), v_allPositive.end());
    if (v_allPositive.size() == 1) matchedGenTrk = v_allPositive.at(0); // if a unique track was linked, this is the matched gen track, else will be -1

    if (DEBUG) cout << "Returning " << std::boolalpha << (v_allPositive.size() == 1) << " -- nMissingInfoHits = " << nMissingInfoHits << " -- matchedGenTrk = " << matchedGenTrk << endl; 
    return (v_allPositive.size() == 1); // is a unique track only if a single parent track idx is found

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
        ("output",   po::value<string>()->default_value("output_rate_files.root"), "output file name")
        ("maxEvts",  po::value<int>()->default_value(10), "max events to be picked up")
        ("skip",     po::value<int>()->default_value(0), "skip (initial offset)")
        ("pt_tolerance", po::value<float>()->default_value(0), "pick only events with a relative difference in percent from gen trk")
        ("pt_min",       po::value<float>()->default_value(0), "minimum reco track pt")
        ("singleMuQual", po::value<bool>()->default_value(false), "restrict to muons satisfying single mu quality")
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

    string filelist    = args["filelist"].as<string>();
    string outputname  = args["output"].as<string>();
    int maxEvts        = args["maxEvts"].as<int>();
    int offset         = args["skip"].as<int>();
    float pt_tolerance = args["pt_tolerance"].as<float>();
    float pt_min       = args["pt_min"].as<float>();
    bool  singleMuQual = args["singleMuQual"].as<bool>();

    std::ofstream ofs( boost::replace_all_copy(outputname, ".root", ".txt"));
    TeeDevice my_tee(cout, ofs); 
    TeeStream my_split(my_tee);

    my_split << "** INFO: using file list " << filelist << endl;
    my_split << "** INFO: opening files and creating TChain" << endl;
    TChain * bigChain = new TChain ("ntupler/tree") ;
    appendFromFileList (bigChain, filelist);
    std::unique_ptr<MuonTree> mt (new MuonTree(bigChain));

    // my_split << "** INFO: creating histograms to store in file " << outputname << endl;
    TFile* fOut = new TFile(outputname.c_str(), "recreate");
    TTree* tOut = (TTree*) mt->fChain->CloneTree(0);
    
    my_split << "** INFO: starting loop over events" << endl;
    if (offset > 0)
        my_split << "** INFO: skipping first " << offset << " events" << endl;
    if (maxEvts >= 0)
        my_split << "** INFO: searching for " << maxEvts << " faulty events" << endl;

    my_split << "** INFO: picking events with (ptL1 - pttrue)/pttrue > " << pt_tolerance << "%" << endl;
    my_split << "** INFO: Asking for L1 single mu quality? " << std::boolalpha << singleMuQual << endl;

    // the output collection
    evtAndTracksCollection selected;

    for (uint iEv = offset; true; ++iEv)
    {
        if (iEv % 10000 == 0 || DEBUG)
            my_split << "... processing " << iEv << endl;
        
        // if (maxEvts >= 0 && iEv >= (maxEvts-offset))
        if (selected.size() >= maxEvts)
            break;

        int got = mt->GetEntry(iEv);
        if (!got) break;

        // ////// counting the number of hits
        // int n_CSC = 0;
        // int n_RPC = 0;
        // int n_GEM = 0;
        // for (uint ihit = 0; ihit < mt->vh_size; ++ihit)
        // {
        //     if (mt->vh_bx->at(ihit) != 0)
        //         continue;
        //     int htype = mt->vh_type->at(ihit);
        //     if (htype == 1) ++n_CSC;
        //     if (htype == 2) ++n_RPC;
        //     if (htype == 3) ++n_GEM;
        // }
        // int npu = mt->nPU; // the in-time PU

        std::vector<int> mismatchedTrks;
        for (uint itrack = 0; itrack < mt->vt_size; ++itrack)
        {
            float pt = mt->vt_pt->at(itrack);
            int   nMissingInfoHits = 0;
            int   nTotalHits = 0;
            int   matchedGenTrk = -1;
            bool  sameTrkPart = allFromSameParticle(itrack, &(*mt), nMissingInfoHits, nTotalHits, matchedGenTrk);

            float pt_difference = -1;
            bool accept = (!sameTrkPart || nMissingInfoHits > 0) ;
            
            if (matchedGenTrk >= 0)
            {           
                float trueTrkPt = mt->vp_pt->at(matchedGenTrk);
                float rel_diff = std::abs(trueTrkPt - pt)/trueTrkPt;
                accept = accept && (rel_diff*100. > pt_tolerance); // pt_tolerance is the required minimal difference w.r.t true pt in percent
            }
  
            // always = accept is singleMuQual = false
            // if singleMuQual = true, eqivalent to accept = accept & passSingleMu
            short mode = mt->vt_mode->at(itrack);
            bool passSingleMu = (mode == 11 || mode == 13 || mode == 14 || mode == 15);
            accept =  accept  && (!singleMuQual || passSingleMu);

            accept = accept && (pt > pt_min);

            if (accept)
                mismatchedTrks.push_back(itrack);
        }

        if (mismatchedTrks.size() > 0)
            selected.push_back(make_pair(iEv, mismatchedTrks));
    }

    for (uint idxEv = 0; idxEv < selected.size(); ++idxEv)
    {
        my_split << idxEv << " ** Event " << selected.at(idxEv).first;
        std::vector<int>& trks = selected.at(idxEv).second;
        my_split << "   >> trks ";
        for (int trk : trks)
        {
            my_split << " " << trk;
        }
        my_split << endl;
    }

    my_split << endl;
    my_split << ".... dumping tracks" << endl;
    for (uint idxEv = 0; idxEv < selected.size(); ++idxEv)
    {
        my_split << idxEv << " ** Event " << selected.at(idxEv).first;
        mt->GetEntry(selected.at(idxEv).first);
        dumpTracks(&(*mt), my_split);
        // store evt to outptu tree
        tOut->Fill();
    }

    fOut->cd();
    tOut->Write();
}