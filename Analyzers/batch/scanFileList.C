void scanFileList(TString s)
{
   TFile* fIN = TFile::Open(s);
   TTree* tIN = (TTree*) fIN->Get("ntupler/tree");
   int nfailures = tIN->GetEntries("nPU_true < 100");
   cout << nfailures << endl;
   if (nfailures > 0)
    cout << "*** OH NO FAULTY NTUPLE *** " << endl;
}