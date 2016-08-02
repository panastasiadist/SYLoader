#ifndef HELPER_H
#define HELPER_H

#include "download.h"


class Helper
{
public:
    Helper();

    static Download decorateDownload(Download download);
};

#endif // HELPER_H
