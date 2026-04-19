// VariantaParalela1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <cctype>
#include <chrono>
#include <thread>
#include <mutex>

using namespace std;

// Set de cuvinte de legatura (Stop Words)
const unordered_set<string> stopWords = {
    "the", "a", "an", "and", "or", "but", "is", "are", "was", "were", "to", "of", "in", "for", "with", "on", "at", "by", "from", "it", "this", "that",
    "si", "la", "de", "un", "o", "pe", "cu", "din", "ca", "sa", "prin", "pentru", "este", "sunt", "fost", "care", "dar", "sau", "nu", "mai"
};

mutex mtx; // Folosit doar pentru faza finala de combinare a rezultatelor

string normalize(string word) {
    string cleanWord = "";
    for (char c : word) {
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
            cleanWord += (char)tolower(c);
        }
    }
    return cleanWord;
}

// Functia pe care o va rula fiecare thread
void processChunk(string fileName, long start, long end, unordered_map<string, int>& globalMap) {
    ifstream file(fileName, ios::binary);
    file.seekg(start);

    // Daca nu suntem la inceputul fisierului, sarim peste fragmentul de cuvant curent
    // pana la primul spatiu, deoarece thread-ul anterior se va ocupa de el
    if (start != 0) {
        string skip;
        file >> skip;
    }

    unordered_map<string, int> localMap;
    string rawWord;

    while (file.tellg() < end && file >> rawWord) {
        string cleanWord = normalize(rawWord);
        if (!cleanWord.empty() && stopWords.find(cleanWord) == stopWords.end()) {
            localMap[cleanWord]++;
        }
    }

    // Faza de Reduce: Combinam localMap in globalMap folosind un mutex
    lock_guard<mutex> lock(mtx);
    for (auto const& [word, count] : localMap) {
        globalMap[word] += count;
    }
}

int main() {
    string fileName = "D:\\Notepad++\\SAVES\\test_1000mb.txt";
    int N = 10;
    unsigned int numThreads = thread::hardware_concurrency(); // Detecteaza cate nuclee are procesorul tau

    ifstream file(fileName, ios::binary | ios::ate);
    if (!file.is_open()) {
        cerr << "Eroare la deschiderea fisierului!" << endl;
        return 1;
    }

    long fileSize = file.tellg();
    file.close();

    unordered_map<string, int> finalWordFrequency;
    vector<thread> threads;
    long chunkSize = fileSize / numThreads;

    cout << "Incep procesarea paralela cu " << numThreads << " thread-uri..." << endl;
    auto startTime = chrono::high_resolution_clock::now();

    // Lansarea thread-urilor
    for (int i = 0; i < numThreads; ++i) {
        long start = i * chunkSize;
        long end = (i == numThreads - 1) ? fileSize : (i + 1) * chunkSize;
        threads.emplace_back(processChunk, fileName, start, end, ref(finalWordFrequency));
    }

    // Asteptam ca toate thread-urile sa termine
    for (auto& t : threads) {
        t.join();
    }

    // Sortare rezultate (aceeasi logica)
    vector<pair<string, int>> sortedWords(finalWordFrequency.begin(), finalWordFrequency.end());
    sort(sortedWords.begin(), sortedWords.end(), [](const pair<string, int>& a, const pair<string, int>& b) {
        return a.second > b.second;
        });

    auto endTime = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = endTime - startTime;

    cout << "\n--- TOP " << N << " CUVINTE (Varianta Paralela) ---" << endl;
    for (int i = 0; i < min((int)sortedWords.size(), N); ++i) {
        cout << i + 1 << ". " << sortedWords[i].first << ": " << sortedWords[i].second << endl;
    }

    cout << "\nProcesare paralela finalizata in: " << duration.count() << " secunde." << endl;

    return 0;
}
