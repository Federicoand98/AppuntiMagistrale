#pragma once
#include <string>
#include <vector>

/*
Questa classe statica contiene un singleton utilizzato come ipotetico logger a schermo.
*/
class HUD_Logger
{
private:
	HUD_Logger();
public:
	/*
	restituisce il logger, se non esiste lo crea...
	*/
	static HUD_Logger* get();
	void printInfo(std::vector<std::string> lines);
private:
	static HUD_Logger* logger;
	static bool ready;
};

