#include <iostream>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <fstream>
#include <boost/filesystem.hpp>
#include "tinyxml2.h"
#include "tinyxml2.cpp"
#include <string.h>

using namespace tinyxml2;
using namespace std;
namespace fs = boost::filesystem;

struct Data {		//здесь сохраняются данные о каждом файле
	string path; //путь
	string hash;//хэш
	int size; //размер
	string flag = "New file"; //флаг(используется чтобы вывести состояние файла)
};

string adler32(const string & str) //стандартыай алгоритм для расчета хэш-суммы Adler32
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

vector<Data> compareData(vector<Data> newData, vector<Data> oldData) {//сравниваем данные, 2 вектора с новыми и старыми данными
	for (vector<Data>::iterator itnew = newData.begin(); itnew < newData.end(); itnew++) {
		for (vector<Data>::iterator itold = oldData.begin(); itold < oldData.end(); itold++) {
			if ((itnew->path == itold->path) && (itnew->hash == itold->hash)){ //берем путь и сравниваем хэш, если хэш не изменился - старый файл
				itnew->flag = "Original file";
				oldData.erase(itold);
				break;
			}
			if ((itnew->path == itold->path) && (itnew->hash != itold->hash)) {//если хэш изменился - измененный файл
				itnew->flag = "Modified file";
				oldData.erase(itold);
				break;
			}
		}
	}
	for (vector<Data>::iterator itold = oldData.begin(); itold < oldData.end(); itold++) {//иначе - файл удалее
		itold->flag = "Deleted file";
		newData.push_back(*itold);
	}//по умолчанию файлы имеют тэг "Новый Файл", так что их тоже видно
	return newData;
}

void saveToXml(string filename, vector<Data> data) { //используем tinyxml2 чтобы сохранить данные
	tinyxml2::XMLDocument document;
	tinyxml2::XMLDeclaration * decl = document.NewDeclaration();
	document.LinkEndChild(decl);
	for (Data data: data) {
		tinyxml2::XMLElement * element = document.NewElement("File");// для каждого отдельного файла создается елемент "Файл"
		document.LinkEndChild(element);
		//для данных о файле создаются дети
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

void loadxml(string filename, vector<Data> & data) { //парсим с помощью tinyxml2
	Data it;
	tinyxml2::XMLDocument doc;
	doc.LoadFile(filename.c_str());
	XMLHandle docHandle(&doc);
	XMLElement* child = docHandle.FirstChildElement("File").ToElement();//исходя из того, как мы сохраняли, сначала ищем "Файл"
	for (child; child; child = child->NextSiblingElement())//для всех его детей
	{//подгружаем данные в соответсвующий элемент вектора
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

void checkDirectory(fs::directory_iterator iterator, vector<Data> & data, Data & finfo) { //сканируем заданный путь
	for (; iterator != fs::directory_iterator(); ++iterator)
	{
		if (fs::is_directory(iterator->status())) { //если внутри папка-рекурсивно запускаем для самого себя
			fs::directory_iterator sub_dir(iterator->path());
			checkDirectory(sub_dir, data, finfo);

		}
		else
		{//если не папка, а файл, пихаем данные в вектор
			finfo.path = iterator->path().string();
			replace(finfo.path.begin(), finfo.path.end(), '\\', '/');//фиксим возможные ошибки со слэшами
			finfo.size = fs::file_size(iterator->path());
			finfo.hash = adler32(iterator->path().string());
			data.push_back(finfo);
		}

	}
}

void consoleOutput(vector<Data> vec) { //для наглядности выводим на экран все, что мы делаем
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
	cout << "Type <save>, to analise the new folder, type <check>, to find changes" << endl;//даем выбрать что нужно сделать
	getline(cin, answer); 
	cout << "Folder path:" << endl; //путь к папке
	getline(cin, path);
	vector<Data> newData;
	vector<Data> oldData;
	try {//обработка ошибок
		fs::directory_iterator home_dir(path);
		checkDirectory(home_dir, newData, finfo);
	}
	catch (const boost::filesystem::filesystem_error& e) {
		cout << "Wrong path, please try another" << endl;
		getline(cin, path);//даем одну возможность ввести правильный путь, если со 2 попытки не получилось - списываем пользователя как безнадежного, пусть запускает заново
	}
	if (answer == "save") {//если команда сохранить - сохраняем
		saveToXml("data.xml", newData);
		consoleOutput(newData);//пока не работает, но должно по идее запускать сохраненный xml 
	}
	if (answer == "check") {//если команда проверить - проверяем
		loadxml("data.xml", oldData);
		consoleOutput(compareData(newData, oldData));
	}
	if ((answer != "save") && (answer != "check")) {//не было времени сделать чтобы он давал еще раз ввести, пока просто говорит чтобы ввели сохранть или загрузить
		cout << "Please type check or save" << endl;
	}
	cin.get();
	return 0;
}
