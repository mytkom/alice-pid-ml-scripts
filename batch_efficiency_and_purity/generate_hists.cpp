#include "TFile.h"
#include "TTree.h"
#include "TDirectoryFile.h"
#include "TMap.h"
#include "TKey.h"
#include "TClass.h"
#include "TList.h"
#include "TH1F.h"
#include "TCanvas.h"
#include <iostream>
#include <vector>

#define PT_TITLE "#it{p}_{T} (GeV/#it{c})"
#define MAX_PT 5.0f

// Function to extract TTrees from TDirectoryFile
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

// Main function
void generate_hists(const char* fileName, float certaintyThreshold, float nSigmaThreshold) {
    // Open the ROOT file
    TFile file(fileName, "READ");

    // Check if the file is opened successfully
    if (file.IsZombie()) {
        std::cerr << "Error opening file: " << fileName << std::endl;
        return;
    }

    // Get the TMap containing TDirectoryFiles
    TMap* map = (TMap*)file.Get("parentFiles");
    if (!map) {
        std::cerr << "Error: TMap 'parentFiles' not found in file: " << fileName << std::endl;
        return;
    }

    // Vector to store TTrees
    std::vector<TTree*> trees;

    // Extract TTrees from all TDirectoryFiles in the TMap
    TIter next(map);
    while (TObject* obj = next()) {
        const char* dirName = obj->GetName();
        TDirectoryFile* dir = (TDirectoryFile*) file.Get(dirName);
        extract_ttrees(dir, trees);
    }

    constexpr int pidsCount = 6;
    constexpr int pids[pidsCount] = {-211, 211, -321, 321, 2212, -2212};
    std::vector<TH1F*> h_ml_tp(pidsCount), h_ml_fp(pidsCount), h_ml_selected(pidsCount), h_ns_tp(pidsCount), h_ns_fp(pidsCount), h_ns_selected(pidsCount), h_total_positives(pidsCount);

    for (size_t i = 0; i < pidsCount; ++i) {
        int searchedPid = pids[i];
        h_ns_tp[i] = new TH1F(Form("h_ns_tp_%d", searchedPid), Form("NSigma True Positives for PID %d", searchedPid), 100, 0, 5);
        h_ns_fp[i] = new TH1F(Form("h_ns_fp_%d", searchedPid), Form("NSigma False Positives for PID %d", searchedPid), 100, 0, 5);
        h_ml_tp[i] = new TH1F(Form("h_ml_tp_%d", searchedPid), Form("ML True Positives for PID %d", searchedPid), 100, 0, 5);
        h_ml_fp[i] = new TH1F(Form("h_ml_fp_%d", searchedPid), Form("ML False Positives for PID %d", searchedPid), 100, 0, 5);
        h_total_positives[i] = new TH1F(Form("h_total_positives_%d", searchedPid), Form("Total Positives for PID %d", searchedPid), 100, 0, 5);
        h_ml_selected[i] = new TH1F(Form("h_ml_selected_%d", searchedPid), Form("NSigma Selected for PID %d", searchedPid), 100, 0, 5);
        h_ns_selected[i] = new TH1F(Form("h_ns_selected_%d", searchedPid), Form("ML Selected for PID %d", searchedPid), 100, 0, 5);
    }

    for (TTree* tree : trees) {
        int pid;
        float mlCertainty;
        bool isPidMC;
        float pt;
        float nSigma;

        tree->SetBranchAddress("fPid", &pid);
        tree->SetBranchAddress("fMlCertainty", &mlCertainty);
        tree->SetBranchAddress("fIsPidMC", &isPidMC);
        tree->SetBranchAddress("fPt", &pt);
        tree->SetBranchAddress("fNSigma", &nSigma);

        Long64_t nentries = tree->GetEntries();
        for (Long64_t i = 0; i < nentries; ++i) {
            tree->GetEntry(i);
            for (size_t j = 0; j < pidsCount; ++j) {
                int searchedPid = pids[j];
                if (pid == searchedPid) {
                    if(isPidMC) {
                      h_total_positives[j]->Fill(pt);
                    }

                    if (mlCertainty > certaintyThreshold) {
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

    for (size_t i = 0; i < pidsCount; ++i) {
        int searchedPid = pids[i];

        auto h_ml_recall = (TH1F*)h_ml_tp[i]->Clone();
        h_ml_recall->Sumw2();
        h_ml_recall->Divide(h_total_positives[i]);

        auto h_ml_precision = (TH1F*)h_ml_tp[i]->Clone();
        h_ml_precision->Sumw2();
        h_ml_precision->Divide(h_ml_selected[i]);

        auto h_ns_recall = (TH1F*)h_ns_tp[i]->Clone();
        h_ns_recall->Sumw2();
        h_ns_recall->Divide(h_total_positives[i]);

        auto h_ns_precision = (TH1F*)h_ns_tp[i]->Clone();
        h_ns_precision->Sumw2();
        h_ns_precision->Divide(h_ns_selected[i]);

        // Draw histograms
        TCanvas c1("c1", "PID Efficiency and Purity", 1600, 600);
        c1.Divide(2, 1);

        c1.cd(1);
        draw_hist(h_ml_recall, 20, 38, "Efficiency", PT_TITLE, "Efficiency", 0.0, MAX_PT, 0.0, 1.0, "he,same");
        draw_hist(h_ns_recall, 23, 46, "Efficiency", PT_TITLE, "Efficiency", 0.0, MAX_PT, 0.0, 1.0, "he,same");

        c1.cd(2);
        draw_hist(h_ml_precision, 20, 38, "Purity", PT_TITLE, "Purity", 0.0, MAX_PT, 0.0, 1.0, "he,same");
        draw_hist(h_ns_precision, 23, 46, "Purity", PT_TITLE, "Purity", 0.0, MAX_PT, 0.0, 1.0, "he,same");

        c1.SaveAs(Form("precision_recall_histograms_pid_%d.png", searchedPid));
    }

    // Close the file
    file.Close();
}

