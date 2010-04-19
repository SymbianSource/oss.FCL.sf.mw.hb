TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += .

SOURCES += main.cpp

symbian {
    TARGET.CAPABILITY = CAP_APPLICATION
    TARGET.EPOCALLOWDLLDATA = 1
    
    LIBS += -lcone
    LIBS += -lavkon
    LIBS += -leikcore

    myrssrules = \
     "hidden = KAppIsHidden;" \
     "launch = KAppLaunchInBackground;"
    
    RSS_RULES += myrssrules
}

hbAddLibrary(hbcore/HbCore)
hbAddLibrary(hbwidgets/HbWidgets)
hbAddLibrary(hbutils/HbUtils)
