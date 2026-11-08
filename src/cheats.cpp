#include "cheats.hpp"
#include "game_config.hpp"
#include "il2cpp.hpp"
#include <cstring>

namespace cheats {
namespace {
bool CurrencyObject(Currency c, void*& obj, std::string& err) {
    void* mk = il2::FindClass(cfg::kNs, cfg::kManager, err);
    if (!mk) return false;
    void* gi = il2::FindMethod(mk, cfg::kGetInstance, 0, err);
    if (!gi) return false;
    void* mgr = nullptr;
    if (!il2::Invoke(gi, nullptr, nullptr, &mgr, err)) return false;
    if (!mgr) { err = "CurrencyManager.Instance is null (reach the main menu first)"; return false; }
    void* gm = il2::FindMethod(mk, cfg::kCurrencies[static_cast<int>(c)].getter, 0, err);
    if (!gm) return false;
    if (!il2::Invoke(gm, mgr, nullptr, &obj, err)) return false;
    if (!obj) { err = "currency object is null"; return false; }
    return true;
}

Result Run(Currency c, const char* method, int argc, int arg) {
    Result r;
    void* obj = nullptr;
    if (!CurrencyObject(c, obj, r.msg)) return r;
    void* m = il2::FindMethod(il2::ObjectClass(obj), method, argc, r.msg);  // most-derived first
    if (!m) return r;
    int a = arg;
    void* args[1] = {&a};
    void* ret = nullptr;
    if (!il2::Invoke(m, obj, argc ? args : nullptr, &ret, r.msg)) return r;
    if (std::strcmp(method, cfg::kGet) == 0) {
        if (!ret) { r.msg = "null result"; return r; }
        r.value = il2::UnboxInt(ret);
    }
    r.ok = true;
    return r;
}

bool SetBoolProp(void* obj, const char* method, bool value, std::string& err) {
    void* m = il2::FindMethod(il2::ObjectClass(obj), method, 1, err);
    if (!m) return false;
    bool v = value;
    void* args[1] = {&v};
    void* ret = nullptr;
    return il2::Invoke(m, obj, args, &ret, err);
}

// The car the shop screen currently has open — NOT CarsData.PlayerSelectedCarID, which is a
// different concept (the car the player drives). Requires the shop screen (CarsListManager) to
// be on screen right now, which matches how this feature is meant to be used: open a car's page
// in the shop, then press the button.
bool OpenShopCar(void*& car, void*& listInst, std::string& err) {
    void* listClass = il2::FindClass(cfg::kCarsListNs, cfg::kCarsListManager, err);
    if (!listClass) return false;
    listInst = il2::FindActiveObject(listClass, err);
    if (!listInst) { err = "shop screen (\"Тачки\") is not open — open a car's page there first"; return false; }

    int carId = 0;
    if (!il2::GetIntField(listInst, cfg::kFieldSelectedItemID, carId, err)) return false;

    void* ck = il2::FindClass(cfg::kCarsNs, cfg::kCarsData, err);
    if (!ck) return false;
    void* gi = il2::FindMethod(ck, cfg::kGetInstance, 0, err);
    if (!gi) return false;
    void* data = nullptr;
    if (!il2::Invoke(gi, nullptr, nullptr, &data, err)) return false;
    if (!data) { err = "CarsData.Instance is null"; return false; }

    void* gc = il2::FindMethod(il2::ObjectClass(data), cfg::kGetCarByID, 1, err);
    if (!gc) return false;
    void* args[1] = {&carId};
    if (!il2::Invoke(gc, data, args, &car, err)) return false;
    if (!car) { err = "GetCarByID returned null for id " + std::to_string(carId); return false; }
    return true;
}
}  // namespace

bool Validate(std::string& err) {
    if (!il2::Init(err)) return false;
    void* mk = il2::FindClass(cfg::kNs, cfg::kManager, err);
    if (!mk || !il2::FindMethod(mk, cfg::kGetInstance, 0, err)) return false;
    for (const auto& e : cfg::kCurrencies)
        if (!il2::FindMethod(mk, e.getter, 0, err)) return false;
    void* base = il2::FindClass(cfg::kNs, cfg::kBase, err);
    if (!base || !il2::FindMethod(base, cfg::kEarn, 1, err) ||
        !il2::FindMethod(base, cfg::kSet, 1, err) || !il2::FindMethod(base, cfg::kGet, 0, err))
        return false;
    void* ck = il2::FindClass(cfg::kCarsNs, cfg::kCarsData, err);
    if (!ck || !il2::FindMethod(ck, cfg::kGetInstance, 0, err) ||
        !il2::FindMethod(ck, cfg::kGetCarByID, 1, err))
        return false;
    return il2::FindClass(cfg::kCarsListNs, cfg::kCarsListManager, err) != nullptr;
}

Result GetBalance(Currency c)            { return Run(c, cfg::kGet, 0, 0); }
Result SetBalance(Currency c, int value) { return Run(c, cfg::kSet, 1, value); }
Result Earn(Currency c, int value)       { return Run(c, cfg::kEarn, 1, value); }

Result BuyCar() {
    Result r;
    void* car = nullptr;
    void* listInst = nullptr;
    if (!OpenShopCar(car, listInst, r.msg)) return r;
    if (!SetBoolProp(car, cfg::kSetUnlocked, true, r.msg)) return r;
    if (!SetBoolProp(car, cfg::kSetPurchased, true, r.msg)) return r;
    // Belt-and-braces: also write the plain fields directly, in case the property setters above
    // write to a different backing store than what the shop/garage UI actually reads from.
    std::string fieldErr;
    il2::SetBoolField(car, cfg::kFieldUnlocked, true, fieldErr);
    il2::SetBoolField(car, cfg::kFieldPurchased, true, fieldErr);
    r.ok = true;

    // listInst is already the live shop screen (we just read its currentSelectedItemID from it),
    // so refresh it directly — no second FindActiveObject lookup needed.
    std::string refreshErr;
    void* upd = il2::FindMethod(il2::ObjectClass(listInst), cfg::kUpdateList, 0, refreshErr);
    if (upd) {
        void* ret = nullptr;
        if (il2::Invoke(upd, listInst, nullptr, &ret, refreshErr)) r.value = 1;  // live refresh done
    }
    return r;
}
}  // namespace cheats
