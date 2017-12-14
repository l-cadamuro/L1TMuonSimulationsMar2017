//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Fri Dec  1 14:16:30 2017 by ROOT version 6.08/07
// from TTree tree/tree
// found on file: /eos/uscms/store/user/lcadamur/EMTF_ntuples/SingleNeutrino_PU200_30Nov2017/output/ntuple_36.root
//////////////////////////////////////////////////////////

#ifndef MuonTree_h
#define MuonTree_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// Header file for the classes stored in the TTree if any.
#include "vector"

class MuonTree {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

// Fixed size dimensions of array or collections stored in the TTree if any.

   // Declaration of leaf types
   Int_t           nPU;
   Float_t         nPU_true;
   vector<short>   *vh_endcap;
   vector<short>   *vh_station;
   vector<short>   *vh_ring;
   vector<short>   *vh_sector;
   vector<short>   *vh_subsector;
   vector<short>   *vh_chamber;
   vector<short>   *vh_cscid;
   vector<short>   *vh_bx;
   vector<short>   *vh_type;
   vector<short>   *vh_neighbor;
   vector<short>   *vh_zonecode;
   vector<short>   *vh_zonehit;
   vector<short>   *vh_strip;
   vector<short>   *vh_wire;
   vector<short>   *vh_roll;
   vector<short>   *vh_pattern;
   vector<short>   *vh_quality;
   vector<short>   *vh_bend;
   vector<short>   *vh_time;
   vector<short>   *vh_fr;
   vector<int>     *vh_emtf_phi;
   vector<int>     *vh_emtf_theta;
   vector<float>   *vh_sim_phi;
   vector<float>   *vh_sim_theta;
   vector<float>   *vh_sim_eta;
   vector<float>   *vh_sim_r;
   vector<float>   *vh_sim_z;
   vector<int>     *vh_sim_tp1;
   vector<int>     *vh_sim_tp2;
   Int_t           vh_size;
   vector<float>   *vt_pt;
   vector<float>   *vt_xml_pt;
   vector<float>   *vt_phi;
   vector<float>   *vt_eta;
   vector<float>   *vt_theta;
   vector<short>   *vt_q;
   vector<short>   *vt_mode;
   vector<short>   *vt_endcap;
   vector<short>   *vt_sector;
   vector<short>   *vt_bx;
   Int_t           vt_size;
   vector<float>   *vp_pt;
   vector<float>   *vp_phi;
   vector<float>   *vp_eta;
   vector<float>   *vp_theta;
   vector<float>   *vp_vx;
   vector<float>   *vp_vy;
   vector<float>   *vp_vz;
   vector<short>   *vp_q;
   vector<int>     *vp_pdgid;
   Int_t           vp_size;

   // List of branches
   TBranch        *b_nPU;   //!
   TBranch        *b_nPU_true;   //!
   TBranch        *b_vh_endcap;   //!
   TBranch        *b_vh_station;   //!
   TBranch        *b_vh_ring;   //!
   TBranch        *b_vh_sector;   //!
   TBranch        *b_vh_subsector;   //!
   TBranch        *b_vh_chamber;   //!
   TBranch        *b_vh_cscid;   //!
   TBranch        *b_vh_bx;   //!
   TBranch        *b_vh_type;   //!
   TBranch        *b_vh_neighbor;   //!
   TBranch        *b_vh_zonecode;   //!
   TBranch        *b_vh_zonehit;   //!
   TBranch        *b_vh_strip;   //!
   TBranch        *b_vh_wire;   //!
   TBranch        *b_vh_roll;   //!
   TBranch        *b_vh_pattern;   //!
   TBranch        *b_vh_quality;   //!
   TBranch        *b_vh_bend;   //!
   TBranch        *b_vh_time;   //!
   TBranch        *b_vh_fr;   //!
   TBranch        *b_vh_emtf_phi;   //!
   TBranch        *b_vh_emtf_theta;   //!
   TBranch        *b_vh_sim_phi;   //!
   TBranch        *b_vh_sim_theta;   //!
   TBranch        *b_vh_sim_eta;   //!
   TBranch        *b_vh_sim_r;   //!
   TBranch        *b_vh_sim_z;   //!
   TBranch        *b_vh_sim_tp1;   //!
   TBranch        *b_vh_sim_tp2;   //!
   TBranch        *b_vh_size;   //!
   TBranch        *b_vt_pt;   //!
   TBranch        *b_vt_xml_pt;   //!
   TBranch        *b_vt_phi;   //!
   TBranch        *b_vt_eta;   //!
   TBranch        *b_vt_theta;   //!
   TBranch        *b_vt_q;   //!
   TBranch        *b_vt_mode;   //!
   TBranch        *b_vt_endcap;   //!
   TBranch        *b_vt_sector;   //!
   TBranch        *b_vt_bx;   //!
   TBranch        *b_vt_size;   //!
   TBranch        *b_vp_pt;   //!
   TBranch        *b_vp_phi;   //!
   TBranch        *b_vp_eta;   //!
   TBranch        *b_vp_theta;   //!
   TBranch        *b_vp_vx;   //!
   TBranch        *b_vp_vy;   //!
   TBranch        *b_vp_vz;   //!
   TBranch        *b_vp_q;   //!
   TBranch        *b_vp_pdgid;   //!
   TBranch        *b_vp_size;   //!

   MuonTree(TTree *tree=0);
   virtual ~MuonTree();
   // virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   // virtual void     Loop();
   // virtual Bool_t   Notify();
   // virtual void     Show(Long64_t entry = -1);
};

#endif

#ifdef MuonTree_cxx
MuonTree::MuonTree(TTree *tree) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("/eos/uscms/store/user/lcadamur/EMTF_ntuples/SingleNeutrino_PU200_30Nov2017/output/ntuple_36.root");
      if (!f || !f->IsOpen()) {
         f = new TFile("/eos/uscms/store/user/lcadamur/EMTF_ntuples/SingleNeutrino_PU200_30Nov2017/output/ntuple_36.root");
      }
      TDirectory * dir = (TDirectory*)f->Get("/eos/uscms/store/user/lcadamur/EMTF_ntuples/SingleNeutrino_PU200_30Nov2017/output/ntuple_36.root:/ntupler");
      dir->GetObject("tree",tree);

   }
   Init(tree);
}

MuonTree::~MuonTree()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t MuonTree::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t MuonTree::LoadTree(Long64_t entry)
{
// Set the environment to read one entry
   if (!fChain) return -5;
   Long64_t centry = fChain->LoadTree(entry);
   if (centry < 0) return centry;
   if (fChain->GetTreeNumber() != fCurrent) {
      fCurrent = fChain->GetTreeNumber();
      Notify();
   }
   return centry;
}

void MuonTree::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set object pointer
   vh_endcap = 0;
   vh_station = 0;
   vh_ring = 0;
   vh_sector = 0;
   vh_subsector = 0;
   vh_chamber = 0;
   vh_cscid = 0;
   vh_bx = 0;
   vh_type = 0;
   vh_neighbor = 0;
   vh_zonecode = 0;
   vh_zonehit = 0;
   vh_strip = 0;
   vh_wire = 0;
   vh_roll = 0;
   vh_pattern = 0;
   vh_quality = 0;
   vh_bend = 0;
   vh_time = 0;
   vh_fr = 0;
   vh_emtf_phi = 0;
   vh_emtf_theta = 0;
   vh_sim_phi = 0;
   vh_sim_theta = 0;
   vh_sim_eta = 0;
   vh_sim_r = 0;
   vh_sim_z = 0;
   vh_sim_tp1 = 0;
   vh_sim_tp2 = 0;
   vt_pt = 0;
   vt_xml_pt = 0;
   vt_phi = 0;
   vt_eta = 0;
   vt_theta = 0;
   vt_q = 0;
   vt_mode = 0;
   vt_endcap = 0;
   vt_sector = 0;
   vt_bx = 0;
   vp_pt = 0;
   vp_phi = 0;
   vp_eta = 0;
   vp_theta = 0;
   vp_vx = 0;
   vp_vy = 0;
   vp_vz = 0;
   vp_q = 0;
   vp_pdgid = 0;
   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("nPU", &nPU, &b_nPU);
   fChain->SetBranchAddress("nPU_true", &nPU_true, &b_nPU_true);
   fChain->SetBranchAddress("vh_endcap", &vh_endcap, &b_vh_endcap);
   fChain->SetBranchAddress("vh_station", &vh_station, &b_vh_station);
   fChain->SetBranchAddress("vh_ring", &vh_ring, &b_vh_ring);
   fChain->SetBranchAddress("vh_sector", &vh_sector, &b_vh_sector);
   fChain->SetBranchAddress("vh_subsector", &vh_subsector, &b_vh_subsector);
   fChain->SetBranchAddress("vh_chamber", &vh_chamber, &b_vh_chamber);
   fChain->SetBranchAddress("vh_cscid", &vh_cscid, &b_vh_cscid);
   fChain->SetBranchAddress("vh_bx", &vh_bx, &b_vh_bx);
   fChain->SetBranchAddress("vh_type", &vh_type, &b_vh_type);
   fChain->SetBranchAddress("vh_neighbor", &vh_neighbor, &b_vh_neighbor);
   fChain->SetBranchAddress("vh_zonecode", &vh_zonecode, &b_vh_zonecode);
   fChain->SetBranchAddress("vh_zonehit", &vh_zonehit, &b_vh_zonehit);
   fChain->SetBranchAddress("vh_strip", &vh_strip, &b_vh_strip);
   fChain->SetBranchAddress("vh_wire", &vh_wire, &b_vh_wire);
   fChain->SetBranchAddress("vh_roll", &vh_roll, &b_vh_roll);
   fChain->SetBranchAddress("vh_pattern", &vh_pattern, &b_vh_pattern);
   fChain->SetBranchAddress("vh_quality", &vh_quality, &b_vh_quality);
   fChain->SetBranchAddress("vh_bend", &vh_bend, &b_vh_bend);
   fChain->SetBranchAddress("vh_time", &vh_time, &b_vh_time);
   fChain->SetBranchAddress("vh_fr", &vh_fr, &b_vh_fr);
   fChain->SetBranchAddress("vh_emtf_phi", &vh_emtf_phi, &b_vh_emtf_phi);
   fChain->SetBranchAddress("vh_emtf_theta", &vh_emtf_theta, &b_vh_emtf_theta);
   fChain->SetBranchAddress("vh_sim_phi", &vh_sim_phi, &b_vh_sim_phi);
   fChain->SetBranchAddress("vh_sim_theta", &vh_sim_theta, &b_vh_sim_theta);
   fChain->SetBranchAddress("vh_sim_eta", &vh_sim_eta, &b_vh_sim_eta);
   fChain->SetBranchAddress("vh_sim_r", &vh_sim_r, &b_vh_sim_r);
   fChain->SetBranchAddress("vh_sim_z", &vh_sim_z, &b_vh_sim_z);
   fChain->SetBranchAddress("vh_sim_tp1", &vh_sim_tp1, &b_vh_sim_tp1);
   fChain->SetBranchAddress("vh_sim_tp2", &vh_sim_tp2, &b_vh_sim_tp2);
   fChain->SetBranchAddress("vh_size", &vh_size, &b_vh_size);
   fChain->SetBranchAddress("vt_pt", &vt_pt, &b_vt_pt);
   fChain->SetBranchAddress("vt_xml_pt", &vt_xml_pt, &b_vt_xml_pt);
   fChain->SetBranchAddress("vt_phi", &vt_phi, &b_vt_phi);
   fChain->SetBranchAddress("vt_eta", &vt_eta, &b_vt_eta);
   fChain->SetBranchAddress("vt_theta", &vt_theta, &b_vt_theta);
   fChain->SetBranchAddress("vt_q", &vt_q, &b_vt_q);
   fChain->SetBranchAddress("vt_mode", &vt_mode, &b_vt_mode);
   fChain->SetBranchAddress("vt_endcap", &vt_endcap, &b_vt_endcap);
   fChain->SetBranchAddress("vt_sector", &vt_sector, &b_vt_sector);
   fChain->SetBranchAddress("vt_bx", &vt_bx, &b_vt_bx);
   fChain->SetBranchAddress("vt_size", &vt_size, &b_vt_size);
   fChain->SetBranchAddress("vp_pt", &vp_pt, &b_vp_pt);
   fChain->SetBranchAddress("vp_phi", &vp_phi, &b_vp_phi);
   fChain->SetBranchAddress("vp_eta", &vp_eta, &b_vp_eta);
   fChain->SetBranchAddress("vp_theta", &vp_theta, &b_vp_theta);
   fChain->SetBranchAddress("vp_vx", &vp_vx, &b_vp_vx);
   fChain->SetBranchAddress("vp_vy", &vp_vy, &b_vp_vy);
   fChain->SetBranchAddress("vp_vz", &vp_vz, &b_vp_vz);
   fChain->SetBranchAddress("vp_q", &vp_q, &b_vp_q);
   fChain->SetBranchAddress("vp_pdgid", &vp_pdgid, &b_vp_pdgid);
   fChain->SetBranchAddress("vp_size", &vp_size, &b_vp_size);
   Notify();
}

Bool_t MuonTree::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

// void MuonTree::Show(Long64_t entry)
// {
// // Print contents of entry.
// // If entry is not specified, print current entry
//    if (!fChain) return;
//    fChain->Show(entry);
// }
// Int_t MuonTree::Cut(Long64_t entry)
// {
// // This function may be called from Loop.
// // returns  1 if entry is accepted.
// // returns -1 otherwise.
//    return 1;
// }
#endif // #ifdef MuonTree_cxx
