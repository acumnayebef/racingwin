#pragma once
#include <string>
namespace cheats {
enum class Currency { Gold, Silver, Keys, Xp, Tickets, Lapis };
struct Result { bool ok = false; std::string msg; int value = 0; };

bool   Validate(std::string& err);           // checks every class/method the menu needs
Result GetBalance(Currency c);
Result SetBalance(Currency c, int amount);
Result Earn(Currency c, int amount);
Result UnlockAllCars();                      // HR_MainMenuHandler.Instance.UnlockAllCars()
}
