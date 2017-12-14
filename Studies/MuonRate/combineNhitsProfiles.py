import ROOT
import argparse

parser = argparse.ArgumentParser(description='Command line parser of plotting options')

parser.add_argument('--max', dest='max', type=float,  help='ymax', default=12.0)
args = parser.parse_args()


ROOT.gStyle.SetOptStat(0)
ROOT.gStyle.SetOptFit(0)

PUs = [50, 100, 140, 200]
det = 'GEM'
colors = [ROOT.kBlack, ROOT.kGreen+1, ROOT.kBlue, ROOT.kRed]
fProto = 'output_rate_files_PU{0}_bx0.root'
profileName = 'hProfile_HitMult_' + det

fIns  = []
profs = []
for PU in PUs:
    fIn = ROOT.TFile.Open(fProto.format(PU))
    prof = fIn.Get(profileName)
    profs.append(prof)
    fIns.append(fIn)

for idx in range(0, len(PUs)):
    profs[idx].SetLineColor(colors[idx])
    profs[idx].SetMarkerColor(colors[idx])
    profs[idx].Rebin(2)

mmaxs = []
for prof in profs:
    mmaxs.append(prof.GetMaximum())
# profs[0].SetMaximum(1.15*max(mmaxs))
profs[0].SetMaximum(args.max)
profs[0].SetMinimum(0)
profs[0].GetXaxis().SetTitle("Number of in-time PU interactions (nPU)")

## make a combine profile and fit it
combprof = profs[0].Clone("combprof")
for idx in range(1, len(PUs)):
    combprof.Add(profs[idx])

c1 = ROOT.TCanvas('c1', 'c1', 600, 400)
c1.SetFrameLineWidth(3)
profs[0].Draw()
for idx in range(1, len(PUs)):
    profs[idx].Draw('same')

combprof.SetMarkerStyle(4)
combprof.SetMarkerSize(1)
combprof.SetLineColor(ROOT.kOrange)
# combprof.Fit("pol1")
# combprof.Draw("same")
flin = ROOT.TF1("flin", "[0] + [1]*x", 0, 250)
flin.SetParameters(0, 0.01)
combprof.Fit("flin", "0")
flin.SetLineColor(ROOT.kRed+1)
flin.SetLineWidth(2)
flin.Draw('same')

## legend
leg = ROOT.TLegend(0.12, 0.68, 0.42, 0.88)
for idx, PU in enumerate(PUs):
    leg.AddEntry(profs[idx], "<PU> = %i" % PU, "lp")
leg.SetFillStyle(0)
leg.SetBorderSize(0)
leg.SetTextFont(43)
leg.SetTextSize(16)
leg.Draw()

## fit result
A = flin.GetParameter(0)
sA = flin.GetParError(0)
B = flin.GetParameter(1)
sB = flin.GetParError(1)

text = ROOT.TLatex(0.43, 0.14, "(%.2f #pm %.2f) + (%.4f #pm %.4f) #times nPU" % (A, sA, B, sB))
text.SetNDC(True)
text.SetTextFont(43)
text.SetTextSize(16)
text.Draw()

c1.Update()
raw_input()

c1.Print("rate_vs_nPU_%s_plot.pdf" % det, "pdf")
