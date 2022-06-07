#include<vector>
#include<list>
#include<functional>
#include<iostream>

int main() {
	std::list<std::function<void(void)>> commandlist;	// コマンドリストっぽいもの

	auto _out = [](const char* str) {std::cout << str << std::endl; };

	commandlist.push_back([&]() { _out("CPU Set RTV-1"); });	// 命令１
	_out("CPU Set 命令-2");

	commandlist.push_back([&]() { _out("CPU Clear RTV-3"); });	// 命令2
	_out("CPU Clear 命令-4");

	commandlist.push_back([&]() { _out("CPU Close RTV-5"); });	// 命令１
	_out("CPU Close 命令-6");

	std::cout << std::endl;

	// コマンドキューのExcuteCommandっぽい処理
	for (auto& command : commandlist) {
		command();
	}

	return 0;
}