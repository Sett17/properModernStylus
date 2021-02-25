#include <stdio.h>
#include <windows.h>
#include <winuser.h>
#include <string>

using namespace std;

#pragma comment(lib, "User32.lib")

void handleError();

typedef enum {
  PEN_STATE_MASK = (POINTER_FLAG_INRANGE | POINTER_FLAG_INCONTACT | POINTER_FLAG_DOWN | POINTER_FLAG_UP | POINTER_FLAG_UPDATE),

  PEN_HOVER = (POINTER_FLAG_INRANGE | POINTER_FLAG_UPDATE),
  PEN_DOWN = (POINTER_FLAG_INRANGE | POINTER_FLAG_INCONTACT | POINTER_FLAG_DOWN),
  PEN_CONTACT = (POINTER_FLAG_INRANGE | POINTER_FLAG_INCONTACT | POINTER_FLAG_UPDATE),
  PEN_UP = (POINTER_FLAG_INRANGE | POINTER_FLAG_UP),
  PEN_ENDHOVER = (POINTER_FLAG_UPDATE),
} PEN_STATES;

POINTER_TYPE_INFO currPointerInf = {0};
POINTER_TYPE_INFO lastPointerInf = {0};
HSYNTHETICPOINTERDEVICE SyntheticPointer;

int screen = 0;

BOOL WINAPI consoleHandler(DWORD signal) {
  if (signal == CTRL_C_EVENT || signal == CTRL_CLOSE_EVENT) {
    printf("stopping adb server");
    system("adb shell am force-stop m.sett.propermodernstylus");
    // system("adb kill-server");
  }
  ExitProcess(1);
  return TRUE;
}

void getPointer(POINTER_TYPE_INFO *pInfo, PEN_STATES state, int px = -1, int py = -1) {
  POINTER_TYPE_INFO zInfo = {0};
  POINTER_PEN_INFO zPen = {0};
  POINTER_INFO zPinfo = {0};
  POINT p = {px + screen * 1920, py};

  zPinfo.pointerType = PT_PEN;
  //   zPinfo.pointerFlags = ContextualizeFlags(&currPointerInf.penInfo, &lastPointerInf.penInfo);
  zPinfo.pointerFlags = state;
  zPinfo.ptPixelLocation = p;
  if (px < 0 && py < 0)
    zPinfo.ptPixelLocation = lastPointerInf.penInfo.pointerInfo.ptPixelLocation;

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

void execMsg(char *msg) {
  // printf(msg);
  int x, y, buttonPressed, pressure;
  switch (msg[0]) {
    case 'm':
      sscanf_s(msg + 1, "%d,%d:%d:%d", &x, &y, &buttonPressed, &pressure);
      getPointer(&currPointerInf, PEN_CONTACT, x, y);
      currPointerInf.penInfo.penMask = PEN_MASK_PRESSURE;
      currPointerInf.penInfo.pressure = pressure;
      currPointerInf.penInfo.pointerInfo.pointerFlags |= (buttonPressed) ? POINTER_FLAG_SECONDBUTTON : 0;
      // currPointerInf.penInfo.penFlags |= buttonPressed;

      if (!InjectSyntheticPointerInput(SyntheticPointer, &currPointerInf, 1)) {
        handleError();
      }
      lastPointerInf = currPointerInf;
      break;

    case 'n':
      sscanf_s(msg + 1, "%d,%d:%d", &x, &y, &buttonPressed);
      getPointer(&currPointerInf, PEN_HOVER, x, y);
      currPointerInf.penInfo.pointerInfo.pointerFlags |= (buttonPressed) ? POINTER_FLAG_SECONDBUTTON : 0;
      // currPointerInf.penInfo.penFlags |= buttonPressed;
      if (!InjectSyntheticPointerInput(SyntheticPointer, &currPointerInf, 1)) {
        handleError();
      }
      lastPointerInf = currPointerInf;
      break;

    case 's':
      switch (msg[1]) {
        case 'd':
          getPointer(&currPointerInf, PEN_DOWN);
          if (!InjectSyntheticPointerInput(SyntheticPointer, &currPointerInf, 1)) {
            handleError();
          }
          lastPointerInf = currPointerInf;
          break;
        case 'u':
          getPointer(&currPointerInf, PEN_UP);
          if (!InjectSyntheticPointerInput(SyntheticPointer, &currPointerInf, 1)) {
            handleError();
          }
          lastPointerInf = currPointerInf;
          break;

        default:
          break;
      }
      break;
    case 'h':
      switch (msg[1]) {
        case 'd':
          // getPointer(&currPointerInf, PEN_HOVER, x, y);
          // if (!InjectSyntheticPointerInput(SyntheticPointer, &currPointerInf, 1)) {
          //   handleError();
          // }
          // lastPointerInf = currPointerInf;
          break;
        case 'u':
          getPointer(&currPointerInf, PEN_ENDHOVER);
          if (!InjectSyntheticPointerInput(SyntheticPointer, &currPointerInf, 1)) {
            handleError();
          }
          lastPointerInf = currPointerInf;
          break;

        default:
          break;
      }
      break;
    case '3':
      putchar(0x07);
      screen = !screen;
      break;

    default:
      break;
  }
}

int main(int argc, char const *argv[]) {
  SetConsoleCtrlHandler(consoleHandler, TRUE);

  SyntheticPointer = CreateSyntheticPointerDevice(PT_PEN, 1, POINTER_FEEDBACK_DEFAULT);
  if (!SyntheticPointer) {
    handleError();
  }

  //   execCmd("adb connect 192.168.2.39", FALSE);
  execCmd("adb devices", TRUE);
  execCmd("adb shell am start -n m.sett.propermodernstylus/m.sett.propermodernstylus.MainActivity", FALSE);
  execCmd("adb logcat -c", FALSE);

  FILE *fp = _popen("adb logcat m.sett.properModernStylus:D -s PMS", "r");

  char buf[256];
  while (fgets(buf, 256, fp) != NULL) {
    // printf("%s", buf + 43);
    execMsg(buf + 43);
  }
  return 0;
}

void handleError() {
  LPVOID lpMsgBuf;
  DWORD dw = GetLastError();
  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR)&lpMsgBuf, 0, NULL);
  printf("Error %d: %s\n", GetLastError(), (char *)lpMsgBuf);
}