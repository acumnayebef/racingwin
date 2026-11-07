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
    void* mm = il2::FindClass(cfg::kMainMenuNs, cfg::kMainMenuHandler, err);
    return mm && il2::FindMethod(mm, cfg::kGetInstance, 0, err) &&
           il2::FindMethod(mm, cfg::kUnlockAllCars, 0, err);
}

Result GetBalance(Currency c)            { return Run(c, cfg::kGet, 0, 0); }
Result SetBalance(Currency c, int value) { return Run(c, cfg::kSet, 1, value); }
Result Earn(Currency c, int value)       { return Run(c, cfg::kEarn, 1, value); }

Result UnlockAllCars() {
    Result r;
    void* mk = il2::FindClass(cfg::kMainMenuNs, cfg::kMainMenuHandler, r.msg);
    if (!mk) return r;
    void* gi = il2::FindMethod(mk, cfg::kGetInstance, 0, r.msg);
    if (!gi) return r;
    void* inst = nullptr;
    if (!il2::Invoke(gi, nullptr, nullptr, &inst, r.msg)) return r;
    if (!inst) { r.msg = "HR_MainMenuHandler.Instance is null (open the main menu first)"; return r; }
    void* m = il2::FindMethod(il2::ObjectClass(inst), cfg::kUnlockAllCars, 0, r.msg);
    if (!m) return r;
    void* ret = nullptr;
    if (!il2::Invoke(m, inst, nullptr, &ret, r.msg)) return r;
    r.ok = true;
    return r;
}
}  // namespace cheats
