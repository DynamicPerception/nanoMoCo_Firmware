/*
 
 Generalized Menu Library
 
 OpenMoco MoCoBus Core Libraries 
 
 See www.dynamicperception.com for more information
 
 (c) 2008-2012 C.A. Church / Dynamic Perception LLC
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
 
 */

#ifndef	OM_MENUMGR_H
#define OM_MENUMGR_H

#include "Arduino.h"
#include <inttypes.h>
#include <avr/pgmspace.h>
#include <stdlib.h>

#ifndef OM_ANALOG_STARTDIG
    #define OM_ANALOG_STARTDIG 14   
#endif

#ifndef OM_MENU_ROWS
    #define OM_MENU_ROWS    2
#endif

#ifndef OM_MENU_COLS
    #define OM_MENU_COLS    16
#endif

#ifndef OM_MENU_LBLLEN
    #define OM_MENU_LBLLEN OM_MENU_COLS
#endif

#ifndef OM_MENU_MAXDEPTH
    #define OM_MENU_MAXDEPTH    3
#endif

#ifndef OM_MENU_DEBOUNCE
    #define OM_MENU_DEBOUNCE 80
#endif

#ifndef OM_MENU_CURSOR
    #define OM_MENU_CURSOR ">"
#endif

#ifndef OM_MENU_NOCURSOR
    #define OM_MENU_NOCURSOR " "
#endif


#define MENU_ITEM           PROGMEM OMMenuItem
#define MENU_LIST           PROGMEM OMMenuItem*
#define MENU_VALUE          PROGMEM OMMenuValue
#define MENU_SELECT_ITEM    PROGMEM OMMenuSelectListItem
#define MENU_SELECT_LIST    PROGMEM OMMenuSelectListItem*
#define MENU_SELECT         PROGMEM OMMenuSelectValue
#define MENU_SELECT_SIZE(x) sizeof(x) / sizeof(OMMenuSelectListItem*)
#define MENU_SIZE(x)        sizeof(x) / sizeof(OMMenuItem*)
#define MENU_TARGET(x)      reinterpret_cast<void*>(x)


/** Select-Type Item
 
 Item to be added to a list for OMSelectValue
 */
struct OMMenuSelectListItem {
    uint8_t   value;
    prog_char label[OM_MENU_LBLLEN];
};



/** Select-Type Target Value
 */

struct OMMenuSelectValue {
        /** Pointer to target variable */
    uint8_t* targetValue;
    uint8_t listCount;
        /** Void Pointer to list of select items (OMMenuSelectListItem**) */
    void* list;
};


/** Menu Value */
struct OMMenuValue {
    uint8_t type;
    long    max;
    long    min;
    void*   targetValue;
    
};



/** Menu Item Type
 
  Defines a menu item. Stored in PROGMEM
 
 label is the label to be displayed for the item. 
 
 type can be one of:
 
  ITEM_MENU, ITEM_VALUE, ITEM_ACTION
 
 If type is ITEM_MENU, then targetCount will be the 
 count of menu items in the menu, and target will be
 a pointer to an array of OMMenuItems
 
 If type is ITEM_ACTION, then targetCount should be zero
 and target should resolve to a function pointer with the prototype
 void(*)(void)
 
 If type is ITEM_VALUE, then targetCount should be zero, and 
 target should be a pointer to an OMMenuValue.  
 */

struct OMMenuItem {
    prog_char     label[OM_MENU_COLS];
    uint8_t       type;
    uint8_t       targetCount;
    void*         target;
};


enum { ITEM_MENU, ITEM_VALUE, ITEM_ACTION };
enum { MENU_ANALOG, MENU_DIGITAL };
enum { BUTTON_NONE, BUTTON_FORWARD, BUTTON_BACK, BUTTON_INCREASE, BUTTON_DECREASE, BUTTON_SELECT };
enum { CHANGE_DISPLAY, CHANGE_UP, CHANGE_DOWN, CHANGE_SAVE, CHANGE_ABORT };
enum { TYPE_BYTE, TYPE_INT, TYPE_UINT, TYPE_LONG, TYPE_ULONG, TYPE_FLOAT, TYPE_FLOAT_10, TYPE_FLOAT_100, TYPE_FLOAT_1000, TYPE_SELECT };
enum { MODE_INCREMENT, MODE_DECREMENT, MODE_NOOP };


/** Menu Manager Class
 
 */

class OMMenuMgr {
    
    
public:
    
    OMMenuMgr(OMMenuItem* c_first, uint8_t c_type = MENU_ANALOG);
    
    void setAnalogButtonPin(uint8_t p_pin, int p_values[5][2], int p_thresh);
    void setDigitalButtonPins(int p_pins[5][2]);
    
    uint8_t checkInput();
    
    void enable(bool p_en);
    bool enable();
    
    void setDrawHandler(void(*p_func)(char*, int, int, int));
    void setSeekHandler(void(*p_func)(int, int));
    void setExitHandler(void(*p_func)());
    
private:
    
    typedef void(*f_valueHandler)();
    typedef void(*f_drawHandler)(char*, int, int, int);
    typedef void(*f_seekHandler)(int, int);
     
    uint8_t        m_inputType;
    uint8_t        m_anaPin;
    int            m_anaThresh;
    bool           m_enable;
    bool           m_inEdit;
    bool           m_menuActive;
    OMMenuItem*    m_curSel;
    OMMenuItem*    m_curParent;
    OMMenuItem*    m_rootItem;
    OMMenuItem*    m_hist[OM_MENU_MAXDEPTH];
    int            m_butVals[5][2];
    uint8_t        m_curTarget;
    f_drawHandler  m_draw;
    f_seekHandler  m_seek;
    f_valueHandler m_exit;
    char           m_dispBuf[OM_MENU_COLS];
    
    uint8_t       m_temp;
    long          m_tempL;
    int           m_tempI;
    float         m_tempF;
        
       
    int         _checkAnalog();
    int         _checkDigital();
    void        _handleButton(uint8_t p_key);
    void        _activate(OMMenuItem* p_item); 
    void        _edit(OMMenuItem* p_item, uint8_t p_type); 
    void        _displayList(OMMenuItem* p_item, uint8_t p_target = 0);
    void        _displayEdit(OMMenuItem* p_item);
    void        _menuNav(uint8_t p_mode);
    void        _pushHist(OMMenuItem* p_item);
    OMMenuItem* _popHist();
    void        _display(char* p_str, int p_row, int p_col, int p_count);
    void        _seek(int p_row, int p_col);
    void        _displayVoidNum(void* p_ptr, uint8_t p_type, int p_row, int p_col);
    void        _modifyTemp(uint8_t p_type, uint8_t p_mode, long p_min, long p_max);
    void        _exitMenu();
    void        _modifySel(OMMenuValue* p_value, uint8_t p_mode);
    void        _displaySelVal(OMMenuSelectListItem** p_list, uint8_t p_idx);
    
    
    
    
};

#endif //OM_MENUMGR_H