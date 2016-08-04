#ifndef DOWNLOADER_PROGRESS_H
#define DOWNLOADER_PROGRESS_H

struct DownloaderProgress
{
    int seconds;
    int kbps;
    int percent;
};

#endif // DOWNLOADER_PROGRESS_H
