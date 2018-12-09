/*
Black and White 2 Tiger unlocker
(c) 2018 Khral Steelforge <https://github.com/kytulendu>

BW2TigerUnlocker is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#if defined(UNICODE) && !defined(_UNICODE)
    #define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
    #define UNICODE
#endif

#include <tchar.h>
#include <windows.h>
#include <shlwapi.h>
#include <stdio.h>

#include "resource.h"

/* Declare function prototype */
LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);

/* Make the class name into a global variable */
TCHAR szClassName[ ] = _T("BW2TigerUnlocker");

/* Define global variable */
WNDPROC g_hOldButtonProc = NULL;
HBITMAP g_hKittyBitmap = NULL;

LPCTSTR g_sRegKey = "Software\\Microsoft\\Windows\\CurrentVersion";
LPCTSTR g_sNTRegKey = "Software\\Microsoft\\Windows NT\\CurrentVersion";

int WINAPI WinMain(HINSTANCE hThisInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpszArgument,
                     int nCmdShow)
{
    HWND hWnd;               /* This is the handle for our window */
    MSG messages;            /* Here messages to the application are saved */
    WNDCLASSEX wincl;        /* Data structure for the windowclass */

    /* The Window structure */
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
    wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
    wincl.cbSize = sizeof(WNDCLASSEX);

    /* Use default icon and mouse-pointer */
    wincl.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APPICON));
    wincl.hIconSm = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APPICON));
    wincl.hCursor = LoadCursor(NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;                 /* No menu */
    wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;                      /* structure or the window instance */
    /* Use Windows's default colour as the background of the window */
    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

    /* Register the window class, and if it fails quit the program */
    if (!RegisterClassEx(&wincl))
        return 0;

    /* The class is registered, let's create the program*/
    hWnd = CreateWindowEx(
           0,                   /* Extended possibilites for variation */
           szClassName,         /* Classname */
           _T("Black and White 2 Tiger Unlocker"),       /* Title Text */
           WS_OVERLAPPED | WS_SYSMENU, /* default window */
           CW_USEDEFAULT,       /* Windows decides the position */
           CW_USEDEFAULT,       /* where the window ends up on the screen */
           264,                 /* The programs width */
           510,                 /* and height in pixels */
           HWND_DESKTOP,        /* The window is a child-window to desktop */
           NULL,                /* No menu */
           hThisInstance,       /* Program Instance handler */
           NULL                 /* No Window Creation data */
           );

    /* Make the window visible on the screen */
    ShowWindow(hWnd, nCmdShow);

    /* Run the message loop. It will run until GetMessage() returns 0 */
    while (GetMessage(&messages, NULL, 0, 0))
    {
        /* Translate virtual-key messages into character messages */
        TranslateMessage(&messages);
        /* Send message to WindowProcedure */
        DispatchMessage(&messages);
    }

    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    return messages.wParam;
}

void OnPaint(HWND hWnd)
{
    PAINTSTRUCT ps;
    BITMAP bm;
    HBITMAP hBitmapOld;
    HDC hDC;
    HDC hMemoryDC;

    hDC = BeginPaint(hWnd, &ps);

    hMemoryDC = CreateCompatibleDC(hDC);
    hBitmapOld = SelectObject(hMemoryDC, g_hKittyBitmap);

    GetObject(g_hKittyBitmap, sizeof(bm), &bm);

    BitBlt(hDC, 0, 0, bm.bmWidth, bm.bmHeight, hMemoryDC, 0, 0, SRCCOPY);

    SelectObject(hMemoryDC, hBitmapOld);

    DeleteDC(hMemoryDC);

    EndPaint(hWnd, &ps);
}

LRESULT CALLBACK ButtonProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_LBUTTONDOWN:
        {
            OPENFILENAME ofn;
            TCHAR szFileName[MAX_PATH] = "";
            LSTATUS result;
            HKEY hRegKey = NULL;
            HKEY hNTRegKey = NULL;
            HKEY hKey1 = NULL;
            TCHAR buf[255] = {'\0'};
            DWORD bufSize = sizeof(buf);
            DWORD dwType;
            FILE* unlockFile;
            DWORD count;
            TCHAR encoded;
            int key;

            /* always use HKEY_LOCAL_MACHINE\Software\Wow6432Node\Microsoft\Windows\CurrentVersion
             * on 64 bit Windows, KEY_WOW64_32KEY is ignored on 32bit Windows */
            result = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                    g_sRegKey,
                                    0,
                                    KEY_READ | KEY_WOW64_32KEY,
                                    &hRegKey);
            if(result != ERROR_SUCCESS)
            {
                MessageBox(hWnd, "Unable to open rrawrgistry key.", "Error", MB_OK | MB_ICONEXCLAMATION);
                return 0;
            }

            /* get product ID of your Windows system, this value only exist in Windows 2000 */
            result = RegQueryValueEx(hRegKey, "ProductId", NULL, &dwType, (LPBYTE) buf, &bufSize);
            if(result != ERROR_SUCCESS)
            {
                /* ask to create ProductId value if it not exist */
                result = MessageBox(hWnd,
                                        "ProductId string value is not exist!\n"
                                        "Would you like to create it with your Windows Product ID?\n"
                                        "If you select No, it will be create using blank string.\n"
                                        "But if you select Cancel, no kitty for you :c",
                                        "",
                                        MB_YESNOCANCEL | MB_ICONQUESTION);
                if(result == IDCANCEL)
                {
                    RegCloseKey(hRegKey);
                    return 0;
                }
                else if(result == IDYES)
                {
                    /* read ProductId from registry (Windows XP+) */
                    result = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                            g_sNTRegKey,
                                            0,
                                            KEY_QUERY_VALUE | KEY_WOW64_64KEY,
                                            &hNTRegKey);
                    if(result != ERROR_SUCCESS)
                    {
                        MessageBox(hWnd, "Unable to open rrawrgistry key.", "Error", MB_OK | MB_ICONEXCLAMATION);
                        RegCloseKey(hRegKey);
                        return 0;
                    }
                    result = RegQueryValueEx(hNTRegKey, "ProductId", NULL, &dwType, (LPBYTE) buf, &bufSize);
                    /* in case it also not exist */
                    if(result != ERROR_SUCCESS)
                    {
                        /* force empty string */
                        buf[0] = '\0';
                        bufSize = sizeof(BYTE);
                    }

                    RegCloseKey(hNTRegKey);
                }
                else
                    bufSize = sizeof(BYTE); /* we only need 1 byte for empty string */

                result = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                                            g_sRegKey,
                                            0,
                                            NULL,
                                            REG_OPTION_NON_VOLATILE,
                                            KEY_ALL_ACCESS | KEY_WOW64_32KEY,
                                            NULL,
                                            &hKey1,
                                            NULL);
                if(result != ERROR_SUCCESS)
                {
                    MessageBox(hWnd,
                                   "Can't write registry.\nDid you run this program as Administrator?",
                                   "",
                                   MB_OK | MB_ICONEXCLAMATION);
                    RegCloseKey(hRegKey);
                    RegCloseKey(hKey1);
                    return 0;
                }

                /* create a string value */
                RegSetValueEx(hKey1, "ProductId", 0, REG_SZ, (LPBYTE) buf, bufSize);

                /* reread the registry that just written */
                result = RegQueryValueEx(hRegKey, "ProductId", NULL, &dwType, (LPBYTE) buf, &bufSize);
            }

            ZeroMemory(&ofn, sizeof(ofn));

            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hWnd;
            ofn.lpstrTitle = "Choose Black and White 2 (white.exe) executable file";
            ofn.lpstrFilter = "Application (*.exe)\0*.exe";
            ofn.lpstrFile = szFileName;
            ofn.nMaxFile = MAX_PATH;
            ofn.Flags = OFN_EXPLORER;
            ofn.lpstrDefExt = "exe";

            if(GetOpenFileName(&ofn))
            {
                PathRemoveFileSpec(szFileName); /* remove file name from full file path */
                SetCurrentDirectory(szFileName);

                unlockFile = fopen("Unlock.dat", "wb");
                if(!unlockFile)
                {
                    MessageBox(hWnd,
                                   "Unable to write to Black and White 2's directory.\n"
                                   "Check if you have write purrmission on that directory.",
                                   "Error",
                                   MB_OK | MB_ICONEXCLAMATION);
                    return 0;
                }

                /* key generator algorithm */
                count = 0;
                key = 1234567890;
                if(bufSize)
                {
                    do
                    {
                        encoded = buf[count] ^ *((BYTE *)&key + (count & 3));
                        fwrite(&encoded, 1, 1, unlockFile);
                        ++count;
                    }
                    while(count < bufSize);
                }
                fclose(unlockFile);

                MessageBox(hWnd, "Thank you for unlock me ^w^", "Purr~", MB_OK | MB_ICONINFORMATION);
            }
            RegCloseKey(hRegKey);
            RegCloseKey(hKey1);
            return 0;
        }
    }
    return CallWindowProc(g_hOldButtonProc, hWnd, message, wParam, lParam);
}

/*  This function is called by the Windows function DispatchMessage()  */
LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)                  /* handle the messages */
    {
        case WM_CREATE:
        {
            HWND hButton;

            g_hKittyBitmap = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_BITMAP1));

            hButton = CreateWindow("button",
                                        "Unlock me! OwO",
                                        WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                                        70, 455,
                                        120, 25,
                                        hWnd,
                                        (HMENU) IDB_UNLOCK,
                                        NULL,
                                        NULL );

            g_hOldButtonProc = (WNDPROC) SetWindowLongPtr(hButton, GWLP_WNDPROC, (LONG_PTR) ButtonProc);
            break;
        }
        case WM_PAINT:
            OnPaint(hWnd);
            break;
        case WM_DESTROY:
            DeleteObject(g_hKittyBitmap);

            PostQuitMessage (0);       /* send a WM_QUIT to the message queue */
            break;
        default:                      /* for messages that we don't deal with */
            return DefWindowProc (hWnd, message, wParam, lParam);
    }

    return 0;
}
