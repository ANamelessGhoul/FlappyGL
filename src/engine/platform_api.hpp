#pragma once

#ifndef PLATFORM_API_H
#define PLATFORM_API_H

#include "melon_types.hpp"

#if defined (__cplusplus)
extern "C"{
#endif 

void PlatformInit(); // Opens window and does any initialization needed by the platform
void PlatformShutdown();

void PlatformSetWindowTitle(const char* title);

void PlatformSwapScreenBuffer();
void PlatformPollInput();

bool PlatformWindowShouldClose();

void PlatformPrint(const char* text);

void PlatformBeginFrame();
void PlatformEndFrame();

void PlatformInitTimer();
double PlatformGetTime();

void* PlatformGetProcAddressPtr();

unsigned char *PlatformLoadFileBinary(const char *fileName, size_t *dataSize);
void PlatformUnloadFileBinary(unsigned char *data);
bool PlatformSaveFileBinary(const char *fileName, void *data, size_t dataSize);

char *PlatformLoadFileText(const char *fileName);
void PlatformUnloadFileText(char *text);
bool PlatformSaveFileText(const char *fileName, char *text);

#if defined (__cplusplus)
} // extern "C"
#endif 

#endif // PLATFORM_API_H