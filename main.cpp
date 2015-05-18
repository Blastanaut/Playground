#include <iostream>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <fstream>
#include <boost/filesystem.hpp>
#include "tinyxml2.h"
#include "tinyxml2.cpp"

using namespace tinyxml2;
using namespace std;
namespace fs = boost::filesystem;

struct Data {
	string path;
	string hash;
	int size;
	string flag = "New file";
};

string adler32(const string & str)
{
	unsigned int s1 = 1;
	unsigned int s2 = 0;
	for (unsigned int i = 0; i<str.size(); i++)
	{
		s1 = (s1 + str[i]) % 65521;
		s2 = (s2 + s1) % 65521;
	}
	stringstream ss;
	string s;
	ss << hex << (s2 << 16) + s1;
	ss >> s;
	return s;
}

vector<Data> compareData(vector<Data> newData, vector<Data> oldData) {
	for (vector<Data>::iterator itnew = newData.begin(); itnew < newData.end(); itnew++) {
		for (vector<Data>::iterator itold = oldData.begin(); itold < oldData.end(); itold++) {
			if ((itnew->path == itold->path) && (itnew->hash == itold->hash)) {
				itnew->flag = "Original file";
				oldData.erase(itold);
				break;
			}
			if ((itnew->path == itold->path) && (itnew->hash != itold->hash)) {
				itnew->flag = "Modified file";
				oldData.erase(itold);
				break;
			}
		}
	}
	for (vector<Data>::iterator itold = oldData.begin(); itold < oldData.end(); itold++) {
		itold->flag = "Deleted file";
		newData.push_back(*itold);
	}
	return newData;
}

void saveToXml(string filename, vector<Data> data) {
	tinyxml2::XMLDocument document;
	tinyxml2::XMLDeclaration * decl = document.NewDeclaration();
	document.LinkEndChild(decl);
	for (Data data: data) {
		tinyxml2::XMLElement * element = document.NewElement("File");
		document.LinkEndChild(element);

		tinyxml2::XMLElement* pathElement = document.NewElement("Path");
		pathElement->SetText(data.path.c_str());
		element->InsertEndChild(pathElement);

		tinyxml2::XMLElement* sizeElement = document.NewElement("Size");
		sizeElement->SetText(data.size);
		element->InsertEndChild(sizeElement);

		tinyxml2::XMLElement* hashElement = document.NewElement("Hash");
		hashElement->SetText(data.hash.c_str());
		element->InsertEndChild(hashElement);

		tinyxml2::XMLElement* flagElement = document.NewElement("Flag");
		flagElement->SetText(data.flag.c_str());
		element->InsertEndChild(flagElement);
	}
	document.SaveFile(filename.c_str());
}

void loadxml(string filename, vector<Data> & data) {
	Data it;
	tinyxml2::XMLDocument doc;
	doc.LoadFile(filename.c_str());
	XMLHandle docHandle(&doc);
	XMLElement* child = docHandle.FirstChildElement("File").ToElement();
	for (child; child; child = child->NextSiblingElement())
	{
		int temporarySize;
		XMLElement * sizeData = child->FirstChildElement("Size");
		sizeData->QueryIntText(&temporarySize);
		it.size = temporarySize;

		XMLElement * hashData = child->FirstChildElement("Hash");
		const char* temporaryHash = hashData->GetText();
		it.hash = temporaryHash;

		XMLElement * pathData = child->FirstChildElement("Path");
		const char* temporaryPath = pathData->GetText();
		it.path = temporaryPath;
		data.push_back(it);
	}
}

void checkDirectory(fs::directory_iterator iterator, vector<Data> & data, Data & finfo) {
	for (; iterator != fs::directory_iterator(); ++iterator)
	{
		if (fs::is_directory(iterator->status())) {
			fs::directory_iterator sub_dir(iterator->path());
			checkDirectory(sub_dir, data, finfo);

		}
		else
		{

			finfo.path = iterator->path().string();
			replace(finfo.path.begin(), finfo.path.end(), '\\', '/');
			finfo.size = fs::file_size(iterator->path());
			finfo.hash = adler32(iterator->path().string());
			data.push_back(finfo);
		}

	}
}

void consoleOutput(vector<Data> vec) {
	for (Data element : vec) {
		cout <<
			"Path:	" << element.path << endl <<
			"Size:	" << element.size << endl <<
			"Hash:	" << element.hash << endl <<
			"Info:	" << element.flag << endl << "\n" << endl;
	}
	cout << "Done!" << endl;
	cin.get();
}

int main() {
	string path, dirpath;
	Data finfo;
	string answer;
	cout << "Type <save>, to analise the new folder, type <check>, to find changes" << endl;
	getline(cin, answer);
	cout << "Folder path:" << endl;
	getline(cin, path);
	vector<Data> newData;
	vector<Data> oldData;
	try {
		fs::directory_iterator home_dir(path);
		checkDirectory(home_dir, newData, finfo);
	}
	catch (const boost::filesystem::filesystem_error& e) {
		cout << "Wrong path, please try another" << endl;
		getline(cin, path);
	}
	if (answer == "save") {
		saveToXml("data.xml", newData);
		consoleOutput(newData);
	}
	if (answer == "check") {
		loadxml("data.xml", oldData);
		consoleOutput(compareData(newData, oldData));
	}
	if ((answer != "save") && (answer != "check")) {
		consoleOutput(newData);
	}
	cin.get();
	return 0;
}
