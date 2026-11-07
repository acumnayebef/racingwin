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

bool SelectedCar(void*& car, std::string& err) {
    void* ck = il2::FindClass(cfg::kCarsNs, cfg::kCarsData, err);
    if (!ck) return false;
    void* gi = il2::FindMethod(ck, cfg::kGetInstance, 0, err);
    if (!gi) return false;
    void* data = nullptr;
    if (!il2::Invoke(gi, nullptr, nullptr, &data, err)) return false;
    if (!data) { err = "CarsData.Instance is null"; return false; }
    void* gc = il2::FindMethod(il2::ObjectClass(data), cfg::kGetSelectedCar, 0, err);
    if (!gc) return false;
    if (!il2::Invoke(gc, data, nullptr, &car, err)) return false;
    if (!car) { err = "no car selected (open its page in the shop first)"; return false; }
    return true;
}

bool SetBoolProp(void* obj, const char* method, bool value, std::string& err) {
    void* m = il2::FindMethod(il2::ObjectClass(obj), method, 1, err);
    if (!m) return false;
    bool v = value;
    void* args[1] = {&v};
    void* ret = nullptr;
    return il2::Invoke(m, obj, args, &ret, err);
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
    return ck && il2::FindMethod(ck, cfg::kGetInstance, 0, err) &&
           il2::FindMethod(ck, cfg::kGetSelectedCar, 0, err);
}

Result GetBalance(Currency c)            { return Run(c, cfg::kGet, 0, 0); }
Result SetBalance(Currency c, int value) { return Run(c, cfg::kSet, 1, value); }
Result Earn(Currency c, int value)       { return Run(c, cfg::kEarn, 1, value); }

Result BuyCar() {
    Result r;
    void* car = nullptr;
    if (!SelectedCar(car, r.msg)) return r;
    if (!SetBoolProp(car, cfg::kSetUnlocked, true, r.msg)) return r;
    if (!SetBoolProp(car, cfg::kSetPurchased, true, r.msg)) return r;
    r.ok = true;

    // Best-effort live refresh: if the shop list screen (CarsListManager) is open right now,
    // rebuild it so the car shows as owned without leaving the screen. Not finding it, or the
    // refresh call failing, does not undo the purchase above — it just means a manual reopen
    // of the shop screen is needed to see it.
    std::string refreshErr;
    void* listClass = il2::FindClass(cfg::kCarsListNs, cfg::kCarsListManager, refreshErr);
    if (listClass) {
        void* inst = il2::FindActiveObject(listClass, refreshErr);
        if (inst) {
            void* upd = il2::FindMethod(il2::ObjectClass(inst), cfg::kUpdateList, 0, refreshErr);
            if (upd) {
                void* ret = nullptr;
                if (il2::Invoke(upd, inst, nullptr, &ret, refreshErr)) r.value = 1;  // live refresh done
            }
        }
    }
    return r;
}
}  // namespace cheats
