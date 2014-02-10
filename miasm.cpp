#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <stdlib.h>
#include <sstream>
#include <iomanip>
#include "miasm.h"
using namespace std;

#define MAX_WORDS_PER_LINE 10
#define MEMORY_LENGTH 0xFF

bool debug = false;
int numberToDisplay = 0x10;

string *currentLine;
int currentLineNum;

map<string, int> labels;
int memoryLocation = 0;
int primMemory[MEMORY_LENGTH]; // primary memory
string assembly[MEMORY_LENGTH];

map<string, int> ops;
map<string, int> special;

// the number of bits to leftshift to get correct weight
enum {
  WEIGHT_OP = 12,
  WEIGHT_GRx = 10,
  WEIGHT_M = 8,
  WEIGHT_ADR = 0
};

enum {
  MODE_DIRECT =    0x0,
  MODE_IMMEDIATE = 0x1,
  MODE_INDIRECT =  0x2,
  MODE_INDEXED =   0x3
};

enum {
  SPECIAL_OP_DAT,
  SPECIAL_OP_ORG
};

void initializeOps() {
  string line;
  ifstream myfile (".csf");

  if (myfile.is_open()) {
    while ( getline (myfile,line) ) {
      
      vector<string> ret(MAX_WORDS_PER_LINE);
      int i;
      int words = getWords(line, ret);
      
      ops.insert(pair<string, int>( ret[1], toHex(ret[0])));
    }
    myfile.close();
  }
  else {
    cout << "Did not find a computer spec file (\".csf\" file)." << endl;
    cout << "The .csf file should follow the following format:" << endl;
    cout << "\"addr  OP\"" << endl;
    cout << "Example:" << endl
	 << "A0  LOAD" << endl
	 << "A1  STORE" << endl;
    exit(2);
  }

  special.insert(pair<string, int>( "DAT",   SPECIAL_OP_DAT ));
  special.insert(pair<string, int>( "ORG",   SPECIAL_OP_ORG ));
}

int main (int argc, char* argv[]) {
  int i = 1;
  
  if (!(argc > 1)) { // no arguments
    usage(argv[0]);
    return 0;
  }

  while (argv[i][0] == '-') { // fetching flags
    switch (argv[i][1]) {
    case 'v': // verbose
      debug = true;
      break;
    case 'n': // number of lines to display as output
      if (isHex(argv[i+1]))
	numberToDisplay = toHex(argv[i+1]);
      else {
	cerr << "'" << argv[i+1] << "' is not a valid hexadecimal number" << endl;
	exit(1);
      }
      i++;
      break;
    }
    i++;
  }

  if (argc-i-1 > 0) {
    usage(argv[0]);
    return 0;
  }
  initializeOps();
  string line;
  currentLine = &line;
  ifstream myfile (argv[i]);

  if (myfile.is_open()) {
    while ( getline (myfile,line) ) {
      currentLineNum++;
      vector<string> ret(MAX_WORDS_PER_LINE);
      int i;
      int words = getWords(line, ret);
      //cout << "contained " << words << " words" << endl;
      loadLabels(ret, words);
    }
    myfile.close();
  }
  else 
    cout << "Unable to open '" << argv[i];

  memoryLocation = 0;
  currentLineNum = 0;

  ifstream second (argv[i]);
  if (second.is_open()) {
    while ( getline (second,line) ) {
      currentLineNum++;
      vector<string> ret(MAX_WORDS_PER_LINE);
      int i;
      int words = getWords(line, ret);
      //cout << "contained " << words << " words" << endl;
      fillMemory(ret, words);
    }
    second.close();
  }
  else 
    cout << "Unable to open '" << argv[i];

  /*
  map<string, int>::iterator p;
  for (std::map<string,int>::iterator it=labels.begin(); it!=labels.end(); ++it)
    std::cout << it->second << ":" << it->first << '\n';
  */

  memoryDump();
  return 0;
}

void loadLabels(vector<string> & ret, int words) {
  int i = 0;

  for (i = 0; i < words; i++) {
    if (isLabel(ret[i])) {
      if (!labelExists(ret[i]))
        addLabel(ret[i]);
    }
    else if (isOperation(ret[i])) {
      memoryLocation++;
      if (words == 3) { // standard operation
        if (getAddressMode(ret[i+2]) == MODE_IMMEDIATE) {
          memoryLocation++;
        }
      }
    }
    else if (isSpecial(ret[i])) {
      // Assuming the next word is a value
      performSpecialOp(special[ret[i]], ret[i+1]);
    }
    else {
      if (i == 0) {
	reportError("Unknown operation '" + ret[i] + "'");
      }
    }
  }
}

void fillMemory(vector<string> & ret, int words) {
  int i = 0;
  for (i = 0; i < words; i++) {
    if (isLabel(ret[i])) {
      // do nothing
    }
    else if (isOperation(ret[i])) {
      int finalValue = 0;
      int op = 0;
      int reg = 0;
      int mode = 0;
      int adr = 0;
      int immediateValue = 0;

      op = ops[ret[i]];
      if (words == 3) {
	assembly[memoryLocation] = ret[i] + " " + ret[i+1] + ", " + ret[i+2];
        reg = getRegisterNumber(ret[i+1]);
        mode = getAddressMode(ret[i+2]);
        adr = getAdr(ret[i+2]);
      }
      else if (words == 2) {
	assembly[memoryLocation] = ret[i] + " " + ret[i+1];
        adr = getAdr(ret[i+1]);
      }
      else if (words == 1) {
	assembly[memoryLocation] = ret[i];
	adr = 0;
      }
      else {
	reportError("invalid line.");
      }

      if (debug) cout << assembly[memoryLocation] << " ";
      

      if (mode == MODE_IMMEDIATE) {
        immediateValue = adr;
        adr = memoryLocation+1;
        adr = 0;
      }

      
      if (debug)
	cout << "op:" << hex << op << ", "
	     << "reg:" << hex << reg << ", "
	     << "mode:" << hex << mode << ", "
	     << "adr:" << hex << adr << ", ";
      

      finalValue += op << WEIGHT_OP;
      finalValue += reg << WEIGHT_GRx;
      finalValue += mode << WEIGHT_M;
      finalValue += adr << WEIGHT_ADR;

      if (debug)
	cout << "Instruction: 0x" << setfill ('0') << setw(4)
	     << hex << finalValue << endl;

      primMemory[memoryLocation] = finalValue;
      memoryLocation++;

      if (mode == MODE_IMMEDIATE) {
        //cout << "MEMORY FROM MODE_IMMEDIATE: 0x" << immediateValue << endl;
        primMemory[memoryLocation] = immediateValue;
        memoryLocation++;
      }
      return;
    }
    else if (isSpecial(ret[i])) {
      // Assuming the next word is a value,
      performSpecialOp(special[ret[i]], ret[i+1]);
      i++;
    }
    else {
      cerr << "WARNING: unknown word '" << ret[i] << "'" << endl;
    }
  }
}

void performSpecialOp(int operation, string word) {
  //int arg = atoi(word.c_str()); // decimal
  int arg = toHex(word);

  switch(operation) {
    case SPECIAL_OP_DAT:
      primMemory[memoryLocation] = arg;
      assembly[memoryLocation] = "DAT " + word;
      memoryLocation++;
      break;
    case SPECIAL_OP_ORG:
      memoryLocation = arg;
      break;
  }
}

int getAddressMode(string word) {
  switch(word[0]) {
    case '#':
      return MODE_IMMEDIATE;
    case '[':
      return MODE_INDIRECT;
    case '(':
      return MODE_INDEXED;
    default:
      return MODE_DIRECT;
  }
}

bool isLabel(string word) {
  return word[0] == ':' || labelExists(word);
}

bool labelExists(string word) {
  if (labels.count(word) > 0)
    return true;
  return false;
}

bool addLabel(string word) {
  // save where the label was found
  labels.insert(pair<string, int>( word.substr(1),  memoryLocation ));
  cout << "addr: " << setfill('0') << setw(2) << memoryLocation << " label '" << word.substr(1) << "'" << endl;
  return true;
}

bool isOperation(string word) {
  if (ops.count(word)) {
    return true;
  }
  return false;
}

bool isSpecial(string word) {
  if (special.count(word)) {
    return true;
  }
  return false;
}

int getWords(string line, vector<string> & ret) {
  int count = 0;
  bool open = false;
  bool lineDone = false;
  int i;
  for (i = 0; i < line.length(); i++) {
    if (lineDone) break;
    switch(line[i]) {
    case '\n': // newlines
    case '\r': // carriage return
    case ';': // comments, we know we found all results
      lineDone = true;
    break;
    case ' ': // the 2 types of separators for arguments
    case '\t': // tab
    case ',':
      if (open) {
	count++;
	open = false;
      }
    break;
    default:
      open = true;
      ret[count] += toupper(line[i]);
      break;
    }
  }

  if (open)
    count++;
  return count;
}

bool isHex(string number) {
  int i;
  for (i = 0; i < number.length(); i++) {
    if (!isxdigit(number[i]))
      return false;
  }
  return true;
}

int toHex(string number) {
  return (int)strtol(number.c_str(), NULL, 16);
}

int toDec(string number) {
  return atoi(number.c_str());
}

void toUpper(string *word) {
  transform(word->begin(), word->end(), word->begin(), ::toupper);
}

void toLower(string *word) {
  transform(word->begin(), word->end(), word->begin(), ::tolower);
}

int getRegisterNumber(string word) {
  if (word[0] == 'R') {
    return atoi(&word[1]);
  }
  else {
    reportError("Argument '" + word + "' is not a register");
    return -1;
  }
}

int getAdr(string word) {
  string actual;
  switch(word[0]) {
    case '(': // strip away junk, we already know the addressing mode
    case '[':
      actual = word.substr(1, word.length()-2);
      break;
    case '#':
      actual = word.substr(1);
      break;
    default:
      actual = word;
  }

  if (labelExists(actual))
    return labels[actual];
  else if (isHex(actual))
    return toHex(actual);
  else
    reportError("'" + actual + "' is not a valid label or value.");
  return -1;
}

int evalExpr(string word) {
  int i;
  string label;
  string value;
  string *current = &label;
  for (i = 0; i < word.length(); i++) {
    switch(word[i]) {
    case '+':
    case '-':
      current = &value;
      break;
    case ' ':
      break;
    default:
      *current += word[i];
      break;
    }
  }

  if (current == &label)
    return -1;
  
  cout << "label: '" << label << "' value: '" << value << "'" << endl;
  return labels[label] + toHex(value);
}

void memoryDump() {
  int i;
  
  for (i = 0; i < numberToDisplay; i++) {
    cout << hex << setfill ('0') << setw(2);
    cout << i << ": " << setw(4) << primMemory[i];
    if (assembly[i].compare("") != 0)
      cout << "    ; " << assembly[i];
    cout << endl;
  }
  cout << "Full dump at masm.out" << endl;

  ofstream outFile("masm.out");
  for (i = 0; i < MEMORY_LENGTH+1; i++) {
    outFile << hex << setfill ('0') << setw(2);
    outFile << i << ": " << setw(4) << primMemory[i] << endl;
  }
  outFile.close();
  
}

void usage(string name) {
  cout << "Usage: " << name << " [flag] 'filename'" << endl;
  cout << "Common flags" << endl;
  cout << "  -v        verbose/debug output, see exactly how it interpreted the statements." << endl;
  cout << "  -n 'k'    shows 'k' number of lines in console, does not affect output in miasm.out." << endl;
}

void reportError(string message) {
  cerr << "On line " << currentLineNum << ": \"" 
       << *currentLine << "\"" << endl;
  cerr << "ERROR: " << message << endl;
  exit(1);
}
