#include "TH1F.h"
#include "TH2F.h"
#include <TCanvas.h>
#include <TColor.h>
#include <TFile.h>
#include <TH1.h>
#include <TLegend.h>
#include <TStyle.h>
#include <TVirtualPad.h>
#include <vector>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

void draw_hist(std::vector<TFile*> files, TCanvas* canvas, const char* histName, const char* outputDir);

void save_histos(const std::string& directoryPath, const std::string& outputDir = "graphs") {
    TCanvas *canvas = new TCanvas("canvas", "canvas", 0, 0, 1920, 1080);

    gStyle->SetPalette(kRainBow);

    // Create output directories
    fs::create_directories(outputDir);
    fs::create_directories(outputDir + "/minus/filtered");
    fs::create_directories(outputDir + "/plus/filtered");

    // Collect .root files from the directory
    std::vector<TFile*> files;
    for (const auto& entry : fs::directory_iterator(directoryPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".root") {
            TFile *f = new TFile(entry.path().c_str());
            if (f->IsZombie()) {
                std::cerr << "Failed to open file " << entry.path() << std::endl;
                delete f;
                continue;
            }
            files.push_back(f);
        }
    }

    if (files.empty()) {
        std::cerr << "No valid .root files found in directory " << directoryPath << std::endl;
        return;
    }

    std::cout << "Files to read: " << files.size() << std::endl;

    draw_hist(files, canvas, "minus/hTPCSigvsP", outputDir.c_str());
    draw_hist(files, canvas, "minus/hTOFBetavsP", outputDir.c_str());
    draw_hist(files, canvas, "minus/hTOFSigvsP", outputDir.c_str());
    draw_hist(files, canvas, "minus/filtered/hTOFSigvsP", outputDir.c_str());
    draw_hist(files, canvas, "plus/hTPCSigvsP", outputDir.c_str());
    draw_hist(files, canvas, "plus/hTOFBetavsP", outputDir.c_str());
    draw_hist(files, canvas, "plus/hTOFSigvsP", outputDir.c_str());
    draw_hist(files, canvas, "plus/filtered/hTOFSigvsP", outputDir.c_str());

    // Cleanup
    for (auto& f : files) {
        f->Close();
        delete f;
    }
}

void draw_hist(std::vector<TFile*> files, TCanvas* canvas, const char* histName, const char* outputDir) {
    TH2F* sumHist = nullptr;

    for (auto& f : files) {
        TH2F* hist = (TH2F*)f->Get(Form("pid-ml-producer/%s", histName));
        if (!hist) {
            std::cerr << "Histogram " << histName << " not found in file " << f->GetName() << std::endl;
            continue;
        }
        if (!sumHist) {
            sumHist = (TH2F*)hist->Clone();
            sumHist->SetDirectory(0); // To avoid deletion when the file is closed
        } else {
            sumHist->Add(hist);
        }
    }

    if (!sumHist) {
        std::cerr << "No histograms found for " << histName << std::endl;
        return;
    }

    gPad->SetLogz();
    sumHist->Draw("colz");
    canvas->Print(Form("%s/%s.png", outputDir, histName));
}

