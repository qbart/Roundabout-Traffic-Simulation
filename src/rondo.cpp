#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "Env.h"
#include "Car.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "Resource.h"

using namespace std;

#define screenW 800
#define screenH 600

static char appname[] = "Symulacja ronda";
HINSTANCE hInstance;

LRESULT CALLBACK WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DlgProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
void EnableOpenGL (HWND hWnd, HDC *hDC, HGLRC *hRC);
void DisableOpenGL (HWND hWnd, HDC hDC, HGLRC hRC);
void DrawText(char *s);
void Render();
void CreateDlg();

Car *cars[MAX]; //cars as threads

//main
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,LPSTR lpCmdLine,int iCmdShow)
{
  WNDCLASS wc;
  HWND hWnd;

  HDC hDC;
  HGLRC hRC;
  MSG msg;

  BOOL bQuit = FALSE;

  ::hInstance = hInstance;

  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = WndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = hInstance;
  wc.hIcon = LoadIcon (NULL, IDI_APPLICATION);
  wc.hCursor = LoadCursor (NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
  wc.lpszMenuName = NULL;
  wc.lpszClassName = appname;
  RegisterClass(&wc);

  int extra_height = GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYBORDER);
  int extra_width = GetSystemMetrics(SM_CXBORDER);

  hWnd = CreateWindow (appname, appname, WS_CAPTION | WS_POPUPWINDOW,
       0, 0, screenW + extra_width, screenH + extra_height, NULL, NULL, hInstance, NULL);

  ShowWindow(hWnd, SW_SHOW);
  UpdateWindow(hWnd);

  //register dialog
  WNDCLASSEX wcdlg = {0};
  wcdlg.cbSize = sizeof(WNDCLASSEX);
  wcdlg.lpfnWndProc = (WNDPROC)DlgProc;
  wcdlg.hInstance = hInstance;
  wcdlg.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
  wcdlg.lpszClassName = TEXT("DialogClass");
  RegisterClassEx(&wcdlg);
  CreateDlg();

  srand(time(NULL));

  EnableOpenGL (hWnd, &hDC, &hRC);

  Env::init();

  for (int i=0; i<MAX; i++)
    {
      cars[i] = new Car(i);
      cars[i]->Start();
    }

  for (int i=0; i<MAX; i++)
    cars[i]->signal();

  while (!bQuit)
  {
    if (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
    {
      if (msg.message == WM_QUIT)
      {
        bQuit = TRUE;
      }
      else
      {
        TranslateMessage (&msg);
        DispatchMessage (&msg);
      }
    }
    else
    {

      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glLoadIdentity();

      glDisable(GL_TEXTURE_2D);
      glPushMatrix();
      glTranslated(R+1.5,7,-4);
      glRasterPos2d(0,0);

      for (int i=0; i<MAX; i++)
        {
          char s[100];
          sprintf(s,"Car%2d:",i+1);

          glColor3ub(0,128,255);
          glRasterPos2d(0,-i);
          DrawText(s);

          sprintf(s,"%d",(int)Env::cars[i]);
          glColor3ub(255,0,0);
          glRasterPos2d(2.5,-i);
          DrawText(s);
        }

      glPopMatrix();
      glEnable(GL_TEXTURE_2D);

      Render();

      glFlush();
      SwapBuffers(hDC);
    }
  }

  for (int i=0; i<MAX; i++)
    {
      cars[i]->Stop(true);
      delete cars[i];
    }

  DisableOpenGL (hWnd, hDC, hRC);

  DestroyWindow(hWnd);

  return msg.wParam;
}

//main window proc
LRESULT CALLBACK WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
    case WM_CREATE: return 0; break;

    case WM_CLOSE: PostQuitMessage (0); return 0; break;

    case WM_DESTROY: return 0; break;

    case WM_SIZE:
      int h = HIWORD(lParam);
      int w = LOWORD(lParam);

      if (h == 0) h = 1;

      glViewport(0,0,w,h);
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      gluPerspective(45.0f, (GLfloat)w/(GLfloat)h, 1.0f, 1000.0f );
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();

      return 0;
      break;

    case WM_KEYDOWN:

      switch (wParam)
      {
        case VK_ESCAPE:
          PostQuitMessage(0);
          return 0;
          break;

        case VK_SPACE:
          CreateDlg();
          return 0;
          break;
      }
      return 0;
      break;

    default:
      return DefWindowProc (hWnd, message, wParam, lParam);
  }
}

HWND buttons[MAX], edit;
HWND dlg;

void CreateDlg()
{
  dlg = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST, TEXT("DialogClass"), TEXT("Symulation panel"),
          WS_SYSMENU | WS_CAPTION , screenW + 10, 0, 130, 400,
          NULL, NULL, hInstance,  NULL);

  for (int i=0; i<MAX; i++)
    {
      char tmp[20];
      sprintf(tmp, "Stop car %d", i+1);

      buttons[i] = CreateWindow ("Button", tmp, BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE,
                                  10, 5 + i * 25 , 100, 20, dlg,
                                  (HMENU)(ID_BUTTON + i), hInstance, NULL);
    }

  CreateWindow ("Button", "Reset", BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE,
                10, (MAX+4) * 25 , 100, 20, dlg, (HMENU)ID_RESET, hInstance, NULL);

  CreateWindow("Static", "Stop for [ms]:", WS_CHILD | WS_VISIBLE,
                10, 5 + (MAX+1) * 25 , 100, 20, dlg, NULL, hInstance, NULL);

  edit = CreateWindow("Edit", "1000",WS_BORDER | WS_CHILD | WS_VISIBLE | ES_CENTER | ES_NUMBER,
                      10, 5 + (MAX+2) * 25 , 100, 20,
                      dlg, (HMENU)ID_EDIT, hInstance, NULL);

  SendMessageA(edit, EM_SETLIMITTEXT, 4, 0);

  ShowWindow(dlg, SW_SHOW);
  UpdateWindow(dlg);
}

LRESULT CALLBACK DlgProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  switch(Msg)
  {
    case WM_COMMAND:

      if ( (wParam >= ID_BUTTON) && (wParam < ID_BUTTON + MAX) )
        {
          int id = wParam - ID_BUTTON;

          char tmp[10];
          GetWindowTextA(edit, tmp, 10);

          int value = atoi(tmp);
          if (value <= 0)
            {
              value = 1000;
              sprintf(tmp, "%d", value);
              SetWindowTextA(edit, tmp);
            }

          cars[id]->freeze(value);
        }
      else if (wParam == ID_RESET)
        {
          for (int i=0; i<MAX; i++)
            cars[i]->Stop(true);

          Env::init();

          for (int i=0; i<MAX; i++)
            {
              cars[i]->Start();
              cars[i]->init();
            }

          for (int i=0; i<MAX; i++)
            cars[i]->signal();
        }
      break;

    case WM_CLOSE:
      DestroyWindow(hWnd);
      break;
  }

  return DefWindowProc(hWnd, Msg, wParam, lParam);
}

//OPENGL DECLARATIONS

typedef unsigned char * BMP;
typedef BITMAPINFOHEADER BMPINFO;

unsigned int carTex, mapTex, bkgTex, maskTex;
BMP carBmp, mapBmp, bkgBmp, maskBmp;
BITMAPINFOHEADER carBmpInfo, mapBmpInfo, bkgBmpInfo, maskBmpInfo;
unsigned int listBase;

BMP LoadBmp(char *fname, BMPINFO *bmp_info);

//OPENGL FUNCTIONS
void EnableOpenGL (HWND hWnd, HDC *hDC, HGLRC *hRC)
{
  PIXELFORMATDESCRIPTOR pfd;
  int iFormat;
  *hDC = GetDC(hWnd);
  ZeroMemory (&pfd, sizeof (pfd));
  pfd.nSize = sizeof (pfd);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 24;
  pfd.cDepthBits = 16;
  pfd.iLayerType = PFD_MAIN_PLANE;

  iFormat = ChoosePixelFormat(*hDC, &pfd);
  SetPixelFormat(*hDC, iFormat, &pfd);
  *hRC = wglCreateContext( *hDC );
  wglMakeCurrent( *hDC, *hRC );

  glViewport(0,0,screenW,screenH);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(128.0f, (GLfloat)screenW/(GLfloat)screenH, 1.0f, 1000.0f );
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_TEXTURE_2D);
  glFrontFace(GL_CW);
  glShadeModel(GL_SMOOTH);

  glClearColor(0, 0, 0, 0);
  glClearDepth(1.0);

  glGenTextures(1, &carTex);
  carBmp = LoadBmp("./gfx/car.bmp", &carBmpInfo);
  glBindTexture(GL_TEXTURE_2D, carTex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, carBmpInfo.biWidth, carBmpInfo.biHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, carBmp);

  glGenTextures(1, &maskTex);
  maskBmp = LoadBmp("./gfx/mask.bmp", &maskBmpInfo);
  glBindTexture(GL_TEXTURE_2D, maskTex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, maskBmpInfo.biWidth, maskBmpInfo.biHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, maskBmp);

  glGenTextures(1, &mapTex);
  mapBmp = LoadBmp("./gfx/map.bmp", &mapBmpInfo);
  glBindTexture(GL_TEXTURE_2D, mapTex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mapBmpInfo.biWidth, mapBmpInfo.biHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, mapBmp);

  glGenTextures(1, &bkgTex);
  bkgBmp = LoadBmp("./gfx/bkg.bmp", &bkgBmpInfo);
  glBindTexture(GL_TEXTURE_2D, bkgTex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bkgBmpInfo.biWidth, bkgBmpInfo.biHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, bkgBmp);

  HFONT hFont;
  listBase = glGenLists(96);
  hFont = CreateFont(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET,
                      OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
                      FF_DONTCARE | DEFAULT_PITCH, "Arial");

  SelectObject(*hDC, hFont);
  wglUseFontBitmapsA(*hDC, 32, 96, listBase);
}

void DisableOpenGL (HWND hWnd, HDC hDC, HGLRC hRC)
{
  wglMakeCurrent(NULL, NULL);
  wglDeleteContext(hRC);
  ReleaseDC(hWnd, hDC);

  free(mapBmp);
  free(maskBmp);
  free(bkgBmp);
  free(carBmp);

  glDeleteLists(listBase, 96);
}

void DrawText(char * s)
{
  glPushAttrib(GL_LIST_BIT);
    glListBase(listBase-32);
    glCallLists(strlen(s), GL_UNSIGNED_BYTE, s);
  glPopAttrib();
  glColor3ub(255,255,255);
}

BMP LoadBmp(char *fname, BMPINFO *bmp_info)
{
  FILE *file;
  BITMAPFILEHEADER fheader;
  BMP bmp;

  file = fopen(fname, "rb");
  if (!file) return NULL;

  fread(&fheader, sizeof(BITMAPFILEHEADER), 1, file);

  //check if BMP header
  if (fheader.bfType != 0x4D42)
    {
      fclose(file);
      return NULL;
    }

  fread(bmp_info, sizeof(BMPINFO), 1, file);

  fseek(file, fheader.bfOffBits, SEEK_SET);

  bmp = (BMP) malloc(bmp_info->biSizeImage);

  //check if mem allocated
  if (!bmp)
    {
      free(bmp);
      fclose(file);
      return NULL;
    }

  fread(bmp, 1, bmp_info->biSizeImage, file);

  if (bmp == NULL)
    {
      fclose(file);
      return NULL;
    }

  //swap BGR -> RGB
  for (unsigned int i=0; i < bmp_info->biSizeImage; i+=3)
    {
      unsigned char tmp = bmp[i];
      bmp[i] = bmp[i+2];
      bmp[i+2] = tmp;
    }

  fclose(file);
  return bmp;
}

void drawCar(GLdouble angle)
{
  glEnable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);

  glPushMatrix();
  glRotated(angle, 0.0, 0.0, 1.0);
  glTranslated(R,0,-4);

  glBlendFunc(GL_DST_COLOR,GL_ZERO);
  glBindTexture(GL_TEXTURE_2D, maskTex);
    glBegin(GL_QUADS);
      glTexCoord2d(0,0); glVertex3d(-1, -1, 0);
      glTexCoord2d(0,1); glVertex3d(-1, 1, 0);
      glTexCoord2d(1,1); glVertex3d(1, 1, 0);
      glTexCoord2d(1,0); glVertex3d(1, -1, 0);
    glEnd();

    glBlendFunc(GL_ONE, GL_ONE);
    glBindTexture(GL_TEXTURE_2D, carTex);
      glBegin(GL_QUADS);
        glTexCoord2d(0,0); glVertex3d(-1, -1, 0);
        glTexCoord2d(0,1); glVertex3d(-1, 1, 0);
        glTexCoord2d(1,1); glVertex3d(1, 1, 0);
        glTexCoord2d(1,0); glVertex3d(1, -1, 0);
      glEnd();
  glPopMatrix();

  glEnable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);
}

void drawMap()
{
  GLdouble size = 8;
  glBindTexture(GL_TEXTURE_2D, mapTex);

  glPushMatrix();
  glTranslated(0,0,-4.1);
    glBegin(GL_QUADS);
      glTexCoord2d(0,0); glVertex3d(-size, -size, 0);
      glTexCoord2d(0,1); glVertex3d(-size, size, 0);
      glTexCoord2d(1,1); glVertex3d(size, size, 0);
      glTexCoord2d(1,0); glVertex3d(size, -size, 0);
    glEnd();
  glPopMatrix();
}

void drawBkg()
{
  GLdouble size = 10;
  glBindTexture(GL_TEXTURE_2D, bkgTex);

  glPushMatrix();
  glTranslated(0,0,-4.2);
    glBegin(GL_QUADS);
      glTexCoord2d(0,0); glVertex3d(-size, -size, 0);
      glTexCoord2d(0,10); glVertex3d(-size, size, 0);
      glTexCoord2d(10,10); glVertex3d(size, size, 0);
      glTexCoord2d(10,0); glVertex3d(size, -size, 0);
    glEnd();
  glPopMatrix();
}

void Render()
{
  glTranslated(-2.65,0,0);

  drawBkg();
  drawMap();

  for (int i=0; i<MAX; i++)
    {
      double pos = Env::position(i);

      drawCar(pos);

      char s[5];
      sprintf(s,"%d",i+1);
      glDisable(GL_TEXTURE_2D);
      glPushMatrix();
      glRotated(pos, 0.0, 0.0, 1.0);
      glTranslated(R+1,0,-4);
      glColor3ub(255,255,255);
      glRasterPos2d(0,0);
      DrawText(s);
      glPopMatrix();
      glColor3ub(255,255,255);
      glEnable(GL_TEXTURE_2D);
    }
}
