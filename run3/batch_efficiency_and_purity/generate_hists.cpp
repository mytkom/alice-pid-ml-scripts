#include "TFile.h"
#include "TTree.h"
#include "TDirectoryFile.h"
#include "TMap.h"
#include "TKey.h"
#include "TClass.h"
#include "TList.h"
#include "TH1F.h"
#include "TCanvas.h"
#include <array>
#include <iostream>
#include <string_view>
#include <vector>


// Data related consts
constexpr size_t pidsCount = 6;
constexpr size_t binCount = 50;
constexpr int pids[pidsCount] = {
   211,
   321,
   2212,
  -211,
  -321,
  -2212
};
constexpr std::array<std::string_view, pidsCount> mcTrackedHistLabels = {
    "pid-ml-batch-eff-and-pur-producer/211/hPtMCTracked",
    "pid-ml-batch-eff-and-pur-producer/321/hPtMCTracked",
    "pid-ml-batch-eff-and-pur-producer/2212/hPtMCTracked",
    "pid-ml-batch-eff-and-pur-producer/0211/hPtMCTracked",
    "pid-ml-batch-eff-and-pur-producer/0321/hPtMCTracked",
    "pid-ml-batch-eff-and-pur-producer/02212/hPtMCTracked"
};
constexpr std::array<std::string_view, pidsCount> mcPositiveHistLabels = {
    "pid-ml-batch-eff-and-pur-producer/211/hPtMCPositive",
    "pid-ml-batch-eff-and-pur-producer/321/hPtMCPositive",
    "pid-ml-batch-eff-and-pur-producer/2212/hPtMCPositive",
    "pid-ml-batch-eff-and-pur-producer/0211/hPtMCPositive",
    "pid-ml-batch-eff-and-pur-producer/0321/hPtMCPositive",
    "pid-ml-batch-eff-and-pur-producer/02212/hPtMCPositive"
};

// Plotting related consts
constexpr double maxPt = 3.1;
constexpr std::string_view ptTitle = "#it{p}_{T} (GeV/#it{c})";
constexpr int contextStyle = 21, contextColor = 30;
constexpr int mlStyle = 20, mlColor = 38;
constexpr int nsStyle = 20, nsColor = 46;

enum TrackSet {
  WithoutTOF,
  WithTOF,
  All
};

void extract_ttrees(TDirectoryFile* dir, std::vector<TTree*>& trees) {
    TList* keys = dir->GetListOfKeys();
    TIter next(keys);
    TKey* key;

    while ((key = (TKey*)next())) {
        if (std::string(key->GetClassName()) == "TTree") {
            TTree* tree = (TTree*)dir->Get(key->GetName());
            trees.push_back(tree);
        } else if (std::string(key->GetClassName()) == "TDirectoryFile") {
            TDirectoryFile* subdir = (TDirectoryFile*)dir->Get(key->GetName());
            extract_ttrees(subdir, trees);
        }
    }
}

void draw_hist(TH1* hist, int markerStyle, int color, const char* title, const char* xLabel,
    const char* yLabel, double minX, double maxX, double minY, double maxY, const char* drawOption) {
    hist->SetMarkerColor(color);
    hist->SetLineColor(color);
    hist->SetMarkerStyle(markerStyle);
    hist->SetMarkerSize(1.3);
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

void generate_hists(const char* aodFilePath, const char* histFilePath, float certaintyThreshold, float nSigmaThreshold, const char* saveDir = ".", TrackSet trackSet = All) {
    TFile aodFile(aodFilePath, "READ");

    if (aodFile.IsZombie()) {
        std::cerr << "Error opening aodFile: " << aodFilePath << std::endl;
        return;
    }

    TMap* map = (TMap*)aodFile.Get("parentFiles");
    if (!map) {
        std::cerr << "Error: TMap 'parentFiles' not found in aodFile: " << aodFilePath << std::endl;
        return;
    }

    std::vector<TTree*> trees;

    TIter next(map);
    while (TObject* obj = next()) {
        const char* dirName = obj->GetName();
        TDirectoryFile* dir = (TDirectoryFile*) aodFile.Get(dirName);
        extract_ttrees(dir, trees);
    }

    std::vector<TH1F*> h_ml_tp(pidsCount), h_ml_fp(pidsCount), h_ml_selected(pidsCount), h_ns_tp(pidsCount), h_ns_fp(pidsCount), h_ns_selected(pidsCount), h_total_positives(pidsCount);

    for (size_t i = 0; i < pidsCount; ++i) {
        int searchedPid = pids[i];
        h_ns_tp[i] = new TH1F(Form("h_ns_tp_%d", searchedPid), Form("NSigma True Positives for PID %d", searchedPid), binCount, 0, maxPt);
        h_ns_fp[i] = new TH1F(Form("h_ns_fp_%d", searchedPid), Form("NSigma False Positives for PID %d", searchedPid), binCount, 0, maxPt);
        h_ml_tp[i] = new TH1F(Form("h_ml_tp_%d", searchedPid), Form("ML True Positives for PID %d", searchedPid), binCount, 0, maxPt);
        h_ml_fp[i] = new TH1F(Form("h_ml_fp_%d", searchedPid), Form("ML False Positives for PID %d", searchedPid), binCount, 0, maxPt);
        h_total_positives[i] = new TH1F(Form("h_total_positives_%d", searchedPid), Form("Total Positives for PID %d", searchedPid), binCount, 0, maxPt);
        h_ml_selected[i] = new TH1F(Form("h_ml_selected_%d", searchedPid), Form("NSigma Selected for PID %d", searchedPid), binCount, 0, maxPt);
        h_ns_selected[i] = new TH1F(Form("h_ns_selected_%d", searchedPid), Form("ML Selected for PID %d", searchedPid), binCount, 0, maxPt);
    }

    for (TTree* tree : trees) {
        int pid;
        float mlCertainty;
        float pt;
        float nSigma;
        bool isPidMC;
        bool hasTOF;

        tree->SetBranchAddress("fPid", &pid);
        tree->SetBranchAddress("fMlCertainty", &mlCertainty);
        tree->SetBranchAddress("fPt", &pt);
        tree->SetBranchAddress("fNSigma", &nSigma);
        tree->SetBranchAddress("fIsPidMC", &isPidMC);
        tree->SetBranchAddress("fHasTOF", &hasTOF);

        Long64_t nentries = tree->GetEntries();
        for (Long64_t i = 0; i < nentries; ++i) {
            tree->GetEntry(i);
            if (
              (trackSet == WithoutTOF && hasTOF) ||
              (trackSet == WithTOF && !hasTOF)
            ) {
              continue;
            }
            for (size_t j = 0; j < pidsCount; ++j) {
                int searchedPid = pids[j];
                if (pid == searchedPid) {
                    if(isPidMC) {
                      h_total_positives[j]->Fill(pt);
                    }

                    if (mlCertainty >= certaintyThreshold) {
                        h_ml_selected[j]->Fill(pt);
                        if (isPidMC) {
                            h_ml_tp[j]->Fill(pt);
                        } else {
                            h_ml_fp[j]->Fill(pt);
                        }
                    }

                    if(nSigma < nSigmaThreshold) {
                        h_ns_selected[j]->Fill(pt);
                        if (isPidMC) {
                            h_ns_tp[j]->Fill(pt);
                        } else {
                            h_ns_fp[j]->Fill(pt);
                        }
                    }
                }
            }
        }
    }

    TFile histFile(histFilePath, "READ");

    const char* ml_label = Form("ML (certainty=%.2f)", certaintyThreshold);
    const char* ns_label = Form("n#sigma (n=%.2f)", nSigmaThreshold);

    for (size_t i = 0; i < pidsCount; ++i) {
        int searchedPid = pids[i];

        TH1F* h_mc_tracked = (TH1F*) histFile.Get(mcTrackedHistLabels[i].data());
        TH1F* h_mc_positive = (TH1F*) histFile.Get(mcPositiveHistLabels[i].data());

        if(h_mc_tracked == nullptr || h_mc_positive == nullptr) {
          std::cout << "There is no histogram for: " << searchedPid << std::endl;
          std::cout << "\tskipping..." << std::endl;
          continue;
        }

        // Draw histograms
        TCanvas c1("c1", "PID Efficiency and Purity", 1000, 1800);
        c1.Divide(1, 3);

        TLatex *latex = new TLatex();
        latex->SetNDC();
        latex->SetTextAlign(31);
        latex->SetTextSize(0.04); // Set the text size

        // PID Efficiency
        c1.cd(1);

        auto h_ml_pid_eff = (TH1F*)h_ml_tp[i]->Clone();
        h_ml_pid_eff->Sumw2();
        h_ml_pid_eff->Divide(h_total_positives[i]);
        draw_hist(h_ml_pid_eff, mlStyle, mlColor, "PID Efficiency", ptTitle.data(), "Efficiency", 0.0, maxPt, 0.0, 1.0, "hist,same,pl");

        auto h_ns_pid_eff = (TH1F*)h_ns_tp[i]->Clone();
        h_ns_pid_eff->Sumw2();
        h_ns_pid_eff->Divide(h_total_positives[i]);
        draw_hist(h_ns_pid_eff, nsStyle, nsColor, "PID Efficiency", ptTitle.data(), "Efficiency", 0.0, maxPt, 0.0, 1.0, "hist,same,pl");

        TLegend *pid_eff_pleg = new TLegend(0.2, 0.08);
        pid_eff_pleg->AddEntry(h_ml_pid_eff, ml_label);
        pid_eff_pleg->AddEntry(h_ns_pid_eff, ns_label);
        pid_eff_pleg->Draw();

        latex->DrawLatex(0.90, 0.95, Form("PDG: %d", pids[i]));

        // Purity
        c1.cd(2);

        auto h_ml_purity = (TH1F*)h_ml_tp[i]->Clone();
        h_ml_purity->Sumw2();
        h_ml_purity->Divide(h_ml_selected[i]);
        draw_hist(h_ml_purity, mlStyle, mlColor, "Purity", ptTitle.data(), "Purity", 0.0, maxPt, 0.0, 1.0, "hist,same,pl");

        auto h_ns_purity = (TH1F*)h_ns_tp[i]->Clone();
        h_ns_purity->Sumw2();
        h_ns_purity->Divide(h_ns_selected[i]);
        draw_hist(h_ns_purity, nsStyle, nsColor, "Purity", ptTitle.data(), "Purity", 0.0, maxPt, 0.0, 1.0, "hist,same,pl");

        TLegend *pur_pleg = new TLegend(0.2, 0.08);
        pur_pleg->AddEntry(h_ml_purity, ml_label);
        pur_pleg->AddEntry(h_ns_purity, ns_label);
        pur_pleg->Draw();

        latex->DrawLatex(0.90, 0.95, Form("PDG: %d", pids[i]));

        // Full Efficiency
        c1.cd(3);

        auto h_recon_eff = h_mc_tracked;
        h_recon_eff->Sumw2();
        h_recon_eff->Divide(h_mc_positive);
        draw_hist(h_recon_eff, contextStyle, contextColor, "Full Efficiency", ptTitle.data(), "Efficiency", 0.0, maxPt, 0.0, 1.0, "hist,same,pl");

        auto h_ml_full_eff = (TH1F*)h_ml_tp[i]->Clone();
        h_ml_full_eff->Sumw2();
        h_ml_full_eff->Divide(h_mc_positive);
        draw_hist(h_ml_full_eff, mlStyle, mlColor, "Full Efficiency", ptTitle.data(), "Efficiency", 0.0, maxPt, 0.0, 1.0, "hist,same,pl");

        auto h_ns_full_eff = (TH1F*)h_ns_tp[i]->Clone();
        h_ns_full_eff->Sumw2();
        h_ns_full_eff->Divide(h_mc_positive);
        draw_hist(h_ns_full_eff, nsStyle, nsColor, "Full Efficiency", ptTitle.data(), "Efficiency", 0.0, maxPt, 0.0, 1.0, "hist,same,pl");

        TLegend *full_eff_pleg = new TLegend(0.2, 0.1);
        full_eff_pleg->AddEntry(h_recon_eff, "Reconstruction");
        full_eff_pleg->AddEntry(h_ml_full_eff, ml_label);
        full_eff_pleg->AddEntry(h_ns_full_eff, ns_label);
        full_eff_pleg->Draw();

        latex->DrawLatex(0.90, 0.95, Form("PDG: %d", pids[i]));


        // Save
        c1.SaveAs(Form("%s/pid_%d_cert_%d_n_%.1f.png", saveDir, searchedPid, (int) (certaintyThreshold * 100), nSigmaThreshold));
    }

    histFile.Close();
    aodFile.Close();
}

