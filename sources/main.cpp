#include <iostream>
#include <string>
#include <boost/filesystem.hpp>
#include "tinyxml2.h"
#include "tinyxml2.cpp"

using namespace std;
namespace fs = boost::filesystem;
string test;

struct Fileinfo {
	string path;
	string hash;
	int size;
	char flag;
};

void save2xml(string filename, vector<Fileinfo> vec_finfo) {
	tinyxml2::XMLDocument document;
	tinyxml2::XMLDeclaration * decl = document.NewDeclaration("1.0");
	document.LinkEndChild(decl);
	for (Fileinfo it : vec_finfo) {
		tinyxml2::XMLElement * element = document.NewElement("File");
		document.LinkEndChild(element);
		element->SetAttribute("path", it.path.c_str());
		element->SetAttribute("size", it.size);
		element->SetAttribute("hash", it.hash.c_str());
		element->SetAttribute("flag", it.flag);
		tinyxml2::XMLText * text = document.NewText("");
		element->LinkEndChild(text);
	}
	document.SaveFile(filename.c_str());
}

void get_dir_list(fs::directory_iterator iterator, vector<Fileinfo> * vec_finfo) {
	Fileinfo finfo;
	for (; iterator != fs::directory_iterator(); ++iterator)
	{
		if (fs::is_directory(iterator->status())) {
			fs::directory_iterator sub_dir(iterator->path());
			get_dir_list(sub_dir, vec_finfo);
		}
		else
		{
			finfo.path = iterator->path().string();
			finfo.size = fs::file_size(iterator->path());
			finfo.hash = "hash";
			finfo.flag = 'f';
			vec_finfo->push_back(finfo);
		}
	}
}

int main() {
	string path, dirpath;
	cout << "Enter the full path to your folder:" << endl;
	getline(cin, path);


	cout << endl;
	vector<Fileinfo> vec_finfo;
	fs::directory_iterator home_dir(path);
	get_dir_list(home_dir, &vec_finfo);

	for (Fileinfo element : vec_finfo) {
		cout << element.path << endl <<
			element.size << endl;
	}
	save2xml("Info.xml", vec_finfo);
	cin.clear();
	fflush(stdin);
	cin.get();
	return 0;
}



