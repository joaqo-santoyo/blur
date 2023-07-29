#import <Cocoa/Cocoa.h>
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>
#include "app.h"


@interface OpenGLView : NSOpenGLView
@end
@implementation OpenGLView

- (void) prepareOpenGL {
    [super prepareOpenGL];
    appInit();
}

- (void) drawRect: (NSRect) bounds {
    NSLog(@"OpenGLView::drawRect\n");
    appRender();
    glSwapAPPLE();
}
@end

@interface AppDelegate : NSObject<NSApplicationDelegate>
@property (nonatomic, strong) NSWindow *window;
@property (nonatomic, strong) NSOpenGLView *openGLView;
@end
@implementation AppDelegate
- (void) applicationDidFinishLaunching:(NSNotification*) notification {
    NSLog(@"AppDelegate::applicationDidFinishLaunching");
    
    NSRect rect = NSMakeRect(0, 0, screenInfo.width, screenInfo.height);
    
    NSOpenGLPixelFormatAttribute attributes[] = {
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFADepthSize, 8,
        NSOpenGLPFAColorSize, 32,
        NSOpenGLPFAColorFloat,
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersionLegacy,
        0
    };
    NSOpenGLPixelFormat *pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
    self.openGLView = [[OpenGLView alloc] initWithFrame:rect pixelFormat:pixelFormat];
    
    self.window = [[NSWindow alloc]
       initWithContentRect:rect
       styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable
       backing:NSBackingStoreBuffered
       defer:NO
    ];
    [self.window setTitle:@"Blur"];
    [self.window cascadeTopLeftFromPoint:NSMakePoint(100,100)];
    [self.window.contentView addSubview:self.openGLView];
    [self.window makeKeyAndOrderFront:nil];
}
@end

int main(int argc, char* argv[]) {
    appEntry(argc, argv);
    @autoreleasepool {
        AppDelegate* appDelegate = [[AppDelegate alloc] init];
        NSApplication* app = NSApplication.sharedApplication;
        [app activateIgnoringOtherApps:YES];
        [app setDelegate:appDelegate];
        [app run];
    }
    return 0;
}
