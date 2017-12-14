#include "MuonTree.h"
#include "PtAssignmentEngineAux2017.h"
#include <utility>
#include <iostream>

namespace EMTFBDTAdapter
{

  const PtAssignmentEngineAux2017& aux() {
  static const PtAssignmentEngineAux2017 instance; // KK: arguable design solution, but const qualifier makes it thread-safe anyway
  return instance;
}

  const int bw_fph = 13;  // bit width of ph, full precision
  const int bw_th = 7;    // bit width of th
  const int invalid_dtheta = (1<<bw_th) - 1;  // = 127
  const int invalid_dphi   = (1<<bw_fph) - 1; // = 8191

  struct EMTFPtLUT {
    uint64_t address;
    uint16_t mode;
    uint16_t theta;
    uint16_t st1_ring2;
    uint16_t eta;
    uint16_t delta_ph [6]; // index: 0=12, 1=13, 2=14, 3=23, 4=24, 5=34
    uint16_t delta_th [6]; // ^
    uint16_t sign_ph  [6]; // ^
    uint16_t sign_th  [6]; // ^
    uint16_t cpattern [4]; // index: 0=ME1, 1=ME2, 2=ME3, 3=ME4
    uint16_t fr       [4]; // ^
    uint16_t bt_vi    [5]; // index: 0=ME1sub1, 1=ME1sub2, 2=ME2, 3=ME3, 4=ME4
    uint16_t bt_hi    [5]; // ^
    uint16_t bt_ci    [5]; // ^
    uint16_t bt_si    [5]; // ^
  };

  enum Subsystem
  {
    kDT,
    kCSC,
    kRPC,
    kGEM
  };

  // ------- - -------- - ------- - -------- - ------- - -------- - ------- - -------- - ------- - --------

  std::pair<int,int> getEMTFThetaPhi(MuonTree* mt, uint iTrack)
  {
    int theta = -1;
    int phi   = -1;
    int ihit = -1;
    if (mt -> vt_hitref2 -> at(iTrack) >= 0)
      ihit = mt -> vt_hitref2 -> at(iTrack);
    else if (mt -> vt_hitref3 -> at(iTrack) >= 0)
      ihit = mt -> vt_hitref3 -> at(iTrack);
    else if (mt -> vt_hitref4 -> at(iTrack) >= 0)
      ihit = mt -> vt_hitref4 -> at(iTrack);
    else
    {
      std::cout << " *** WARNING: getEMTFThetaPhi : something strange is happening, track has <= 1 hit ??" << std::endl;
      return std::make_pair(-1,-1);
    }

    return std::make_pair(
      mt->vh_emtf_theta->at(ihit),
      mt->vh_emtf_phi->at(ihit)
    );

  }

  // ------- - -------- - ------- - -------- - ------- - -------- - ------- - -------- - ------- - --------

  bool isFront(int station, int ring, int chamber, int type)
  {
    // copied and adapted from CalcAngles

    // // RPCs are behind the CSCs in stations 1, 3, and 4; in front in 2
    // if (subsystem == TriggerPrimitive::kRPC)
    //   return (station == 2);

    // In EMTF firmware, RPC hits are treated as if they came from the corresponding 
    // CSC chamber, so the FR bit assignment is the same as for CSCs - AWB 06.06.17


    // GEMs are in front of the CSCs
    if (type == (int) Subsystem::kGEM)
      return true;

    bool result = false;
    bool isOverlapping = !(station == 1 && ring == 3);
    // not overlapping means back
    if(isOverlapping)
    {
      bool isEven = (chamber % 2 == 0);
      // odd chambers are bolted to the iron, which faces
      // forward in 1&2, backward in 3&4, so...
      result = (station < 3) ? isEven : !isEven;
    }
    return result;
  }

  // ------- - -------- - ------- - -------- - ------- - -------- - ------- - -------- - ------- - --------

  EMTFPtLUT computeInputs(MuonTree* mt, uint iTrack)
  {
    EMTFPtLUT ptlut;

    int hitref1 = mt -> vt_hitref1 -> at(iTrack);
    int hitref2 = mt -> vt_hitref2 -> at(iTrack);
    int hitref3 = mt -> vt_hitref3 -> at(iTrack);
    int hitref4 = mt -> vt_hitref4 -> at(iTrack);
    std::vector<int> hitref = {hitref1, hitref2, hitref3, hitref4};
    std::vector<uint16_t> dthetas;
    std::vector<uint16_t> dphis;
    std::vector<uint16_t> dthetas_sign;
    std::vector<uint16_t> dphis_sign;
    
    // guarantees same order wanted by the ptlut
    for (uint iA = 0; iA < hitref.size(); ++iA)
      for (uint iB = iA+1; iB < hitref.size(); ++iB)
      {
        int idxhitA = hitref.at(iA);
        int idxhitB = hitref.at(iB);
        if (idxhitA == -1 || idxhitB == -1)
        {
          dthetas.push_back(invalid_dtheta);
          dphis.push_back(invalid_dphi);
          dthetas_sign.push_back(1);
          dphis_sign.push_back(1);
        }
        else
        {
          int thetaA = mt->vh_emtf_theta->at(iA);
          int thetaB = mt->vh_emtf_theta->at(iB);
          int phiA = mt->vh_emtf_phi->at(iA);
          int phiB = mt->vh_emtf_phi->at(iB);
          dthetas.push_back(abs(thetaB - thetaA));
          dphis.push_back(abs(phiB - phiA));
          dthetas_sign.push_back((thetaA <= thetaB));
          dphis_sign.push_back((phiA <= phiB));
        }
      }

    // st1_ring2 is true if there is a hit in S1 in ring 2 or 3, else it is false
    bool b_st1_ring2 = false;
    if (hitref1 != -1)
    {
      int ring = mt->vh_ring->at(hitref1);
      if (ring == 2 || ring == 3)
        b_st1_ring2 = true;
    }


    ptlut.address   = 0; // unused
    ptlut.mode      = 0; // unused
    ptlut.theta     = 0; // unused
    ptlut.st1_ring2 = b_st1_ring2;
    ptlut.eta       = 0; // unused
    for (uint i = 0; i < 6; ++i) ptlut.delta_ph [i] = dphis.at(i);
    for (uint i = 0; i < 6; ++i) ptlut.delta_th [i] = dthetas.at(i);
    for (uint i = 0; i < 6; ++i) ptlut.sign_ph  [i] = dphis_sign.at(i);
    for (uint i = 0; i < 6; ++i) ptlut.sign_th  [i] = dthetas_sign.at(i);
    for (uint i = 0; i < 4; ++i) ptlut.cpattern [i] = (hitref.at(i) < 0 ? 0 : mt->vh_pattern->at(hitref.at(i)));
    for (uint i = 0; i < 4; ++i) ptlut.fr       [i] = (hitref.at(i) < 0 ? 0 : isFront(
                                                                              mt->vh_station ->at(hitref.at(i)),
                                                                              mt->vh_ring    ->at(hitref.at(i)),
                                                                              mt->vh_chamber ->at(hitref.at(i)),
                                                                              mt->vh_type    ->at(hitref.at(i))
                                                                            ));
    for (uint i = 0; i < 5; ++i) ptlut.bt_vi[i] = 0; // unused  
    for (uint i = 0; i < 5; ++i) ptlut.bt_hi[i] = 0; // unused
    for (uint i = 0; i < 5; ++i) ptlut.bt_ci[i] = 0; // unused
    for (uint i = 0; i < 5; ++i) ptlut.bt_si[i] = 0; // unused
  }

  // ------- - -------- - ------- - -------- - ------- - -------- - ------- - -------- - ------- - --------

  uint64_t calculate_address(MuonTree* mt, uint iTrack) {
      uint64_t address = 0;

      // EMTFPtLUT data = track.PtLUT();
      EMTFPtLUT data = computeInputs(mt, iTrack); // will just replicate the inputs

      int mode   = (int) mt->vt_mode->at(iTrack);
      int theta  = getEMTFThetaPhi(mt, iTrack).first;
      int endcap = (int) mt->vt_endcap->at(iTrack);
      
      // int mode   = track.Mode();
      // int theta  = track.Theta_fp();
      // int endcap = track.Endcap();
      int nHits  = (mode / 8) + ((mode % 8) / 4) + ((mode % 4) / 2) + ((mode % 2) / 1);
      if (not(nHits > 1 && nHits < 5))
        // { edm::LogError("L1T") << "nHits = " << nHits; return 0; }
          { std::cout << " Muon Pt BDT error " << "nHits = " << nHits; return 0; }
      
      // 'A' is first station in the track, 'B' the second, etc.
      int mode_ID = -1;
      int iA = -1, iB = -1, iC = -1, iD = -1;
      int iAB, iAC, iAD, iBC, iCD;

      switch (mode) { // Indices for quantities by station or station pair
      case 15: mode_ID = 0b1  ; iA = 0; iB = 1; iC = 2; iD = 3; break;
      case 14: mode_ID = 0b11 ; iA = 0; iB = 1; iC = 2; break;
      case 13: mode_ID = 0b10 ; iA = 0; iB = 1; iC = 3; break;
      case 11: mode_ID = 0b01 ; iA = 0; iB = 2; iC = 3; break;
      case  7: mode_ID = 0b1  ; iA = 1; iB = 2; iC = 3; break;
      case 12: mode_ID = 0b111; iA = 0; iB = 1; break;
      case 10: mode_ID = 0b110; iA = 0; iB = 2; break;
      case  9: mode_ID = 0b101; iA = 0; iB = 3; break;
      case  6: mode_ID = 0b100; iA = 1; iB = 2; break;
      case  5: mode_ID = 0b011; iA = 1; iB = 3; break;
      case  3: mode_ID = 0b010; iA = 2; iB = 3; break;
      default: break;
      }
      iAB = (iA >= 0 && iB >= 0) ? iA + iB - (iA == 0) : -1;
      iAC = (iA >= 0 && iC >= 0) ? iA + iC - (iA == 0) : -1;
      iAD = (iA >= 0 && iD >= 0) ? 2                   : -1;
      iBC = (iB >= 0 && iC >= 0) ? iB + iC             : -1;
      iCD = (iC >= 0 && iD >= 0) ? 5                   : -1;

      
      // Fill variable info from pT LUT data
      int st1_ring2, dTheta;
      int dPhiAB, dPhiBC, dPhiCD;
      int sPhiAB, sPhiBC, sPhiCD;
      int frA, frB;
      int clctA, clctB, clctC, clctD;
      int mode15_8b; // Combines track theta, stations with RPC hits, and station 1 bend information
      int rpc_2b;    // Identifies which stations have RPC hits in 3-station tracks

      st1_ring2 = data.st1_ring2;
      if        (nHits == 4) {
        dPhiAB = data.delta_ph[iAB];
        dPhiBC = data.delta_ph[iBC];
        dPhiCD = data.delta_ph[iCD];
        sPhiAB = data.sign_ph[iAB];
        sPhiBC = (data.sign_ph[iBC] == sPhiAB);
        sPhiCD = (data.sign_ph[iCD] == sPhiAB);
        dTheta = data.delta_th[iAD] * (data.sign_th[iAD] ? 1 : -1);
        frA    = data.fr      [iA];
        clctA  = data.cpattern[iA];
        clctB  = data.cpattern[iB];
        clctC  = data.cpattern[iC];
        clctD  = data.cpattern[iD];
      } else if (nHits == 3) {
        dPhiAB = data.delta_ph[iAB];
        dPhiBC = data.delta_ph[iBC];
        sPhiAB = data.sign_ph[iAB];
        sPhiBC = (data.sign_ph[iBC] == sPhiAB);
        dTheta = data.delta_th[iAC] * (data.sign_th[iAC] ? 1 : -1);
        frA    = data.fr      [iA];
        frB    = data.fr      [iB];
        clctA  = data.cpattern[iA];
        clctB  = data.cpattern[iB];
        clctC  = data.cpattern[iC];
      } else if (nHits == 2) {
        dPhiAB = data.delta_ph[iAB];
        sPhiAB = data.sign_ph[iAB];
        dTheta = data.delta_th[iAB] * (data.sign_th[iAB] ? 1 : -1);
        frA    = data.fr      [iA];
        frB    = data.fr      [iB];
        clctA  = data.cpattern[iA];
        clctB  = data.cpattern[iB];
      }


      // Convert variables to words for pT LUT address
      if        (nHits == 4) {
        dPhiAB = aux().getNLBdPhiBin ( dPhiAB, 7, 512 );
        dPhiBC = aux().getNLBdPhiBin ( dPhiBC, 5, 256 );
        dPhiCD = aux().getNLBdPhiBin ( dPhiCD, 4, 256 );
        dTheta = aux().getdTheta     ( dTheta, 2 );
        mode15_8b = aux().get8bMode15( theta, st1_ring2, endcap, (sPhiAB == 1 ? 1 : -1), clctA, clctB, clctC, clctD );
      } else if (nHits == 3) {
        dPhiAB = aux().getNLBdPhiBin( dPhiAB, 7, 512 );
        dPhiBC = aux().getNLBdPhiBin( dPhiBC, 5, 256 );
        dTheta = aux().getdTheta    ( dTheta, 3 );
        rpc_2b = aux().get2bRPC     ( clctA, clctB, clctC ); // Have to use un-compressed CLCT words
        clctA  = aux().getCLCT      ( clctA, endcap, (sPhiAB == 1 ? 1 : -1), 2 );
        theta  = aux().getTheta     ( theta, st1_ring2, 5 );
      } else if (nHits == 2) {
        dPhiAB = aux().getNLBdPhiBin( dPhiAB, 7, 512 );
        dTheta = aux().getdTheta    ( dTheta, 3 );
        clctA  = aux().getCLCT      ( clctA, endcap, (sPhiAB == 1 ? 1 : -1), 3 );
        clctB  = aux().getCLCT      ( clctB, endcap, (sPhiAB == 1 ? 1 : -1), 3 );
        theta  = aux().getTheta     ( theta, st1_ring2, 5 );
      }

      
      // Form the pT LUT address
      if      (nHits == 4) {
        address |= (dPhiAB    & ((1<<7)-1)) << (0);
        address |= (dPhiBC    & ((1<<5)-1)) << (0+7);
        address |= (dPhiCD    & ((1<<4)-1)) << (0+7+5);
        address |= (sPhiBC    & ((1<<1)-1)) << (0+7+5+4);
        address |= (sPhiCD    & ((1<<1)-1)) << (0+7+5+4+1);
        address |= (dTheta    & ((1<<2)-1)) << (0+7+5+4+1+1);
        address |= (frA       & ((1<<1)-1)) << (0+7+5+4+1+1+2);
        address |= (mode15_8b & ((1<<8)-1)) << (0+7+5+4+1+1+2+1);
        address |= (mode_ID   & ((1<<1)-1)) << (0+7+5+4+1+1+2+1+8);
        if (not(address < pow(2, 30) && address >= pow(2, 29)))
      // { edm::LogError("L1T") << "address = " << address; return 0; }
          { std::cout << " Muon Pt BDT error " << "address = " << address; return 0; }
      } 
      else if (nHits == 3) {
        address |= (dPhiAB    & ((1<<7)-1)) << (0);
        address |= (dPhiBC    & ((1<<5)-1)) << (0+7);
        address |= (sPhiBC    & ((1<<1)-1)) << (0+7+5);
        address |= (dTheta    & ((1<<3)-1)) << (0+7+5+1);
        address |= (frA       & ((1<<1)-1)) << (0+7+5+1+3);
        int bit = 0;
        if (mode != 7) { 
      address |= (frB     & ((1<<1)-1)) << (0+7+5+1+3+1); bit = 1;
        }
        address |= (clctA     & ((1<<2)-1)) << (0+7+5+1+3+1+bit);
        address |= (rpc_2b    & ((1<<2)-1)) << (0+7+5+1+3+1+bit+2);
        address |= (theta     & ((1<<5)-1)) << (0+7+5+1+3+1+bit+2+2);
        if (mode != 7) {
      address |= (mode_ID & ((1<<2)-1)) << (0+7+5+1+3+1+bit+2+2+5); 
      if (not(address < pow(2, 29) && address >= pow(2, 27)))
        // { edm::LogError("L1T") << "address = " << address; return 0; }
          { std::cout << " Muon Pt BDT error " << "address = " << address; return 0; }
        } else {
      address |= (mode_ID & ((1<<1)-1)) << (0+7+5+1+3+1+bit+2+2+5); 
      if (not(address < pow(2, 27) && address >= pow(2, 26)))
        // { edm::LogError("L1T") << "address = " << address; return 0; }
          { std::cout << " Muon Pt BDT error " << "address = " << address; return 0; }
        }
      } 
      else if (nHits == 2) { 
        address |= (dPhiAB    & ((1<<7)-1)) << (0);
        address |= (dTheta    & ((1<<3)-1)) << (0+7);
        address |= (frA       & ((1<<1)-1)) << (0+7+3);
        address |= (frB       & ((1<<1)-1)) << (0+7+3+1);
        address |= (clctA     & ((1<<3)-1)) << (0+7+3+1+1);
        address |= (clctB     & ((1<<3)-1)) << (0+7+3+1+1+3);
        address |= (theta     & ((1<<5)-1)) << (0+7+3+1+1+3+3);
        address |= (mode_ID   & ((1<<3)-1)) << (0+7+3+1+1+3+3+5);
        if (not(address < pow(2, 26) && address >= pow(2, 24)))
      // { edm::LogError("L1T") << "address = " << address; return 0; }
          { std::cout << " Muon Pt BDT error " << "address = " << address; return 0; }
      }

      return address;
  } // End function: PtAssignmentEngine2017::calculate_address()

}
