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

// The car the "Тачки" screen is currently showing/selling — read directly off
// CarSelectionScreen's own `activeCar` field. NOT CarsData.PlayerSelectedCarID (a different
// concept: the car the player drives). Requires that screen to be open right now, matching how
// this feature is meant to be used: open a car's page in the shop, then press the button.
bool OpenShopCar(void*& car, void*& screenInst, std::string& err) {
    void* screenClass = il2::FindClass(cfg::kCarsListNs, cfg::kCarSelectionScreen, err);
    if (!screenClass) return false;
    screenInst = il2::FindActiveObject(screenClass, err);
    if (!screenInst) {
        // FindActiveObject leaves err empty when it genuinely just found no instance (not an
        // error); only then show the generic hint. If err is non-empty, something upstream
        // actually broke (missing export/method/exception) — surface that real message instead
        // of masking it, so a wrong diagnosis here doesn't send the next fix down the wrong path.
        if (err.empty()) err = "shop screen (\"Тачки\") is not open — open a car's page there first";
        return false;
    }

    if (!il2::GetObjectField(screenInst, cfg::kFieldActiveCar, car, err)) return false;
    if (!car) { err = "no car open on screen (activeCar is null) — open a car's page first"; return false; }
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
    return il2::FindClass(cfg::kCarsListNs, cfg::kCarSelectionScreen, err) != nullptr;
}

Result GetBalance(Currency c)            { return Run(c, cfg::kGet, 0, 0); }
Result SetBalance(Currency c, int value) { return Run(c, cfg::kSet, 1, value); }
Result Earn(Currency c, int value)       { return Run(c, cfg::kEarn, 1, value); }

Result BuyCar() {
    Result r;
    void* car = nullptr;
    void* screenInst = nullptr;
    if (!OpenShopCar(car, screenInst, r.msg)) return r;
    if (!SetBoolProp(car, cfg::kSetUnlocked, true, r.msg)) return r;
    if (!SetBoolProp(car, cfg::kSetPurchased, true, r.msg)) return r;
    // Belt-and-braces: also write the plain fields directly, in case the property setters above
    // write to a different backing store than what the screen's own UI actually reads from.
    std::string fieldErr;
    il2::SetBoolField(car, cfg::kFieldUnlocked, true, fieldErr);
    il2::SetBoolField(car, cfg::kFieldPurchased, true, fieldErr);
    r.ok = true;

    // screenInst is the live CarSelectionScreen we just read activeCar from — call its own
    // UpdateView() so the price/status/buy-button UI on THIS screen refreshes immediately.
    std::string refreshErr;
    void* upd = il2::FindMethod(il2::ObjectClass(screenInst), cfg::kUpdateView, 0, refreshErr);
    if (upd) {
        void* ret = nullptr;
        if (il2::Invoke(upd, screenInst, nullptr, &ret, refreshErr)) r.value = 1;  // live refresh done
    }
    return r;
}
}  // namespace cheats
