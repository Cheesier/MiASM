#include <string>
#include <vector>
using namespace std;

int toMachineCode(string line);
int getWords(string line, std::vector<std::string> & ret);
void loadLabels(vector<string> & ret, int words);
void fillMemory(vector<string> & ret, int words);
bool isLabel(string word);
bool isRegister(string word);
bool isArgument(string word);
bool isOperation(string word);
bool isSpecial(string word);
void performSpecialOp(int operation, string word);

bool addLabel(string word);
int getLabelValue(string word);
bool labelExists(string word);

int getAddressMode(string word);

int toHex(string number);

int getRegisterNumber(string word);
int getAdr(string word);

bool isNumeric(string word);
int evalExpr(string word);

void memoryDump();
void usage(string name);
