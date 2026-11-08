#pragma once
// All build-specific names live here. Target: Highway Racer (RCC), Unity 2022.3.62f3, IL2CPP arm64.
// Verified against dump__2_.cs (2026-11-07 update).
namespace cfg {
inline constexpr const char* kImage = "Assembly-CSharp.dll";

// --- currency (TOJGAMES.Economy.CurrencyManager) ---
inline constexpr const char* kNs = "TOJGAMES.Economy";
inline constexpr const char* kManager = "CurrencyManager";
inline constexpr const char* kBase = "CurrencyBase";
inline constexpr const char* kGetInstance = "get_Instance";  // static, 0 args
inline constexpr const char* kEarn = "Earn";                 // (int)
inline constexpr const char* kSet = "SetBalance";             // (int)
inline constexpr const char* kGet = "GetBalance";             // () -> int

struct Entry { const char* label; const char* getter; };  // getter = CurrencyManager property getter
inline constexpr Entry kCurrencies[] = {
    {"GOLD",    "get_GoldCurrency"},
    {"SILVER",  "get_SilverCurrency"},
    {"KEYS",    "get_KeyCurrency"},
    {"XP",      "get_ExperienceCurrency"},
    {"TICKETS", "get_TicketCurrency"},
    {"LAPIS",   "get_LapisLazuliCurrency"},
};
inline constexpr int kCount = sizeof(kCurrencies) / sizeof(kCurrencies[0]);

// --- cars: TOJGAMES.CarsData (ScriptableObject, scene-independent) + TOJGAMES.Car ---
inline constexpr const char* kCarsNs = "TOJGAMES";
inline constexpr const char* kCarsData = "CarsData";
inline constexpr const char* kGetCarByID = "GetCarByID";                // instance, (int) -> Car
inline constexpr const char* kSetUnlocked = "set_IsUnlocked";           // Car instance, (bool) property
inline constexpr const char* kSetPurchased = "set_IsPurchased";         // Car instance, (bool) property
// Car also has plain public fields with the same meaning (offsets 0x78/0x79 in the dump) that may
// have a separate compiler-generated backing store from the IsUnlocked/IsPurchased properties above —
// written directly as a safety net in case the game's UI reads these instead of the properties.
inline constexpr const char* kFieldUnlocked = "Unlocked";
inline constexpr const char* kFieldPurchased = "Purchased";

// --- shop list screen: CarsListManager (global namespace, MonoBehaviour, only alive while the
// "Тачки"/shop screen is open). currentSelectedItemID is the car currently open in THAT screen —
// this is what "buy the car I have open" must read, not CarsData.PlayerSelectedCarID (which is a
// different concept: the car the player currently drives, unrelated to shop browsing). ---
inline constexpr const char* kCarsListNs = "";
inline constexpr const char* kCarsListManager = "CarsListManager";
inline constexpr const char* kFieldSelectedItemID = "currentSelectedItemID";  // public int field
inline constexpr const char* kUpdateList = "UpdateList";  // instance, 0 args
}
