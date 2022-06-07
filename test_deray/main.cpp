#include<vector>
#include<list>
#include<functional>
#include<iostream>

int main() {
	std::list<std::function<void(void)>> commandlist;	// �R�}���h���X�g���ۂ�����

	auto _out = [](const char* str) {std::cout << str << std::endl; };

	commandlist.push_back([&]() { _out("CPU Set RTV-1"); });	// ���߂P
	_out("CPU Set ����-2");

	commandlist.push_back([&]() { _out("CPU Clear RTV-3"); });	// ����2
	_out("CPU Clear ����-4");

	commandlist.push_back([&]() { _out("CPU Close RTV-5"); });	// ���߂P
	_out("CPU Close ����-6");

	std::cout << std::endl;

	// �R�}���h�L���[��ExcuteCommand���ۂ�����
	for (auto& command : commandlist) {
		command();
	}

	return 0;
}