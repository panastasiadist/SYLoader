#ifndef UPDATE_INFO_H
#define UPDATE_INFO_H

#include <QString>

struct UpdateInfo
{
    int ProgramMajor;
    int ProgramMinor;
    int ProgramPatch;
    bool ProgramUpdate;

    int YdlMajor;
    int YdlMinor;
    int YdlPatch;
    bool YdlUpdate;
    QString YdlPackageUrl;
};

#endif // UPDATE_INFO_H
