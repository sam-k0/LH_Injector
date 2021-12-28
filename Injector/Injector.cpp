// Injector.cpp : Diese Datei enthält die Funktion "main". Hier beginnt und endet die Ausführung des Programms.
//
#pragma warning(disable : 4996)
#include <iostream>
#include <windows.h>
#include <string>
#include <thread>
#include <libloaderapi.h>
#include <stdio.h>
#include <direct.h>
#include <processthreadsapi.h>
#include "gms.h"

#define INJECTOR_VER 1.0

// Discord GMRPC
typedef gmbool(*DISCORD_INIT)(const char* APPLICATION_ID); // Init function pointer
typedef gmbool(*DISCORD_SETPRESENCE)(const char* state, const char* details, const char* smallImage, const char* largeImage);
typedef gmint(*DISCORD_CHECK)(gmint n);

/**
* @brief Global Variables
*/
const char* THIS_APP_ID = "832672175122546701";
const char* GMRPC_DLL_NAME = "GMRPC.dll";
gmbool rpc_ready = gmfalse;
gmbool rpc_quit = gmfalse;
static DISCORD_INIT gmrpc_init; // Pointers to the functions in GMRPC dll
static DISCORD_SETPRESENCE gmrpc_setPresence;// Pointers to the functions in GMRPC dll
static DISCORD_CHECK gmrpc_checkConnection;
HMODULE hModule; // GMRPC module handle

using namespace std;

// Get ID from Window
void getProcessId(const char* window_title, DWORD& process_id)
{
    GetWindowThreadProcessId(FindWindow(NULL, window_title), &process_id);
}

// Show err
void showErr(const char* errTitle, const char* errMessage)
{
    MessageBox(0, errMessage, errTitle, NULL);
    exit(-1);
}
// Show message
void showMsg(const char* errTitle, const char* errMessage)
{
    MessageBox(0, errMessage, errTitle, NULL);
}

// Check if file exists
bool fileExists(string fileName)
{
    struct stat buffer;
    return (stat(fileName.c_str(), &buffer) == 0);
}

string getCurrentDir() // Returns EXE directory
{
    /*
    https://stackoverflow.com/questions/143174/how-do-i-get-the-directory-that-a-program-is-running-from
    */

    // get working directory
    char cCurrentPath[FILENAME_MAX]; // Buffer

    if (!_getcwd(cCurrentPath, sizeof(cCurrentPath)))
    {
        showErr("Could not get current directory", "Something went wrong.");
        exit(-1);
    }

    cCurrentPath[sizeof(cCurrentPath) - 1] = '\0'; /* not really required */

    //printf("The current working directory is %s\n", cCurrentPath);
    char* s = cCurrentPath;
    return string(s);
}

/* */
bool InjectDLL(const int& pid, const string& DLL_Path)
{
    long dll_size = DLL_Path.length() + 1;
    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

    if (hProc == NULL)
    {
        cerr << "[!]Fail to open target process!" << endl;
        return false;
    }
    cout << "[+]Opening Target Process..." << endl;

    LPVOID MyAlloc = VirtualAllocEx(hProc, NULL, dll_size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (MyAlloc == NULL)
    {
        cerr << "[!]Fail to allocate memory in Target Process." << endl;
        return false;
    }

    cout << "[+]Allocating memory in Target Process." << endl;
    int IsWriteOK = WriteProcessMemory(hProc, MyAlloc, DLL_Path.c_str(), dll_size, 0);
    if (IsWriteOK == 0)
    {
        cerr << "[!]Fail to write in Target Process memory." << endl;
        return false;
    }
    cout << "[+]Creating Remote Thread in Target Process" << endl;

    DWORD dWord;
    LPTHREAD_START_ROUTINE addrLoadLibrary = (LPTHREAD_START_ROUTINE)GetProcAddress(LoadLibrary("kernel32"), "LoadLibraryA");
    HANDLE ThreadReturn = CreateRemoteThread(hProc, NULL, 0, addrLoadLibrary, MyAlloc, 0, &dWord);
    if (ThreadReturn == NULL)
    {
        cerr << "[!]Fail to create Remote Thread" << endl;
        return false;
    }

    if ((hProc != NULL) && (MyAlloc != NULL) && (IsWriteOK != ERROR_INVALID_HANDLE) && (ThreadReturn != NULL))
    {
        cout << "[+]DLL Successfully Injected :)" << endl;
        return true;
    }

    return false;
}

void showHello(const char* dllname)
{
    cout << "Welcome to LH-Injector! Version " << INJECTOR_VER << "\n"
        << "Attempting to load dll: " << dllname << endl;

    if (dllname == "raw")
    {
        cout << "ATTENTION: parameter 'raw' is default when no dll is specified as startup parameter.\n" <<
            "Parameter 'raw' will only execute the Injector and load Discord presence." <<
            "If you want to load other mods / dlls, pass them as startup parameter." <<
            ">> LH_Injector.exe 'YourDLLName.dll'\n\n" << endl;
    }
}

/**
        Discord GMRPC stuff goes here
*/

/**
* @brief This will initialize the function pointers from GMRPC.dll
*/
int loadGMRPC()
{
    // Get Directory
    char* _buf;
    _buf = _getcwd(NULL, 0);
    string __buf = string(_buf);
    __buf = __buf + "\\" + "GMRPC.dll";
    const char* GMRPCDLLPATH = __buf.c_str();
    cout << "[LH DLL] About to load GMRPC" << GMRPCDLLPATH << endl;

    hModule = LoadLibrary(TEXT(GMRPCDLLPATH)); // Load the DLL

    if (hModule == NULL)
    {
        cout << "Could not LoadLibrary " << GMRPC_DLL_NAME << endl;
        return 0;
    }
    // Load the gmrpc_init function to pointer of type DISCORD_INIT
    gmrpc_init = (DISCORD_INIT)GetProcAddress(hModule, "gmrpc_init");

    // Load the gmrpc_setPresence function to pointer of type DISCORD_SETPRESENCE
    gmrpc_setPresence = (DISCORD_SETPRESENCE)GetProcAddress(hModule, "gmrpc_setPresence");

    // Load the gmrpc_setPresence function to pointer of type DISCORD_SETPRESENCE
    gmrpc_checkConnection = (DISCORD_CHECK)GetProcAddress(hModule, "gmrpc_checkConnection");

    if (gmrpc_init != NULL && gmrpc_setPresence != NULL && gmrpc_checkConnection != NULL)
    {
        cout << "[LH DLL] Done loading GMRPC" << endl;
        return 1;
    }
    cout << "[LH DLL]Error loading GMRPC" << endl;
    return 0;

}

/**
* @brief This will make a quick math check on GMRPC.dll
*/
bool checkGMRPC_Connection()
{
    gmint n = 5;
    gmint r = gmrpc_checkConnection(n);
    if (n * 2 == r)
    {
        cout << "[LH DLL] GMRPC Connection check successfull!" << endl;
        return true;
    }
    else
    {
        cout << "[LH DLL] GMRPC Connection check failed!" << endl;
        return false;
    }
}

/**
*@brief This will initialize the GMRPC.dll by calling the init function gmrpc_init
*/
void initGMRPC()
{
    rpc_ready = gmrpc_init(THIS_APP_ID);
    if (rpc_ready == gmtrue)
    {
        cout << "Initialized GMRPC in LH_RPC with app id" << THIS_APP_ID << endl;
    }
    else {
        cout << "Failed initializing GMRPC in LH_RPC with app id" << THIS_APP_ID << endl;
    }
}

/**
*@brief Set Presence
*/
void setGMRPC()
{
    cout << "[LH DLL]Set presence" << endl;

    while (!rpc_ready)
    {
        //wait
        cout << "Waiting for RPC..." << endl;
    }

    gmrpc_setPresence("Loop Hero", "Playing with mods", "", "lhicon");
}

/**
*@brief Handles all GMRPC calls
*/
void doDiscordGMRPC()
{
    loadGMRPC(); // load gmrpc dll
    
    if (checkGMRPC_Connection())
    {
        // Success
        initGMRPC(); // Connect to discord client with APPID
        setGMRPC();  // Set the presence in discord
    }
    else
    {
        showErr("Discord RPC", "Could not load GMRPC.dll or connect to discord.");
    }

}

/**
        Main
*/
int main(int argc, char* argv[])
{   
    DWORD processID = NULL;
    //char dllPath[MAX_PATH];

    /* Select dll */
    const char* dllName = "raw"; //
    if (argc == 2)
    {
        dllName = argv[1]; // If parameter passed, use that 
    }
    // Show hello message
    showHello(dllName);
    

    // Init path
    string programDirectory = getCurrentDir();
    string programPath = programDirectory + "\\" + "Loop Hero.exe";

    cout << "Parameter: " << dllName << endl; // The passed dll name
    cout << "Program Path: ";
    cout << programPath << endl;
    cout << "Program Directory: ";
    cout << programDirectory << endl;



    // Start the Process
    PROCESS_INFORMATION ProcI;
    STARTUPINFO StartI = { sizeof(StartI) };
    StartI.cb = sizeof(STARTUPINFO);
    // Try to create the process
    if (CreateProcessA(NULL, &programPath[0], NULL, NULL, FALSE, 0, NULL, NULL, &StartI, &ProcI))
    {
        WaitForSingleObject(ProcI.hProcess, 5*1000); // wait for the thing to finish
        CloseHandle(ProcI.hProcess); // Close handle for now
        CloseHandle(ProcI.hThread);
    }
    else // Create proc Failed
    {
        showErr("CreateProcess", "Could not start the Executable. Check if 'Loop Hero.exe' exists in current folder.");
        exit(-1);
    }

    // Check if we only want to start loop hero ("raw" is passed)
    if (dllName != "raw")
    {
        // Check if DLL exists
        string dllPathStr = getCurrentDir() + "\\mods\\" + dllName;
        cout << "DLL path: " << dllPathStr << endl;
        if (!fileExists(dllPathStr)) // File not found
        {
            showErr("File Not Found", "File doesn't exist");
        }
        else
        {
            cout << dllName << "has been located." << endl;
        }
        /*
                    Somehow get a ref to the processID of the process...
        */
        processID = ProcI.dwProcessId; // Use the one from start process
        if (processID == NULL)
        {
            showErr("getProcessId", "Could not get process ID");
        }
        // Try to inject
        bool returnVal = InjectDLL(processID, dllPathStr);
        if (returnVal)
        {
            //showMsg("Success!", "Injected dll");
            cout << "LH_Injector : Success!" << endl;
        }
        else
        {
            showMsg("Fail!", "Please restart.");
            HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
            ExitProcess(GetExitCodeProcess(hProc, NULL));
        }
    }
     
    // Do all the discord stuff
    doDiscordGMRPC();
    // Check if quit

    cout << "Enter 'q' to quit." << endl;
    char input = ' ';
    while (true)
    {
        input = getchar();
        if (input == 'q') {
            HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
            ExitProcess(GetExitCodeProcess(hProc, NULL));
        }
    }

    return 0;
}

