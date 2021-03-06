#ifndef CDOHELPERS_H
#define CDOHELPERS_H

#include <cloudeo_sdk.h>

#include <string>

class CDOHelpers
{
private:
    CDOHelpers();

public:

    static void stdString2CdoString(CDOString* target, const std::string& src);
    static CDOString stdString2CdoString(const std::string& src);

};

#endif // CDOHELPERS_H
