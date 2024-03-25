#include "logging.h"

Log Log::log{};

void Log::generateLogfilePath(char* buffer, const struct tm& time) { strftime(buffer, 16, "/%Y-%m-%d.log", &time); }

void Log::pruneLogfiles()
{
    struct LogFile
    {
        char path[16];
        size_t size;

        bool operator<(const LogFile& other) { return strcmp(this->path, other.path) < 0;}
    };
    
    std::vector<LogFile> logFiles;
    logFiles.reserve(32);
    size_t logfileSizeSum{0};
    File root = LittleFS.open("/");
    size_t logfilesCount{0};
    while(true)
    {
        File file = root.openNextFile();
        if (!file)
            break;
        const char* filePath{file.path()};
        if (strstr(filePath, ".log"))
        {
            LogFile& logFile = logFiles.emplace_back();
            strncpy(logFile.path, filePath, sizeof(logFile.path));
            logFile.size = file.size();
            logfileSizeSum += logFile.size;
            logfilesCount++;
        }
        file.close();
    }
    root.close();
    if (logfilesCount < 1 || logfileSizeSum <= maxSize * 1024)
        return;
    std::sort(std::begin(logFiles), std::end(logFiles));
    for (const auto& logFile : logFiles)
    { 
        if (!LittleFS.remove(logFile.path))
        {
            this->error("Could not remove logfile '", logFile.path, "'.");
        }
        else
        {
            this->info("Removed old logfile '", logFile.path, "'.");
            logfileSizeSum -= logFile.size;
        }
        if (logfileSizeSum <= maxSize * 1024)
        {
            this->info("Logfiles pruned to ", logfileSizeSum / 1024, "KiB");
            break;
        }
    }
    
}

void Log::openLogfile(const struct tm& time)
{
    char logfilePath[16];
    generateLogfilePath(logfilePath, time);
    if (LittleFS.exists(logfilePath))
    {
        this->logfile = LittleFS.open(logfilePath, "a");
    }
    else
    {
        this->logfile = LittleFS.open(logfilePath, "w", true);
    }
    this->logfileTime = time;
    if (!this->logfile)
    {
        this->logfileEnabled = false;
        this->error("Unable to open/create logfile! Log will disable logging to file.");
    }
}

void Log::manageLogfile(struct tm& time)
{
    if (!this->logfileEnabled)
        return;
    if ((this->logfileTime.tm_mday != time.tm_mday) || (this->logfileTime.tm_mon != time.tm_mon) || (this->logfileTime.tm_year != time.tm_year) ||
        (!this->logfile))
    {
        if (this->logfile)
            this->logfile.close();
        openLogfile(time);
        if (!this->logfile)
            return;
        pruneLogfiles();
    }
}

void Log::logfilePrint()
{
    if (!this->logfileEnabled)
        return;
    this->logfile.println(buffer);
}

void Log::close()
{
    this->logfile.close();
    this->logfileEnabled = false;
}
