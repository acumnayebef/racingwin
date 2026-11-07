#include "il2cpp.hpp"
#include "game_config.hpp"
#include <cstring>
#include <dlfcn.h>
#if defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

namespace il2 {
namespace {
struct {
    void*        (*domain_get)();
    void*        (*thread_attach)(void*);
    void**       (*domain_get_assemblies)(void*, size_t*);
    void*        (*assembly_get_image)(void*);
    const char*  (*image_get_name)(void*);
    void*        (*class_from_name)(void*, const char*, const char*);
    void*        (*class_get_method_from_name)(void*, const char*, int);
    void*        (*runtime_invoke)(void*, void*, void**, void**);
    void*        (*object_unbox)(void*);
    void*        (*object_get_class)(void*);
    const char*  (*class_get_name)(void*);
    const void*  (*class_get_type)(void*);          // Il2CppClass* -> const Il2CppType*
    void*        (*type_get_object)(const void*);   // const Il2CppType* -> System.Type object
} A;

void* g_image = nullptr;       // Assembly-CSharp.dll
void* g_coreImage = nullptr;   // UnityEngine.CoreModule.dll

void* UnityHandle() {
    static void* h = nullptr;
#if defined(__APPLE__)
    if (!h) {
        for (uint32_t i = 0, n = _dyld_image_count(); i < n; ++i) {
            const char* nm = _dyld_get_image_name(i);
            if (nm && std::strstr(nm, "/UnityFramework.framework/UnityFramework")) {
                h = dlopen(nm, RTLD_NOLOAD | RTLD_LAZY);
                break;
            }
        }
    }
#endif
    return h;
}

template <class F>
bool Load(F& f, const char* name, std::string& err) {
    void* p = UnityHandle() ? dlsym(UnityHandle(), name) : nullptr;
    if (!p) p = dlsym(RTLD_DEFAULT, name);
    f = reinterpret_cast<F>(p);
    if (!f) err += std::string("missing export: ") + name + "\n";
    return f != nullptr;
}
}  // namespace

bool Init(std::string& err) {
    if (g_image) return true;
    std::string e;
    bool ok = true;
    ok = Load(A.domain_get, "il2cpp_domain_get", e) && ok;
    ok = Load(A.thread_attach, "il2cpp_thread_attach", e) && ok;
    ok = Load(A.domain_get_assemblies, "il2cpp_domain_get_assemblies", e) && ok;
    ok = Load(A.assembly_get_image, "il2cpp_assembly_get_image", e) && ok;
    ok = Load(A.image_get_name, "il2cpp_image_get_name", e) && ok;
    ok = Load(A.class_from_name, "il2cpp_class_from_name", e) && ok;
    ok = Load(A.class_get_method_from_name, "il2cpp_class_get_method_from_name", e) && ok;
    ok = Load(A.runtime_invoke, "il2cpp_runtime_invoke", e) && ok;
    ok = Load(A.object_unbox, "il2cpp_object_unbox", e) && ok;
    ok = Load(A.object_get_class, "il2cpp_object_get_class", e) && ok;
    ok = Load(A.class_get_name, "il2cpp_class_get_name", e) && ok;
    ok = Load(A.class_get_type, "il2cpp_class_get_type", e) && ok;
    ok = Load(A.type_get_object, "il2cpp_type_get_object", e) && ok;
    if (!ok) { err = e; return false; }

    void* dom = A.domain_get();
    if (!dom) { err = "il2cpp domain not ready"; return false; }
    A.thread_attach(dom);

    size_t n = 0;
    void** asms = A.domain_get_assemblies(dom, &n);
    for (size_t i = 0; asms && i < n; ++i) {
        void* img = A.assembly_get_image(asms[i]);
        const char* nm = img ? A.image_get_name(img) : nullptr;
        if (!nm) continue;
        if (!g_image && std::strcmp(nm, cfg::kImage) == 0) g_image = img;
        if (!g_coreImage && std::strcmp(nm, "UnityEngine.CoreModule.dll") == 0) g_coreImage = img;
    }
    if (!g_image) { err = std::string(cfg::kImage) + " not loaded"; return false; }
    return true;
}

void* FindClass(const char* ns, const char* name, std::string& err) {
    if (!Init(err)) return nullptr;
    void* k = A.class_from_name(g_image, ns, name);
    if (!k) err = std::string("class not found: ") + ns + "." + name;
    return k;
}

void* FindActiveObject(void* klass, std::string& err) {
    if (!Init(err)) return nullptr;
    if (!g_coreImage) { err = "UnityEngine.CoreModule.dll not loaded"; return nullptr; }
    void* objClass = A.class_from_name(g_coreImage, "UnityEngine", "Object");
    if (!objClass) { err = "class not found: UnityEngine.Object"; return nullptr; }
    void* find = A.class_get_method_from_name(objClass, "FindObjectOfType", 1);
    if (!find) { err = "method not found: Object.FindObjectOfType(Type)/1"; return nullptr; }

    const void* type = A.class_get_type(klass);
    if (!type) { err = "il2cpp_class_get_type failed"; return nullptr; }
    void* typeObj = A.type_get_object(type);
    if (!typeObj) { err = "il2cpp_type_get_object failed"; return nullptr; }

    void* args[1] = {&typeObj};
    void* ret = nullptr;
    if (!Invoke(find, nullptr, args, &ret, err)) return nullptr;
    return ret;  // may legitimately be nullptr: no active instance right now
}

void* FindMethod(void* klass, const char* name, int argc, std::string& err) {
    if (!klass) { err = "null class"; return nullptr; }
    void* m = A.class_get_method_from_name(klass, name, argc);
    if (!m) err = std::string("method not found: ") + name + "/" + std::to_string(argc);
    return m;
}

void* ObjectClass(void* obj) { return obj ? A.object_get_class(obj) : nullptr; }

bool Invoke(void* method, void* obj, void** args, void** ret, std::string& err) {
    void* exc = nullptr;
    void* r = A.runtime_invoke(method, obj, args, &exc);
    if (exc) {
        void* k = A.object_get_class(exc);
        err = std::string("managed exception: ") + (k ? A.class_get_name(k) : "?");
        return false;
    }
    if (ret) *ret = r;
    return true;
}

int UnboxInt(void* boxed) { return *static_cast<int*>(A.object_unbox(boxed)); }
}  // namespace il2
