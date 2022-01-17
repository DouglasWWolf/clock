//=========================================================================================================
// buttons.cpp - Implements the interfaces to our buttons
//=========================================================================================================
#include "globals.h"



//=========================================================================================================
// handler() - Specifies what happens when the provisioning button is pressed or released
//=========================================================================================================
void CProvButton::handler(bool state, uint32_t elapsed)
{
    // Tell the display manager to display the system IP address
    if (state == 0 && elapsed < 2000) DisplayMgr.display_ip_address();

    // Keep track of how much stack space we use
    StackMgr.record_hwm(TASK_IDX_PROV_BUTTON);
}
//=========================================================================================================
