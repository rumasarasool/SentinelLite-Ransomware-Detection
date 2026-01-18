#include <windows.h>
#include <stdio.h>
#include <string.h>

#define MAX_EVENTS 200
#define WINDOW_MS 3000 // 3-second sliding window


// 1. Data Structures
struct Event {
    DWORD time;
    char type[20];
    char name[MAX_PATH];
};

Event queue[MAX_EVENTS];
int front = 0, rear = -1, count = 0;


// 2. GUI Handles & Severity Flags
HWND hwndStatus, hwndAlert;
bool warned = false, renameModify = false, extensionAlert = false, ransomAlert = false;


// 3. Utility: Add log to Edit control
void addLog(HWND box, const char* prefix, const char* msg) {
    char buffer[512];
    sprintf(buffer, "%s %s\r\n", prefix, msg);
    int len = GetWindowTextLength(box);
    SendMessage(box, EM_SETSEL, len, len);
    SendMessage(box, EM_REPLACESEL, 0, (LPARAM)buffer);
    SendMessage(box, WM_VSCROLL, SB_BOTTOM, 0);
}


// 4. Sliding Window Logic
void pushEvent(const char* type, const char* name) {
    DWORD now = GetTickCount();
    rear = (rear + 1) % MAX_EVENTS;
    strcpy(queue[rear].type, type);
    strcpy(queue[rear].name, name);
    queue[rear].time = now;

    if (count < MAX_EVENTS) count++;
    else front = (front + 1) % MAX_EVENTS;
}

void cleanQueue() {
    DWORD now = GetTickCount();
    while (count > 0 && now - queue[front].time > WINDOW_MS) {
        front = (front + 1) % MAX_EVENTS;
        count--;
    }
}

bool isSuspiciousExtension(const char* filename) {
    const char* ext = strrchr(filename, '.');
    if (!ext) return false;

    // Normal extensions whitelist
    if (!strcmp(ext, ".txt") ||
        !strcmp(ext, ".docx") ||
        !strcmp(ext, ".pdf") ||
        !strcmp(ext, ".jpg") ||
        !strcmp(ext, ".png") ||
        !strcmp(ext, ".exe"))
        return false;

    // Random ransomware extensions are usually 5–10 chars
    int len = strlen(ext);
    if (len >= 5 && len <= 12)
        return true;

    return false;
}



// 5. Detection Engine
void detect() {
    int mods = 0, renames = 0, enc = 0;
    bool noteFound = false;

    for (int i = 0; i < count; i++) {
        int idx = (front + i) % MAX_EVENTS;
        if (strcmp(queue[idx].type, "MODIFIED") == 0) mods++;
        if (strcmp(queue[idx].type, "RENAMED") == 0) renames++;

        if (isSuspiciousExtension(queue[idx].name))
    enc++;

        if (strstr(queue[idx].name, "READ_ME") || strstr(queue[idx].name, "HOW_TO")) noteFound = true;
    }

    if (mods >= 3 && mods <= 5 && !warned)
        addLog(hwndAlert, "[ALERT]", "Suspicious file activity detected.");

    if (mods > 6 && !warned) {
        addLog(hwndAlert, "[WARNING]", "High frequency modifications detected.");
        warned = true;
    }

    if (mods > 5 && renames > 5 && !renameModify) {
        addLog(hwndAlert, "[CRITICAL]", "Ransomware Pattern: Mass Rename + Modify.");
        MessageBox(NULL, "Ransomware Attack Pattern Detected!", "SentinelLite", MB_ICONSTOP);
        renameModify = true;

        // Reset sliding window to prevent repeated alerts
        front = 0; rear = -1; count = 0;
    }

    if (enc > 3 && !extensionAlert) {
        addLog(hwndAlert, "[CRITICAL]", "Mass encryption with random extensions detected.");
        extensionAlert = true;
        front = 0; rear = -1; count = 0;
    }

    if (noteFound && !ransomAlert) {
        addLog(hwndAlert, "[CRITICAL]", "Ransom note identified in directory.");
        ransomAlert = true;
        front = 0; rear = -1; count = 0;
    }
}


// 6. File Monitoring Thread
DWORD WINAPI monitor(LPVOID lp) {
    const char* path = (const char*)lp;
    HANDLE hDir = CreateFileA(path, FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);

    if (hDir == INVALID_HANDLE_VALUE) {
        addLog(hwndStatus, "[ERROR]", "Failed to open directory for monitoring.");
        return 1;
    }

    addLog(hwndStatus, "[INFO]", "Monitoring folder started...");

    char buffer[4096];
    DWORD bytes;

    while (TRUE) {
        if (ReadDirectoryChangesW(hDir, buffer, sizeof(buffer), TRUE,
            FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE,
            &bytes, NULL, NULL)) {

            FILE_NOTIFY_INFORMATION* f = (FILE_NOTIFY_INFORMATION*)buffer;
            bool newEvent = false;

            do {
                char name[MAX_PATH] = {0};
                WideCharToMultiByte(CP_ACP, 0, f->FileName, f->FileNameLength / 2, name, MAX_PATH, NULL, NULL);

                if (f->Action == FILE_ACTION_MODIFIED) {
                    pushEvent("MODIFIED", name);
                    newEvent = true;
                }
                if (f->Action == FILE_ACTION_RENAMED_NEW_NAME) {
                    pushEvent("RENAMED", name);
                    newEvent = true;
                }

                if (!f->NextEntryOffset) break;
                f = (FILE_NOTIFY_INFORMATION*)((char*)f + f->NextEntryOffset);

            } while (true);

            if (newEvent) {
                cleanQueue();
                detect(); // detect only when new event arrives
            }
        }
        Sleep(50); // reduce CPU usage
    }

    return 0;
}


// 7. Win32 Window Procedure

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd,msg,wParam,lParam);
    }
    return 0;
}


// 8. Main Function (Console + GUI Windows)
int main() {
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "SentinelLiteGUI";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClass(&wc)) {
        MessageBox(NULL, "Window Registration Failed!","Error",MB_ICONEXCLAMATION|MB_OK);
        return 0;
    }

    HWND mainWin = CreateWindow("SentinelLiteGUI", "SentinelLite - System Status",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        100,100,400,300,0,0,wc.hInstance,0);

    HWND alertWin = CreateWindow("SentinelLiteGUI", "SentinelLite - Security Alerts",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        550,100,400,300,0,0,wc.hInstance,0);

    hwndStatus = CreateWindow("EDIT","",WS_CHILD|WS_VISIBLE|ES_MULTILINE|ES_READONLY|WS_VSCROLL,
        10,10,360,240,mainWin,0,wc.hInstance,0);

    hwndAlert = CreateWindow("EDIT","",WS_CHILD|WS_VISIBLE|ES_MULTILINE|ES_READONLY|WS_VSCROLL,
        10,10,360,240,alertWin,0,wc.hInstance,0);

    // Start monitoring thread
    const char* targetFolder = "C:\\Users\\hp\\Documents\\TestMonitor";
    HANDLE hThread = CreateThread(NULL,0,monitor,(LPVOID)targetFolder,0,NULL);
    if (!hThread) {
        MessageBox(NULL,"Failed to start monitor thread","Error",MB_ICONERROR);
        return 0;
    }

    printf("SentinelLite running. GUI windows opened.\n");

    MSG msg;
    while (GetMessage(&msg, NULL,0,0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

