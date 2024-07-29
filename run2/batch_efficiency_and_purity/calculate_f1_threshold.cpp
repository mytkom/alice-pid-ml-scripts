#include "ROOT/RDataFrame.hxx"
#include "TFile.h"
#include "TDirectoryFile.h"
#include "TMap.h"
#include "TKey.h"
#include "TClass.h"
#include "TList.h"
#include <iostream>
#include <cmath>
#include <vector>

// Function to calculate f1 score
float f1_score(float precision, float recall, float eps = 1e-10) {
    return 2 * precision * recall / (precision + recall + eps);
}

// Function to calculate precision and recall given a threshold
std::pair<float, float> calculate_precision_recall(std::vector<TTree*>& trees, float threshold, int searchedPid) {
    float tp = 0;
    float positives = 0;
    float selected = 0;
    float fp = 0;

    for (TTree* tree : trees) {
      ROOT::RDataFrame df(*tree);
      auto df_searched_only = df.Filter([searchedPid](int pid) {
          return  searchedPid == pid;
      }, {"fPid"});
      auto df_filtered = df_searched_only.Filter([threshold](float mlCertainty) {
          return  mlCertainty > threshold;
      }, {"fMlCertainty"});

      auto true_positives = df_filtered.Filter("fIsPidMC == true").Count();
      auto total_positives = df_searched_only.Filter("fIsPidMC == true").Count();
      auto total_selected = df_filtered.Count();

      tp += *true_positives;
      positives += *total_positives;
      selected += *total_selected;
      fp += *total_selected - *true_positives;
    }

    float precision = tp / (tp + fp);
    float recall = tp / positives;

    return {precision, recall};
}

void find_best_threshold(std::vector<TTree*> trees, int pid) {
    // float best_f1 = 0.0;
    // float best_threshold = 0.0;
    // float low = 0.0;
    // float high = 1.0;
    // float eps = 1e-4; // precision of the binary search

    // while (high - low > eps) {
    //     float mid1 = low + (high - low) / 3;
    //     float mid2 = high - (high - low) / 3;

    //     auto [precision1, recall1] = calculate_precision_recall(trees, mid1, pid);
    //     auto [precision2, recall2] = calculate_precision_recall(trees, mid2, pid);

    //     float f1_1 = f1_score(precision1, recall1);
    //     float f1_2 = f1_score(precision2, recall2);

    //     if (f1_1 > f1_2) {
    //         high = mid2;
    //     } else {
    //         low = mid1;
    //     }

    //     if (f1_1 > best_f1) {
    //         best_f1 = f1_1;
    //         best_threshold = mid1;
    //     }
    //     if (f1_2 > best_f1) {
    //         best_f1 = f1_2;
    //         best_threshold = mid2;
    //     }
    // }

    float best_f1 = 0.0;
    float best_threshold = 0.0;
    float threshold = 0.5; // Start with the middle value
    float learning_rate = 0.001; // Learning rate for gradient ascent
    float tolerance = 1e-4; // Tolerance to stop the gradient ascent
    float gradient;
    float eps = 1e-10; // Small value to prevent division by zero

    while (true) {
        auto [precision, recall] = calculate_precision_recall(trees, threshold, pid);
        float f1 = f1_score(precision, recall, eps);

        // Calculate the gradient as the numerical derivative of the f1 score
        auto [precision_up, recall_up] = calculate_precision_recall(trees, threshold + tolerance, pid);
        float f1_up = f1_score(precision_up, recall_up, eps);

        gradient = (f1_up - f1) / tolerance;

        // Update the threshold
        float new_threshold = threshold + learning_rate * gradient;

        // Check if the change is smaller than the tolerance
        if (std::abs(new_threshold - threshold) < tolerance) {
            break;
        }

        threshold = new_threshold;

        if (f1 > best_f1) {
            best_f1 = f1;
            best_threshold = threshold;
        }
    }

    // Print the results for this tree
    std::cout << "Best threshold: " << best_threshold << std::endl;
    std::cout << "Maximum f1 score: " << best_f1 << std::endl;
}

// Function to extract TTrees from TDirectoryFile
void extract_ttrees(TDirectoryFile* dir, std::vector<TTree*>& trees) {
    TList* keys = dir->GetListOfKeys();
    TIter next(keys);
    TKey* key;

    while ((key = (TKey*)next())) {
        if (key->GetClassName() == std::string("TTree")) {
            TTree* tree = (TTree*)dir->Get(key->GetName());
            trees.push_back(tree);
        } else if (key->GetClassName() == std::string("TDirectoryFile")) {
            TDirectoryFile* subdir = (TDirectoryFile*)dir->Get(key->GetName());
            extract_ttrees(subdir, trees);
        }
    }
}

// Main function
void calculate_f1_threshold(const char* fileName, int pid) {
    ROOT::EnableImplicitMT();
    // Open the ROOT file
    TFile file(fileName);

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

    // Calculate the best threshold for each TTree
    find_best_threshold(trees, pid);

    // Close the file
    file.Close();
}

