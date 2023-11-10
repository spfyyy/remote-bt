#ifndef REMOTE_BT_H
#define REMOTE_BT_H

#ifdef WINDOWS
#define DllExport __declspec( dllexport )
#else
#define DllExport 
#endif

DllExport int remote_bt_init(void);
DllExport void remote_bt_shutdown(void);
DllExport int remote_bt_download(char *link);

#endif
