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
            @"cars.hint":  @[ @"Открой любую машину в магазине (чтобы игра выбрала её текущей), затем нажми кнопку — она будет куплена и появится в гараже.",
                               @"Open any car's page in the shop (so the game marks it as selected), then press the button — it will be purchased and appear in the garage.",
                               @"افتح صفحة أي سيارة في المتجر (ليتم تحديدها في اللعبة)، ثم اضغط الزر — سيتم شراؤها وستظهر في المرآب." ],
            @"cars.button": @[ @"КУПИТЬ МАШИНУ", @"BUY CAR", @"شراء السيارة" ],
            @"cars.ok":     @[ @"✓ машина куплена и в гараже", @"✓ car purchased and in garage", @"✓ تم شراء السيارة وهي في المرآب" ],

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
