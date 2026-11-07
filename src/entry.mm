#import <UIKit/UIKit.h>
#include <string>
#include "il2cpp.hpp"
#include "menu.hpp"

static void Poll(int attempt) {
    std::string err;
    bool ready = il2::Init(err);
    // Show once IL2CPP is up; after ~90 s show anyway so the menu can report the error.
    if ((ready || attempt >= 90) && xmenu::Show()) return;
    if (attempt >= 300) return;
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1 * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
        Poll(attempt + 1);
    });
}

__attribute__((constructor)) static void XantaMenuLoad(void) {
    dispatch_async(dispatch_get_main_queue(), ^{ Poll(0); });
}
