import ROOT

fIn_JF = ROOT.TFile.Open('histos_tb.root')
fIn_LC = ROOT.TFile.Open('plots_EMTF_PhaseI.root')

hIn_JF = fIn_JF.Get('trackcounting/trk_dtheta1_real')
hIn_LC = fIn_LC.Get('MC_JiaFu_sync_dtheta12')

print "JF : ", hIn_JF.Integral(), hIn_JF.Integral(-1, -1)
print "LC : ", hIn_LC.Integral(), hIn_LC.Integral(-1, -1)

hIn_JF.SetLineColor(ROOT.kRed)
hIn_JF.SetLineWidth(2)

hIn_LC.SetMarkerColor(ROOT.kBlack)
hIn_LC.SetMarkerSize(0.8)
hIn_LC.SetMarkerStyle(8)

c1 = ROOT.TCanvas()
hIn_JF.Draw('hist')
hIn_LC.Draw('p same')

c1.Update()
raw_input()