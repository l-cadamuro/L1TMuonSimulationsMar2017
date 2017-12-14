import ROOT
import argparse
import math
import collections

##  python drawPlots.py --drawlist MC_wi_match_gt16 MC_no_match_gt16 --text "p_{T} > 16 GeV" --var dphi23
##  python drawPlots.py --drawlist MC_wi_match_gt16_singleMu_ptresol40 MC_no_match_gt16_singleMu_ptresol40_and MC_no_match_gt16_singleMu_ptresol40_or --text "p_{T} > 16 GeV" --var dtheta12 --no-norm
def getName(tag, var): 
    return tag + '_'+ var


parser = argparse.ArgumentParser(description='Command line parser of plotting options')
parser.add_argument('--var',       dest='var',                 help='variable name',                                            default=None)
parser.add_argument('--input',     dest='input',               help='variable name',                                            default='plots_EMTF_PhaseI.root')
parser.add_argument('--log',       dest='log',                 help='log y',                              action='store_true',  default=False)
parser.add_argument('--text',      dest='text',                help='text on plot',                                             default=None)
parser.add_argument('--no-norm',   dest='norm',                help='normalize area',                     action='store_false', default=True)
parser.add_argument('--no-pdf',    dest='pdf',                 help='do not create pdf output',           action='store_false', default=True)
parser.add_argument('--drawlist',  dest='drawlist', nargs='+', help="Provide a list of files to analyze", type=str,             default=['MC_wi_match', 'MC_no_match'])
args = parser.parse_args()

##################################################
colors = {
    'MC_wi_match' : ROOT.kRed,
    'MC_no_match' : ROOT.kBlack,
    #######
    'MC_wi_match_gt16' : ROOT.kRed,
    'MC_no_match_gt16' : ROOT.kBlack,
    #######
    'MC_wi_match_gt16_ptresol40'     : ROOT.kRed,
    'MC_no_match_gt16_ptresol40_and' : ROOT.kBlack,
    'MC_no_match_gt16_ptresol40_or'  : ROOT.kBlue,
    #######
    'MC_wi_match_gt16_singleMu_ptresol40'     : ROOT.kRed,
    'MC_no_match_gt16_singleMu_ptresol40_and' : ROOT.kBlack,
    'MC_no_match_gt16_singleMu_ptresol40_or'  : ROOT.kBlue,

}

legNames = {
    'MC_wi_match' : 'Single trk part.',
    'MC_no_match' : 'Other',
    #######
    'MC_wi_match_gt16' : 'Single trk part.',
    'MC_no_match_gt16' : 'Other',
    #######
    'MC_wi_match_gt16_ptresol40'     : 'Single trk part. within 40%',
    'MC_no_match_gt16_ptresol40_and' : 'Other (AND)',
    'MC_no_match_gt16_ptresol40_or'  : 'Other (OR)',
    #######
    'MC_wi_match_gt16_singleMu_ptresol40'     : 'Single trk part. within 40%',
    'MC_no_match_gt16_singleMu_ptresol40_and' : 'Other (AND)',
    'MC_no_match_gt16_singleMu_ptresol40_or'  : 'Other (OR)',
}

#################################################

print 'Opening file', args.input
fIn = ROOT.TFile.Open(args.input)

print 'Drawing plots for', args.drawlist

histos = collections.OrderedDict()
for tag in args.drawlist:
    n = getName(tag, args.var)
    h = fIn.Get(n)
    histos[tag] = h
    if args.norm:
        integ = histos[tag].Integral()
        if integ > 0: histos[tag].Scale(1./integ)

c1 = ROOT.TCanvas('c1', 'c1', 600, 600)
c1.SetFrameLineWidth(3)
c1.SetLeftMargin(0.15)
c1.SetBottomMargin(0.12)
if args.log:
    c1.SetLogy()

#####
mmaxs = [h.GetMaximum() for h in histos.values()]
mmins = [h.GetMinimum() for h in histos.values()]
mmax = max(mmaxs)
mmin = min(mmins)
if mmin == 0 and args.log: mmin = 1.E-5

extraspace = 0.15
if not args.log:
    mmax += extraspace * (mmax-mmin)
else:
    ymaxnew = extraspace * (math.log(mmax, 10) - math.log(mmin, 10)) + math.log(mmax, 10)
    mmax = math.pow(10, ymaxnew)
####

for idx, val in enumerate(histos.items()):
    
    tag   = val[0]
    histo = val[1]
    
    ### common
    histo.SetLineWidth(2)
    if tag in colors:
        histo.SetLineColor(colors[tag])
        histo.SetMarkerColor(colors[tag])

    ## only for first (axes etc.)
    if idx == 0:

        histo.SetMaximum(mmax)
        if args.norm: histo.GetYaxis().SetTitle('a.u.')
        histo.GetYaxis().SetTitleOffset(1.4)
        histo.GetXaxis().SetTitleSize(0.05)
        histo.GetYaxis().SetTitleSize(0.05)
        histo.GetXaxis().SetLabelSize(0.04)
        histo.GetYaxis().SetLabelSize(0.04)
        histo.SetStats(0)
        histo.Draw()
    ## only for non-first
    else:
        histo.Draw('same')

## legend
leg = ROOT.TLegend(0.16, 0.93, 0.97, 0.97)
leg.SetNColumns(2)
leg.SetBorderSize(0)
leg.SetFillStyle(0)
for val in histos.items():
    lname = legNames[val[0]] if val[0] in legNames else val[0]
    leg.AddEntry(val[1], lname, "lep")
leg.Draw()

### text 
if args.text:
    ttext = ROOT.TLatex(0.18, 0.85, args.text)
    ttext.SetNDC()
    ttext.SetTextSize(0.04)
    ttext.SetTextFont(42)
    ttext.Draw()

c1.Update()
raw_input()
    

if args.pdf:
    c1.Print('plot_' + args.var + '.pdf', 'pdf')