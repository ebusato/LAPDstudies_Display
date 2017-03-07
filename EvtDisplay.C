#include "TFile.h"
#include "TTree.h"
#include <TTreeReader.h>
#include <TTreeReaderValue.h>
#include <TTreeReaderArray.h>

void EvtDisplay(TString evtNumber, TString fileName) {
	gStyle->SetOptStat(0);
	gStyle->SetOptTitle(0);
	
	TFile* f = new TFile(fileName, "read");
	TTree* t = (TTree*) f->Get("tree");
	
	Double_t Pulse[240][999];
	t->SetBranchAddress("Pulse",Pulse);
	
        TTreeReader reader(t);
        TTreeReaderValue<UInt_t> Evt(reader, "Evt");
	TTreeReaderValue<Int_t> NoPulses(reader, "NoPulses");
        TTreeReaderArray<UShort_t> IChanAbs240(reader, "IChanAbs240");
        TTreeReaderArray<Double_t> E(reader, "E");
        TTreeReaderArray<Double_t> Ampl(reader, "Ampl");
	TTreeReaderArray<Double_t> SampleTimes(reader, "SampleTimes");
        //TTreeReaderArray<Double_t*> Pulse(reader, "Pulse");

	int Nbins = 999;
	float minX = 0;
	float maxX = 200;
	
	std::vector<TH1F*> histos;
	for(int i = 0; i < 240; ++i) {
		histos.push_back(new TH1F(Form("histo_%i", i), Form("histo_%i", i), Nbins, minX, maxX));
	}
	
	TString cut("Evt == "+evtNumber);
	t->Draw(">>evtlist", cut);
	TEventList *evtlist = (TEventList*)gDirectory->Get("evtlist");
	int Nevents = evtlist->GetN();
	
	for(int i = 0; i < Nevents; i++) {
        //while (reader.Next()) {
		reader.SetEntry(evtlist->GetEntry(i));
		t->GetEntry(evtlist->GetEntry(i));
		if(*NoPulses == 2 && IChanAbs240[0] >= 120) {
			cout << "ERROR: IChanAbs240[0] >= 120" << endl;
			return;
		}
		if(*NoPulses == 2 && IChanAbs240[1] < 120) {
			cout << "ERROR: IChanAbs240[1] < 120" << endl;
			return;
		}
		for(int j = 0; j < *NoPulses; j++) {
			//cout << "IChanAbs240[" << j << "] = " << IChanAbs240[j]<< endl;
			for(int k = 0; k < 999; k++) {
				//histos[IChanAbs240[j]]->Fill(SampleTimes[k], Pulse[j][k]);
				//cout << "Pulse[" << j << "][" << k << "] = " << Pulse[j][k] << endl;
				histos[IChanAbs240[j]]->Fill(SampleTimes[k], Pulse[j][k]);
			}
		}
        }
	
	TCanvas* cLeft = new TCanvas("cLeft", "cLeft", 1500, 800);
	TCanvas* cRight = new TCanvas("cRight", "cRight", 1500, 800);
	cLeft->SetFillColor(7);
	cRight->SetFillColor(kYellow);
	cLeft->Divide(5, 6);
	cRight->Divide(5, 6);
	
	// Draw right hemisphere
	for (int iQ = 0; iQ < 60; iQ++) {
		if(iQ < 30) {
			int irow = iQ/5;
			int icol = 4-iQ%5+1;
			int ipad = irow*5 + icol;
			cRight->cd(ipad);
		} else {
			int iQprime = iQ - 30;
			int irow = 5-iQprime/5;
			int icol = iQprime%5+1;
			int ipad = irow*5 + icol;
			cLeft->cd(ipad);
		}
		gPad->SetFillColor(kWhite);
// 		gPad->SetBottomMargin(0);
// 		gPad->SetTopMargin(0);
// 		gPad->SetLeftMargin(0);
// 		gPad->SetRightMargin(0);
		TLegend* leg = new TLegend(0.5,0.6,1,1);
		for (int iC = 0; iC < 4; iC++) {
			int iChanAbs240 = 4*iQ + iC;
			if(histos[iChanAbs240]->Integral() != 0) {
				int color;
				if(iC == 0)
					color = kRed;
				else if(iC == 1)
					color = kGreen+2;
				else if(iC == 2)
					color = kBlue;
				else if(iC == 3)
					color = kMagenta;
				histos[iChanAbs240]->SetMarkerColor(color);
				histos[iChanAbs240]->SetLineColor(color);
				histos[iChanAbs240]->SetLineWidth(1);
				histos[iChanAbs240]->GetXaxis()->SetLabelSize(0.055);
				histos[iChanAbs240]->GetYaxis()->SetLabelSize(0.055);
				if(iC == 0) {
					histos[iChanAbs240]->Draw("hist");
				} else {
					histos[iChanAbs240]->Draw("histsame");
				}
				leg->AddEntry(histos[iChanAbs240], Form("iChanAbs240=%i", iChanAbs240), "l");
			}
		}
		leg->SetLineWidth(0);
		leg->SetBorderSize(0);
		leg->Draw();
		//gPad->SaveAs(Form("pad_%i.png", iQ));
	}
}
