#include "Utils.h"
#include "Parser.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <regex>

using namespace std;
using namespace rle;

Board rle::parse(const char* fn) {
    ifstream f(fn);
    if (f.good()) {
        Board b = Board(0, 0, "unknown", "");
        string line;
        bool dimensionsGotten = false;
        bool forcedEOF = false;
        unsigned xpos = 0, ypos = 0;
        string quantity = "";
        while (getline(f, line) && !forcedEOF) {
            // Remove empty lines
            if (line.empty()) continue;

            if (line[0] == '#') {
                // Line is a comment
                parseComment(b, line.substr(1, line.npos));
            } else if (!dimensionsGotten) {
                // Line is a dimension
                // Match with regex
                parseDimension(b, line);
                b.populate();
                dimensionsGotten = true;
            } else {
                // Line is the contents
                // TODO Parse the pattern itself
                for (auto c : line) {
                    if (isdigit(c)) {
                        quantity += c;
                    } else if (c == 'b' || c == 'o') {
                        // TODO Dead cell or Alive cell
                        bool isAlive = c == 'o';
                        unsigned amount = quantity.empty()? 1 : stoi(quantity);
                        quantity = "";
                        for (unsigned i = 0; i < amount; ++i) {
                            b.set(xpos + i, ypos, isAlive);
                        }
                        xpos += amount;
                    } else if (c == '$') {
                        // End of line
                        ypos++;
                        xpos = 0;
                    } else if (c == '!') {
                        // End of file
                        forcedEOF = true;
                        break;
                    } else {
                        // TODO Handle other characters
                    }
                }
            }
        }
        return b;
    } else {
        cerr << "Error: file not found" << endl;
        throw 0;
    }
}

void rle::parseComment(Board& b, string line) {
    try {
        switch (line[0]) {
            case 'N':
                // Name of pattern
                b.setName(rle::trim(line.substr(1, line.npos)));
                break;
            case 'O':
                // Says when and whom the file was created, usually produced by
                // XLife
                break;
            case 'c':
                // Indicates comment, but not recommended. Same as 'C'.
                // TODO Warn user
            case 'C':
                // Indicates that a line of comment follows. A really common use
                // of # lines
                break;
            case 'P':
                // Same as R, produced by Life32, representing the top-left
                // corner of the pattern
            case 'R':
                // Gives coordinates of the top-left corner of pattern, usually
                // negative, with the intention of placing the center of the
                // pattern at the point of origin.
                break;
            case 'r':
                // Gives the rules for a pattern in the form of
                // '<survive>/<birth>' (e.g. 23/3 for Life). Usually present for
                // files created by XLife. Standard method is not in comments.
                break;
            default:
                // TODO Give warning about irregular comment
                break;
        }
    } catch (exception ex) {
        // TODO
    }
}

void rle::parseDimension(Board& b, string line) {
    // Remove all spaces from line
    line.erase(remove(line.begin(), line.end(), ' '),
               line.end());
    // Iterate through every comma
    smatch m;
    for (int i = line.find(',');
            i != line.npos;
            line.erase(0, i+1), i = line.find(',')) {
        string s = line.substr(0, i);

        if (regex_match(s, m, OBJEQ)) {
            if (m[1] == "x") {
                b.setW(stoi(m[2]));
            } else if (m[1] == "y") {
                b.setH(stoi(m[2]));
            } else {
                // TODO Unknown dimension handling
            }
        } else {
            // TODO Invalid format handling
        }
    }

    // If there is still something left (rule)
    if (!line.empty() && regex_match(line, m, OBJEQ)) {
        if (m[1] == "rule") {
            b.setRule(m[2]);
        } else {
            // TODO Unknown rule handling
        }
    } else {
        // TODO Invalid format handling
    }
}
