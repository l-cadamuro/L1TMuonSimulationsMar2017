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

// c++ -lm -o makePlots makePlots.cpp `root-config --glibs --cflags` -lboost_program_options


class trkPlotCollection{
    
    public:
        struct trkInfo
        {
            int iTrk;
            int iGenTrk;
        };

        // functions
        trkPlotCollection(std::string name);
        ~trkPlotCollection(){};
        // void fillPlots(MuonTree* mt, int iTrk);
        void fillPlots(MuonTree* mt, trkInfo info);
        // std::tuple<int,int,int,int,int,int,int,int,int,int,int,int> calcAngles(MuonTree* mt, int iTrk); //6 x dtheta + 6 x dphi, 12-23-34-13-14-24
        int deltaPhi(MuonTree* mt, int ihitA, int ihitB);
        int deltaTheta(MuonTree* mt, int ihitA, int ihitB);
        int nHitsInSectorRingStation(MuonTree* mt, int sector, int ring, int station); // if -1, inclusive over all possible values
        void saveToFile(TFile* fOut);

        // member data
        std::string name_;

        std::unique_ptr<TH1D> h_dtheta12_ ;
        std::unique_ptr<TH1D> h_dtheta23_ ;
        std::unique_ptr<TH1D> h_dtheta34_ ;
        std::unique_ptr<TH1D> h_dtheta13_ ;
        std::unique_ptr<TH1D> h_dtheta14_ ;
        std::unique_ptr<TH1D> h_dtheta24_ ;

        std::unique_ptr<TH1D> h_dphi12_ ;
        std::unique_ptr<TH1D> h_dphi23_ ;
        std::unique_ptr<TH1D> h_dphi34_ ;
        std::unique_ptr<TH1D> h_dphi13_ ;
        std::unique_ptr<TH1D> h_dphi14_ ;
        std::unique_ptr<TH1D> h_dphi24_ ;
        
        std::unique_ptr<TH1D> h_nhits_sec_s1_ ;
        std::unique_ptr<TH1D> h_nhits_sec_s2_ ;
        std::unique_ptr<TH1D> h_nhits_sec_s3_ ;
        std::unique_ptr<TH1D> h_nhits_sec_s4_ ;

        std::unique_ptr<TH1D> h_nhits_around_s1_ ;
        std::unique_ptr<TH1D> h_nhits_around_s2_ ;
        std::unique_ptr<TH1D> h_nhits_around_s3_ ;
        std::unique_ptr<TH1D> h_nhits_around_s4_ ;

        std::unique_ptr<TH1D> h_abseta_ ;
        std::unique_ptr<TH1D> h_mode_ ;
        
        std::unique_ptr<TH1D> h_ptresol_ ;
        std::unique_ptr<TH1D> h_ptresol_xml_ ;
};

trkPlotCollection::trkPlotCollection(std::string name)
{
    name_ = name;

    h_dtheta12_ = std::unique_ptr<TH1D>(new TH1D(Form("%s_dtheta12", name_.c_str()) , ";#Delta#theta_{12}; events", 41, -20.5, 20.5));
    h_dtheta23_ = std::unique_ptr<TH1D>(new TH1D(Form("%s_dtheta23", name_.c_str()) , ";#Delta#theta_{23}; events", 41, -20.5, 20.5));
    h_dtheta34_ = std::unique_ptr<TH1D>(new TH1D(Form("%s_dtheta34", name_.c_str()) , ";#Delta#theta_{34}; events", 41, -20.5, 20.5));
    h_dtheta13_ = std::unique_ptr<TH1D>(new TH1D(Form("%s_dtheta13", name_.c_str()) , ";#Delta#theta_{13}; events", 41, -20.5, 20.5));
    h_dtheta14_ = std::unique_ptr<TH1D>(new TH1D(Form("%s_dtheta14", name_.c_str()) , ";#Delta#theta_{14}; events", 41, -20.5, 20.5));
    h_dtheta24_ = std::unique_ptr<TH1D>(new TH1D(Form("%s_dtheta24", name_.c_str()) , ";#Delta#theta_{24}; events", 41, -20.5, 20.5));

    h_dphi12_   = std::unique_ptr<TH1D>(new TH1D(Form("%s_dphi12", name_.c_str()) , ";#Delta#varphi_{12}; events", 61, -61, 61));
    h_dphi23_   = std::unique_ptr<TH1D>(new TH1D(Form("%s_dphi23", name_.c_str()) , ";#Delta#varphi_{23}; events", 61, -61, 61));
    h_dphi34_   = std::unique_ptr<TH1D>(new TH1D(Form("%s_dphi34", name_.c_str()) , ";#Delta#varphi_{34}; events", 61, -61, 61));
    h_dphi13_   = std::unique_ptr<TH1D>(new TH1D(Form("%s_dphi13", name_.c_str()) , ";#Delta#varphi_{13}; events", 61, -61, 61));
    h_dphi14_   = std::unique_ptr<TH1D>(new TH1D(Form("%s_dphi14", name_.c_str()) , ";#Delta#varphi_{14}; events", 61, -61, 61));
    h_dphi24_   = std::unique_ptr<TH1D>(new TH1D(Form("%s_dphi24", name_.c_str()) , ";#Delta#varphi_{24}; events", 61, -61, 61));

    h_nhits_sec_s1_   = std::unique_ptr<TH1D>(new TH1D(Form("%s_nhits_sec_s1", name_.c_str()) , ";Number of hits in sector for S1; events", 30, 0, 30));
    h_nhits_sec_s2_   = std::unique_ptr<TH1D>(new TH1D(Form("%s_nhits_sec_s2", name_.c_str()) , ";Number of hits in sector for S2; events", 30, 0, 30));
    h_nhits_sec_s3_   = std::unique_ptr<TH1D>(new TH1D(Form("%s_nhits_sec_s3", name_.c_str()) , ";Number of hits in sector for S3; events", 30, 0, 30));
    h_nhits_sec_s4_   = std::unique_ptr<TH1D>(new TH1D(Form("%s_nhits_sec_s4", name_.c_str()) , ";Number of hits in sector for S4; events", 30, 0, 30));

    h_nhits_around_s1_   = std::unique_ptr<TH1D>(new TH1D(Form("%s_nhits_around_s1", name_.c_str()) , ";Number of hits around selected one for S1; events", 30, 0, 30));
    h_nhits_around_s2_   = std::unique_ptr<TH1D>(new TH1D(Form("%s_nhits_around_s2", name_.c_str()) , ";Number of hits around selected one for S2; events", 30, 0, 30));
    h_nhits_around_s3_   = std::unique_ptr<TH1D>(new TH1D(Form("%s_nhits_around_s3", name_.c_str()) , ";Number of hits around selected one for S3; events", 30, 0, 30));
    h_nhits_around_s4_   = std::unique_ptr<TH1D>(new TH1D(Form("%s_nhits_around_s4", name_.c_str()) , ";Number of hits around selected one for S4; events", 30, 0, 30));

    h_abseta_   = std::unique_ptr<TH1D>(new TH1D(Form("%s_abseta", name_.c_str()) , ";track |#eta|; events", 40, 1, 2.5));
    h_mode_     = std::unique_ptr<TH1D>(new TH1D(Form("%s_mode",   name_.c_str()) , ";track mode; events", 16, 0, 16));

    h_ptresol_      = std::unique_ptr<TH1D>(new TH1D(Form("%s_ptresol",     name_.c_str()) , ";p_{T}^{EMTF} / p_{T}^{gen}; events", 100, 0, 5));
    h_ptresol_xml_  = std::unique_ptr<TH1D>(new TH1D(Form("%s_ptresol_xml", name_.c_str()) , ";p_{T}^{EMTF, unscaled} / p_{T}^{gen}; events", 100, 0, 5));


}

void trkPlotCollection::fillPlots(MuonTree* mt, trkInfo info)
{
    int iTrk     = info.iTrk;
    int iGenTrk  = info.iGenTrk;
    int sector   = mt->vt_sector->at(iTrk);
    float pt     = mt->vt_pt->at(iTrk);
    float xml_pt = mt->vt_xml_pt->at(iTrk);
    float genpt  = (iGenTrk >= 0 ? mt->vp_pt->at(iGenTrk) : -1);


    int hitref1 = mt->vt_hitref1->at(iTrk);
    int hitref2 = mt->vt_hitref2->at(iTrk);
    int hitref3 = mt->vt_hitref3->at(iTrk);
    int hitref4 = mt->vt_hitref4->at(iTrk);

    int dtheta12 = deltaTheta(mt, hitref1, hitref2);
    int dtheta23 = deltaTheta(mt, hitref2, hitref3);
    int dtheta34 = deltaTheta(mt, hitref3, hitref4);
    int dtheta13 = deltaTheta(mt, hitref1, hitref3);
    int dtheta14 = deltaTheta(mt, hitref1, hitref4);
    int dtheta24 = deltaTheta(mt, hitref2, hitref4);

    int dphi12 = deltaPhi(mt, hitref1, hitref2);
    int dphi23 = deltaPhi(mt, hitref2, hitref3);
    int dphi34 = deltaPhi(mt, hitref3, hitref4);
    int dphi13 = deltaPhi(mt, hitref1, hitref3);
    int dphi14 = deltaPhi(mt, hitref1, hitref4);
    int dphi24 = deltaPhi(mt, hitref2, hitref4);

    h_dtheta12_ -> Fill(dtheta12);
    h_dtheta23_ -> Fill(dtheta23);
    h_dtheta34_ -> Fill(dtheta34);
    h_dtheta13_ -> Fill(dtheta13);
    h_dtheta14_ -> Fill(dtheta14);
    h_dtheta24_ -> Fill(dtheta24);

    h_dphi12_ -> Fill(dphi12);
    h_dphi23_ -> Fill(dphi23);
    h_dphi34_ -> Fill(dphi34);
    h_dphi13_ -> Fill(dphi13);
    h_dphi14_ -> Fill(dphi14);
    h_dphi24_ -> Fill(dphi24);

    h_nhits_sec_s1_ -> Fill(nHitsInSectorRingStation(mt, sector, -1, 1));
    h_nhits_sec_s2_ -> Fill(nHitsInSectorRingStation(mt, sector, -1, 2));
    h_nhits_sec_s3_ -> Fill(nHitsInSectorRingStation(mt, sector, -1, 3));
    h_nhits_sec_s4_ -> Fill(nHitsInSectorRingStation(mt, sector, -1, 4));

    h_nhits_around_s1_ -> Fill(-1); // FIXME: to do 
    h_nhits_around_s2_ -> Fill(-1); // FIXME: to do 
    h_nhits_around_s3_ -> Fill(-1); // FIXME: to do 
    h_nhits_around_s4_ -> Fill(-1); // FIXME: to do 

    h_abseta_ -> Fill(std::abs(mt->vt_eta->at(iTrk)));
    h_mode_   -> Fill(mt->vt_mode->at(iTrk));

    float ptresol     = (genpt > 0 ?     pt/genpt : -1); // use "scaled" pt
    float ptresol_xml = (genpt > 0 ? xml_pt/genpt : -1); // use "unscaled" xml pt
    h_ptresol_     -> Fill(ptresol);
    h_ptresol_xml_ -> Fill(ptresol_xml);
}

void trkPlotCollection::saveToFile(TFile* fOut)
{
    fOut->cd();

    h_dtheta12_ -> Write();
    h_dtheta23_ -> Write();
    h_dtheta34_ -> Write();
    h_dtheta13_ -> Write();
    h_dtheta14_ -> Write();
    h_dtheta24_ -> Write();
    
    h_dphi12_ -> Write();
    h_dphi23_ -> Write();
    h_dphi34_ -> Write();
    h_dphi13_ -> Write();
    h_dphi14_ -> Write();
    h_dphi24_ -> Write();

    h_nhits_sec_s1_ -> Write();
    h_nhits_sec_s2_ -> Write();
    h_nhits_sec_s3_ -> Write();
    h_nhits_sec_s4_ -> Write();

    h_nhits_around_s1_ -> Write();
    h_nhits_around_s2_ -> Write();
    h_nhits_around_s3_ -> Write();
    h_nhits_around_s4_ -> Write();
    
    h_abseta_ -> Write();
    h_mode_   -> Write();

    h_ptresol_ -> Write();
    h_ptresol_xml_ ->Write();
}

int trkPlotCollection::nHitsInSectorRingStation(MuonTree* mt, int sector, int ring, int station)
{
    int nhits = 0;
    for (uint ihit = 0; ihit < mt->vh_size; ++ihit)
    {
        int h_sector  = mt->vh_sector  ->at(ihit);
        int h_ring    = mt->vh_ring    ->at(ihit);
        int h_station = mt->vh_station ->at(ihit);

        bool pass_sector  = (h_sector  == sector  || sector  == -1);
        bool pass_ring    = (h_ring    == ring    || ring    == -1);
        bool pass_station = (h_station == station || station == -1);

        if (pass_sector && pass_ring && pass_station)
            ++nhits;
    }
    return nhits;
}

// std::tuple<int,int,int,int,int,int,int,int,int,int,int,int> trkPlotCollection::calcAngles(MuonTree* mt, int iTrk)
// {
//     int hitref1 = mt->vt_hitref1->at(iTrk);
//     int hitref2 = mt->vt_hitref2->at(iTrk);
//     int hitref3 = mt->vt_hitref3->at(iTrk);
//     int hitref4 = mt->vt_hitref4->at(iTrk);

//     int dtheta12 = deltaTheta(mt, hitref1, hitref2);
//     int dtheta23 = deltaTheta(mt, hitref2, hitref3);
//     int dtheta34 = deltaTheta(mt, hitref3, hitref4);
//     int dtheta13 = deltaTheta(mt, hitref1, hitref3);
//     int dtheta14 = deltaTheta(mt, hitref1, hitref4);
//     int dtheta24 = deltaTheta(mt, hitref2, hitref4);

//     int dphi12 = deltaTheta(mt, hitref1, hitref2);
//     int dphi23 = deltaTheta(mt, hitref2, hitref3);
//     int dphi34 = deltaTheta(mt, hitref3, hitref4);
//     int dphi13 = deltaTheta(mt, hitref1, hitref3);
//     int dphi14 = deltaTheta(mt, hitref1, hitref4);
//     int dphi24 = deltaTheta(mt, hitref2, hitref4);
// }

int trkPlotCollection::deltaPhi(MuonTree* mt, int ihitA, int ihitB)
{
    if (ihitA < 0 || ihitB < 0) return -999;
    int phiA = mt->vh_emtf_phi->at(ihitA);
    int phiB = mt->vh_emtf_phi->at(ihitB);
    return (phiB - phiA);
}

int trkPlotCollection::deltaTheta(MuonTree* mt, int ihitA, int ihitB)
{
    if (ihitA < 0 || ihitB < 0) return -999;
    int thetaA = mt->vh_emtf_theta->at(ihitA);
    int thetaB = mt->vh_emtf_theta->at(ihitB);
    return (thetaB - thetaA);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class MCmatch{

    public:
        enum status {
            kNoMissInfoOneTrk,
            kNoMissInfoMoreTrk,
            kMissInfoZeroTrk,
            kMissInfoOneTrk,
            kMissInfoMoreTrk,
            kError,
            kTOTAL
        };

        MCmatch(){};
        ~MCmatch(){};
        std::pair<status,int> isTrkMatched(MuonTree* mt, int iTrk); //<match code, matched gen trk idx>
};


std::pair<MCmatch::status,int> MCmatch::isTrkMatched(MuonTree* mt, int iTrk)
{

    // get the hits associated to this track
    std::vector<int> idxHits = {-1, -1, -1, -1};
    idxHits.at(0) = mt->vt_hitref1->at(iTrk); // S1
    idxHits.at(1) = mt->vt_hitref2->at(iTrk); // S2
    idxHits.at(2) = mt->vt_hitref3->at(iTrk); // S3
    idxHits.at(3) = mt->vt_hitref4->at(iTrk); // S4

    if (DEBUG) cout << " -- MCmatch -- Listing trk num " << iTrk << " hits " << endl;
    std::vector<int> idxTracks_tp1 = {-1, -1, -1, -1};
    std::vector<int> idxTracks_tp2 = {-1, -1, -1, -1};
    // nTotalHits = 0;
    
    // if idxTrack >= 0 : I have the link to the tracking particle
    // if idxTrack == -1 : there was no tracking particle linked
    // if idxTrack == -999 : there was no hit at all

    for (uint itr = 0; itr < idxHits.size(); ++itr)
    {
        int ihit = idxHits.at(itr);
        idxTracks_tp1.at(itr) = (ihit == -1 ? -999 : mt->vh_sim_tp1->at(ihit));
        idxTracks_tp2.at(itr) = (ihit == -1 ? -999 : mt->vh_sim_tp2->at(ihit));
        // if (ihit != -1) ++nTotalHits;
        if (DEBUG) cout << " ---- hit " << itr << " -- ihit " << setw(2) << ihit << " -- tp1 " << setw(5) << idxTracks_tp1.at(itr) << " -- tp2 " << setw(5) << idxTracks_tp2.at(itr) << endl;
    }

    std::vector<int> v_all_unique;     // all hits refs, excluding -999 (not in the EMTF track)
    std::vector<int> v_all_unique_pos; // all hits refs, positive only

    for (uint idx = 0; idx < idxTracks_tp1.size(); ++idx)
    {
        int tp1 = idxTracks_tp1.at(idx) ;
        int tp2 = idxTracks_tp2.at(idx) ;
        
        if (tp1 >= -1) v_all_unique.push_back(tp1);
        if (tp2 >= -1) v_all_unique.push_back(tp2);

        if (tp1 >= 0)  v_all_unique_pos.push_back(tp1);
        if (tp2 >= 0)  v_all_unique_pos.push_back(tp2);
    }

    // reduce to a set of unique elements
    sort(v_all_unique.begin(), v_all_unique.end());
    sort(v_all_unique_pos.begin(), v_all_unique_pos.end());

    v_all_unique.erase(unique(v_all_unique.begin(), v_all_unique.end()), v_all_unique.end());
    v_all_unique_pos.erase(unique(v_all_unique_pos.begin(), v_all_unique_pos.end()), v_all_unique_pos.end());

    if (v_all_unique.size() == 0)
    {
        cout << " ** ERROR: MCmatch::trkMatched : no hits found in this track with index " << iTrk << endl;
        return make_pair(kError, -1);
    }

    // return based on the hit multiplicity
    if (DEBUG){
        cout << " v_all_unique.size() = " << v_all_unique.size() << " , v_all_unique_pos.size() = " << v_all_unique_pos.size() << endl;
        cout << "  v_all_unique : ";     for (auto& el: v_all_unique)      std::cout << el << ' '; std::cout << endl;
        cout << "  v_all_unique_pos : "; for (auto& el: v_all_unique_pos)  std::cout << el << ' '; std::cout << endl;
    }

    if (v_all_unique.size() == v_all_unique_pos.size()) // no missing info
    {
        if (v_all_unique_pos.size() == 1){
            if (DEBUG) cout << "  >> returning kNoMissInfoOneTrk = " << kNoMissInfoOneTrk << " , itrk " << v_all_unique_pos.at(0) << endl;
            return make_pair(kNoMissInfoOneTrk, v_all_unique_pos.at(0));
        }
        else{
            if (DEBUG) cout << "  >> returning kNoMissInfoMoreTrk = " << kNoMissInfoMoreTrk << " , itrk " << -1 << endl;
            return make_pair(kNoMissInfoMoreTrk, -1);
        }
    }

    else // there is missing info
    {
        if (v_all_unique_pos.size() == 0){
            if (DEBUG) cout << "  >> returning kMissInfoZeroTrk = " << kMissInfoZeroTrk << " , itrk " << -1 << endl;
            return make_pair(kMissInfoZeroTrk, -1);
        }
        else if (v_all_unique_pos.size() == 1){
            if (DEBUG) cout << "  >> returning kMissInfoOneTrk = " << kMissInfoOneTrk << " , itrk " << v_all_unique_pos.at(0) << endl;
            return make_pair(kMissInfoOneTrk, v_all_unique_pos.at(0));
        }
        else{
            if (DEBUG) cout << "  >> returning kMissInfoMoreTrk = " << kMissInfoMoreTrk << " , itrk " << -1 << endl;
            return make_pair(kMissInfoMoreTrk, -1);
        }
    }

}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// void dumpTracks (MuonTree* mt, TeeStream& my_split)
void dumpTracks (MuonTree* mt, std::ostream& my_split)
{
    my_split << "** TRACKS INFO ** " << " .. tot hits " << mt->vh_size << " .. tot tracks " << mt->vt_size << endl;
    for (uint itrack = 0; itrack < mt->vt_size; ++itrack)
    {
        short mode = mt->vt_mode->at(itrack);
        bool passSingleMu = (mode == 11 || mode == 13 || mode == 14 || mode == 15);

        my_split << " Trk " << itrack
                 << " - single mu qual? " << std::boolalpha << passSingleMu << " "
                 << " pt = " << mt->vt_pt->at(itrack)
                 << " , xmlpt = " << mt->vt_xml_pt->at(itrack)
                 << " " ;
        
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
        my_split << "   - " << itrack << " " <<  mt->vt_sector->at(itrack) << " " <<  mt->vt_mode->at(itrack) << " " <<  mt->vt_pt->at(itrack) << " " <<  mt->vt_phi->at(itrack) << " " <<  mt->vt_eta->at(itrack) << " " <<  mt->vt_theta->at(itrack) << " " <<  mt->vt_q->at(itrack) << endl;
    }
    my_split << endl;
}



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


int main (int argc, char** argv)
{

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
        ("filelist", po::value<string>()->default_value("JiaFuNtuples.txt"), "input file list")
        ("output",   po::value<string>()->default_value("plots_EMTF_PhaseI.root"), "output file name")
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


    /////////////////////////////////////////////////////////////////////////////////////////

    TChain * bigChain = new TChain ("ntupler/tree") ;
    // bigChain->Add("/uscms/home/lcadamur/nobackup/JiaFu_Ntuples_EMTF_PhaseI_12_12_2017/rateplots_mc_r305310_run2_all.0.root");
    // string filelist = "JiaFuNtuples.txt";
    cout << "** INFO: opening files from " << filelist << endl;
    appendFromFileList (bigChain, filelist);
    std::unique_ptr<MuonTree> mt (new MuonTree(bigChain));

    // std::string outputname = "plots_EMTF_PhaseI.root";
    cout << "** INFO: output file is " << outputname << endl;
    TFile* fOut = new TFile(outputname.c_str(), "recreate");

    cout << "** INFO: creating plot containers" << endl;

    trkPlotCollection plots_allTrks ("all");
    trkPlotCollection plots_allTrks_wi_match ("MC_wi_match");
    trkPlotCollection plots_allTrks_no_match ("MC_no_match");

    trkPlotCollection plots_allTrks_gt16 ("all_gt16");
    trkPlotCollection plots_allTrks_wi_match_gt16 ("MC_wi_match_gt16");
    trkPlotCollection plots_allTrks_no_match_gt16 ("MC_no_match_gt16");

    // reproducing Jia Fu classification for synch
    trkPlotCollection plots_allTrks_wi_match_gt16_singleMu_ptresol40     ("MC_wi_match_gt16_singleMu_ptresol40");         // matched and within 40% resolution
    trkPlotCollection plots_allTrks_no_match_gt16_singleMu_ptresol40_not ("MC_no_match_gt16_singleMu_ptresol40_not");     // failing previous one
    trkPlotCollection plots_allTrks_no_match_gt16_singleMu_ptresol40_and ("MC_no_match_gt16_singleMu_ptresol40_and");     // unmatched/missing, AND outside pT resolution
    trkPlotCollection plots_allTrks_no_match_gt16_singleMu_ptresol40_or  ("MC_no_match_gt16_singleMu_ptresol40_or");      // unmatched/missing, OR outside 40% resolution

    trkPlotCollection plots_JiaFuSync     ("MC_JiaFu_sync");         // same selection of passing as Jia Fu for synch

    MCmatch mcm;

    cout << "** INFO: starting loop on events" << endl;
    if (offset > 0)
        cout << "** INFO: skipping first " << offset << " events" << endl;
    if (maxEvts >= 0)
        cout << "** INFO: limiting to up to " << maxEvts << " events" << endl;

    // uint offset = 0;
    for (uint iEv = offset; true; ++iEv)
    {
        if (iEv % 10000 == 0 || DEBUG)
            cout << "... processing " << iEv << endl;

        if (maxEvts >= 0 && iEv >= (maxEvts+offset))
            break;

        int got = mt->GetEntry(iEv);
        if (!got) break;

        for (uint iTrk = 0; iTrk < mt->vt_size; ++iTrk)
        {
            std::pair<MCmatch::status, int> match = mcm.isTrkMatched(mt.get(), iTrk);
            MCmatch::status st = match.first;

            trkPlotCollection::trkInfo info;
            info.iTrk = iTrk;
            info.iGenTrk = match.second;
            
            if (DEBUG) dumpTracks(mt.get(), std::cout);

            float pt     = mt->vt_pt->at(iTrk);     // the one sent to uGMT
            float xml_pt = mt->vt_xml_pt->at(iTrk); // the one with the scale as the gen particle

            float eta = mt->vt_eta->at(iTrk); // pt or xml pt ? they are largely different

            short mode = mt->vt_mode->at(iTrk);
            bool passSingleMu = (mode == 11 || mode == 13 || mode == 14 || mode == 15);

            // most inclusive
            plots_allTrks.fillPlots(mt.get(), info);
            if (st == MCmatch::kNoMissInfoOneTrk)
                plots_allTrks_wi_match.fillPlots(mt.get(), info);
            else
                plots_allTrks_no_match.fillPlots(mt.get(), info);

            // pt restriction
            if (pt >= 16)
            {
                plots_allTrks_gt16.fillPlots(mt.get(), info);            
                if (st == MCmatch::kNoMissInfoOneTrk)
                    plots_allTrks_wi_match_gt16.fillPlots(mt.get(), info);
                else
                    plots_allTrks_no_match_gt16.fillPlots(mt.get(), info);
            }

            // other quality restrictions
            if (pt >= 16 && info.iGenTrk >= 0 && passSingleMu)
            {
                float genTrkPt = mt->vp_pt->at(info.iGenTrk);
                // float resol = (genTrkPt > 0 ? xml_pt/genTrkPt : 0);
                // resol = resol - 1.0; // relative
                float resol = std::abs(xml_pt - genTrkPt) / genTrkPt;
                if (st == MCmatch::kNoMissInfoOneTrk && std::abs(resol) < 0.4)
                    plots_allTrks_wi_match_gt16_singleMu_ptresol40.fillPlots(mt.get(), info);
                else
                    plots_allTrks_no_match_gt16_singleMu_ptresol40_not.fillPlots(mt.get(), info);
                ///
                if (st != MCmatch::kNoMissInfoOneTrk && std::abs(resol) > 0.4)
                    plots_allTrks_no_match_gt16_singleMu_ptresol40_and.fillPlots(mt.get(), info);
                if (st != MCmatch::kNoMissInfoOneTrk || std::abs(resol) > 0.4)
                    plots_allTrks_no_match_gt16_singleMu_ptresol40_or.fillPlots(mt.get(), info);
            }

            // Jia Fu Synch
            if (DEBUG) cout << "pt >= 16 ? " << (pt >= 16) << " "
                            << "std::abs(eta) > 0 ? " << (std::abs(eta) > 0) << " "
                            << "std::abs(eta) < 2.5 ? " << (std::abs(eta) < 2.5) << " "
                            << "passSingleMu ? " << (passSingleMu) << " "
                            << "st == MCmatch::kNoMissInfoOneTrk ? "<< (st == MCmatch::kNoMissInfoOneTrk) << " "
                            << endl;
            if (pt >= 16 && std::abs(eta) > 0 && std::abs(eta) < 2.5 && passSingleMu && st == MCmatch::kNoMissInfoOneTrk)
            {
                float genTrkPt = mt->vp_pt->at(info.iGenTrk);
                float resol = std::abs(xml_pt - genTrkPt) / genTrkPt;
                if (resol < 0.4){
                    plots_JiaFuSync.fillPlots(mt.get(), info);
                    if (DEBUG) cout << " @@@ this enters the good selected evts" << endl;
                    // cout << "  >>>>> Filling as real event number  " << iEv << endl;
                }
            }

        }
    }

    ////////////////// save the plots to a file

    plots_allTrks.saveToFile(fOut);
    plots_allTrks_wi_match.saveToFile(fOut);
    plots_allTrks_no_match.saveToFile(fOut);

    plots_allTrks_gt16.saveToFile(fOut);
    plots_allTrks_wi_match_gt16.saveToFile(fOut);
    plots_allTrks_no_match_gt16.saveToFile(fOut);

    plots_allTrks_wi_match_gt16_singleMu_ptresol40.saveToFile(fOut);
    plots_allTrks_no_match_gt16_singleMu_ptresol40_not.saveToFile(fOut);
    plots_allTrks_no_match_gt16_singleMu_ptresol40_and.saveToFile(fOut);
    plots_allTrks_no_match_gt16_singleMu_ptresol40_or.saveToFile(fOut);

    plots_JiaFuSync.saveToFile(fOut);
}