#pragma once

#define PRIVATE_MESSAGE_BASE (0x6100)

//
// WM_CONTROL_MESSAGE
//
// wParam - LOWORD : Control Code (CODE_xxx)
//          HIWORD : must be zero
//
// lParam - A pointer to a structure that contains control code specific data.
//          Its format depends on the value of the LOWORD(wParam) parameter.
//          For more information, refer to the documentation for each application.
//
#define WM_CONTROL_MESSAGE  (PRIVATE_MESSAGE_BASE+0)

#define WM_NOTIFY_MESSAGE   (PRIVATE_MESSAGE_BASE+1)

// Control/Notify code
// WPARAM LOWORD(f)
#define CTRL_PATH_SELECTED       (0x0001)  // -,N
#define CTRL_ITEM_SELECTED       (0x0002)  // -,N
#define CTRL_DIRECTORY_CHANGED   (0x0003)  // -,N
#define CTRL_SELECT_ITEM         (0x0004)  // C,-
#define CTRL_SELECT_VOLUME       (0x0005)  // C,-
#define CTRL_SET_DIRECTORY       (0x0006)  // C,-
#define CTRL_INIT_LAYOUT         (0x0007)

typedef struct _SELECT_ITEM
{
	UINT mask;          // Reserved
	UINT Flags;         // Reserved
	PWSTR pszPath;
	PWSTR pszName;
	PWSTR pszCurDir;
	UINT ViewType;      // Depends an application.
} SELECT_ITEM;

//
// WM_QUERY_CMDSTATE
//
// wParam - LOWORD : Command ID
//          HIWORD : 0
// lParam - Pointer to UINT that receives the state (UPDUI_xxx) flag.
//
#define WM_QUERY_CMDSTATE   (PRIVATE_MESSAGE_BASE+2)

enum {
    UPDUI_ENABLED  = 0x00000000,
    UPDUI_DISABLED = 0x00000100,
    UPDUI_CHECKED  = 0x00000200,
    UPDUI_CHECKED2 = 0x00000400,
    UPDUI_RADIO	   = 0x00000800,
};
