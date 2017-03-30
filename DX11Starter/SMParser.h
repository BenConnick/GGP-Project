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
	// returns success
	bool OpenFile(char* filepath) {
		string line;
		ifstream myfile(filepath);
		if (myfile.is_open())
		{
			while (getline(myfile, line))
			{
				// debug
				cout << line << '\n';

				// 4 notes and a newline
				if (line.size() == 4) {
					notes.push_back(NoteFromLine(line));
				}
			}
			cout << "\n";
			cout << "notes: " << notes.size();

			myfile.close();
		}

		else return false;

		return true;
	}

	int GetNote(int index) {
		return notes[index];
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
	vector<int> notes;
};