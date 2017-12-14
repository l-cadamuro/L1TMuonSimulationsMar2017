#include "MuonTree.h"
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

// c++ -lm -o MakeRate MakeRate.cpp `root-config --glibs --cflags` -lboost_program_options
// ./MakeRate --filelist ../../Analyzers/batch/flatNtuples_SingleNeutrino_PU50_01Dic2017_bugFix.txt --output output_rate_files_PU50.root --maxEvts 50000
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

    // if (argc > 1)
    //     filelist = string(argv[1]);
    // making a log with both 
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

    my_split << "** INFO: creating histograms to store in file " << outputname << endl;
    TFile* fOut = new TFile(outputname.c_str(), "recreate");
    
    TH1D* h_HitMult_CSC  = new TH1D("h_HitMult_CSC",  ";Number of EMTF CSC hits; Events", 200, 0, 200);
    TH1D* h_HitMult_RPC  = new TH1D("h_HitMult_RPC",  ";Number of EMTF RPC hits; Events", 200, 0, 200);
    TH1D* h_HitMult_GEM  = new TH1D("h_HitMult_GEM",  ";Number of EMTF GEM hits; Events", 200, 0, 200);
    TH2D* h2D_HitMult_CSC  = new TH2D("h2D_HitMult_CSC",  ";Number of in-time PU interactions; Number of EMTF CSC hits", 250, 0, 250, 200, 0, 200);
    TH2D* h2D_HitMult_RPC  = new TH2D("h2D_HitMult_RPC",  ";Number of in-time PU interactions; Number of EMTF RPC hits", 250, 0, 250, 200, 0, 200);
    TH2D* h2D_HitMult_GEM  = new TH2D("h2D_HitMult_GEM",  ";Number of in-time PU interactions; Number of EMTF GEM hits", 250, 0, 250, 200, 0, 200);
    TProfile* hProfile_HitMult_CSC  = new TProfile("hProfile_HitMult_CSC",  ";Number of in-time PU interactions; Number of EMTF CSC hits", 300, 0, 300);
    TProfile* hProfile_HitMult_RPC  = new TProfile("hProfile_HitMult_RPC",  ";Number of in-time PU interactions; Number of EMTF RPC hits", 300, 0, 300);
    TProfile* hProfile_HitMult_GEM  = new TProfile("hProfile_HitMult_GEM",  ";Number of in-time PU interactions; Number of EMTF GEM hits", 300, 0, 300);

    TH1D* h_PtRes_true               = new TH1D("h_PtRes_true",  "EMTF candidates with correct matching;p_{T}^{EMTF}/p_{T}^{gen}; Events", 100, 0, 5);
    TH1D* h_PtRes_true_missingInfo   = new TH1D("h_PtRes_true_missingInfo",  "EMTF candidates with missing MC info;p_{T}^{EMTF}/p_{T}^{gen}; Events", 100, 0, 5);
    TH1D* h_PtRes_true_NomissingInfo = new TH1D("h_PtRes_true_NomissingInfo",  "EMTF candidates w/o missing MC info;p_{T}^{EMTF}/p_{T}^{gen}; Events", 100, 0, 5);
    
    TH2D* h2D_PtRes_true               = new TH2D("h2D_PtRes_true",               "EMTF candidates with correct matching; p_{T}^{gen};p_{T}^{EMTF}/p_{T}^{gen}", 100, 0, 20, 100, 0, 20);
    TH2D* h2D_PtRes_true_missingInfo   = new TH2D("h2D_PtRes_true_missingInfo",   "EMTF candidates with missing MC info;  p_{T}^{gen};p_{T}^{EMTF}/p_{T}^{gen}", 100, 0, 20, 100, 0, 20);
    TH2D* h2D_PtRes_true_NomissingInfo = new TH2D("h2D_PtRes_true_NomissingInfo", "EMTF candidates w/o missing MC info;   p_{T}^{gen};p_{T}^{EMTF}/p_{T}^{gen}", 100, 0, 20, 100, 0, 20);

    TH2D* h2D_Pt_EMTF_vs_true               = new TH2D("h2D_Pt_EMTF_vs_true",               "EMTF candidates with correct matching; p_{T}^{gen};p_{T}^{EMTF}", 100, 0, 20, 100, 0, 20);
    TH2D* h2D_Pt_EMTF_vs_true_missingInfo   = new TH2D("h2D_Pt_EMTF_vs_true_missingInfo",   "EMTF candidates with missing MC info;  p_{T}^{gen};p_{T}^{EMTF}", 100, 0, 20, 100, 0, 20);
    TH2D* h2D_Pt_EMTF_vs_true_NomissingInfo = new TH2D("h2D_Pt_EMTF_vs_true_NomissingInfo", "EMTF candidates w/o missing MC info;   p_{T}^{gen};p_{T}^{EMTF}", 100, 0, 20, 100, 0, 20);

    // pt plots for rate
    TH1D* h_PtLead_all  = new TH1D("h_PtLead_all",  "All L1 EMTF candidates;p_{T} threshold [GeV]; Events", 200, 0, 100);
    TH1D* h_PtLead_true = new TH1D("h_PtLead_true", "EMTF candidates with correct matching;p_{T} threshold [GeV]; Events", 200, 0, 100);
    TH1D* h_PtLead_true_missingInfo = new TH1D("h_PtLead_true_missingInfo", "EMTF candidates with missing MC info;p_{T} threshold [GeV]; Events", 200, 0, 100);
    TH1D* h_PtLead_true_NomissingInfo = new TH1D("h_PtLead_true_NomissingInfo", "EMTF candidates w/o missing MC info;p_{T} threshold [GeV]; Events", 200, 0, 100);
    
    h_PtLead_all->SetLineColor(kBlack);
    h_PtLead_true->SetLineColor(kRed);
    h_PtLead_true_missingInfo->SetLineColor(kBlue);
    h_PtLead_true_NomissingInfo->SetLineColor(kGreen+1);

    // my_split << "** INFO: deactivating branches to speed up" << endl;
    // my_split << "    >>>> NB: remember to check the branches you need are on!" << endl;

    // mt->fChain->SetBranchStatus("*", 0);

    // mt->fChain->SetBranchStatus("nPU", 1);

    // mt->fChain->SetBranchStatus("vh_size", 1);
    // mt->fChain->SetBranchStatus("vh_type", 1);
    // mt->fChain->SetBranchStatus("vh_bx", 1);
    // mt->fChain->SetBranchStatus("vh_sim_tp1", 1);
    // mt->fChain->SetBranchStatus("vh_sim_tp2", 1);

    // mt->fChain->SetBranchStatus("vp_size", 1);
    // mt->fChain->SetBranchStatus("vp_pt", 1);

    // mt->fChain->SetBranchStatus("vt_size", 1);
    // mt->fChain->SetBranchStatus("vt_pt", 1);
    // mt->fChain->SetBranchStatus("vt_hitref4", 1);
    // mt->fChain->SetBranchStatus("vt_hitref3", 1);
    // mt->fChain->SetBranchStatus("vt_hitref2", 1);
    // mt->fChain->SetBranchStatus("vt_hitref1", 1);

    my_split << "** INFO: starting loop over events" << endl;
    if (offset > 0)
        my_split << "** INFO: skipping first " << offset << " events" << endl;
    if (maxEvts >= 0)
        my_split << "** INFO: limiting to up to " << maxEvts << " events" << endl;

    // my_split << "** INFO: chain has " << mt->fChain->GetEntries() << " events" << endl; // this will open all chains.. not great
    // loop over entries

    // number of tracks missing 0/1/2/3/4 hits information
    // i.e., the hit is there, but I cannot associate it to a tracking particle
    std::vector<std::vector<int>> trksMissingHitsInfo(5);
    for (uint it = 0; it < 5; ++it)
        trksMissingHitsInfo.at(it) = std::vector<int>(5,0);

    for (uint iEv = offset; true; ++iEv)
    {
        if (iEv % 10000 == 0 || DEBUG)
            my_split << "... processing " << iEv << endl;
        
        if (maxEvts >= 0 && iEv >= (maxEvts+offset))
            break;

        int got = mt->GetEntry(iEv);
        if (!got) break;

        ////// counting the number of hits
        int n_CSC = 0;
        int n_RPC = 0;
        int n_GEM = 0;
        for (uint ihit = 0; ihit < mt->vh_size; ++ihit)
        {
            if (mt->vh_bx->at(ihit) != 0)
                continue;
            int htype = mt->vh_type->at(ihit);
            if (htype == 1) ++n_CSC;
            if (htype == 2) ++n_RPC;
            if (htype == 3) ++n_GEM;
        }
        int npu = mt->nPU; // the in-time PU
        // if (npu < 150 || npu > 170) continue;

        // 1D histo
        h_HitMult_CSC->Fill(n_CSC);
        h_HitMult_RPC->Fill(n_RPC);
        h_HitMult_GEM->Fill(n_GEM);
        // 2D histo
        h2D_HitMult_CSC->Fill(npu, n_CSC);
        h2D_HitMult_RPC->Fill(npu, n_RPC);
        h2D_HitMult_GEM->Fill(npu, n_GEM);
        // Profile
        hProfile_HitMult_CSC->Fill(npu, n_CSC);
        hProfile_HitMult_RPC->Fill(npu, n_RPC);
        hProfile_HitMult_GEM->Fill(npu, n_GEM);

        ////// evaluating the rate
        // my_split << mt->vp_size << endl;
        std::vector<float> v_pt_all;
        std::vector<float> v_pt_true;
        std::vector<float> v_pt_true_missingInfo;
        std::vector<float> v_pt_true_NomissingInfo;

        for (uint itrack = 0; itrack < mt->vt_size; ++itrack)
        {
            float pt = mt->vt_pt->at(itrack);
            v_pt_all.push_back(pt);
            int nMissingInfoHits = 0;
            int nTotalHits = 0;
            int matchedGenTrk = -1;
            bool sameTrkPart = allFromSameParticle(itrack, &(*mt), nMissingInfoHits, nTotalHits, matchedGenTrk);
            // my_split << "Yo 1" << endl;
            trksMissingHitsInfo.at(nTotalHits).at(nMissingInfoHits) += 1; // by definition, I have at most 4 "no info" hits
            // my_split << "Yo 2" << endl;
            // if (sameTrkPart)
            // {
            //     float trueTrkPt = mt->vp_pt->at(matchedGenTrk);

            //     v_pt_true.push_back(pt);
            //     h_PtRes_true->Fill(pt/trueTrkPt)

            //     if (missingInfo)
            //     {
            //         v_pt_true_missingInfo.push_back(pt);
            //     }
            //     else
            //     {
            //         v_pt_true_NomissingInfo.push_back(pt);
            //     }
            // }

            // fill the vectors
            if (sameTrkPart)
                v_pt_true.push_back(pt);
            if (sameTrkPart && nMissingInfoHits > 0){
                v_pt_true_missingInfo.push_back(pt);
            }
            if (sameTrkPart && nMissingInfoHits == 0){
                v_pt_true_NomissingInfo.push_back(pt);
            }

            // fill the pt resolution histos
            if (sameTrkPart && matchedGenTrk >= 0) // need to avoid cases where all hits have trk = -1 (i.e. none is matched)
            {
                float trueTrkPt = mt->vp_pt->at(matchedGenTrk);
                float resol = (trueTrkPt != 0 ? pt/trueTrkPt : 0.0);
                h_PtRes_true->Fill(resol);
                h2D_PtRes_true->Fill(trueTrkPt, resol);
                h2D_Pt_EMTF_vs_true->Fill(trueTrkPt, pt);
                if (nMissingInfoHits > 0){
                    h_PtRes_true_missingInfo->Fill(resol);
                    h2D_PtRes_true_missingInfo->Fill(trueTrkPt, resol);
                    h2D_Pt_EMTF_vs_true_missingInfo->Fill(trueTrkPt, pt);
                }
                else{
                    h_PtRes_true_NomissingInfo->Fill(resol);
                    h2D_PtRes_true_NomissingInfo->Fill(trueTrkPt, resol);
                    h2D_Pt_EMTF_vs_true_NomissingInfo->Fill(trueTrkPt, pt);
                }
            }


        }

        sort(v_pt_all.begin(), v_pt_all.end());
        sort(v_pt_true.begin(), v_pt_true.end());
        sort(v_pt_true_missingInfo.begin(), v_pt_true_missingInfo.end());
        sort(v_pt_true_NomissingInfo.begin(), v_pt_true_NomissingInfo.end());

        h_PtLead_all->Fill( v_pt_all.size() > 0 ? v_pt_all.back() : -1);  
        h_PtLead_true->Fill( v_pt_true.size() > 0 ? v_pt_true.back() : -1);  
        h_PtLead_true_missingInfo->Fill( v_pt_true_missingInfo.size() > 0 ? v_pt_true_missingInfo.back() : -1);  
        h_PtLead_true_NomissingInfo->Fill( v_pt_true_NomissingInfo.size() > 0 ? v_pt_true_NomissingInfo.back() : -1);  
    }

    // making rate plots
    makeRatePlot(h_PtLead_all);
    makeRatePlot(h_PtLead_true);
    makeRatePlot(h_PtLead_true_missingInfo);
    makeRatePlot(h_PtLead_true_NomissingInfo);

    my_split << "** INFO: loop finished" << endl;
    my_split << "  -- Inclusive events accepted " << h_PtLead_all->Integral() << endl;
    my_split << "  -- Single gen track events accepted " << h_PtLead_true->Integral() << endl;
    my_split << "  -- Single gen track events accepted including missing MC info tracks " << h_PtLead_true_missingInfo->Integral() << endl;
    my_split << "  -- Single gen track events accepted excluding missing MC info tracks " << h_PtLead_true_NomissingInfo->Integral() << endl;
    my_split << endl;
    my_split << "  ----- Number of tracks with no info on N hits:" << endl;
    my_split << "              " << 
        setw(10) << "0 miss"     << 
        setw(10) << "1 miss"     << 
        setw(10) << "2 miss"     << 
        setw(10) << "3 miss"     << 
        setw(10) << "4 miss"     << 
        endl;
    
    for (uint ntotHits = 0; ntotHits < trksMissingHitsInfo.size(); ++ntotHits)
    {
        my_split << "nTothits: " << ntotHits << "  ";
        for (uint nMiss = 0; nMiss < trksMissingHitsInfo.at(ntotHits).size(); ++nMiss)
        {
            string toprint = (nMiss > ntotHits ? string("-") : to_string(trksMissingHitsInfo.at(ntotHits).at(nMiss))); // set a dash if entry is unphysical (more missing hits than in the track)
            my_split << setw(10) << toprint;
        }
        my_split << endl;
    }
    my_split.flush();
    my_split.close();


    fOut->Write();
}