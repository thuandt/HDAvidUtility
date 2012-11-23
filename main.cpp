#include <QtSingleApplication>
#include "HDAvidUtility.h"

int main(int argc, char *argv[])
{
    QtSingleApplication a(argc, argv);

    a.setOrganizationName("HD Vietnam Ltd,. Co.");
    a.setOrganizationDomain("http://hdvietnam.com.vn");
    a.setApplicationName("HD Avid Utility");
    a.setApplicationVersion("0.0.1");

    if(a.isRunning())
        return 0;

    HDAvidUtility w;
    w.setFixedSize(w.size());
    w.show();

    return a.exec();
}
