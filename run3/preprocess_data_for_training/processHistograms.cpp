#include "TH1F.h"
#include "TH2F.h"
#include <TCanvas.h>
#include <TColor.h>
#include <TFile.h>
#include <TLegend.h>
#include <TStyle.h>
#include <TVirtualPad.h>
#include <cassert>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace fs = std::filesystem;

const std::vector<std::string> th2fHistNames = {
    "minus/hTPCSigvsP", "minus/hTOFBetavsP", "minus/hTOFSigvsP",
    "minus/filtered/hTOFSigvsP", "plus/hTPCSigvsP", "plus/hTOFBetavsP",
    "plus/hTOFSigvsP", "plus/filtered/hTOFSigvsP"
};

const std::vector<std::string> th1fHistNames = {
    "minus/hP", "minus/hPt", "minus/hPx", "minus/hPy", "minus/hPz",
    "minus/hX", "minus/hY", "minus/hZ", "minus/hAlpha", "minus/hTrackType",
    "minus/hTPCNClsShared", "minus/hDcaXY", "minus/hDcaZ", "plus/hP",
    "plus/hPt", "plus/hPx", "plus/hPy", "plus/hPz", "plus/hX", "plus/hY",
    "plus/hZ", "plus/hAlpha", "plus/hTrackType", "plus/hTPCNClsShared",
    "plus/hDcaXY", "plus/hDcaZ"
};


struct HistCollection {
    std::vector<TH2F> normalized2D;
    std::vector<TH2F> summed2D;
    std::vector<TH1F> normalized1D;
    std::vector<TH1F> summed1D;
};

std::vector<std::unique_ptr<TFile>> getRootFiles(const std::string &dirPath) {
    std::vector<std::unique_ptr<TFile>> files;
    for (const auto &entry : fs::directory_iterator(dirPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".root") {
            auto file = std::make_unique<TFile>(entry.path().c_str());
            if (!file->IsZombie()) {
                files.push_back(std::move(file));
            } else {
                std::cerr << "Failed to open file " << entry.path() << std::endl;
            }
        }
    }
    if (files.empty()) {
        std::cerr << "No valid .root files found in directory " << dirPath << std::endl;
    }
    return files;
}

template<typename THist>
THist* sumHistograms(const std::vector<std::unique_ptr<TFile>>& files, const char *histName) {
    THist *sumHist = nullptr;
    for (const auto &file : files) {
        auto *hist = dynamic_cast<THist*>(file->Get(Form("pid-ml-producer/%s", histName)));
        if (!hist) {
            std::cerr << "Histogram " << histName << " not found in file " << file->GetName() << std::endl;
            continue;
        }
        if (!sumHist) {
            sumHist = static_cast<THist*>(hist->Clone());
            sumHist->SetDirectory(nullptr);
        } else {
            sumHist->Add(hist);
        }
    }
    if (!sumHist) {
        std::cerr << "No histograms found for " << histName << std::endl;
    }
    return sumHist;
}

template<typename THist>
void normalizeHistogram(THist& hist) {
    if (hist.Integral() > 0) {
        hist.Scale(1.0 / hist.Integral());
    }
}

HistCollection mergeHistograms(const std::string &dirPath) {
    HistCollection collection;
    auto files = getRootFiles(dirPath);

    for (const auto &histName : th2fHistNames) {
        auto summedHist = sumHistograms<TH2F>(files, histName.c_str());
        if (summedHist) {
            collection.summed2D.push_back(*summedHist);
            collection.normalized2D.push_back(*summedHist);
            normalizeHistogram(collection.normalized2D.back());
        }
    }

    for (const auto &histName : th1fHistNames) {
        auto summedHist = sumHistograms<TH1F>(files, histName.c_str());
        if (summedHist) {
            collection.summed1D.push_back(*summedHist);
            collection.normalized1D.push_back(*summedHist);
            normalizeHistogram(collection.normalized1D.back());
        }
    }

    return collection;
}

template<typename THist>
void drawAndSaveHistograms(std::vector<THist>& hists, const std::string& path, bool isLog, std::optional<std::pair<double, double>> range, TCanvas& canvas) {
    for (size_t i = 0; i < hists.size(); ++i) {
        if constexpr (std::is_same_v<THist, TH2F>) {
            gPad->SetLogy(0);
            gPad->SetLogz(isLog ? 1 : 0);
            hists[i].Draw("colz");
            if (range) {
                hists[i].GetZaxis()->SetRangeUser(range->first, range->second);
            }
        } else if constexpr (std::is_same_v<THist, TH1F>) {
            gPad->SetLogy(isLog ? 1 : 0);
            gPad->SetLogz(0);
            hists[i].Draw("HIST E0");
            if (range) {
                hists[i].GetYaxis()->SetRangeUser(range->first, range->second);
            }
        }
        canvas.Print((path + "/" + (std::is_same_v<THist, TH2F> ? th2fHistNames[i] : th1fHistNames[i]) + ".png").c_str());
    }
}

template<typename THist>
void drawDividedHistograms(std::vector<THist>& normHistsA, std::vector<THist>& normHistsB, const std::string& outputPath, bool isLog, std::optional<std::pair<double, double>> range, TCanvas& canvas) {
    std::vector<THist> divHists;
    divHists.reserve(normHistsA.size());
    for (size_t i = 0; i < normHistsA.size(); ++i) {
        auto divHist = static_cast<THist*>(normHistsA[i].Clone());
        divHist->Divide(&normHistsB[i]);
        divHists.push_back(*divHist);
    }
    drawAndSaveHistograms(divHists, outputPath, isLog, range, canvas);
}

void processHistograms(const std::string &analysisDirPathA, const std::string &analysisDirPathB, const std::string &outputDir) {
    TCanvas canvas("canvas", "canvas", 0, 0, 1920, 1080);
    gStyle->SetPalette(kRainBow);

    std::vector<std::string> subDirs = {
        "/normalized/minus/filtered", "/normalized/plus/filtered",
        "/minus/filtered", "/plus/filtered"
    };

    for (const auto &subDir : subDirs) {
        fs::create_directories(analysisDirPathA + subDir);
        fs::create_directories(analysisDirPathB + subDir);
        fs::create_directories(outputDir + subDir);
    }

    // Load histograms from directories
    auto histsA = mergeHistograms(analysisDirPathA);
    auto histsB = mergeHistograms(analysisDirPathB);

    assert(histsA.normalized2D.size() == histsB.normalized2D.size());
    assert(histsA.normalized1D.size() == histsB.normalized1D.size());

    drawAndSaveHistograms(histsA.normalized2D, analysisDirPathA + "/normalized", true, std::make_pair<double>(1e-10, 1), canvas);
    drawAndSaveHistograms(histsB.normalized2D, analysisDirPathB + "/normalized", true, std::make_pair<double>(1e-10, 1), canvas);
    drawAndSaveHistograms(histsA.summed2D, analysisDirPathA, true, {}, canvas);
    drawAndSaveHistograms(histsB.summed2D, analysisDirPathB, true, {}, canvas);

    drawAndSaveHistograms(histsA.normalized1D, analysisDirPathA + "/normalized", true, std::make_pair<double>(1e-10, 1), canvas);
    drawAndSaveHistograms(histsB.normalized1D, analysisDirPathB + "/normalized", true, std::make_pair<double>(1e-10, 1), canvas);
    drawAndSaveHistograms(histsA.summed1D, analysisDirPathA, false, {}, canvas);
    drawAndSaveHistograms(histsB.summed1D, analysisDirPathB, false, {}, canvas);

    drawDividedHistograms(histsA.normalized2D, histsB.normalized2D, outputDir, true, std::make_pair<double>(1e-2, 1e2), canvas);
    drawDividedHistograms(histsA.summed2D, histsB.summed2D, outputDir, false, std::make_pair<double>(1e-2, 1e2), canvas);
    drawDividedHistograms(histsA.normalized1D, histsB.normalized1D, outputDir, true, {}, canvas);
    drawDividedHistograms(histsA.summed1D, histsB.summed1D, outputDir, false, {}, canvas);
}

