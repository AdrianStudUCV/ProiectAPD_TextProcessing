// VariantaSecventiala.cpp : This file contains the 'main' function. Program execution begins and ends there.
//m

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <cctype>
#include <chrono>

using namespace std;

// Set de cuvinte de legatura (Stop Words) pentru Engleza si Romana
const unordered_set<string> stopWords = {
    // English
    "the", "a", "an", "and", "or", "but", "is", "are", "was", "were", "to", "of", "in", "for", "with", "on", "at", "by", "from", "it", "this", "that",
    // Romana (fara diacritice conform cerintei)
    "si", "la", "de", "un", "o", "pe", "cu", "din", "ca", "sa", "ca", "prin", "pentru", "este", "sunt", "fost", "care", "dar", "sau", "nu", "mai"
};

// Functie care pastreaza doar caracterele alfabetului englez (A-Z, a-z)
string normalize(string word) {
    string cleanWord = "";
    for (char c : word) {
        // Verificam daca este caracter din alfabetul englez (ASCII standard)
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
            cleanWord += (char)tolower(c);
        }
        // Orice altceva (diacritice, cifre, punctuatie) este ignorat complet
    }
    return cleanWord;
}

int main() {
    string fileName = "D:\\Notepad++\\SAVES\\test_1000mb.txt";
    int N = 10;

    ifstream file(fileName);
    if (!file.is_open()) {
        cerr << "Eroare la deschiderea fisierului!" << endl;
        return 1;
    }

    unordered_map<string, int> wordFrequency;
    string rawWord;

    cout << "Incep procesarea secventiala..." << endl;
    auto start = chrono::high_resolution_clock::now();

    while (file >> rawWord) {
        string cleanWord = normalize(rawWord);

        // Verificam daca dupa curatare cuvantul nu e gol si NU este in lista de stop words
        if (!cleanWord.empty() && stopWords.find(cleanWord) == stopWords.end()) {
            wordFrequency[cleanWord]++;
        }
    }
    file.close();

    // Sortare rezultate
    vector<pair<string, int>> sortedWords(wordFrequency.begin(), wordFrequency.end());
    sort(sortedWords.begin(), sortedWords.end(), [](const pair<string, int>& a, const pair<string, int>& b) {
        return a.second > b.second;
        });

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;

    cout << "\n--- TOP " << N << " CUVINTE RELEVANTE (fara legatura) ---" << endl;
    for (int i = 0; i < min((int)sortedWords.size(), N); ++i) {
        cout << i + 1 << ". " << sortedWords[i].first << ": " << sortedWords[i].second << endl;
    }

    cout << "\nProcesare finalizata in: " << duration.count() << " secunde." << endl;

    return 0;
}