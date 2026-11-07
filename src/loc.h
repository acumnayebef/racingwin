#pragma once
// Minimal RU/EN/AR localization for the menu. Text only — layout is not mirrored for AR (see README).
#import <Foundation/Foundation.h>

typedef NS_ENUM(NSInteger, XLang) { XLangRU = 0, XLangEN = 1, XLangAR = 2 };

static NSString* const kLangKey = @"xp_menu_lang";

static XLang XDefaultLang() {
    NSString* pref = [NSLocale.preferredLanguages firstObject] ?: @"en";
    if ([pref hasPrefix:@"ar"]) return XLangAR;
    if ([pref hasPrefix:@"ru"]) return XLangRU;
    return XLangEN;
}

static NSDictionary<NSString*, NSArray<NSString*>*>* XStrings() {
    static NSDictionary* d;
    static dispatch_once_t once;
    dispatch_once(&once, ^{
        // Each entry: @[ru, en, ar]
        d = @{
            @"tab.currency": @[ @"Валюта", @"Currency", @"العملة" ],
            @"tab.cars":     @[ @"Машины", @"Cars", @"السيارات" ],
            @"tab.creators": @[ @"Создатели", @"Creators", @"المنشئون" ],
            @"header.settings": @[ @"Настройки", @"Settings", @"الإعدادات" ],

            @"currency.amount":  @[ @"Сумма", @"Amount", @"المبلغ" ],
            @"currency.add":     @[ @"+ДОБ", @"+ADD", @"+إضافة" ],
            @"currency.set":     @[ @"ВВОД", @"SET", @"تعيين" ],
            @"currency.refresh": @[ @"↻ ОБНОВИТЬ", @"↻ REFRESH", @"↻ تحديث" ],
            @"currency.added":   @[ @"✓ начислено", @"✓ added", @"✓ تمت الإضافة" ],
            @"currency.setok":   @[ @"✓ баланс выставлен", @"✓ balance set", @"✓ تم ضبط الرصيد" ],

            @"cars.title": @[ @"МАШИНЫ", @"CARS", @"السيارات" ],
            @"cars.hint":  @[ @"Разблокирует все машины через встроенный метод игры. Жми эту кнопку на главном экране, до захода в «Тачки» — иначе игра ещё не создала нужный объект.",
                               @"Unlocks every car via the game's own built-in method. Press this on the main menu screen, before opening \"Cars\" — otherwise the game hasn't created the object yet.",
                               @"يفتح جميع السيارات عبر وظيفة اللعبة المدمجة. اضغط هذا في الشاشة الرئيسية قبل فتح \"السيارات\"، وإلا فلم يُنشئ اللعبة الكائن المطلوب بعد." ],
            @"cars.button": @[ @"РАЗБЛОКИРОВАТЬ ВСЕ", @"UNLOCK ALL CARS", @"فتح كل السيارات" ],
            @"cars.ok":     @[ @"✓ все машины разблокированы", @"✓ all cars unlocked", @"✓ تم فتح كل السيارات" ],

            @"creators.label":  @[ @"TELEGRAM-КАНАЛ", @"TELEGRAM CHANNEL", @"قناة تيليجرام" ],
            @"creators.blurb":  @[ @"Официальный канал автора.", @"Official channel of the author.",
                                    @"القناة الرسمية للمطور." ],
            @"creators.open":   @[ @"ОТКРЫТЬ TELEGRAM", @"OPEN IN TELEGRAM", @"فتح في تيليجرام" ],
            @"creators.copy":   @[ @"КОПИРОВАТЬ @%@", @"COPY @%@", @"نسخ @%@" ],
            @"creators.copied": @[ @"✓ скопировано", @"✓ copied", @"✓ تم النسخ" ],

            @"settings.opacity": @[ @"ПРОЗРАЧНОСТЬ МЕНЮ", @"MENU OPACITY", @"شفافية القائمة" ],
            @"settings.lang":    @[ @"ЯЗЫК", @"LANGUAGE", @"اللغة" ],
        };
    });
    return d;
}

static NSString* XL(XLang lang, NSString* key) {
    NSArray<NSString*>* v = XStrings()[key];
    if (!v) return key;
    NSInteger i = lang;
    if (i < 0 || i >= (NSInteger)v.count) i = 1;
    return v[i];
}
