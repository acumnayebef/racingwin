#pragma once
#include <string>
namespace il2 {
bool  Init(std::string& err);                                   // resolves exports + Assembly-CSharp image
void* FindClass(const char* ns, const char* name, std::string& err);
void* FindMethod(void* klass, const char* name, int argc, std::string& err);  // walks base classes
void* ObjectClass(void* obj);
bool  Invoke(void* method, void* obj, void** args, void** ret, std::string& err);
int   UnboxInt(void* boxed);

// UnityEngine.Object.FindObjectOfType(Type) — returns the active scene instance of `klass`,
// or nullptr if none exists right now (e.g. the matching screen isn't open). Not an error either way.
void* FindActiveObject(void* klass, std::string& err);

// Direct field access (bypasses properties entirely) — for fields with no get_/set_ wrapper,
// or when a property's backing field is a separate compiler-generated field from a same-named
// public field, so writing one does not necessarily update the other.
bool SetBoolField(void* obj, const char* fieldName, bool value, std::string& err);
bool GetIntField(void* obj, const char* fieldName, int& out, std::string& err);
}
