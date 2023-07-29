#pragma once
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int width;
    int height;
} ScreenInfo;

extern ScreenInfo screenInfo;
int appEntry(int argc, char** argv);
int appInit(void);
int appRender(void);
int appDeinit(void);

#ifdef __cplusplus
}
#endif
