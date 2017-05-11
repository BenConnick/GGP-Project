#pragma once
#include <vector>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

// parses stepmania (.sm) files 
class SMParser {
public:
	SMParser() {
	}
	~SMParser() {
	}
	// the beats per minute
	int BPMS = 0;
	// the smallest note size
	int MaxNotesPerMeasure = 4;
	// current measure
	int measureNum = 0;

	// returns success
	bool OpenFile(char* filepath) {
		string line;
		ifstream myfile(filepath);
		bool bpmsFound = false;
		bool startCodeFound = false;
		if (myfile.is_open())
		{
			// start with an empty measure
			measures = vector<vector<int>>();
			// first measure
			measures.push_back(vector<int>());

			while (getline(myfile, line))
			{
				// debug - print out every line in the file
	//			cout << line << '\n';

				// look for beats per minute
				if (!bpmsFound) {
					if (line.find("#BPMS:") != string::npos) {
						int startIdx = line.find("=") + 1;
						// get bpms and convert string to int
						BPMS = stoi(line.substr(startIdx));
						bpmsFound = true;
					}
					else {
						continue;
						// don't start looking for data until bpms is found
					}
				}

				// collect valid data
				if (!startCodeFound) {
					// START CODE
					if (line.find("     Easy:") != string::npos) {
		//				cout << "start";
						// start code found, begin collecting notes
						startCodeFound = true;
						continue;
					}
				}
				else {
					// END CODE
					if (line.find("#NOTES:") != std::string::npos) {
						// end code found, stop collecting notes
			//			cout << "end";
						startCodeFound = false;
						break;
					}

					// comma marks start of a measure
					if (line == ",") {
						// deal with previous measure
						if (measures[measures.size() - 1].size() > MaxNotesPerMeasure) {
							MaxNotesPerMeasure = measures[measures.size() - 1].size();
						}

						// new measure
						measures.push_back(vector<int>());
						continue;
					}
					else if (line.size() == 4) {
						measures[measures.size()-1].push_back(NoteFromLine(line));
					}
				}
			}

			// done, print success
	//		cout << "\n notes: " << measures.size() << "\n";

			// close stream
			myfile.close();

			// return success (file read)
			return true;
		}
		// file was not opened
		else { return false; }
	}

	// pointer to measure
	vector<int>* GetMeasure(int index) {
		return &measures[index];
	}

	int GetNote(int measure, int index) {
		return measures[measure][index];
	}
private:
	// takes the 
	int NoteFromLine(string input) {
		int numValuesPerLine = 4;
		int numLanes = 3;
		for (int i = 0; i < numValuesPerLine; i++) {
			if (isdigit(input[i])) {
				// value
				if ((int)(input[i] - '0') > 0) {
					return i % numLanes;
				};
			}
		}
		// no notes
		return -1;
	}
	vector<vector<int>> measures;
};