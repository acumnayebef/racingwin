#pragma once
#include <string>
namespace il2 {
bool  Init(std::string& err);                                   // resolves exports + Assembly-CSharp image
void* FindClass(const char* ns, const char* name, std::string& err);
void* FindMethod(void* klass, const char* name, int argc, std::string& err);  // walks base classes
void* ObjectClass(void* obj);
bool  Invoke(void* method, void* obj, void** args, void** ret, std::string& err);
int   UnboxInt(void* boxed);
}
