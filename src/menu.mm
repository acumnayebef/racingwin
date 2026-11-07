// Xantapers menu — sidebar layout with RU/EN/AR localization.
// Cheats are wired to src/cheats.cpp / il2cpp.cpp; this file is UI only.
#import <UIKit/UIKit.h>
#include <cstdlib>
#include "cheats.hpp"
#include "game_config.hpp"
#include "icon.h"
#include "loc.h"
#include "menu.hpp"

// ---- config --------------------------------------------------------------------------------
static NSString* const kChannelHandle = @"xantapers";
static NSString* const kMenuTitle = @"Xantapers";
static const CGFloat kFabSize = 58.0;
static const CGFloat kSidebarW = 108.0;
static const CGFloat kHeaderH = 52.0;
static NSString* const kOpacityKey = @"xp_menu_opacity";
static const CGFloat kOpacityMin = 0.20;

static UIColor* XPHex(uint32_t h, CGFloat a = 1.0) {
    return [UIColor colorWithRed:((h >> 16) & 0xFF) / 255.0 green:((h >> 8) & 0xFF) / 255.0
                             blue:(h & 0xFF) / 255.0 alpha:a];
}
#define XP_BG XPHex(0x14121C)
#define XP_SIDE XPHex(0x0E0C14)
#define XP_CARD XPHex(0x1F1B2B)
#define XP_ACCENT XPHex(0xE0408F)
#define XP_TEXT XPHex(0xF4F2FA)
#define XP_MUTED XPHex(0x9A94B0)
#define XP_GOOD XPHex(0x6BFFB0)
#define XP_BAD XPHex(0xFF6B8A)

static UIFont* XPFont(CGFloat size, UIFontWeight weight) {
    UIFontDescriptor* base = [UIFont systemFontOfSize:size weight:weight].fontDescriptor;
    UIFontDescriptor* rounded = [base fontDescriptorWithDesign:UIFontDescriptorSystemDesignRounded];
    return [UIFont fontWithDescriptor:(rounded ?: base) size:size];
}

static UIImage* XPIcon() {
    static UIImage* img;
    static dispatch_once_t once;
    dispatch_once(&once, ^{
        NSData* d = [[NSData alloc] initWithBase64EncodedString:@(kXantapersIconB64)
                                                          options:NSDataBase64DecodingIgnoreUnknownCharacters];
        if (d) img = [UIImage imageWithData:d scale:2.0];
    });
    return img;
}

// pass-through window: only our own subviews catch touches
@interface XPWindow : UIWindow
@end
@implementation XPWindow
- (UIView*)hitTest:(CGPoint)p withEvent:(UIEvent*)e {
    UIView* v = [super hitTest:p withEvent:e];
    return (v == self || v == self.rootViewController.view) ? nil : v;
}
@end

typedef void (^XPTabBuilder)(UIView* host);

@interface XMenuVC : UIViewController
@end

@implementation XMenuVC {
    UIButton* _fab;
    UIView* _dim;
    UIView* _panel;
    UIView* _sidebar;
    UIView* _content;
    UILabel* _headerTitle;
    UIButton* _closeBtn;
    UIButton* _gearBtn;
    NSArray<NSDictionary*>* _tabs;
    NSMutableArray<UIButton*>* _tabButtons;
    NSInteger _selected;
    BOOL _open;
    BOOL _fabPlaced;
    BOOL _inSettings;
    UITextField* _amount;
    UILabel* _status;
    NSMutableArray<UILabel*>* _bal;
    CGFloat _opacity;
    UILabel* _opacityLabel;
    XLang _lang;
    NSMutableArray<UIButton*>* _langButtons;
}

#pragma mark - tabs

- (void)buildTabs {
    __weak __typeof(self) ws = self;
    _tabs = @[
        @{@"key" : @"tab.currency", @"build" : (XPTabBuilder) ^(UIView* host) { [ws buildCurrencyInto:host]; }},
        @{@"key" : @"tab.cars",     @"build" : (XPTabBuilder) ^(UIView* host) { [ws buildCarsInto:host]; }},
        @{@"key" : @"tab.creators", @"build" : (XPTabBuilder) ^(UIView* host) { [ws buildCreatorsInto:host]; }},
    ];
}

#pragma mark - lifecycle

- (void)loadView {
    UIView* v = [UIView new];
    v.backgroundColor = UIColor.clearColor;
    self.view = v;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    NSUserDefaults* ud = NSUserDefaults.standardUserDefaults;
    NSNumber* langPref = [ud objectForKey:kLangKey];
    _lang = langPref ? (XLang)langPref.integerValue : XDefaultLang();

    [self buildTabs];
    NSNumber* saved = [ud objectForKey:kOpacityKey];
    _opacity = saved ? MAX(kOpacityMin, MIN(1.0, saved.doubleValue)) : 1.0;

    _dim = [[UIView alloc] init];
    _dim.backgroundColor = [UIColor colorWithWhite:0 alpha:0.45];
    _dim.hidden = YES;
    _dim.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    [_dim addGestureRecognizer:[[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(closeMenu)]];
    [self.view addSubview:_dim];

    _panel = [[UIView alloc] init];
    _panel.backgroundColor = XP_BG;
    _panel.layer.cornerRadius = 22;
    _panel.layer.borderWidth = 1;
    _panel.layer.borderColor = [XP_ACCENT colorWithAlphaComponent:0.45].CGColor;
    _panel.clipsToBounds = YES;
    _panel.hidden = YES;
    [self.view addSubview:_panel];
    [self applyOpacity];

    _sidebar = [[UIView alloc] init];
    _sidebar.backgroundColor = XP_SIDE;
    _sidebar.autoresizingMask = UIViewAutoresizingFlexibleHeight;
    [_panel addSubview:_sidebar];

    UIImageView* logo = [[UIImageView alloc] initWithImage:XPIcon()];
    logo.frame = CGRectMake((kSidebarW - 48) / 2, 14, 48, 48);
    logo.layer.cornerRadius = 24;
    logo.clipsToBounds = YES;
    logo.contentMode = UIViewContentModeScaleAspectFill;
    logo.layer.borderWidth = 1.5;
    logo.layer.borderColor = [XP_ACCENT colorWithAlphaComponent:0.7].CGColor;
    [_sidebar addSubview:logo];

    _tabButtons = [NSMutableArray array];
    CGFloat y = 78;
    for (NSInteger i = 0; i < (NSInteger)_tabs.count; i++) {
        UIButton* b = [UIButton buttonWithType:UIButtonTypeCustom];
        b.frame = CGRectMake(8, y, kSidebarW - 16, 38);
        b.layer.cornerRadius = 11;
        b.tag = i;
        b.titleLabel.font = XPFont(12, UIFontWeightBold);
        b.titleLabel.adjustsFontSizeToFitWidth = YES;
        b.titleLabel.minimumScaleFactor = 0.7;
        [b addTarget:self action:@selector(tabTapped:) forControlEvents:UIControlEventTouchUpInside];
        [_sidebar addSubview:b];
        [_tabButtons addObject:b];
        y += 44;
    }

    _headerTitle = [[UILabel alloc] init];
    _headerTitle.font = XPFont(20, UIFontWeightBold);
    _headerTitle.textColor = XP_TEXT;
    _headerTitle.text = kMenuTitle;
    [_panel addSubview:_headerTitle];

    _closeBtn = [UIButton buttonWithType:UIButtonTypeSystem];
    [_closeBtn setTitle:@"✕" forState:UIControlStateNormal];
    _closeBtn.titleLabel.font = [UIFont systemFontOfSize:16 weight:UIFontWeightSemibold];
    [_closeBtn setTitleColor:XP_TEXT forState:UIControlStateNormal];
    _closeBtn.backgroundColor = XP_CARD;
    _closeBtn.layer.cornerRadius = 15;
    _closeBtn.autoresizingMask = UIViewAutoresizingFlexibleLeftMargin;
    [_closeBtn addTarget:self action:@selector(closeMenu) forControlEvents:UIControlEventTouchUpInside];
    [_panel addSubview:_closeBtn];

    _gearBtn = [UIButton buttonWithType:UIButtonTypeSystem];
    UIImage* gi = [UIImage systemImageNamed:@"gearshape.fill"
                         withConfiguration:[UIImageSymbolConfiguration configurationWithPointSize:15
                                                                                             weight:UIImageSymbolWeightSemibold]];
    if (gi) [_gearBtn setImage:gi forState:UIControlStateNormal];
    else { [_gearBtn setTitle:@"\u2699" forState:UIControlStateNormal]; }
    _gearBtn.tintColor = XP_TEXT;
    _gearBtn.backgroundColor = XP_CARD;
    _gearBtn.layer.cornerRadius = 15;
    _gearBtn.autoresizingMask = UIViewAutoresizingFlexibleLeftMargin;
    [_gearBtn addTarget:self action:@selector(toggleSettings) forControlEvents:UIControlEventTouchUpInside];
    [_panel addSubview:_gearBtn];

    _content = [[UIView alloc] init];
    _content.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    [_panel addSubview:_content];

    _fab = [UIButton buttonWithType:UIButtonTypeCustom];
    _fab.frame = CGRectMake(0, 0, kFabSize, kFabSize);
    [_fab setBackgroundImage:XPIcon() forState:UIControlStateNormal];
    _fab.layer.cornerRadius = kFabSize / 2;
    _fab.clipsToBounds = YES;
    _fab.layer.borderWidth = 2.5;
    _fab.layer.borderColor = XP_ACCENT.CGColor;
    _fab.layer.shadowColor = XP_ACCENT.CGColor;
    _fab.layer.shadowRadius = 10;
    _fab.layer.shadowOpacity = 0.7;
    _fab.layer.shadowOffset = CGSizeZero;
    [_fab addTarget:self action:@selector(toggleMenu) forControlEvents:UIControlEventTouchUpInside];
    [_fab addGestureRecognizer:[[UIPanGestureRecognizer alloc] initWithTarget:self action:@selector(panFab:)]];
    [self.view addSubview:_fab];

    [self refreshTabTitles];
    [self selectTab:0];
    [self validate];
}

- (void)viewDidLayoutSubviews {
    [super viewDidLayoutSubviews];
    CGRect b = self.view.bounds;
    UIEdgeInsets sa = self.view.safeAreaInsets;
    _dim.frame = b;

    CGFloat w = MIN(b.size.width - 24, 420);
    CGFloat h = MIN(b.size.height - sa.top - sa.bottom - 24, 460);
    CGAffineTransform t = _panel.transform;
    _panel.transform = CGAffineTransformIdentity;
    _panel.frame = CGRectMake((b.size.width - w) / 2, (b.size.height - h) / 2, w, h);
    _panel.transform = t;

    _sidebar.frame = CGRectMake(0, 0, kSidebarW, h);
    _headerTitle.frame = CGRectMake(kSidebarW + 18, 0, w - kSidebarW - 18 - 96, kHeaderH);
    _closeBtn.frame = CGRectMake(w - 44, (kHeaderH - 30) / 2, 30, 30);
    _gearBtn.frame = CGRectMake(w - 44 - 38, (kHeaderH - 30) / 2, 30, 30);
    _content.frame = CGRectMake(kSidebarW, kHeaderH, w - kSidebarW, h - kHeaderH);

    if (!_fabPlaced && b.size.width > 0) {
        _fab.center = CGPointMake(sa.left + 16 + kFabSize / 2, sa.top + 120);
        _fabPlaced = YES;
    }
    [self clampFab];
}

#pragma mark - FAB

- (void)clampFab {
    CGRect b = self.view.bounds;
    UIEdgeInsets sa = self.view.safeAreaInsets;
    CGFloat r = kFabSize / 2;
    CGPoint c = _fab.center;
    c.x = MAX(sa.left + r, MIN(b.size.width - sa.right - r, c.x));
    c.y = MAX(sa.top + r, MIN(b.size.height - sa.bottom - r, c.y));
    _fab.center = c;
}

- (void)panFab:(UIPanGestureRecognizer*)g {
    CGPoint t = [g translationInView:self.view];
    _fab.center = CGPointMake(_fab.center.x + t.x, _fab.center.y + t.y);
    [g setTranslation:CGPointZero inView:self.view];
    if (g.state == UIGestureRecognizerStateEnded || g.state == UIGestureRecognizerStateCancelled)
        [UIView animateWithDuration:0.2 animations:^{ [self clampFab]; }];
}

#pragma mark - open/close

- (void)toggleMenu { _open ? [self closeMenu] : [self openMenu]; }

- (void)openMenu {
    if (_open) return;
    _open = YES;
    _dim.hidden = NO;
    _dim.alpha = 0;
    _panel.hidden = NO;
    _panel.alpha = 0;
    _panel.transform = CGAffineTransformMakeScale(0.92, 0.92);
    [self.view bringSubviewToFront:_fab];
    [UIView animateWithDuration:0.22 delay:0 options:UIViewAnimationOptionCurveEaseOut animations:^{
        self->_dim.alpha = 1;
        self->_panel.alpha = 1;
        self->_panel.transform = CGAffineTransformIdentity;
    } completion:nil];
    [self refresh];
}

- (void)closeMenu {
    if (!_open) return;
    _open = NO;
    [self.view endEditing:YES];
    [UIView animateWithDuration:0.18 animations:^{
        self->_dim.alpha = 0;
        self->_panel.alpha = 0;
        self->_panel.transform = CGAffineTransformMakeScale(0.92, 0.92);
    } completion:^(BOOL) {
        self->_dim.hidden = YES;
        self->_panel.hidden = YES;
    }];
}

#pragma mark - tabs / settings

- (void)tabTapped:(UIButton*)b { _inSettings = NO; [self selectTab:b.tag]; }

- (void)refreshTabTitles {
    for (NSInteger i = 0; i < (NSInteger)_tabs.count; i++)
        [_tabButtons[i] setTitle:XL(_lang, _tabs[i][@"key"]) forState:UIControlStateNormal];
}

- (void)selectTab:(NSInteger)i {
    _selected = i;
    for (UIView* v in _content.subviews) [v removeFromSuperview];
    for (NSInteger j = 0; j < (NSInteger)_tabButtons.count; j++) {
        UIButton* b = _tabButtons[j];
        BOOL on = (j == i) && !_inSettings;
        b.backgroundColor = on ? [XP_ACCENT colorWithAlphaComponent:0.22] : UIColor.clearColor;
        [b setTitleColor:(on ? XP_ACCENT : XP_MUTED) forState:UIControlStateNormal];
    }
    _headerTitle.text = kMenuTitle;
    XPTabBuilder build = _tabs[i][@"build"];
    build(_content);
}

- (void)toggleSettings {
    _inSettings = !_inSettings;
    for (UIButton* b in _tabButtons) [b setTitleColor:XP_MUTED forState:UIControlStateNormal];
    for (UIView* v in _content.subviews) [v removeFromSuperview];
    if (_inSettings) {
        _headerTitle.text = XL(_lang, @"header.settings");
        [self buildSettingsInto:_content];
    } else {
        [self selectTab:_selected];
    }
}

#pragma mark - currency tab

- (void)buildCurrencyInto:(UIView*)host {
    UIScrollView* sc = [[UIScrollView alloc] initWithFrame:host.bounds];
    sc.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    sc.showsVerticalScrollIndicator = NO;
    CGFloat W = host.bounds.size.width, y = 10;

    UILabel* al = [self label:XL(_lang, @"currency.amount") size:11 weight:UIFontWeightHeavy color:XP_MUTED];
    al.frame = CGRectMake(14, y + 10, 72, 16);
    al.adjustsFontSizeToFitWidth = YES;
    [sc addSubview:al];
    _amount = [[UITextField alloc] initWithFrame:CGRectMake(88, y, W - 88 - 14, 36)];
    _amount.text = @"1000000";
    _amount.keyboardType = UIKeyboardTypeNumberPad;
    _amount.textColor = XP_TEXT;
    _amount.font = [UIFont monospacedDigitSystemFontOfSize:15 weight:UIFontWeightBold];
    _amount.backgroundColor = XP_CARD;
    _amount.layer.cornerRadius = 10;
    _amount.layer.borderWidth = 1;
    _amount.layer.borderColor = [XP_ACCENT colorWithAlphaComponent:0.5].CGColor;
    _amount.leftView = [[UIView alloc] initWithFrame:CGRectMake(0, 0, 10, 36)];
    _amount.leftViewMode = UITextFieldViewModeAlways;
    UIToolbar* tb = [[UIToolbar alloc] initWithFrame:CGRectMake(0, 0, 320, 40)];
    tb.items = @[
        [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace target:nil action:nil],
        [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemDone target:_amount
                                                        action:@selector(resignFirstResponder)]
    ];
    _amount.inputAccessoryView = tb;
    [sc addSubview:_amount];
    y += 46;

    _bal = [NSMutableArray new];
    for (int i = 0; i < cfg::kCount; ++i) {
        UIView* row = [[UIView alloc] initWithFrame:CGRectMake(0, y, W, 42)];
        row.backgroundColor = XP_CARD;
        row.layer.cornerRadius = 12;
        UILabel* n = [self label:[NSString stringWithUTF8String:cfg::kCurrencies[i].label] size:12
                            weight:UIFontWeightHeavy color:XP_ACCENT];
        n.frame = CGRectMake(10, 0, 64, 42);
        [row addSubview:n];
        UILabel* v = [self label:@"—" size:13 weight:UIFontWeightSemibold color:XP_TEXT];
        v.font = [UIFont monospacedDigitSystemFontOfSize:13 weight:UIFontWeightSemibold];
        v.frame = CGRectMake(72, 0, row.bounds.size.width - 72 - 112, 42);
        v.adjustsFontSizeToFitWidth = YES;
        [row addSubview:v];
        [_bal addObject:v];
        UIButton* add = [self pill:XL(_lang, @"currency.add") color:XP_ACCENT];
        add.frame = CGRectMake(row.bounds.size.width - 106, 7, 50, 28);
        add.tag = i;
        [add addTarget:self action:@selector(onAdd:) forControlEvents:UIControlEventTouchUpInside];
        [row addSubview:add];
        UIButton* set = [self pill:XL(_lang, @"currency.set") color:XP_BAD];
        set.frame = CGRectMake(row.bounds.size.width - 52, 7, 44, 28);
        set.tag = i;
        [set addTarget:self action:@selector(onSet:) forControlEvents:UIControlEventTouchUpInside];
        [row addSubview:set];
        [sc addSubview:row];
        y += 48;
    }
    UIButton* rf = [self pill:XL(_lang, @"currency.refresh") color:XP_MUTED];
    rf.frame = CGRectMake(0, y + 2, W, 34);
    [rf addTarget:self action:@selector(refresh) forControlEvents:UIControlEventTouchUpInside];
    [sc addSubview:rf];
    y += 44;

    _status = [self label:@"…" size:11 weight:UIFontWeightSemibold color:XP_ACCENT];
    _status.frame = CGRectMake(0, y + 4, W, 16);
    _status.adjustsFontSizeToFitWidth = YES;
    [sc addSubview:_status];
    y += 26;

    sc.contentSize = CGSizeMake(W, y);
    [host addSubview:sc];
}

- (int)amount {
    long long v = std::atoll(_amount.text.UTF8String ?: "0");
    if (v < 0) v = 0;
    if (v > 1000000000LL) v = 1000000000LL;
    return (int)v;
}

- (void)report:(const cheats::Result&)r ok:(NSString*)okText {
    _status.textColor = r.ok ? XP_GOOD : XP_BAD;
    _status.text = r.ok ? okText : [NSString stringWithUTF8String:r.msg.c_str()];
}

- (void)onAdd:(UIButton*)b {
    [self.view endEditing:YES];
    cheats::Result r = cheats::Earn((cheats::Currency)b.tag, [self amount]);
    [self report:r ok:XL(_lang, @"currency.added")];
    [self refresh];
}

- (void)onSet:(UIButton*)b {
    [self.view endEditing:YES];
    cheats::Result r = cheats::SetBalance((cheats::Currency)b.tag, [self amount]);
    [self report:r ok:XL(_lang, @"currency.setok")];
    [self refresh];
}

- (void)refresh {
    if (!_bal) return;
    for (int i = 0; i < cfg::kCount; ++i) {
        cheats::Result r = cheats::GetBalance((cheats::Currency)i);
        _bal[i].text = r.ok ? [NSString stringWithFormat:@"%d", r.value] : @"—";
    }
}

- (void)validate {
    std::string err;
    if (!cheats::Validate(err) && _status) {
        _status.textColor = XP_BAD;
        _status.text = [NSString stringWithUTF8String:err.c_str()];
    }
}

#pragma mark - cars tab

- (void)buildCarsInto:(UIView*)host {
    CGFloat W = host.bounds.size.width;
    UILabel* h = [self label:XL(_lang, @"cars.title") size:18 weight:UIFontWeightHeavy color:XP_ACCENT];
    h.frame = CGRectMake(0, 4, W, 24);
    [host addSubview:h];

    UILabel* hint = [self label:XL(_lang, @"cars.hint") size:12 weight:UIFontWeightMedium color:XP_MUTED];
    hint.frame = CGRectMake(0, 34, W, 60);
    hint.numberOfLines = 0;
    [host addSubview:hint];

    UIButton* btn = [self pill:XL(_lang, @"cars.button") color:XP_ACCENT];
    btn.frame = CGRectMake(0, 102, W, 44);
    btn.titleLabel.font = XPFont(13, UIFontWeightHeavy);
    [btn addTarget:self action:@selector(onUnlockCars) forControlEvents:UIControlEventTouchUpInside];
    [host addSubview:btn];

    _status = [self label:@"…" size:11 weight:UIFontWeightSemibold color:XP_ACCENT];
    _status.frame = CGRectMake(0, 154, W, 32);
    _status.numberOfLines = 0;
    [host addSubview:_status];
}

- (void)onUnlockCars {
    cheats::Result r = cheats::UnlockAllCars();
    [self report:r ok:XL(_lang, @"cars.ok")];
}

#pragma mark - creators tab

- (void)buildCreatorsInto:(UIView*)host {
    CGFloat W = host.bounds.size.width;
    UILabel* h = [self label:XL(_lang, @"tab.creators").uppercaseString size:18 weight:UIFontWeightHeavy
                        color:XP_ACCENT];
    h.frame = CGRectMake(0, 4, W, 24);
    [host addSubview:h];

    UIView* card = [[UIView alloc] initWithFrame:CGRectMake(0, 36, W, 150)];
    card.backgroundColor = XP_CARD;
    card.layer.cornerRadius = 16;
    [host addSubview:card];

    UILabel* k = [self label:XL(_lang, @"creators.label") size:10 weight:UIFontWeightHeavy color:XP_MUTED];
    k.frame = CGRectMake(16, 14, card.bounds.size.width - 32, 14);
    [card addSubview:k];
    UILabel* hd = [self label:[@"@" stringByAppendingString:kChannelHandle] size:24 weight:UIFontWeightHeavy
                          color:XP_ACCENT];
    hd.frame = CGRectMake(16, 32, card.bounds.size.width - 32, 32);
    hd.adjustsFontSizeToFitWidth = YES;
    [card addSubview:hd];
    UILabel* ds = [self label:XL(_lang, @"creators.blurb") size:12 weight:UIFontWeightMedium color:XP_MUTED];
    ds.frame = CGRectMake(16, 66, card.bounds.size.width - 32, 16);
    [card addSubview:ds];
    UIButton* open = [self pill:XL(_lang, @"creators.open") color:XP_ACCENT];
    open.frame = CGRectMake(16, 96, card.bounds.size.width - 32, 36);
    [open addTarget:self action:@selector(openChannel) forControlEvents:UIControlEventTouchUpInside];
    [card addSubview:open];

    NSString* copyTitle = [XL(_lang, @"creators.copy") stringByReplacingOccurrencesOfString:@"%@"
                                                                                   withString:kChannelHandle.uppercaseString];
    UIButton* copy = [self pill:copyTitle color:XP_BAD];
    copy.frame = CGRectMake(0, 198, W, 36);
    [copy addTarget:self action:@selector(copyHandle) forControlEvents:UIControlEventTouchUpInside];
    [host addSubview:copy];
}

- (void)openChannel {
    NSURL* tg = [NSURL URLWithString:[@"tg://resolve?domain=" stringByAppendingString:kChannelHandle]];
    NSURL* web = [NSURL URLWithString:[@"https://t.me/" stringByAppendingString:kChannelHandle]];
    [UIApplication.sharedApplication openURL:tg options:@{} completionHandler:^(BOOL ok) {
        if (!ok) [UIApplication.sharedApplication openURL:web options:@{} completionHandler:nil];
    }];
}

- (void)copyHandle {
    UIPasteboard.generalPasteboard.string = [@"@" stringByAppendingString:kChannelHandle];
    if (_status) { _status.textColor = XP_GOOD; _status.text = XL(_lang, @"creators.copied"); }
}

#pragma mark - settings tab

- (void)buildSettingsInto:(UIView*)host {
    CGFloat W = host.bounds.size.width;
    UILabel* ol = [self label:XL(_lang, @"settings.opacity") size:12 weight:UIFontWeightHeavy color:XP_MUTED];
    ol.frame = CGRectMake(0, 8, W, 16);
    [host addSubview:ol];

    _opacityLabel = [self label:[NSString stringWithFormat:@"%.0f%%", _opacity * 100] size:12
                           weight:UIFontWeightBold color:XP_ACCENT];
    _opacityLabel.frame = CGRectMake(0, 8, W, 16);
    _opacityLabel.textAlignment = NSTextAlignmentRight;
    [host addSubview:_opacityLabel];

    UISlider* slider = [[UISlider alloc] initWithFrame:CGRectMake(0, 32, W, 30)];
    slider.minimumValue = kOpacityMin;
    slider.maximumValue = 1.0;
    slider.value = _opacity;
    slider.minimumTrackTintColor = XP_ACCENT;
    [slider addTarget:self action:@selector(onOpacity:) forControlEvents:UIControlEventValueChanged];
    [host addSubview:slider];

    UILabel* ll = [self label:XL(_lang, @"settings.lang") size:12 weight:UIFontWeightHeavy color:XP_MUTED];
    ll.frame = CGRectMake(0, 76, W, 16);
    [host addSubview:ll];

    NSArray<NSString*>* codes = @[ @"RU", @"EN", @"AR" ];
    _langButtons = [NSMutableArray array];
    CGFloat bw = (W - 16) / 3;
    for (NSInteger i = 0; i < 3; i++) {
        BOOL on = (i == _lang);
        UIButton* b = [self pill:codes[i] color:(on ? XP_ACCENT : XP_MUTED)];
        b.frame = CGRectMake(i * (bw + 8), 98, bw, 34);
        b.tag = i;
        [b addTarget:self action:@selector(onLang:) forControlEvents:UIControlEventTouchUpInside];
        [host addSubview:b];
        [_langButtons addObject:b];
    }

    UILabel* ver = [self label:@"Xantapers menu" size:11 weight:UIFontWeightSemibold color:XP_MUTED];
    ver.frame = CGRectMake(0, host.bounds.size.height - 24, W, 16);
    [host addSubview:ver];
}

- (void)onOpacity:(UISlider*)s {
    _opacity = s.value;
    _opacityLabel.text = [NSString stringWithFormat:@"%.0f%%", _opacity * 100];
    [self applyOpacity];
    [NSUserDefaults.standardUserDefaults setDouble:_opacity forKey:kOpacityKey];
}

- (void)applyOpacity {
    _panel.backgroundColor = [XP_BG colorWithAlphaComponent:_opacity];
}

- (void)onLang:(UIButton*)b {
    _lang = (XLang)b.tag;
    [NSUserDefaults.standardUserDefaults setInteger:_lang forKey:kLangKey];
    [self refreshTabTitles];
    for (NSInteger i = 0; i < (NSInteger)_langButtons.count; i++) {
        UIButton* lb = _langButtons[i];
        UIColor* c = (i == _lang) ? XP_ACCENT : XP_MUTED;
        [lb setTitleColor:c forState:UIControlStateNormal];
        lb.backgroundColor = [c colorWithAlphaComponent:0.16];
        lb.layer.borderColor = [c colorWithAlphaComponent:0.8].CGColor;
    }
    _headerTitle.text = XL(_lang, @"header.settings");
    for (UIView* v in _content.subviews) [v removeFromSuperview];
    [self buildSettingsInto:_content];
}

#pragma mark - helpers

- (UILabel*)label:(NSString*)text size:(CGFloat)size weight:(UIFontWeight)w color:(UIColor*)c {
    UILabel* l = [UILabel new];
    l.text = text;
    l.font = XPFont(size, w);
    l.textColor = c;
    return l;
}

- (UIButton*)pill:(NSString*)title color:(UIColor*)color {
    UIButton* b = [UIButton buttonWithType:UIButtonTypeCustom];
    [b setTitle:title forState:UIControlStateNormal];
    [b setTitleColor:color forState:UIControlStateNormal];
    b.titleLabel.font = XPFont(11, UIFontWeightHeavy);
    b.backgroundColor = [color colorWithAlphaComponent:0.16];
    b.layer.cornerRadius = 9;
    b.layer.borderWidth = 1;
    b.layer.borderColor = [color colorWithAlphaComponent:0.8].CGColor;
    return b;
}
@end

// ---- public ---------------------------------------------------------------------------------
static XPWindow* g_win;

bool xmenu::Show() {
    if (g_win) return true;
    UIWindowScene* scene = nil;
    for (UIScene* s in UIApplication.sharedApplication.connectedScenes) {
        if ([s isKindOfClass:UIWindowScene.class] && s.activationState == UISceneActivationStateForegroundActive) {
            scene = (UIWindowScene*)s;
            break;
        }
    }
    if (!scene) return false;
    g_win = [[XPWindow alloc] initWithWindowScene:scene];
    g_win.frame = scene.coordinateSpace.bounds;
    g_win.windowLevel = UIWindowLevelAlert + 100;
    g_win.backgroundColor = UIColor.clearColor;
    g_win.rootViewController = [XMenuVC new];
    g_win.hidden = NO;
    return true;
}
