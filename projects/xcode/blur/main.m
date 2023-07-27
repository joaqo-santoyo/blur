#import <Cocoa/Cocoa.h>
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>



@interface AppDelegate : NSObject<NSApplicationDelegate>
@property (nonatomic, strong) NSWindow *window;
@property (nonatomic, strong) NSOpenGLView *openGLView;
@end
@implementation AppDelegate
- (void) applicationDidFinishLaunching:(NSNotification*) notification {
    NSRect rect = NSMakeRect(0, 0, 512, 512);
    self.window = [[NSWindow alloc]
       initWithContentRect:rect
       styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable
       backing:NSBackingStoreBuffered
       defer:NO
    ];
    [self.window setTitle:@"Hello AppKit"];
    [self.window cascadeTopLeftFromPoint:NSMakePoint(100,100)];
    
    NSTextView* textView = [[NSTextView alloc] initWithFrame:rect];
    [textView setString:@"Hello AppKit"];
    [self.window.contentView addSubview:textView];
    
    NSOpenGLPixelFormatAttribute attributes[] = {
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFADepthSize, 24,
        0
    };
    NSOpenGLPixelFormat *pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
    self.openGLView = [[NSOpenGLView alloc] initWithFrame:rect pixelFormat:pixelFormat];
    [self.window.contentView addSubview:self.openGLView];
    
    [self.window makeKeyAndOrderFront:nil];
}
@end


@interface ViewController : NSViewController
@end
@implementation ViewController
- (void)viewDidLoad {
    [super viewDidLoad];
}
- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];
}
@end


int main(int argc, char* argv[]) {
    @autoreleasepool {
        AppDelegate* appDelegate = [[AppDelegate alloc] init];
        NSApplication* app = NSApplication.sharedApplication;
        [app activateIgnoringOtherApps:YES];
        [app setDelegate:appDelegate];
        [app run];
    }
    return 0;
}
