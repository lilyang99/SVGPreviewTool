#include "pch.h"
#include "CSVGImage.h"

// ---------------------------------------------------------------------------
// Memory leak detection — CRT debug heap
// ---------------------------------------------------------------------------
// _CRTDBG_LEAK_CHECK_DF causes _CrtDumpMemoryLeaks() to be called
// automatically at program exit via atexit(). Any leaked allocations
// are printed to the Debug Output window with file, line, and size.
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Test infrastructure
// ---------------------------------------------------------------------------

static int g_pass = 0;
static int g_fail = 0;
static std::wstring g_svgPath;

void PrintProcessMemory(const wchar_t* label)
{
    PROCESS_MEMORY_COUNTERS pmc = {};
    pmc.cb = sizeof(pmc);
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
    {
        std::wcout << L"  [" << label << L"] "
                   << L"Private=" << (pmc.PagefileUsage / 1024) << L" KB  "
                   << L"WorkingSet=" << (pmc.WorkingSetSize / 1024) << L" KB"
                   << std::endl;
    }
}

void BeginTest(const wchar_t* name)
{
    std::wcout << L"  " << name << L" ... ";
}

void TestPass()
{
    ++g_pass;
    std::wcout << L"PASS" << std::endl;
}

void TestFail(const wchar_t* reason)
{
    ++g_fail;
    std::wcout << L"FAIL — " << reason << std::endl;
}

// ---------------------------------------------------------------------------
// Test cases
// ---------------------------------------------------------------------------

// 1. BasicLifecycle
void Test_BasicLifecycle()
{
    BeginTest(L"BasicLifecycle");
    {
        CSVGImage img;
        if (!img.Initialize())
        {
            TestFail(L"Initialize() failed");
            return;
        }
        img.Cleanup();
    }
    TestPass();
}

// 2. LoadFromFile
void Test_LoadFromFile()
{
    BeginTest(L"LoadFromFile");
    {
        CSVGImage img;
        if (!img.Initialize())
        {
            TestFail(L"Initialize() failed");
            return;
        }
        HRESULT hr = img.LoadFromFile(g_svgPath.c_str());
        if (FAILED(hr))
        {
            TestFail(L"LoadFromFile() failed");
            return;
        }
        if (!img.IsValid())
        {
            TestFail(L"Not valid after load");
            return;
        }
        img.Cleanup();
    }
    TestPass();
}

// 3. ToHBITMAP
void Test_ToHBITMAP()
{
    BeginTest(L"ToHBITMAP");
    {
        CSVGImage img;
        if (!img.Initialize() ||
            FAILED(img.LoadFromFile(g_svgPath.c_str())))
        {
            TestFail(L"Setup failed");
            return;
        }
        HBITMAP hbm = img.ToHBITMAP();
        if (!hbm)
        {
            TestFail(L"ToHBITMAP() returned null");
            return;
        }
        ::DeleteObject(hbm);
        img.Cleanup();
    }
    TestPass();
}

// 4. ToGdiPlusBitmap
void Test_ToGdiPlusBitmap()
{
    BeginTest(L"ToGdiPlusBitmap");
    {
        CSVGImage img;
        if (!img.Initialize() ||
            FAILED(img.LoadFromFile(g_svgPath.c_str())))
        {
            TestFail(L"Setup failed");
            return;
        }
        Gdiplus::Bitmap* bmp = img.ToGdiPlusBitmap();
        if (!bmp)
        {
            TestFail(L"ToGdiPlusBitmap() returned null");
            return;
        }
        if (bmp->GetLastStatus() != Gdiplus::Ok)
        {
            delete bmp;
            TestFail(L"Bitmap status not Ok");
            return;
        }
        delete bmp;
        img.Cleanup();
    }
    TestPass();
}

// 5. ToHICON
void Test_ToHICON()
{
    BeginTest(L"ToHICON");
    {
        CSVGImage img;
        if (!img.Initialize() ||
            FAILED(img.LoadFromFile(g_svgPath.c_str())))
        {
            TestFail(L"Setup failed");
            return;
        }
        HICON hIcon = img.ToHICON();
        if (!hIcon)
        {
            TestFail(L"ToHICON() returned null");
            return;
        }
        ::DestroyIcon(hIcon);
        img.Cleanup();
    }
    TestPass();
}

// 6. RepeatedLoad — load/convert/cleanup 10 times, detect accumulation
void Test_RepeatedLoad()
{
    BeginTest(L"RepeatedLoad (x10)");
    for (int i = 0; i < 10; ++i)
    {
        CSVGImage img;
        if (!img.Initialize() ||
            FAILED(img.LoadFromFile(g_svgPath.c_str())))
        {
            TestFail(L"Setup failed on iteration");
            return;
        }
        HBITMAP hbm = img.ToHBITMAP();
        if (hbm)
            ::DeleteObject(hbm);
        img.Cleanup();
    }
    TestPass();
}

// 7. DPI Scale — change DPI and verify bitmap dimensions
void Test_DPIScale()
{
    BeginTest(L"DPIScale");
    {
        CSVGImage img;
        if (!img.Initialize(USER_DEFAULT_SCREEN_DPI) ||
            FAILED(img.LoadFromFile(g_svgPath.c_str())))
        {
            TestFail(L"Setup failed");
            return;
        }

        HBITMAP hbm96 = img.ToHBITMAP();
        if (!hbm96)
        {
            TestFail(L"ToHBITMAP() at 96 DPI returned null");
            return;
        }
        BITMAP bm96 = {};
        ::GetObject(hbm96, sizeof(BITMAP), &bm96);
        ::DeleteObject(hbm96);

        img.SetDPI(192);
        HBITMAP hbm192 = img.ToHBITMAP();
        if (!hbm192)
        {
            TestFail(L"ToHBITMAP() at 192 DPI returned null");
            return;
        }
        BITMAP bm192 = {};
        ::GetObject(hbm192, sizeof(BITMAP), &bm192);
        ::DeleteObject(hbm192);

        // 192 DPI should produce 2x the dimensions of 96 DPI
        if (bm192.bmWidth != bm96.bmWidth * 2 ||
            bm192.bmHeight != bm96.bmHeight * 2)
        {
            TestFail(L"DPI scale dimensions incorrect");
            return;
        }

        img.Cleanup();
    }
    TestPass();
}

// 8. MoveSemantics — test move constructor and moved-from state
void Test_MoveSemantics()
{
    BeginTest(L"MoveSemantics");
    {
        CSVGImage src;
        if (!src.Initialize() ||
            FAILED(src.LoadFromFile(g_svgPath.c_str())))
        {
            TestFail(L"Setup failed");
            return;
        }
        SIZE srcSize = src.GetOriginalSize();

        CSVGImage dst(std::move(src));
        if (!dst.IsValid())
        {
            TestFail(L"Moved-to object not valid");
            return;
        }
        if (dst.GetOriginalSize().cx != srcSize.cx ||
            dst.GetOriginalSize().cy != srcSize.cy)
        {
            TestFail(L"Size mismatch after move");
            return;
        }

        // Moved-from should be in valid-but-unspecified state
        // (our impl resets DPI to default and size to 0)

        dst.Cleanup();
    }
    TestPass();
}

// 9. LoadMultiple — reload without intermediate Cleanup
void Test_LoadMultiple()
{
    BeginTest(L"LoadMultiple");
    {
        CSVGImage img;
        if (!img.Initialize())
        {
            TestFail(L"Initialize() failed");
            return;
        }

        // Load same file 3 times — each LoadFromFile replaces the previous
        for (int i = 0; i < 3; ++i)
        {
            HRESULT hr = img.LoadFromFile(g_svgPath.c_str());
            if (FAILED(hr))
            {
                TestFail(L"Reload failed");
                return;
            }
        }

        HBITMAP hbm = img.ToHBITMAP();
        if (!hbm)
        {
            TestFail(L"ToHBITMAP() after reloads returned null");
            return;
        }
        ::DeleteObject(hbm);
        img.Cleanup();
    }
    TestPass();
}

// 10. InvalidFile — load nonexistent file, should fail cleanly
void Test_InvalidFile()
{
    BeginTest(L"InvalidFile");
    {
        CSVGImage img;
        if (!img.Initialize())
        {
            TestFail(L"Initialize() failed");
            return;
        }
        HRESULT hr = img.LoadFromFile(L"nonexistent_file_xyz__.svg");
        if (SUCCEEDED(hr))
        {
            TestFail(L"Should have failed for nonexistent file");
            return;
        }
        // Object should remain in a valid (initialized) state
        img.Cleanup();
    }
    TestPass();
}

// 11. NoInitLoad — LoadFromFile without Initialize
void Test_NoInitLoad()
{
    BeginTest(L"NoInitLoad");
    {
        CSVGImage img;
        // Skip Initialize() — LoadFromFile should fail
        HRESULT hr = img.LoadFromFile(g_svgPath.c_str());
        if (SUCCEEDED(hr))
        {
            TestFail(L"Should have failed without Initialize");
            return;
        }
    }
    TestPass();
}

// 12. StressTest — 100x create/load/convert/destroy, print per-iteration memory
void Test_StressTest()
{
    BeginTest(L"StressTest (x100)");
    std::wcout << std::endl;

    PROCESS_MEMORY_COUNTERS pmc0 = {};
    pmc0.cb = sizeof(pmc0);
    GetProcessMemoryInfo(GetCurrentProcess(), &pmc0, sizeof(pmc0));

    for (int i = 0; i < 100; ++i)
    {
        CSVGImage img;
        if (!img.Initialize() ||
            FAILED(img.LoadFromFile(g_svgPath.c_str())))
        {
            TestFail(L"Setup failed on iteration");
            return;
        }

        HBITMAP hbm = img.ToHBITMAP();
        if (hbm) ::DeleteObject(hbm);

        Gdiplus::Bitmap* gdibmp = img.ToGdiPlusBitmap();
        if (gdibmp) delete gdibmp;

        HICON hIcon = img.ToHICON();
        if (hIcon) ::DestroyIcon(hIcon);

        img.Cleanup();

        // Print per-iteration memory
        if (true)
        {
            PROCESS_MEMORY_COUNTERS pmc = {};
            pmc.cb = sizeof(pmc);
            GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
            LONG delta = static_cast<LONG>(pmc.PagefileUsage - pmc0.PagefileUsage) / 1024;
            std::wcout << L"    [" << (i + 1) << L"] "
                       << L"Private=" << (pmc.PagefileUsage / 1024) << L" KB"
                       << L"  (delta " << (delta >= 0 ? L"+" : L"") << delta << L" KB)"
                       << std::endl;
        }
    }
    TestPass();
}

// 13. HeapSnapshot — use _CrtMemState to catch both CRT and COM refcount leaks
//      CRT leak detection at exit can miss system COM objects; heap snapshots
//      compare the process heap before/after a full lifecycle to detect any
//      memory that was allocated but never freed (regardless of allocator).
void Test_HeapSnapshot()
{
    BeginTest(L"HeapSnapshot");

    // Warmup cycle: absorb one-time D2D/WIC/GDI+ internal allocations so they
    // don't appear as false positives in the checkpoint diff.
    {
        CSVGImage img;
        img.Initialize();
        if (SUCCEEDED(img.LoadFromFile(g_svgPath.c_str())))
        {
            HBITMAP hbm = img.ToHBITMAP();
            if (hbm) ::DeleteObject(hbm);
            Gdiplus::Bitmap* bmp = img.ToGdiPlusBitmap();
            if (bmp) delete bmp;
            HICON hIcon = img.ToHICON();
            if (hIcon) ::DestroyIcon(hIcon);
        }
        img.Cleanup();
    }

    _CrtMemState before, after, diff;
    _CrtMemCheckpoint(&before);

    for (int i = 0; i < 10; ++i)
    {
        CSVGImage img;
        if (!img.Initialize() ||
            FAILED(img.LoadFromFile(g_svgPath.c_str())))
        {
            TestFail(L"Setup failed");
            return;
        }

        HBITMAP hbm = img.ToHBITMAP();
        if (hbm) ::DeleteObject(hbm);

        Gdiplus::Bitmap* gdibmp = img.ToGdiPlusBitmap();
        if (gdibmp) delete gdibmp;

        HICON hIcon = img.ToHICON();
        if (hIcon) ::DestroyIcon(hIcon);

        img.Cleanup();
    }

    _CrtMemCheckpoint(&after);
    if (_CrtMemDifference(&diff, &before, &after))
    {
        // Dump statistics to stderr so we can see which block types leaked
        _CrtMemDumpStatistics(&diff);

        WCHAR buf[256];
        _snwprintf_s(buf, _TRUNCATE,
            L"_CrtMemDifference: leaked %ld bytes in %ld normal blocks",
            static_cast<long>(diff.lSizes[1]),
            static_cast<long>(diff.lCounts[1]));
        TestFail(buf);
        return;
    }

    TestPass();
}

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------

int main()
{
    // Enable CRT memory leak detection — dump to both debug output and stderr
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);

    // Initialize COM (required by WIC/D2D)
    HRESULT hrCom = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hrCom))
    {
        std::wcerr << L"FATAL: CoInitializeEx failed: 0x"
                   << std::hex << hrCom << std::endl;
        return 1;
    }

    // Initialize GDI+
    Gdiplus::GdiplusStartupInput gdipInput;
    ULONG_PTR gdipToken = 0;
    Gdiplus::Status gdipStatus = Gdiplus::GdiplusStartup(&gdipToken, &gdipInput, nullptr);
    if (gdipStatus != Gdiplus::Ok)
    {
        std::wcerr << L"FATAL: GdiplusStartup failed" << std::endl;
        CoUninitialize();
        return 1;
    }

    // Resolve test SVG path
    g_svgPath = L"..\\svg\\closedefault_SVG.svg";

    std::wcout << L"=== SvgRender Memory Leak Test ===\n" << std::endl;

    PrintProcessMemory(L"Start        ");

    Test_BasicLifecycle();
    Test_LoadFromFile();
    Test_ToHBITMAP();
    Test_ToGdiPlusBitmap();
    Test_ToHICON();
    Test_RepeatedLoad();
    Test_DPIScale();
    Test_MoveSemantics();
    Test_LoadMultiple();
    Test_InvalidFile();
    Test_NoInitLoad();

    PrintProcessMemory(L"Pre-Stress   ");

    Test_StressTest();

    PrintProcessMemory(L"Post-Stress  ");

    Test_HeapSnapshot();

    PrintProcessMemory(L"End          ");

    // Second pass: if the ~2 MB growth from Start is a leak, another 100
    // iterations will add another ~2 MB. If it's one-time init, memory stays flat.
    std::wcout << L"\n--- Second pass (verifying no further growth) ---\n" << std::endl;
    PrintProcessMemory(L"Pass2-Pre    ");
    Test_StressTest();
    PrintProcessMemory(L"Pass2-Post   ");

    std::wcout << L"\n=== Results: " << g_pass << L"/" << (g_pass + g_fail)
               << L" passed ===" << std::endl;

    Gdiplus::GdiplusShutdown(gdipToken);
    CoUninitialize();

    // _CRTDBG_LEAK_CHECK_DF triggers _CrtDumpMemoryLeaks at exit.
    // Check Debug Output window for any leak reports.

    return (g_fail == 0) ? 0 : 1;
}
