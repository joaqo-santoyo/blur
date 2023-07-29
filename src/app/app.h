#pragma once
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int width;          // Read by the platform
    int height;         // Read by the platform
    float scaleFactor;  // Written to by the platform before appInit
} WindowInfo;

extern WindowInfo windowInfo;
int appEntry(int argc, char** argv);
int appInit(void);
int appRender(void);
int appDeinit(void);

#ifdef __cplusplus
}
#endif
