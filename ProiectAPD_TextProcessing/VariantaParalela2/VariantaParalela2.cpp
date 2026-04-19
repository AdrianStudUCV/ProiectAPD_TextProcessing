// VariantaParalela2.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <mpi.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <chrono>

using namespace std;

// Set de cuvinte de legatura (Stop Words) - fara diacritice
const unordered_set<string> stopWords = {
    "the", "a", "an", "and", "or", "but", "is", "are", "was", "were", "to", "of", "in", "for", "with", "on", "at", "by", "from", "it", "this", "that",
    "si", "la", "de", "un", "o", "pe", "cu", "din", "ca", "sa", "prin", "pentru", "este", "sunt", "fost", "care", "dar", "sau", "nu", "mai"
};

// Functie de normalizare: doar alfabet englez, litere mici
string normalize(string word) {
    string cleanWord = "";
    for (char c : word) {
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
            cleanWord += (char)tolower(c);
        }
    }
    return cleanWord;
}

int main(int argc, char** argv) {
    // 1. Initializarea mediului MPI
    MPI_Init(&argc, &argv);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size); // Numarul total de procese

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank); // Identificatorul procesului curent

    string fileName = "D:\\Notepad++\\SAVES\\test_1000mb.txt";
    int N = 10;

    auto startTime = chrono::high_resolution_clock::now();

    // 2. Determinarea dimensiunii fisierului (doar de catre Rank 0)
    long fileSize = 0;
    if (world_rank == 0) {
        ifstream file(fileName, ios::binary | ios::ate);
        if (file.is_open()) {
            fileSize = file.tellg();
            file.close();
        }
    }

    // Distribuim dimensiunea fisierului catre toate procesele
    MPI_Bcast(&fileSize, 1, MPI_LONG, 0, MPI_COMM_WORLD);

    if (fileSize == 0) {
        if (world_rank == 0) cerr << "Eroare: Fisier gol sau inexistent!" << endl;
        MPI_Finalize();
        return 1;
    }

    // 3. Impartirea muncii [cite: 46]
    long chunkSize = fileSize / world_size;
    long start = world_rank * chunkSize;
    long end = (world_rank == world_size - 1) ? fileSize : (world_rank + 1) * chunkSize;
    long localSize = end - start;

    // 4. Citirea locala in buffer si procesarea 
    ifstream file(fileName, ios::binary);
    file.seekg(start);
    vector<char> buffer(localSize);
    file.read(buffer.data(), localSize);
    file.close();

    unordered_map<string, int> localMap;
    string currentWord = "";
    for (char c : buffer) {
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
            currentWord += (char)tolower(c);
        }
        else {
            if (!currentWord.empty() && stopWords.find(currentWord) == stopWords.end()) {
                localMap[currentWord]++;
            }
            currentWord = "";
        }
    }

    // 5. Agregarea rezultatelor (Reduce-ul conceptual) 
    // In MPI, pentru structuri complexe ca map-urile, trimitem datele catre Rank 0
    if (world_rank != 0) {
        // Trimitem numarul de cuvinte unice
        int numUnique = localMap.size();
        MPI_Send(&numUnique, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

        for (auto const& [word, count] : localMap) {
            int wordLen = word.length();
            MPI_Send(&wordLen, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
            MPI_Send(word.c_str(), wordLen, MPI_CHAR, 0, 2, MPI_COMM_WORLD);
            MPI_Send(&count, 1, MPI_INT, 0, 3, MPI_COMM_WORLD);
        }
    }
    else {
        // Master-ul (Rank 0) primeste si combina datele
        unordered_map<string, int> globalMap = localMap;

        for (int i = 1; i < world_size; ++i) {
            int remoteUnique;
            MPI_Recv(&remoteUnique, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            for (int j = 0; j < remoteUnique; ++j) {
                int wordLen;
                MPI_Recv(&wordLen, 1, MPI_INT, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                vector<char> wordBuf(wordLen + 1, 0);
                MPI_Recv(&wordBuf[0], wordLen, MPI_CHAR, i, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                int count;
                MPI_Recv(&count, 1, MPI_INT, i, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                string word(wordBuf.begin(), wordBuf.begin() + wordLen);
                globalMap[word] += count;
            }
        }

        // 6. Sortare si afisare finala
        vector<pair<string, int>> sortedWords(globalMap.begin(), globalMap.end());
        sort(sortedWords.begin(), sortedWords.end(), [](const pair<string, int>& a, const pair<string, int>& b) {
            return a.second > b.second;
            });

        auto endTime = chrono::high_resolution_clock::now();
        chrono::duration<double> duration = endTime - startTime;

        cout << "\n--- TOP " << N << " CUVINTE (Varianta MS-MPI) ---" << endl;
        for (int i = 0; i < min((int)sortedWords.size(), N); ++i) {
            cout << i + 1 << ". " << sortedWords[i].first << ": " << sortedWords[i].second << endl;
        }
        cout << "\nProcesare MPI finalizata in: " << duration.count() << " secunde." << endl;
    }

    MPI_Finalize();
    return 0;
}