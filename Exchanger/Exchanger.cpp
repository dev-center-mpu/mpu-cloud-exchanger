// Exchanger.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <string> 
#include <iostream>
#include <fstream>

#include "Translate.h"

int main(int argc, char* argv[])
{
	if (argc < 2) std::cerr << "Arguments required" << std::endl;
	std::string inFilePath(argv[1]), outFilePath(argv[2]);
	std::string inFileExt, outFileExt;
	inFileExt = inFilePath.substr(inFilePath.find_last_of('.') + 1);
	outFileExt = outFilePath.substr(outFilePath.find_last_of('.') + 1);

	std::ifstream is;
	is.open(inFilePath);
	if (!is.is_open()) return -1;
	is.seekg(0, is.end);
	size_t inFileLen = is.tellg();
	is.seekg(0, is.beg);
	char* inFileBuffer = new char[inFileLen];
	is.read(inFileBuffer, inFileLen);
	is.close();

	char* outFileBuffer = 0;
	size_t outFileLen = 0;
	char* thumbnailBuffer = 0;
	size_t thumbnailLen = 0;
	std::string errorMessage;
	/* mpu::Translate(inFileBuffer, inFileLen, inFileExt,
		outFileBuffer, outFileLen, outFileExt, 
		thumbnailBuffer, thumbnailLen,
		errorMessage);
		*/

	std::ofstream os;
	os.open(outFilePath, std::ofstream::binary);
	if (!os.is_open()) return -1;
	os.write(outFileBuffer, outFileLen);
	os.close();

	return 0;
}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
