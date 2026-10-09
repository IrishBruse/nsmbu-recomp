

#import <AppKit/AppKit.h>
#include <Carbon/Carbon.h>

#include "../overlay/overlay.h"
#include "../overlay/text_entry.h"

namespace gfx { void* display_tv_window(); }

@interface WWTextClient : NSObject <NSTextInputClient>
@end

@implementation WWTextClient {
    NSString* _marked;
}
static NSString* plain(id s) { return [s isKindOfClass:[NSAttributedString class]] ? [(NSAttributedString*)s string] : (NSString*)s; }
- (void)clear {
    _marked = nil;
    text_entry::preedit("");
}
- (void)insertText:(id)string replacementRange:(NSRange)range {
    (void)range;
    _marked = nil;
    text_entry::preedit("");
    if (NSString* s = plain(string); s.length) text_entry::text(s.UTF8String);
}
- (void)setMarkedText:(id)string selectedRange:(NSRange)sel replacementRange:(NSRange)range {
    (void)sel, (void)range;
    _marked = [plain(string) copy];
    text_entry::preedit(_marked.length ? _marked.UTF8String : "");
}
- (void)unmarkText {
    NSString* s = _marked;
    [self clear];
    if (s.length) text_entry::text(s.UTF8String);
}
- (BOOL)hasMarkedText { return _marked.length > 0; }
- (NSRange)markedRange { return _marked.length ? NSMakeRange(0, _marked.length) : NSMakeRange(NSNotFound, 0); }
- (NSRange)selectedRange { return NSMakeRange(_marked.length, 0); }
- (NSArray<NSAttributedStringKey>*)validAttributesForMarkedText { return @[]; }
- (NSAttributedString*)attributedSubstringForProposedRange:(NSRange)range actualRange:(NSRangePointer)actual {
    (void)range, (void)actual;
    return nil;
}
- (NSUInteger)characterIndexForPoint:(NSPoint)point {
    (void)point;
    return NSNotFound;
}

- (NSRect)firstRectForCharacterRange:(NSRange)range actualRange:(NSRangePointer)actual {
    (void)range, (void)actual;
    NSWindow* tv = (__bridge NSWindow*)gfx::display_tv_window();
    NSRect f = tv ? tv.frame : NSScreen.mainScreen.frame;
    return NSMakeRect(NSMidX(f) - 160, NSMidY(f) + 60, 320, 28);
}

- (void)doCommandBySelector:(SEL)selector { (void)selector; }
@end

namespace gfx {

bool text_input_key(void* event) {
    static WWTextClient* client;
    static NSTextInputContext* context;
    static bool activated = false;
    NSEvent* e = (__bridge NSEvent*)event;
    if (!text_entry::active() || overlay::is_open()) {
        if (activated) {
            [context discardMarkedText];
            [context deactivate];
            [client clear];
            activated = false;
        }
        return false;
    }
    if (!context) {
        client = [[WWTextClient alloc] init];
        context = [[NSTextInputContext alloc] initWithClient:client];
    }
    if (!activated) {
        [context activate];
        activated = true;
    }
    if (e.type == NSEventTypeFlagsChanged) return false;
    if (!client.hasMarkedText) {

        if (e.type != NSEventTypeKeyDown || (e.modifierFlags & (NSEventModifierFlagFunction | NSEventModifierFlagControl))) return false;
        switch (e.keyCode) {
        case kVK_Return: case kVK_ANSI_KeypadEnter: case kVK_Escape: case kVK_Delete: case kVK_ForwardDelete:
        case kVK_Tab: case kVK_LeftArrow: case kVK_RightArrow: case kVK_UpArrow: case kVK_DownArrow:
            return false;
        default: break;
        }
    } else if (e.type != NSEventTypeKeyDown) {
        return true;
    }
    if (![context handleEvent:e]) {

        NSString* s = e.characters;
        if (s.length && [s characterAtIndex:0] >= 0x20 && ([s characterAtIndex:0] < 0xF700 || [s characterAtIndex:0] > 0xF8FF))
            text_entry::text(s.UTF8String);
    }
    return true;
}

}
