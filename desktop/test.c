#include <stdio.h>
#include <windows.h>
#include <winuser.h>

#pragma comment(lib, "User32.lib")

POINTER_TYPE_INFO dbgPointers[1];
POINTER_TYPE_INFO dbgPointers2[1];

POINTER_TYPE_INFO getPointer(int x) {
  POINT pt = {500 + x * 10, 500};
  POINTER_INFO zInfo = {
      PT_PEN,  // pointerType
      17,      // pointerID
      NULL,    // frameID
      POINTER_FLAG_NONE | POINTER_FLAG_PRIMARY | POINTER_FLAG_CONFIDENCE |
          POINTER_FLAG_INRANGE,  // pointerFlags
      NULL,                      // sourceDevice
      NULL,                      // hwndTarget
      pt,                        // ptPixelLocation
      NULL,                      // ptHimetricLocation
      NULL,                      // ptPixelLocationRaw
      NULL,                      // ptHimetricLocationRaw
      NULL,                      // dwTime
      1,                         // historyCount
      // ,                                          // InputData
      0,                    // dwKeyStates
      x,                    // PerformanceCount
      POINTER_CHANGE_NONE,  // ButtonChangeType
  };

  POINTER_PEN_INFO zPen = {
      zInfo,          // pointerInfo
      PEN_FLAG_NONE,  // penFlags
      PEN_MASK_NONE,  // penMask
      0,              // pressure
      0,              // rotation
      0,              // tiltX
      0               // tiltY
  };

  POINTER_TYPE_INFO zPointer = {PT_PEN, .penInfo = zPen};
  return zPointer;
}

int main(int argc, char const *argv[]) {
  printf("hola que tal\n\n");

  dbgPointers[0] = getPointer(0);
  dbgPointers2[0] = getPointer(1);

  HSYNTHETICPOINTERDEVICE device =
      CreateSyntheticPointerDevice(PT_PEN, 1, POINTER_FEEDBACK_NONE);
  if (device == NULL) {
    printf("Error: %s\n", strerror(GetLastError()));
  }

  if (!InjectSyntheticPointerInput(device, dbgPointers, 1)) {
    printf("Error: %s\n", strerror(GetLastError()));
  } else {
    printf("joooooo\n");
  }

  Sleep(500);

  if (!InjectSyntheticPointerInput(device, dbgPointers2, 1)) {
    LPVOID lpMsgBuf;
    DWORD dw = GetLastError();
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                      FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR)&lpMsgBuf, 0, NULL);
    printf("Error %d: %s\n", GetLastError(), (char *)lpMsgBuf);
  } else {
    printf("joooooo\n");
  }

  printf("done\n\n");
  return 0;
}
