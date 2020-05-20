#include <fstream>
#include <iostream>
#include <vector>
#include <regex>
#include <unordered_map>

using namespace std;

#pragma warning (disable : 4996)

bool SortByVal(const pair<string, int> &a, const pair<string, int> &b)
{
	if (a.second == b.second)
	{
		return a.first < b.first;
	}
	return a.second > b.second;
}

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		cout << "Example: " << argv[0] << " in.txt out.txt" << endl;
		return 1;
	}

	unordered_map <string, int> dict;
	vector<pair<string, int>> sortedList;
	
	ifstream fIn(argv[1]);

	const std::regex re("[a-z]+", std::regex_constants::icase);
	std::regex::optimize;
	std::locale loc;
	std::smatch match;

	string line, word;

	while (std::getline(fIn, line))
	{			
		while (std::regex_search(line, match, re))
		{
			word = match[0].str();
		
			for (auto &chr : word)
			{
				chr = std::tolower(chr, loc);
			}
			
			if (dict[word])
			{
				dict[word]++;
			}
			else
			{
				dict[word] = 1;
			}
		
			line = match.suffix().str();
		}				
	}

	for (unordered_map<string, int> ::iterator it = dict.begin(); it != dict.end(); it++)
	{
		sortedList.push_back(make_pair(it->first, it->second));
	}
	
	sort(sortedList.begin(), sortedList.end(), SortByVal);

	ofstream fOut;
	fOut.open(argv[2]);

	for (auto &elem : sortedList)
	{
		fOut << elem.first << " " << elem.second << endl;
	}

	fIn.close();
	fOut.close();

	return 0;
}