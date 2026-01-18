#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>

#define TARGET_FOLDER "C:\\Users\\hp\\Documents\\TestMonitor"
#define RANSOM_NOTE "HOW_TO_RESTORE.txt"

void writeRansomNote() {
    std::ofstream note(std::string(TARGET_FOLDER) + "\\" + RANSOM_NOTE);
    note << "Your files are encrypted.\n";
    note << "This is a SAFE simulation.\n";
    note.close();
}

std::string removeExtension(const std::string& name) {
    size_t pos = name.find_last_of(".");
    if (pos == std::string::npos) return name;
    return name.substr(0, pos);
}

int main() {
    std::cout << "[SIMULATOR] Started\n";

    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(
        std::string(TARGET_FOLDER).append("\\*").c_str(),
        &fd
    );

    if (hFind == INVALID_HANDLE_VALUE) {
        std::cout << "Folder not found\n";
        return 1;
    }

    do {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            continue;

        std::string oldName = fd.cFileName;

        if (oldName == RANSOM_NOTE) continue;

        std::string base = removeExtension(oldName);
        std::string newName = base + ".crypt";

        std::string oldPath = std::string(TARGET_FOLDER) + "\\" + oldName;
        std::string newPath = std::string(TARGET_FOLDER) + "\\" + newName;

        // Rename file
        MoveFileA(oldPath.c_str(), newPath.c_str());

        // Simulate encryption write
        HANDLE hFile = CreateFileA(
            newPath.c_str(),
            GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );

        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD written;
            const char* data = "=== SIMULATED ENCRYPTED DATA ===";
            WriteFile(hFile, data, strlen(data), &written, NULL);
            CloseHandle(hFile);
        }

        Sleep(100); // burst behavior

    } while (FindNextFileA(hFind, &fd));

    FindClose(hFind);

    writeRansomNote();

    std::cout << "[SIMULATOR] Finished\n";
    return 0;
}


