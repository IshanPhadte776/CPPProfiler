// Dear ImGui: standalone example application for DirectX 9

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include <d3d9.h>
#include <tchar.h>

#include "extraFunc.h"
#include "NumberProcessor.h"

#include <iostream>
#include <chrono>
#include <windows.h>
#include <thread> // For sleep functionality
#include <filesystem> // C++17 filesystem
namespace fs = std::filesystem;

// Profiling variables
bool startProfiling = false;
bool profilingInProgress = false;



//Time in Seconds
double executionTime = 0.0;
// This measures the actual CPU processing time, excluding idle or waiting periods.
double cpuUsage = 0.0;

double lastExecutionTime = 0.0;
double lastCpuUsage = 0.0;

std::string lastProfiledFile; // Store the name of the last profiled file


// Data
static LPDIRECT3D9              g_pD3D = nullptr;
static LPDIRECT3DDEVICE9        g_pd3dDevice = nullptr;
static bool                     g_DeviceLost = false;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ResetDevice();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void sampleFunction(int N);
void profileFunction(double& executionTime, double& cpuUsage);
double getCpuUsage(FILETIME& prevSysKernel, FILETIME& prevSysUser, FILETIME& prevSysIdle);
void profileFile(const std::string& filePath, double& executionTime, double& cpuUsage);

// Main code
int main(int, char**)
{
    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Dear ImGui DirectX9 Example", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != nullptr);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Handle lost D3D9 device
        if (g_DeviceLost)
        {
            HRESULT hr = g_pd3dDevice->TestCooperativeLevel();
            if (hr == D3DERR_DEVICELOST)
            {
                ::Sleep(10);
                continue;
            }
            if (hr == D3DERR_DEVICENOTRESET)
                ResetDevice();
            g_DeviceLost = false;
        }

        // Handle window resize (we don't resize directly in the WM_SIZE handler)
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            g_d3dpp.BackBufferWidth = g_ResizeWidth;
            g_d3dpp.BackBufferHeight = g_ResizeHeight;
            g_ResizeWidth = g_ResizeHeight = 0;
            ResetDevice();
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Create a separate window for profiling options
        ImGui::Begin("Profiler");

        // Display profiling button if not already profiling
        if (ImGui::Button("Start Profiling")) {
            startProfiling = true; // Trigger profiling
        }

        // Display profiling results if profiling has started
        if (startProfiling) {
            // Profile the function and get results
            profileFunction(executionTime, cpuUsage);

            // Convert execution time to seconds for display
            executionTime = executionTime / 1000.0;

            unsigned int numCores = std::thread::hardware_concurrency();
            std::cout << "Execution Time: " << executionTime << " s" << std::endl;

            // Calculate CPU usage as a percentage relative to the execution time
            double cpuPercentage = (cpuUsage / executionTime) / numCores;
            std::cout << "CPU Usage: " << cpuPercentage << " %" << std::endl;

            // Reset the profiling values
            executionTime = 0.0;
            cpuUsage = 0.0;

            // Stop profiling after showing results
            startProfiling = false;
        }


        // 2. File Selector Window

        static std::vector<std::string> fileList;   // List of files in the current directory
        static std::string selectedFile;           // The selected file
        static bool filesListed = false;           // Flag to indicate if files have been listed

        ImGui::Text("Select a file to profile:");

        // If the file list is not yet loaded, populate it
        if (!filesListed) {
            fileList.clear(); // Clear the list before populating
            for (const auto& entry : fs::directory_iterator(".")) { // Iterate in the current directory
                if (entry.is_regular_file() && entry.path().extension() == ".cpp") {
                    fileList.push_back(entry.path().filename().string());
                }
            }
            filesListed = true; // Files are now listed
        }

        // Display the files as selectable items
        for (const auto& file : fileList) {
            if (ImGui::Selectable(file.c_str(), selectedFile == file)) {
                selectedFile = file; // Set the selected file
            }
        }



        // Run button logic
        if (!selectedFile.empty()) { // Enable profiling only if a file is selected
            if (ImGui::Button("Profile Selected File")) {
                lastProfiledFile = selectedFile; // Update the last profiled file
                std::cout << "Profiling file: " << selectedFile << std::endl;

                // Trigger profiling (you should define profileFunction logic elsewhere)
                profileFile(selectedFile, executionTime, cpuUsage);  // Profile the file

                // Show the profiling results
                std::cout << "Execution Time: " << executionTime << " ms" << std::endl;
                std::cout << "CPU Usage: " << cpuUsage * 100.0 << "%" << std::endl;
            }

            if (!lastProfiledFile.empty() && executionTime > 0) { // Ensure profiling has been run
                ImGui::Text("Profiling Results for: %s", lastProfiledFile.c_str());
                ImGui::Text("Execution Time: %.2f ms", executionTime);
                ImGui::Text("CPU Usage: %.2f%%", cpuUsage * 100.0);
            }

        }

        ImGui::End();  // End file selector window

        

        

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        // Rendering
        ImGui::EndFrame();
        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x * clear_color.w * 255.0f), (int)(clear_color.y * clear_color.w * 255.0f), (int)(clear_color.z * clear_color.w * 255.0f), (int)(clear_color.w * 255.0f));
        g_pd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
        if (g_pd3dDevice->BeginScene() >= 0)
        {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }
        HRESULT result = g_pd3dDevice->Present(nullptr, nullptr, nullptr, nullptr);
        if (result == D3DERR_DEVICELOST)
            g_DeviceLost = true;
    }

    // Cleanup
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr)
        return false;

    // Create the D3DDevice
    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN; // Need to use an explicit format with alpha if needing per-pixel alpha composition.
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
    //g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
    if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
        return false;

    return true;
}

void CleanupDeviceD3D()
{
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
    if (g_pD3D) { g_pD3D->Release(); g_pD3D = nullptr; }
}

void ResetDevice()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
    if (hr == D3DERR_INVALIDCALL)
        IM_ASSERT(0);
    ImGui_ImplDX9_CreateDeviceObjects();
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

//Function to get CPU usage
double getCpuUsage(FILETIME& prevSysKernel, FILETIME& prevSysUser, FILETIME& prevSysIdle) {
    // sysIdle: Time spent in idle mode.
    // sysKernel: Time spent in kernel-mode operations (system-level tasks).
    // sysUser: Time spent in user-mode operations (application-level tasks).
    FILETIME sysIdle, sysKernel, sysUser;

    // Declare variables to hold the difference in kernel and user CPU times.
    ULARGE_INTEGER sysKernelDiff, sysUserDiff, sysIdleDiff, sysTotalDiff;

    // Fetch the current system times for idle, kernel, and user operations.
    // If the function fails, return -1.0 to indicate an error.
    if (!GetSystemTimes(&sysIdle, &sysKernel, &sysUser)) {
        return -1.0; // Error in fetching CPU usage
    }

    // Calculate the differences in system time
    // Calculate the difference in kernel-mode CPU time since the last measurement.
    sysKernelDiff.QuadPart = ((ULARGE_INTEGER&)sysKernel).QuadPart - ((ULARGE_INTEGER&)prevSysKernel).QuadPart;
    sysUserDiff.QuadPart = ((ULARGE_INTEGER&)sysUser).QuadPart - ((ULARGE_INTEGER&)prevSysUser).QuadPart;
    sysIdleDiff.QuadPart = ((ULARGE_INTEGER&)sysIdle).QuadPart - ((ULARGE_INTEGER&)prevSysIdle).QuadPart;

    // Total CPU time is the sum of kernel, user, and idle time.
    sysTotalDiff.QuadPart = sysKernelDiff.QuadPart + sysUserDiff.QuadPart + sysIdleDiff.QuadPart;

    // Update previous system times for the next calculation
    prevSysKernel = sysKernel;
    prevSysUser = sysUser;
    prevSysIdle = sysIdle;

    // If there's no total time difference, return 0.0% CPU usage (no change in usage)
    if (sysTotalDiff.QuadPart == 0) {
        return 0.0;
    }

    // Calculate active CPU time (kernel + user time)
    ULARGE_INTEGER sysActiveDiff;
    sysActiveDiff.QuadPart = sysKernelDiff.QuadPart + sysUserDiff.QuadPart;

    // Calculate CPU usage percentage
    double cpuUsage = (double)sysActiveDiff.QuadPart / (double)sysTotalDiff.QuadPart * 100.0;

    return cpuUsage; // Return the CPU usage percentage
}

// A simple function to profile
void sampleFunction(int N) {
    long long sum = 0;

    for (int j = 0; j < N; j++) {
        sum += 1;
        std::this_thread::sleep_for(std::chrono::seconds(1));

    }
}

// Function to profile the sample function and get execution time
void profileFunction(double& executionTime, double& cpuUsage) {
    // Declare variables to store the kernel-mode and user-mode CPU times before the function execution.
    // These values represent the CPU time used by the system and user processes so far.
    FILETIME prevSysKernel, prevSysUser, prevSysIdle;
    // Fetch the current system times for kernel-mode and user-mode processes.
    // The nullptr argument ignores the idle time, focusing only on active CPU usage.
    //Updates the Values in the process
    GetSystemTimes(&prevSysIdle, &prevSysKernel, &prevSysUser);


    // Start timing the execution
    auto start = std::chrono::high_resolution_clock::now();



    //extraFunction2();



    // End timing the execution
    auto end = std::chrono::high_resolution_clock::now();
    executionTime = std::chrono::duration<double, std::milli>(end - start).count();

    cpuUsage = getCpuUsage(prevSysKernel, prevSysUser, prevSysIdle);
}


// Function to profile the selected file
void profileFile(const std::string& fileName, double& executionTime, double& cpuUsage) {
    // Step 1: Record start time
    auto start = std::chrono::high_resolution_clock::now();

    // Construct the command to compile the file and generate an executable
    std::string command = "\"C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Auxiliary\\Build\\vcvars64.bat\" && cl /EHsc " + fileName + " /Fe:temp.exe";

    // Run the compilation command using system()
    system(command.c_str());

    // Step 2: Record end time (execution time)
    auto end = std::chrono::high_resolution_clock::now();
    executionTime = std::chrono::duration<double>(end - start).count();  // Execution time in seconds

    // Step 3: Run the generated executable and measure its CPU usage
    std::string exeFile = "temp.exe";

    // Convert exeFile to LPCWSTR (wide-character string)
    std::wstring wideExeFile(exeFile.begin(), exeFile.end());
    LPCWSTR exeFileW = wideExeFile.c_str();

    // Start the process (we're interested in the PID and CPU usage of temp.exe)
    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi;
    if (!CreateProcess(exeFileW, NULL, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
        std::cerr << "Error: Unable to start the process." << std::endl;
        return;
    }

    // Wait for the process to finish
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Step 4: Get the CPU times for the running process
    FILETIME creationTime, exitTime, kernelTime, userTime;
    if (GetProcessTimes(pi.hProcess, &creationTime, &exitTime, &kernelTime, &userTime)) {
        // Calculate the total CPU time for the process
        ULARGE_INTEGER kernelTimeU, userTimeU;
        kernelTimeU.LowPart = kernelTime.dwLowDateTime;
        kernelTimeU.HighPart = kernelTime.dwHighDateTime;
        userTimeU.LowPart = userTime.dwLowDateTime;
        userTimeU.HighPart = userTime.dwHighDateTime;

        // Total CPU time used by the process
        ULONGLONG totalCpuTime = kernelTimeU.QuadPart + userTimeU.QuadPart;

        // Step 5: Calculate CPU usage (based on the total CPU time used by the process)
        // Convert execution time to 100-nanosecond intervals for comparison
        double execTimeIn100ns = executionTime * 10000000;  // 1 second = 10,000,000 100-nanosecond intervals

        // Calculate CPU usage as the percentage of CPU time used by the process
        cpuUsage = (double)totalCpuTime / execTimeIn100ns * 100.0;

        // Adjust CPU usage based on the number of cores (scale to 100% per core)
        int numberOfCores = 8;  // Adjust this if you have a different number of cores
        cpuUsage = (cpuUsage / 100.0) * numberOfCores;  // Scale to 8 cores (or your system's number of cores)

        // Ensure CPU usage doesn't exceed 100% per core
        if (cpuUsage > 100.0 * numberOfCores) {
            cpuUsage = 100.0 * numberOfCores;  // Cap it to the maximum usage (100% per core)
        }
    }
    else {
        std::cerr << "Error: Unable to retrieve CPU times for the process." << std::endl;
    }

    // Clean up the process
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    std::string fileEXEFile = "temp.exe";

    // Step 6: Clean up the temporary files (executable and object file)
    if (std::filesystem::exists(fileEXEFile)) {
        std::filesystem::remove(fileEXEFile);
        std::cout << "Deleted " << fileEXEFile << std::endl;

    }

    std::string objFile = fileName.substr(0, fileName.find_last_of('.')) + ".obj";
    if (std::filesystem::exists(objFile)) {
        std::filesystem::remove(objFile);
        std::cout << "Deleted " << objFile << std::endl;
    }
}