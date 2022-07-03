#include "Config.h"
//类外对其静态成员变量进行一个初始化
Config* Config::mcfg = nullptr;
std::mutex Config::mutex_t;
Config* cfg = Config::GetInstance();
