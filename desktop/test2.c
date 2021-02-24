#include <stdio.h>
#include <windows.h>
#include <winuser.h>

#pragma comment(lib, "User32.lib")

POINTER_FLAGS ContextualizeFlags(POINTER_PEN_INFO *, POINTER_PEN_INFO *);

enum {
  PEN_STATE_MASK = (POINTER_FLAG_INRANGE | POINTER_FLAG_INCONTACT | POINTER_FLAG_DOWN | POINTER_FLAG_UP | POINTER_FLAG_UPDATE),

  PEN_HOVER = (POINTER_FLAG_INRANGE | POINTER_FLAG_UPDATE),
  PEN_DOWN = (POINTER_FLAG_INRANGE | POINTER_FLAG_INCONTACT | POINTER_FLAG_DOWN),
  PEN_CONTACT = (POINTER_FLAG_INRANGE | POINTER_FLAG_INCONTACT | POINTER_FLAG_UPDATE),
  PEN_UP = (POINTER_FLAG_INRANGE | POINTER_FLAG_UP),
  PEN_ENDHOVER = (POINTER_FLAG_UPDATE),
};

POINTER_TYPE_INFO currPointerInf = {0};
POINTER_TYPE_INFO lastPointerInf = {0};

BOOL WINAPI consoleHandler(DWORD signal) {
  if (signal == CTRL_C_EVENT || signal == CTRL_CLOSE_EVENT) {
    if (GetKeyState(VK_CAPITAL) & 1) {
      INPUT ip;
      ip.type = INPUT_KEYBOARD;
      ip.ki.wScan = 0;
      ip.ki.time = 0;
      ip.ki.dwExtraInfo = 0;
      ip.ki.wVk = VK_CAPITAL;
      ip.ki.dwFlags = 0;
      SendInput(1, &ip, sizeof(INPUT));
      ip.ki.dwFlags = KEYEVENTF_KEYUP;
      SendInput(1, &ip, sizeof(INPUT));
    }
    printf("stopping adb server");
    system("adb shell am force-stop m.sett.virtualstyluswired");
    // system("adb kill-server");
  }
  ExitProcess(1);
  return TRUE;
}

void getPointer(POINTER_TYPE_INFO *pInfo, int x, int y) {
  POINTER_TYPE_INFO zInfo = {0};
  POINTER_PEN_INFO zPen = {0};
  POINTER_INFO zPinfo = {0};
  POINT p = {x, y};

  zPinfo.pointerType = PT_PEN;
  //   zPinfo.pointerFlags = ContextualizeFlags(&currPointerInf.penInfo, &lastPointerInf.penInfo);
  zPinfo.pointerFlags = PEN_HOVER;
  zPinfo.ptPixelLocation = p;
  zPen.pointerInfo = zPinfo;
  zInfo.type = PT_PEN;
  zInfo.penInfo = zPen;

  *pInfo = zInfo;
}

void execCmd(char *cmd, BOOL showOutput) {
  FILE *fp = _popen(cmd, "r");
  if (!showOutput) {
    return;
  }

  char buf[256];
  while (fgets(buf, 256, fp) != NULL)
    printf("%s", buf);
}

HSYNTHETICPOINTERDEVICE SyntheticPointer;

void execMsg(char *msg) {
  printf(msg);
  switch (msg[0]) {
    case 'm':
      int x, y;
      sscanf_s(msg + 1, "%d,%d", &x, &y);
      getPointer(&currPointerInf, x, y);
      if (!InjectSyntheticPointerInput(SyntheticPointer, &currPointerInf, 1)) {
        LPVOID lpMsgBuf;
        DWORD dw = GetLastError();
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                      NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                      (LPTSTR)&lpMsgBuf, 0, NULL);
        printf("Error %d: %s\n", GetLastError(), (char *)lpMsgBuf);
      }
      lastPointerInf = currPointerInf;
      break;

    default:
      break;
  }
}

int main(int argc, char const *argv[]) {
  SetConsoleCtrlHandler(consoleHandler, TRUE);

  //   currPointerInf.type = PT_PEN;
  SyntheticPointer = CreateSyntheticPointerDevice(PT_PEN, 1, POINTER_FEEDBACK_DEFAULT);
  if (!SyntheticPointer) {
    LPVOID lpMsgBuf;
    DWORD dw = GetLastError();
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR)&lpMsgBuf, 0, NULL);
    printf("Error %d: %s\n", GetLastError(), (char *)lpMsgBuf);
  }

//   execCmd("adb connect 192.168.2.39", FALSE);
  execCmd("adb devices", TRUE);
  execCmd("adb shell am start -n m.sett.virtualstyluswired/m.sett.virtualstyluswired.MainActivity", FALSE);
  execCmd("adb logcat -c", FALSE);

  FILE *fp = _popen("adb logcat m.sett.virtualstyluswired:D -s VSW", "r");

  char buf[256];
  while (fgets(buf, 256, fp) != NULL) {
    // char* msg =strtok(buf, ": ");
    // msg = strtok(NULL, ": ");
    execMsg(buf + 43);
  }

  return 0;
}

POINTER_FLAGS ContextualizeFlags(POINTER_PEN_INFO *PointerInfo, POINTER_PEN_INFO *LastPointerInfo) {
  printf("making flags");
  LPVOID lpMsgBuf;
  DWORD dw = GetLastError();
  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR)&lpMsgBuf, 0, NULL);
  printf("Error %d: %s\n", GetLastError(), (char *)lpMsgBuf);

  POINTER_FLAGS Mask = PEN_STATE_MASK;

  POINTER_FLAGS Result = PointerInfo->pointerInfo.pointerFlags;
  POINTER_FLAGS Last = LastPointerInfo->pointerInfo.pointerFlags & Mask;

  switch (Result & Mask) {
    case PEN_HOVER: {
      if ((Last & PEN_CONTACT) == PEN_CONTACT) {
        Result &= ~PEN_HOVER;
        Result |= PEN_UP;
      }
    } break;
    case PEN_DOWN: {
      if ((Last & PEN_HOVER) == PEN_HOVER) {
        printf("ERROR: Pen went to DOWN state from not HOVERING! %u", Last);
      }
    } break;
    case PEN_CONTACT: {
      if ((Last & PEN_HOVER) == PEN_HOVER) {
        Result &= ~PEN_CONTACT;
        Result |= PEN_DOWN;
      }
    } break;
    case PEN_UP: {
      printf("UP!\n");
    } break;
    case POINTER_FLAG_UPDATE: {
      printf("Ending hover\n");
    } break;
    case POINTER_FLAG_UP: {
      printf("Ending hover!!\n");
    } break;
    default: {
      printf("SOMETHING ELSE: %0x\n", Result);
      {
        printf("  POINTER_FLAG_NEW            %x\n", (Result & 0x00000001) != 0);  // New pointer
        printf("  POINTER_FLAG_INRANGE        %x\n", (Result & 0x00000002) != 0);  // Pointer has not departed
        printf("  POINTER_FLAG_INCONTACT      %x\n", (Result & 0x00000004) != 0);  // Pointer is in contact
        printf("  POINTER_FLAG_PRIMARY        %x\n", (Result & 0x00002000) != 0);  // Pointer is primary
        printf("  POINTER_FLAG_CONFIDENCE     %x\n", (Result & 0x00004000) != 0);  // Pointer is considered unlikely to be accidental
        printf("  POINTER_FLAG_CANCELED       %x\n", (Result & 0x00008000) != 0);  // Pointer is departing in an abnormal manner
        printf("  POINTER_FLAG_DOWN           %x\n", (Result & 0x00010000) != 0);  // Pointer transitioned to down state (made contact)
        printf("  POINTER_FLAG_UPDATE         %x\n", (Result & 0x00020000) != 0);  // Pointer update
        printf("  POINTER_FLAG_UP             %x\n", (Result & 0x00040000) != 0);  // Pointer transitioned from down state (broke contact)
        printf("  POINTER_FLAG_CAPTURECHANGED %x\n", (Result & 0x00200000) != 0);  // Lost capture
        printf("  POINTER_FLAG_HASTRANSFORM   %x\n", (Result & 0x00400000) != 0);
      }
    } break;
  }
}