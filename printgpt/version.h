
#define PGPT_VERSION_FILEVERSION 1,2,0,0
#define PGPT_VERSION_DISPLAY     "1.2"
#define PGPT_DATE_AUTHOR         "2019 Nobody"
#ifdef _DEBUG
#ifdef _WIN64
#define PGPT_VERSION_DISPLAY_FULL PGPT_VERSION_DISPLAY " (debug, x64)"
#else
#define PGPT_VERSION_DISPLAY_FULL PGPT_VERSION_DISPLAY " (debug)"
#endif
#else
#ifdef _WIN64
#define PGPT_VERSION_DISPLAY_FULL PGPT_VERSION_DISPLAY " (x64)"
#else
#define PGPT_VERSION_DISPLAY_FULL PGPT_VERSION_DISPLAY
#endif
#endif

#ifdef _WIN64
#define PGPT_FILE_NAME "printgpt64.exe"
#else
#define PGPT_FILE_NAME "printgpt.exe"
#endif
