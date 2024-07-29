#include "TH1F.h"
#include "TH2F.h"
#include <TCanvas.h>
#include <TColor.h>
#include <TFile.h>
#include <TH1.h>
#include <TLegend.h>
#include <TStyle.h>
#include <TVirtualPad.h>

#define PT_TITLE "#it{p}_{T} (GeV/#it{c})"
#define MAX_PT 5.0f

typedef struct MarkerStyle {
  int style;
  int color;
} MarkerStyle;

typedef struct PidMethod {
  MarkerStyle marker;
  TH1F* TPosIdenHist;
  TH1F* PosIdenHist;
  const TString MethodLabel;
} PidMethod;

typedef struct MonteCarlo {
  TH1F* GeneratedHist;
  TH1F* TrackedHist;
} MonteCarlo;

void eap_calculate_efficencies_and_purity(const PidMethod* pidMethod, const MonteCarlo* mcData, TCanvas *canvas, TLegend *pleg);
void eap_draw_hist(TH1* hist, int markerStyle, int color, const char* title, const char* xLabel,
    const char* yLabel, double minX, double maxX, double minY, double maxY, const char* drawOption);

void efficiency_and_purity(TString analysisFilePath, TString outputImageFilename, TString outputDir = "graphs") {
  TCanvas *canvas = new TCanvas("canvas", "canvas", 0, 0, 1920, 3000);
  canvas->Divide(2, 5);
  TLegend *pleg = new TLegend(0.15, 0.1); 

  gStyle->SetPalette(kRainBow);

  TFile *f = new TFile(analysisFilePath.Data());
  if (f->IsZombie()) {
    printf("Failed to open file %s\n", analysisFilePath.Data());
    return;
  }

  // Machine Learning PID
  const PidMethod mlPid = {
    .marker = MarkerStyle {
      .style = 20,
      .color = 38,
    },
    .TPosIdenHist = (TH1F*) f->Get("pid-ml-eff-and-pur-producer/hPtMLTruePositive"),
    .PosIdenHist = (TH1F*) f->Get("pid-ml-eff-and-pur-producer/hPtMLPositive"),
    .MethodLabel = "ML"
  };

  // Traditional (NSigma) PID
  const PidMethod nSigmaPid = {
    .marker = MarkerStyle {
      .style = 23,
      .color = 46,
    },
    .TPosIdenHist = (TH1F*) f->Get("pid-ml-eff-and-pur-producer/hPtNSigmaTruePositive"),
    .PosIdenHist = (TH1F*) f->Get("pid-ml-eff-and-pur-producer/hPtNSigmaPositive"),
    .MethodLabel = "NSigma"
  };

  // Monte Carlo variables
  const MonteCarlo mcData = {
    .GeneratedHist = (TH1F *)f->Get("pid-ml-eff-and-pur-producer/hPtMCPositive"),
    .TrackedHist = (TH1F *)f->Get("pid-ml-eff-and-pur-producer/hPtMCTracked"),
  };

  eap_calculate_efficencies_and_purity(&mlPid, &mcData, canvas, pleg);
  eap_calculate_efficencies_and_purity(&nSigmaPid, &mcData, canvas, pleg);

  int contextStyle = 20, contextColor = 30;

  // Reconstruction Efficiency
  canvas->cd(3);

  TH1F *phReconstructionTrackEff = (TH1F *)mcData.TrackedHist->Clone();
  phReconstructionTrackEff->Sumw2();
  phReconstructionTrackEff->Divide(mcData.GeneratedHist);

  eap_draw_hist(phReconstructionTrackEff, contextStyle, contextColor, "Reconstruction Efficiency", PT_TITLE, "Reconstruction Efficiency", 0.0, MAX_PT, 0.0, 1.0, "he,same");

  TH2F *pHistAcceptedTOFNSigma = (TH2F *)f->Get("pid-ml-eff-and-pur-producer/hPtTOFNSigma");
  canvas->cd(5);
  eap_draw_hist(pHistAcceptedTOFNSigma, contextStyle, contextColor, "Traditional method accepted #it{n}_{#sigma , TOF}", PT_TITLE, "#it{n}_{#sigma , TOF}", 0.0, MAX_PT, -5.0, 5.0, "colz");

  TH2F *pHistAcceptedTPCNSigma = (TH2F *)f->Get("pid-ml-eff-and-pur-producer/hPtTPCNSigma");
  canvas->cd(6);
  eap_draw_hist(pHistAcceptedTPCNSigma, contextStyle, contextColor, "Traditional method accepted #it{n}_{#sigma , TPC}", PT_TITLE, "#it{n}_{#sigma , TPC}", 0.0, MAX_PT, -5.0, 5.0, "colz");
  gPad->SetLogz();

  TH2F *pHistTOFNSigma = (TH2F *)f->Get("pid-ml-eff-and-pur-producer/full/hPtTOFNSigma");
  canvas->cd(7);
  eap_draw_hist(pHistTOFNSigma, contextStyle, contextColor, "TOF #it{n}_{#sigma} MCParticles with wanted PdgCode", PT_TITLE, "#it{n}_{#sigma , TOF}", 0.0, MAX_PT, -5.0, 5.0, "colz");

  TH2F *pHistTPCNSigma = (TH2F *)f->Get("pid-ml-eff-and-pur-producer/full/hPtTPCNSigma");
  canvas->cd(8);
  eap_draw_hist(pHistTPCNSigma, contextStyle, contextColor, "TPC #it{n}_{#sigma} MCParticles with wanted PdgCode", PT_TITLE, "#it{n}_{#sigma , TPC}", 0.0, MAX_PT, -5.0, 5.0, "colz");
  gPad->SetLogz();

  TH2F *pHistTOFBeta = (TH2F *)f->Get("pid-ml-eff-and-pur-producer/full/hPtTOFBeta");
  canvas->cd(9);
  eap_draw_hist(pHistTOFBeta, contextStyle, contextColor, "TOF #beta for all tracked particles", PT_TITLE, "#beta", 0.0, MAX_PT, 0.0, 1.2, "colz");
  gPad->SetLogz();

  TH2F *pHistTPCSignal = (TH2F *)f->Get("pid-ml-eff-and-pur-producer/full/hPtTPCSignal");
  canvas->cd(10);
  eap_draw_hist(pHistTPCSignal, contextStyle, contextColor, "TPC Signal for all tracked particles", PT_TITLE, "#it{dE}/#it{dx}", 0.0, MAX_PT, 20.0, 120.0, "colz");
  gPad->SetLogz();

  canvas->cd(2);
  pleg->Draw();
  canvas->Print(Form("%s/%s.png", outputDir.Data(), outputImageFilename.Data()));
}

void eap_calculate_efficencies_and_purity(const PidMethod* pidMethod, const MonteCarlo* mcData, TCanvas *canvas, TLegend *pleg) {
  // Full Efficiency
  canvas->cd(1);

  TH1F *phFullTrackEff = (TH1F *)pidMethod->TPosIdenHist->Clone();
  phFullTrackEff->Sumw2();
  phFullTrackEff->Divide(mcData->GeneratedHist);

  eap_draw_hist(phFullTrackEff, pidMethod->marker.style, pidMethod->marker.color, "Full efficiency", PT_TITLE, "Full efficiency", 0.0, MAX_PT, 0.0, 1.0, "he,same");

  // PID Efficiency
  canvas->cd(2);

  TH1F *phPidTrackEff = (TH1F *)pidMethod->TPosIdenHist->Clone();
  phPidTrackEff->Sumw2();
  phPidTrackEff->Divide(mcData->TrackedHist);

  eap_draw_hist(phPidTrackEff, pidMethod->marker.style, pidMethod->marker.color, "PID efficiency", PT_TITLE, "PID efficiency", 0.0, MAX_PT, 0.0, 1.0, "he,same");

  // Purity
  canvas->cd(4);

  TH1F *hPurity = (TH1F *)pidMethod->TPosIdenHist->Clone();
  pidMethod->PosIdenHist->Sumw2();
  hPurity->Divide(pidMethod->PosIdenHist);
  hPurity->Sumw2();

  eap_draw_hist(hPurity, pidMethod->marker.style, pidMethod->marker.color, "Purity", PT_TITLE, "Purity", 0.0, MAX_PT, 0.0, 1.0, "he,same");

  // Add to legend
  pleg->AddEntry(hPurity, pidMethod->MethodLabel);
}

void eap_draw_hist(TH1* hist, int markerStyle, int color, const char* title, const char* xLabel,
    const char* yLabel, double minX, double maxX, double minY, double maxY, const char* drawOption) {
    hist->SetMarkerColor(color);
    hist->SetLineColor(color);
    hist->SetMarkerStyle(markerStyle);
    hist->SetMarkerSize(1.4);
    hist->SetTitle(title);
    hist->GetXaxis()->SetTitle(xLabel);
    hist->GetYaxis()->SetTitle(yLabel);
    hist->GetXaxis()->SetRangeUser(minX, maxX);
    hist->GetYaxis()->SetRangeUser(minY, maxY);
    hist->GetXaxis()->SetTitleSize(0.03);
    hist->GetYaxis()->SetTitleSize(0.03);
    hist->GetXaxis()->SetLabelSize(0.03);
    hist->GetYaxis()->SetLabelSize(0.03);
    hist->SetStats(0);
    hist->SetContour(1000);
    hist->Draw(drawOption);
}

