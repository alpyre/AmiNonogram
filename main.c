/******************************************************************************
 * AmiNonogram                                                                *
 ******************************************************************************/
/******************************************************************************
 This file is part of AmiNonogram.
 Copyright (C) 2022 Ibrahim Alper Sönmez

 AmiNonogram is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 AmiNonogram is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with AmiNonogram. If not, see <https://www.gnu.org/licenses/>.
******************************************************************************/

///definitions
#define PROGRAMNAME     "AmiNonogram"
#define VERSION         1
#define REVISION        0
#define VERSIONSTRING   "1.0"
#define AUTHOR          "Ibrahim Alper Sönmez"
#define COPYRIGHT       "© 2022 " AUTHOR
#define CONTACT         "amithlondestek@gmail.com"
#define DESCRIPTION     "A Nonogram game in MUI."

//define command line syntax and number of options
#define RDARGS_TEMPLATE ""
#define RDARGS_OPTIONS  0

//#define or #undef GENERATEWBMAIN to enable workbench startup
#define GENERATEWBMAIN

//missing definitions in NDK & SDK
#if !defined (__MORPHOS__)
  #define MAX(a, b) (a > b ? a : b)
  #define MIN(a, b) (a > b ? b : a)
#endif

#define WIN_CLOSE               5
#define BTN_START              10
#define BTN_QUIT               11
#define BTN_NEW_PROFILE        12
#define BTN_DEL_PROFILE        13
#define BTN_PLAY               14
#define BTN_EXIT               15
#define BTN_CREATE             16
#define BTN_DEL_PROFILE_TOGGLE 17
#define BTN_PUZZLE_EXIT        18
#define BTN_PUZZLE_NEXT        19

#define EVENT_GAMELOSE         30
#define EVENT_GAMEWIN          31

#define MEN_GAME               50
#define MEN_ABOUT              51
#define MEN_ABOUTMUI           52
#define MEN_QUIT               53
///
///includes
//standard headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

//Amiga headers
#include <exec/exec.h>
#include <dos/dos.h>
#include <dos/dostags.h>
#include <dos/dosextens.h>
#include <dos/datetime.h>
#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>
#include <graphics/layers.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#if defined(__amigaos4__)
  #include <intuition/iobsolete.h>
#endif

#include <workbench/workbench.h>
#include <workbench/startup.h>
#include <workbench/icon.h>
#include <datatypes/pictureclass.h>
#include <libraries/asl.h>
#include <libraries/commodities.h>
#include <libraries/gadtools.h>
#include <libraries/iffparse.h>
#include <libraries/locale.h>
#include <rexx/rxslib.h>
#include <rexx/storage.h>
#include <rexx/errors.h>
#include <utility/hooks.h>

//Amiga protos
#include <clib/alib_protos.h>
#include <proto/asl.h>
#include <proto/commodities.h>
#include <proto/datatypes.h>
#include <proto/diskfont.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/gadtools.h>
#include <proto/graphics.h>
#include <proto/icon.h>
#include <proto/iffparse.h>
#include <proto/intuition.h>
#include <proto/layers.h>
#include <proto/locale.h>
#include <proto/rexxsyslib.h>
#include <proto/utility.h>
#include <proto/wb.h>

/* MUI headers */
#include <libraries/mui.h>
#include <proto/muimaster.h>
#include <workbench/workbench.h>

//SDI headers
#include <SDI_compiler.h>
#include <SDI_hook.h>
#include <SDI_interrupt.h>
#include <SDI_lib.h>
#include <SDI_misc.h>
#include <SDI_stdarg.h>

#include "locale.h"
#include "utility.h"
#include "pack.h"
#include "profile.h"
#include "hintdisplay.h"
#include "nonogram.h"
#include "game.h"
#include "square.h"
#include "livesdisplay.h"
#include "sizeselector.h"
#include "integerdisplay.h"
///
///structures
/***********************************************
* Global configuration struct for this program *
************************************************/
struct Config
{
  struct RDArgs *RDArgs;

  //command line options
  #if RDARGS_OPTIONS
  LONG Options[RDARGS_OPTIONS];
  #endif

  //<YOUR GLOBAL DATA HERE>

};

//<YOUR STRUCTS HERE>

///
///globals
/***********************************************
* Version string for this program              *
************************************************/
#if defined(__SASC)
const UBYTE VersionTag[] = "$VER: " PROGRAMNAME " " VERSIONSTRING " "  __AMIGADATE__ "\n\0";
#elif defined(_DCC)
const UBYTE VersionTag[] = "$VER: " PROGRAMNAME " " VERSIONSTRING " (" __COMMODORE_DATE__ ")\n\0";
#elif defined(__GNUC__)
__attribute__((section(".text"))) volatile static const UBYTE VersionTag[] = "$VER: " PROGRAMNAME " " VERSIONSTRING " (" __DATE__ ")\n\0";
#else
const UBYTE VersionTag[] = "$VER: " PROGRAMNAME " " VERSIONSTRING " (" __DATE__ ")\n\0";
#endif


APTR memoryPool = NULL;
static struct Catalog* catalog;

static ULONG logoP0[102] = {0x0, 0xA8150154, 0x540A80A, 0xA0015401, 0x55000150, 0x1400160,
                            0x0, 0x5C1B02BA, 0x2E0D815, 0xD002BA02, 0xAAA000A8, 0xA002E0,
                            0x0, 0xBE0B05FD, 0x5F0582F, 0xE804FF01, 0x7FD00158, 0x2F005E0,
                            0x0, 0x5F170BFE, 0x82F8B85F, 0xF40BFF82, 0xFFE802FC, 0x1780AE0,
                            0x0, 0xBF0B178F, 0x45F858BC, 0x7A170541, 0x71F405DC, 0x2FC17E0,
                            0x0, 0x5F972F07, 0x22FCB978, 0x392F05E2, 0xF0A802EE, 0x17C2EE0,
                            0x0, 0xBFCB1E03, 0xC5FE58F0, 0x1E1603E1, 0x705C05D6, 0x2FE5DE0,
                            0x0, 0x5FF72E02, 0xA2FFB970, 0x152E0002, 0xEAB80BAF, 0x17F3AE0,
                            0xC1C38E, 0xBFEB3C01, 0xE5FF59E0, 0xF1C0541, 0x55781717, 0x2FF5DE0,
                            0x1C1C78C, 0x5FF73A02, 0xE2FFB9D0, 0x173A02A2, 0xFFB00B0B, 0x17FB2E0,
                            0x3C3CF1C, 0xBDFB3C01, 0xA5EFD9E0, 0xD3C07E1, 0x7FC01607, 0x82F775E0,
                            0x6C3DF18, 0x5CF73A02, 0xE2E7B9D0, 0x173A0CE2, 0xF2E82FFF, 0x8173E2E0,
                            0xCC6F638, 0xBCFB1D05, 0xC5E7D8E8, 0x2E1D0061, 0x71F817FD, 0xC2F3C5E0,
                            0x1FC6E630, 0x5C7F1EAB, 0xC2E3F8F5, 0x5E1EAB62, 0xF1BC2E0A, 0xC17182E0,
                            0x3FCCCC70, 0xBC3B0F57, 0x85E1D87A, 0xBC0F5761, 0x705C5C05, 0xE2F005E0,
                            0x60CCCC60, 0x7C1F07FF, 0x3E0F83F, 0xF807FEE3, 0xF0FCBC03, 0xE1F003E0,
                            0xE1DC9CE0, 0xFC1F01FC, 0x7E0F80F, 0xE001FDE3, 0xF0FEFC03, 0xF3F007E0};
static ULONG logoP1[102] = {0x0, 0xF81F01FC, 0x7C0F80F, 0xE001FC03, 0xFF8001F8, 0x3E003E0,
                            0x0, 0x7C1F07BF, 0x3E0F83D, 0xF807FE03, 0xFFF000F8, 0x1E003E0,
                            0x0, 0xAE0B0FAD, 0x570587D, 0x6806AB01, 0x2AF80178, 0x2F004E0,
                            0x0, 0x57170955, 0x82B8B84A, 0xAC0B5582, 0xD57802FC, 0x13808E0,
                            0x0, 0xBF0B128A, 0xC5F85894, 0x56160541, 0x71F4059C, 0x2FC17E0,
                            0x0, 0x5F972F07, 0x62FCB978, 0x3B2D0562, 0xF0A802EE, 0x15C2EE0,
                            0x0, 0xBBCB1E03, 0xC5DE58F0, 0x1E1602A1, 0x705C05D6, 0x2EE59E0,
                            0x0, 0x5DF72E02, 0xA2EFB970, 0x152E0002, 0xFFF80BAF, 0x17732E0,
                            0x0, 0xBEEB3C01, 0xE5F759E0, 0xF1C0FE1, 0x7FA81717, 0x2EB4DE0,
                            0x0, 0x5D773A02, 0xE2EBB9D0, 0x173A07E2, 0xD5900B0B, 0x17592E0,
                            0x0, 0xBCBB3C01, 0xA5E5D9E0, 0xD3C02E1, 0x6BC017F7, 0x82F325E0,
                            0x0, 0x5C573B03, 0xE2E2B9D8, 0x1F3A04E2, 0xF0E82FFF, 0x817142E0,
                            0x0, 0xBCAB1F8F, 0xC5E558FC, 0x7E0F0261, 0x71F81401, 0xC2F285E0,
                            0x0, 0x5C5F0FFF, 0xC2E2F87F, 0xFE1FFF62, 0xF1BC2E0A, 0xC17102E0,
                            0x0, 0xBC2B0BFE, 0x85E1585F, 0xF40BFE61, 0x705C5C05, 0xE2F005E0,
                            0x0, 0x5C1B0575, 0x2E0D82B, 0xA8055463, 0x70DC9403, 0x61700360,
                            0x0, 0xAC1500A8, 0x560A805, 0x4000A8A2, 0xB0AAA802, 0xA2B006A0};
static struct BitMap bm_Logo = {24, 17, BMF_STANDARD, 2, 0, {logoP0, logoP1, 0, 0, 0, 0, 0, 0}};

static struct Packs* packs = NULL;
static struct Profiles* profiles = NULL;
static Profile* profile = NULL;
static STRPTR dir_profiles = "profiles";
static STRPTR dir_packs = "packs";
static GameBoardData* gbd;
static ULONG gv_lives = 3;
///
///MUI globals
struct Library *MUIMasterBase;
#if defined(__amigaos4__)
struct MUIMasterIFace *IMUIMaster;
#endif

struct MUI_CustomClass *cl_LivesDisplay = NULL;
struct MUI_CustomClass *cl_HintsDisplay = NULL;
struct MUI_CustomClass *cl_Square = NULL;
struct MUI_CustomClass *cl_Nonogram = NULL;
struct MUI_CustomClass *cl_SizeSelector = NULL;
struct MUI_CustomClass *cl_IntegerDisplay = NULL;

Object *AboutMUIWin = NULL;
Object *App, *Win;
Object *pagedGroup;
Object *btn_start, *btn_quit;
Object *lv_profiles, *btn_new_profile, *btn_del_profile;

Object *win_newprofile, *str_newprofile, *btn_create;

Object *dsp_name, *dsp_score, *lv_packs, *cg_sizeselector, *chk_randompack, *chk_randomsize;
Object *btn_play, *btn_exit;

Object *win_puzzle, *dsp_lives, *grp_nonogram, *obj_nonogram, *dsp_puzzle_name, *btn_puzzle_exit, *btn_puzzle_next;

#ifdef __GNUC__
/* Otherwise auto open will try version 37, and muimaster.library has version
   19.x for MUI 3.8 */
int __oslibversion = 0;
#endif
///
///prototypes
/***********************************************
* Function forward declarations                *
************************************************/
int            main   (int argc, char **argv);
int            wbmain (struct WBStartup *wbs);
struct Config *Init   (void);
int            Main   (struct Config *config);
void           CleanUp(struct Config *config);
Object *       buildGUI(void);
///
///init
/***********************************************
* Program initialization                       *
* - Allocates the config struct to store the   *
*   global configuration data.                 *
* - Do your other initial allocations here.    *
************************************************/
struct Config *Init()
{
  struct Config *config = (struct Config*)AllocMem(sizeof(struct Config), MEMF_CLEAR);

  if (config) {
    catalog = OpenCatalog(NULL, "AmiNonogram.catalog", TAG_END);

    #if defined(__amigaos4__)
    memoryPool = AllocSysObjectTags(ASOT_MEMPOOL, ASOPOOL_MFlags, MEMF_ANY,
                                                  ASOPOOL_Puddle, 2048,
                                                  ASOPOOL_Threshold, 2048,
                                                  ASOPOOL_Name, "AmiNonogram pool",
                                                  ASOPOOL_LockMem, FALSE,
                                                  TAG_DONE);
    #else
    memoryPool = CreatePool(MEMF_ANY, 2048, 2048);
    #endif
    if (!memoryPool) {
      CloseCatalog(catalog);
      FreeMem(config, sizeof(struct Config));
      return 0;
    }
    if (!Exists(dir_profiles)) {
      BPTR lock = CreateDir(dir_profiles);
      if (lock) {
        UnLock(lock);
      }
      else {
        printf("%s\n%s\n", GetCatalogStr(catalog, MSG_ERR_NEW_DIRECTORY, "Couldn't create new directory for user profiles on disk!"),
                           GetCatalogStr(catalog, MSG_ERR_DISK_PROTECTED, "AmiNonogram needs to be on a non-write protected disk to run."));
        CloseCatalog(catalog);
        #if defined(__amigaos4__)
        FreeSysObject(ASOT_MEMPOOL, memoryPool);
        #else
        DeletePool(memoryPool);
        #endif
        FreeMem(config, sizeof(struct Config));
        return 0;
      }
    }

  }

  return(config);
}
///
///entry
/***********************************************
 * Ground level entry point                    *
 * - Branches regarding Shell/WB call.         *
 ***********************************************/
int main(int argc, char **argv)
{
  int rc = 20;

  //argc != 0 identifies call from shell
  if (argc)
  {
    struct Config *config = Init();

    if (config)
    {
      #if RDARGS_OPTIONS
        // parse command line arguments
        if (config->RDArgs = ReadArgs(RDARGS_TEMPLATE, config->Options, NULL))
          rc = Main(config);
        else
          PrintFault(IoErr(), PROGRAMNAME);
      #else
        rc = Main(config);
      #endif

      CleanUp(config);
    }
  }
  else
    rc = wbmain((struct WBStartup *)argv);

  return(rc);
}

/***********************************************
 * Workbench main                              *
 * - This executable was called from Workbench *
 ***********************************************/
int wbmain(struct WBStartup *wbs)
{
  int rc = 20;

  #ifdef GENERATEWBMAIN
    struct Config *config = Init();

    if (config)
    {
      //<SET Config->Options[] HERE>

      rc = Main(config);

      CleanUp(config);
    }
  #endif

  return(rc);
}
///
///prepareMenu()
struct NewMenu* prepareMenu()
{
  #define MENU_SIZE 6
  struct NewMenu* menu = (struct NewMenu*) AllocPooled(memoryPool, sizeof(struct NewMenu) * MENU_SIZE);

  if (menu)
  {
    memset(menu, 0, sizeof(struct NewMenu) * MENU_SIZE);

    menu[0].nm_Type     = NM_TITLE;
    menu[0].nm_Label    = GetCatalogStr(catalog, MSG_MENU_GAME, "Game");
    menu[0].nm_UserData = (APTR)MEN_GAME;

    menu[1].nm_Type     = NM_ITEM;
    menu[1].nm_Label    = GetCatalogStr(catalog, MSG_MENU_ABOUT, "About...");
    menu[1].nm_CommKey  = GetCatalogStr(catalog, MSG_MENU_ABOUT_KEY, "?");
    menu[1].nm_UserData = (APTR)MEN_ABOUT;

    menu[2].nm_Type     = NM_ITEM;
    menu[2].nm_Label    = GetCatalogStr(catalog, MSG_MENU_ABOUT_MUI, "About MUI...");
    menu[2].nm_UserData = (APTR)MEN_ABOUTMUI;

    menu[3].nm_Type     = NM_ITEM;
    menu[3].nm_Label    = NM_BARLABEL;

    menu[4].nm_Type     = NM_ITEM;
    menu[4].nm_Label    = GetCatalogStr(catalog, MSG_MENU_QUIT, "Quit");
    menu[4].nm_CommKey  = GetCatalogStr(catalog, MSG_MENU_QUIT_KEY, "Q");
    menu[4].nm_UserData = (APTR)MEN_QUIT;

    menu[5].nm_Type     = NM_END;

    return menu;
  }
  return NULL;
}
///
///custom gadgets
Object *MUI_NewButton(STRPTR text) {
  Object *obj = MUI_NewObject(MUIC_Text,
    MUIA_InputMode, MUIV_InputMode_RelVerify,
    MUIA_Frame, MUIV_Frame_Button,
    MUIA_Background, MUII_ButtonBack,
    MUIA_Font, MUIV_Font_Button,
    MUIA_Text_PreParse, "\33c",
    MUIA_Text_Contents, (ULONG)text,
  TAG_END);

  return obj;
}

Object *MUI_NewLogo() {
  Object *obj = MUI_NewObject(MUIC_Group,
    MUIA_Group_Horiz, TRUE,
    MUIA_Group_Child, MUI_NewObject(MUIC_Rectangle, TAG_END),
    MUIA_Group_Child, MUI_NewObject(MUIC_Bitmap,
      MUIA_Bitmap_Bitmap, (ULONG) &bm_Logo,
      MUIA_Bitmap_Width, 187,
      MUIA_FixWidth, 187,
      MUIA_Bitmap_Height, 17,
      MUIA_FixHeight, 17,
      MUIA_Bitmap_Transparent, 0,
      MUIA_Bitmap_UseFriend, TRUE,
    TAG_END),
    MUIA_Group_Child, MUI_NewObject(MUIC_Rectangle, TAG_END),
  TAG_END);

  return obj;
}

APTR MUI_NewCheckMark(Object** obj, BOOL state, STRPTR label, UBYTE key, STRPTR help)
{
 APTR object;

 object = MUI_NewObject(MUIC_Group,
   MUIA_Group_Horiz, TRUE,
   MUIA_Group_Child, (*obj = MUI_NewObject(MUIC_Image,
     MUIA_Frame, MUIV_Frame_ImageButton,
     MUIA_InputMode, MUIV_InputMode_Toggle,
     MUIA_Image_Spec, MUII_CheckMark,
     MUIA_Image_FreeVert, TRUE,
     MUIA_Selected, state,
     MUIA_Background, MUII_ButtonBack,
     MUIA_ShowSelState, FALSE,
     MUIA_ControlChar, key,
   TAG_END)),
   MUIA_Group_Child, MUI_NewObject(MUIC_Text,
     MUIA_Text_Contents, label,
     MUIA_Text_HiChar, key,
   TAG_END),
   MUIA_ShortHelp, help,
 TAG_END);

return object;
}

///
///MUI hooks
// Packs List Display Hook
HOOKPROTO(l_packs_dispfunc, LONG, char **array, Pack* p)
{
  if (p) *array = p->name;
  return(0);
}
MakeStaticHook(l_packs_disphook, l_packs_dispfunc);

HOOKPROTO(l_profiles_dispfunc, LONG, char **array, struct ProfileFile* pf)
{
  static UBYTE buffer[16];

  if (pf) {
    sprintf(buffer, "%ld", pf->score);
    *array++ = pf->name;
    *array   = buffer;
  }
  else {
    *array++ = GetCatalogStr(catalog, MSG_GUI_LIST_TITLE_PLAYER_NAME, "\33b\33uPlayer Name");
    *array   = GetCatalogStr(catalog, MSG_GUI_LIST_TITLE_PLAYER_SCORE, "\33b\33uScore");
  }

  return(0);
}
MakeStaticHook(l_profiles_disphook, l_profiles_dispfunc);
///
///buildGUI
/***********************************************
 * Program main window                         *
 * - Creates the MUI Application object.       *
 ***********************************************/
Object *buildGUI()
{
  App = MUI_NewObject(MUIC_Application,
    MUIA_Application_Author, (ULONG)AUTHOR,
    MUIA_Application_Base, (ULONG)PROGRAMNAME,
    MUIA_Application_Copyright, (ULONG)COPYRIGHT,
    MUIA_Application_Description, (ULONG)DESCRIPTION,
    MUIA_Application_Title, (ULONG)PROGRAMNAME,
    MUIA_Application_Version, (ULONG)VersionTag,
    MUIA_Application_Window, (Win = MUI_NewObject(MUIC_Window,
      MUIA_Window_Title, (ULONG)PROGRAMNAME,
      //MUIA_Window_Height, MIN(500, MUIV_Window_Height_Screen(80)),
      //MUIA_Window_Width, MIN(512, MUIV_Window_Width_Screen(55)),
      MUIA_Window_Menustrip, MUI_MakeObject(MUIO_MenustripNM, (ULONG)prepareMenu(), 0),
      MUIA_Window_RootObject, MUI_NewObject(MUIC_Group,
        MUIA_Group_Child, MUI_NewLogo(),
        MUIA_Group_Child, (pagedGroup = MUI_NewObject(MUIC_Group,
          MUIA_Group_PageMode, TRUE,

          MUIA_Group_Child, MUI_NewObject(MUIC_Group,
            MUIA_Group_Child, MUI_NewObject(MUIC_Group,
              MUIA_Frame, MUIV_Frame_Group,
              MUIA_FrameTitle, GetCatalogStr(catalog, MSG_GUI_TITLE_PROFILES, "Profiles"),
              MUIA_Group_Child, (lv_profiles = MUI_NewObject(MUIC_Listview,
                MUIA_Frame, MUIV_Frame_InputList,
                MUIA_Listview_List, MUI_NewObject(MUIC_List,
                  MUIA_List_Title, TRUE,
                  MUIA_List_Format, ",",
                  MUIA_List_DisplayHook, &l_profiles_disphook,
                TAG_END),
              TAG_END)),
              MUIA_Group_Child, MUI_NewObject(MUIC_Group,
                MUIA_Group_Horiz, TRUE,
                MUIA_Group_Child, (btn_new_profile = MUI_NewButton(GetCatalogStr(catalog, MSG_GUI_BUTTON_NEW_PROFILE, "New"))),
                MUIA_Group_Child, (btn_del_profile = MUI_NewButton(GetCatalogStr(catalog, MSG_GUI_BUTTON_DELETE_PROFILE, "Delete"))),
              TAG_END),
            TAG_END),
            MUIA_Group_Child, (btn_start = MUI_NewButton(GetCatalogStr(catalog, MSG_GUI_BUTTON_START, "Start"))),
            MUIA_Group_Child, (btn_quit = MUI_NewButton(GetCatalogStr(catalog, MSG_GUI_BUTTON_QUIT, "Quit"))),
          TAG_END),

          MUIA_Group_Child, MUI_NewObject(MUIC_Group,
            MUIA_Group_Child, MUI_NewObject(MUIC_Group,
              MUIA_Background, MUII_ButtonBack,
              MUIA_Frame, MUIV_Frame_ReadList,
              MUIA_Group_Columns, 3,
              MUIA_Group_Child, MUI_NewObject(MUIC_Text, MUIA_Text_Contents, GetCatalogStr(catalog, MSG_GUI_DISPLAY_PLAYER_NAME, "Player:"), MUIA_HorizWeight, 0, TAG_END),
              MUIA_Group_Child, (dsp_name = MUI_NewObject(MUIC_Text, MUIA_Text_Contents, "", TAG_END)),
              MUIA_Group_Child, MUI_NewObject(MUIC_Rectangle, TAG_END),
              MUIA_Group_Child, MUI_NewObject(MUIC_Text, MUIA_Text_Contents, GetCatalogStr(catalog, MSG_GUI_DISPLAY_PLAYER_SCORE, "Score:"), MUIA_HorizWeight, 0, TAG_END),
              MUIA_Group_Child, (dsp_score = NewObject(cl_IntegerDisplay->mcc_Class, NULL, TAG_END)),
              MUIA_Group_Child, MUI_NewObject(MUIC_Rectangle, TAG_END),
            TAG_END),
            MUIA_Group_Child, MUI_NewObject(MUIC_Group,
              MUIA_Frame, MUIV_Frame_Group,
              MUIA_FrameTitle, GetCatalogStr(catalog, MSG_GUI_TITLE_AVAILABLE_PACKS, "Available Packs"),
              MUIA_Group_Child, (lv_packs = MUI_NewObject(MUIC_Listview,
                MUIA_Frame, MUIV_Frame_InputList,
                MUIA_Disabled, TRUE,
                MUIA_Listview_List, MUI_NewObject(MUIC_List, MUIA_List_DisplayHook, &l_packs_disphook, TAG_END),
              TAG_END)),
              MUIA_Group_Child, MUI_NewObject(MUIC_Group,
                MUIA_Group_Horiz, TRUE,
                MUIA_Group_Child, MUI_NewObject(MUIC_Text, MUIA_Text_Contents, GetCatalogStr(catalog, MSG_GUI_LABEL_SIZE, "Size:"), MUIA_Weight, 0, TAG_END),
                MUIA_Group_Child, (cg_sizeselector = NewObject(cl_SizeSelector->mcc_Class, NULL,
                  MUIA_Disabled, TRUE,
                TAG_END)),
              TAG_END),
              MUIA_Group_Child, MUI_NewCheckMark(&chk_randompack, TRUE, GetCatalogStr(catalog, MSG_GUI_CHECK_RANDOM_PACK, "Pick a random pack"), 'p', NULL),
              MUIA_Group_Child, MUI_NewCheckMark(&chk_randomsize, TRUE, GetCatalogStr(catalog, MSG_GUI_CHECK_RANDOM_SIZE, "Pick a random size"), 's', NULL),
            TAG_END),
            MUIA_Group_Child, (btn_play = MUI_NewButton(GetCatalogStr(catalog, MSG_GUI_BUTTON_PLAY, "Play"))),
            MUIA_Group_Child, (btn_exit = MUI_NewButton(GetCatalogStr(catalog, MSG_GUI_BUTTON_EXIT, "Exit"))),
          TAG_END),

        TAG_END)),
      TAG_END),
    TAG_END)),
    MUIA_Application_Window, (win_newprofile = MUI_NewObject(MUIC_Window,
      MUIA_Window_Title, GetCatalogStr(catalog, MSG_GUI_WINDOW_TITLE_NEW_PROFILE_NAME, "Enter new profile name"),
      MUIA_Window_RootObject, MUI_NewObject(MUIC_Group,
        MUIA_Group_Child, (str_newprofile = MUI_NewObject(MUIC_String,
          MUIA_Frame, MUIV_Frame_String,
          MUIA_FixWidthTxt, "Please enter new profile name...",
        TAG_END)),
        MUIA_Group_Child, (btn_create = MUI_NewButton(GetCatalogStr(catalog, MSG_GUI_BUTTON_CREATE_PROFILE, "Create"))),
      TAG_END),
    TAG_END)),
    MUIA_Application_Window, (win_puzzle = MUI_NewObject(MUIC_Window,
      MUIA_Window_Title, "AmiNonogram",
      MUIA_Window_RootObject, MUI_NewObject(MUIC_Group,
        MUIA_Group_Child, (dsp_lives = NewObject(cl_LivesDisplay->mcc_Class, NULL, MUIA_LivesDisplay_MaxLives, gv_lives, TAG_END)),
        MUIA_Group_Child, (grp_nonogram = MUI_NewObject(MUIC_Group, TAG_END)), // <-- nonogram object gets inserted here
        MUIA_Group_Child, MUI_NewObject(MUIC_Group, MUIA_Group_Horiz, TRUE,
          MUIA_Group_Child, MUI_NewObject(MUIC_Text, MUIA_Text_Contents, GetCatalogStr(catalog, MSG_GUI_LABEL_SOLUTION, "Solution:"), TAG_END),
          MUIA_Group_Child, (dsp_puzzle_name = MUI_NewObject(MUIC_Text, TAG_END)),
        TAG_END),
        MUIA_Group_Child, MUI_NewObject(MUIC_Group, MUIA_Group_Horiz, TRUE,
          MUIA_Group_Child, (btn_puzzle_exit = MUI_NewButton(GetCatalogStr(catalog, MSG_GUI_BUTTON_EXIT_PUZZLE, "Exit"))),
          MUIA_Group_Child, (btn_puzzle_next = MUI_NewButton(GetCatalogStr(catalog, MSG_GUI_BUTTON_NEXT_PUZZLE, "Next"))),
        TAG_END),
      TAG_END),
    TAG_END)),
  TAG_END);

  //Notifications
  if (App)
  {
    //fill packs listview
    Pack* pack;
    for (pack = (Pack*) packs->list.mlh_Head; pack->node.mln_Succ; pack = (Pack*) pack->node.mln_Succ) {
      DoMethod(lv_packs, MUIM_List_InsertSingle, pack, MUIV_List_Insert_Bottom);
    }
    //fill profiles listview
    {
      struct ProfileFile *pf;
      for (pf = (struct ProfileFile*) profiles->list.mlh_Head; pf->node.mln_Succ; pf = (struct ProfileFile*) pf->node.mln_Succ) {
        DoMethod(lv_profiles, MUIM_List_InsertSingle, pf, MUIV_List_Insert_Bottom);
      }
    }

    DoMethod(Win, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, App, 2,
      MUIM_Application_ReturnID, WIN_CLOSE);

    DoMethod(win_newprofile, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, win_newprofile, 3,
      MUIM_Set, MUIA_Window_Open, FALSE);

    DoMethod(btn_quit, MUIM_Notify, MUIA_Pressed, FALSE, App, 2,
       MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

    DoMethod(btn_new_profile, MUIM_Notify, MUIA_Pressed, FALSE, App, 2,
      MUIM_Application_ReturnID, BTN_NEW_PROFILE);

    DoMethod(btn_del_profile, MUIM_Notify, MUIA_Pressed, FALSE, App, 2,
      MUIM_Application_ReturnID, BTN_DEL_PROFILE);

    DoMethod(btn_del_profile, MUIM_Set, MUIA_Disabled, TRUE);
    DoMethod(btn_start, MUIM_Set, MUIA_Disabled, TRUE);

    DoMethod(lv_profiles, MUIM_Notify, MUIA_List_Active, MUIV_EveryTime, App, 2,
      MUIM_Application_ReturnID, BTN_DEL_PROFILE_TOGGLE);

    DoMethod(lv_profiles, MUIM_Notify, MUIA_Listview_DoubleClick, TRUE, App, 2,
      MUIM_Application_ReturnID, BTN_START);

    DoMethod(chk_randompack, MUIM_Notify, MUIA_Selected, MUIV_EveryTime, lv_packs, 3,
      MUIM_Set, MUIA_Disabled, MUIV_TriggerValue);

    DoMethod(chk_randomsize, MUIM_Notify, MUIA_Selected, MUIV_EveryTime, cg_sizeselector, 3,
      MUIM_Set, MUIA_Disabled, MUIV_TriggerValue);

    DoMethod(btn_start, MUIM_Notify, MUIA_Pressed, FALSE, App, 2,
      MUIM_Application_ReturnID, BTN_START);

    DoMethod(btn_exit, MUIM_Notify, MUIA_Pressed, FALSE, App, 2,
      MUIM_Application_ReturnID, BTN_EXIT);

    DoMethod(btn_create, MUIM_Notify, MUIA_Pressed, FALSE, App, 2,
      MUIM_Application_ReturnID, BTN_CREATE);

    DoMethod(str_newprofile, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, App, 2,
      MUIM_Application_ReturnID, BTN_CREATE);

    DoMethod(btn_play, MUIM_Notify, MUIA_Pressed, FALSE, App, 2,
      MUIM_Application_ReturnID, BTN_PLAY);

    DoMethod(win_puzzle, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, App, 2,
      MUIM_Application_ReturnID, BTN_PUZZLE_EXIT);

    DoMethod(btn_puzzle_exit, MUIM_Notify, MUIA_Pressed, FALSE, App, 2,
      MUIM_Application_ReturnID, BTN_PUZZLE_EXIT);

    DoMethod(btn_puzzle_next, MUIM_Notify, MUIA_Pressed, FALSE, App, 2,
      MUIM_Application_ReturnID, BTN_PUZZLE_NEXT);

    DoMethod(dsp_lives, MUIM_Notify, MUIA_LivesDisplay_Lives, 0, App, 2,
      MUIM_Application_ReturnID, EVENT_GAMELOSE);
  }

  return App;
}
///
///main
/***********************************************
 * Developer level main                        *
 * - Code your program here.                   *
 ***********************************************/
int Main(struct Config *config)
{
  int rc = 0;

  if ((packs = loadPacks(dir_packs)))
  {
    if ((profiles = readProfileFiles(dir_profiles)))
    {
      MUIMasterBase = OpenLibrary((STRPTR)"muimaster.library", 0);
      if (MUIMasterBase)
      {
        #if defined(__amigaos4__)
        if (IMUIMaster = (struct MUIMasterIFace *)GetInterface(MUIMasterBase, "main", 1, NULL))
        {
        #endif
          if ((cl_LivesDisplay = MUI_Create_LivesDisplay()))
          {
            if ((cl_HintsDisplay = MUI_Create_HintDisplay()))
            {
              if ((cl_Square = MUI_Create_Square()))
              {
                if ((cl_Nonogram = MUI_Create_Nonogram()))
                {
                  if ((cl_SizeSelector = MUI_Create_SizeSelector()))
                  {
                    if ((cl_IntegerDisplay = MUI_Create_IntegerDisplay()))
                    {
                      if (buildGUI())
                      {
                        ULONG signals = 0;
                        BOOL running = TRUE;

                        set(Win, MUIA_Window_Open, TRUE);

                        while(running)
                        {
                          ULONG id = DoMethod (App, MUIM_Application_NewInput, &signals);
                          switch(id)
                          {
                            case MEN_ABOUT:
                              MUI_Request(App, Win, 0,
                                GetCatalogStr(catalog, MSG_GUI_WINDOW_TITLE_ABOUT, "About AmiNonogram"), GetCatalogStr(catalog, MSG_UI_BUTTON_OK, "*_OK"),
                                "AmiNonogram v" VERSIONSTRING "\n\n%s " AUTHOR "\n%s " CONTACT, GetCatalogStr(catalog, MSG_UI_CODER, "Programming:"), GetCatalogStr(catalog, MSG_UI_CONTACT, "Contact:"));
                            break;
                            case MEN_ABOUTMUI:
                              if (!AboutMUIWin) {
                              AboutMUIWin = MUI_NewObject(MUIC_Aboutmui,
                                MUIA_Window_RefWindow, Win,
                                MUIA_Aboutmui_Application, App,
                              TAG_END);
                              }
                              if (AboutMUIWin)
                                set(AboutMUIWin, MUIA_Window_Open, TRUE);
                            break;
                            case MEN_QUIT:
                            case WIN_CLOSE:
                              if (profile)
                                if (!saveProfile(profile, dir_profiles, packs)) {
                                  MUI_Request(App, Win, 0, GetCatalogStr(catalog, MSG_GUI_WINDOW_TITLE_WARNING, "Warning!"), GetCatalogStr(catalog, MSG_UI_BUTTON_OK, "*_OK"), "%s\n%s",
                                  GetCatalogStr(catalog, MSG_ERR_SAVE_PROFILE, "AmiNonogram could not save your profile to disk."),
                                  GetCatalogStr(catalog, MSG_ERR_DISK_PROTECTED, "AmiNonogram needs to be on a non-write protected disk to run."));
                                }
                            case MUIV_Application_ReturnID_Quit:
                              running = FALSE;
                            break;
                            case BTN_START:
                            {
                              struct ProfileFile *pf;
                              DoMethod(lv_profiles, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &pf);
                              if (pf) {
                                profile = newProfile(pf, packs, dir_profiles);
                                if (profile) {
                                  DoMethod(dsp_name, MUIM_Set, MUIA_Text_Contents, profile->name);
                                  DoMethod(dsp_score, MUIM_Set, MUIA_IntegerDisplay_Value, profile->score);

                                  DoMethod(pagedGroup, MUIM_Set, MUIA_Group_ActivePage, 1);
                                }
                              }
                            }
                            break;
                            case BTN_EXIT:
                              {
                                struct ProfileFile* pf;
                                DoMethod(lv_profiles, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &pf);
                                pf->score = profile->score;
                                DoMethod(lv_profiles, MUIM_List_Redraw, MUIV_List_Redraw_Active);
                              }
                              if (!saveProfile(profile, dir_profiles, packs)) {
                                MUI_Request(App, Win, 0, GetCatalogStr(catalog, MSG_GUI_WINDOW_TITLE_WARNING, "Warning!"), GetCatalogStr(catalog, MSG_UI_BUTTON_OK, "*_OK"), "%s\n%s",
                                GetCatalogStr(catalog, MSG_ERR_SAVE_PROFILE, "AmiNonogram could not save your profile to disk."),
                                GetCatalogStr(catalog, MSG_ERR_DISK_PROTECTED, "AmiNonogram needs to be on a non-write protected disk to run."));
                              }
                              freeProfile(profile); profile = NULL;
                              DoMethod(pagedGroup, MUIM_Set, MUIA_Group_ActivePage, 0);
                            break;
                            case BTN_NEW_PROFILE:
                              DoMethod(win_newprofile, MUIM_Set, MUIA_Window_Open, TRUE);
                              DoMethod(str_newprofile, MUIM_Set, MUIA_String_Contents, "");
                              DoMethod(win_newprofile, MUIM_Set, MUIA_Window_ActiveObject, str_newprofile);
                            break;
                            case BTN_CREATE:
                            {
                              STRPTR name;
                              struct ProfileFile* pf;

                              DoMethod(str_newprofile, OM_GET, MUIA_String_Contents, &name);

                              if (!name || !strcmp(name, "")) {
                                MUI_Request(App, win_newprofile, NULL, GetCatalogStr(catalog, MSG_GUI_WINDOW_TITLE_WARNING, "Warning!"), GetCatalogStr(catalog, MSG_UI_BUTTON_OK, "*_OK"),
                                  GetCatalogStr(catalog, MSG_UI_ERR_PROFILE_NAME_EMPTY, "Profile name cannot be empty!"));
                              }
                              else {
                                DoMethod(win_newprofile, MUIM_Set, MUIA_Window_Open, FALSE);
                                pf = addProfileFile(profiles, name, dir_profiles);
                                if (pf) {
                                  DoMethod(lv_profiles, MUIM_List_InsertSingle, pf, MUIV_List_Insert_Bottom);
                                }
                                else {
                                  MUI_Request(App, Win, 0, GetCatalogStr(catalog, MSG_GUI_WINDOW_TITLE_WARNING, "Warning!"), GetCatalogStr(catalog, MSG_UI_BUTTON_OK, "*_OK"), "%s\n%s",
                                  GetCatalogStr(catalog, MSG_ERR_CREATE_PROFILE, "AmiNonogram could not create a new profile file on disk."),
                                  GetCatalogStr(catalog, MSG_ERR_DISK_PROTECTED, "AmiNonogram needs to be on a non-write protected disk to run."));
                                }
                              }
                            }
                            break;
                            case BTN_DEL_PROFILE:
                            {
                              struct ProfileFile* pf;
                              DoMethod(lv_profiles, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &pf);
                              if (pf) {
                                DoMethod(lv_profiles, MUIM_List_Remove, MUIV_List_Remove_Active);
                                removeProfileFile(profiles, pf, dir_profiles);
                              }
                            }
                            break;
                            case BTN_DEL_PROFILE_TOGGLE:
                            {
                              ULONG active;
                              DoMethod(lv_profiles, OM_GET, MUIA_List_Active, &active);
                              if (active == MUIV_List_Active_Off) {
                                DoMethod(btn_del_profile, MUIM_Set, MUIA_Disabled, TRUE);
                                DoMethod(btn_start, MUIM_Set, MUIA_Disabled, TRUE);
                              }
                              else {
                                DoMethod(btn_del_profile, MUIM_Set, MUIA_Disabled, FALSE);
                                DoMethod(btn_start, MUIM_Set, MUIA_Disabled, FALSE);
                              }
                            }
                            break;
                            case BTN_PUZZLE_NEXT:
                              DoMethod(win_puzzle, MUIM_Set, MUIA_Window_Open, FALSE);
                              if (DoMethod(grp_nonogram, MUIM_Group_InitChange)) {
                                DoMethod(grp_nonogram, OM_REMMEMBER, obj_nonogram);
                                DoMethod(grp_nonogram, MUIM_Group_ExitChange);

                                MUI_DisposeObject(obj_nonogram); obj_nonogram = NULL;
                                freeGameBoardData(gbd); gbd = NULL;
                              }
                            case BTN_PLAY:
                            {
                              ULONG randomSize;
                              ULONG randomPack;
                              ULONG size;
                              ULONG result;
                              Pack* pack;

                              DoMethod(chk_randomsize, OM_GET, MUIA_Selected, &randomSize);
                              DoMethod(chk_randompack, OM_GET, MUIA_Selected, &randomPack);
                              if (randomSize) size = 0;
                              else {
                                DoMethod(cg_sizeselector, OM_GET, MUIA_SizeSelector_Value, &size);
                              }
                              if (randomPack) pack = NULL;
                              else {
                                DoMethod(lv_packs, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &pack);
                              }

                              result = bringPuzzle(packs, pack, profile, size, &gbd);
                              if (!result) {
                                obj_nonogram = NewObject(cl_Nonogram->mcc_Class, NULL, MUIA_Nonogram_Data, gbd, TAG_END);
                                if(obj_nonogram) {
                                  DoMethod(obj_nonogram, MUIM_Notify, MUIA_Nonogram_Faults, MUIV_EveryTime, dsp_lives, 1, MUIM_LivesDisplay_LoseLife);
                                  DoMethod(obj_nonogram, MUIM_Notify, MUIA_Nonogram_Complete, TRUE, App, 2, MUIM_Application_ReturnID, EVENT_GAMEWIN);
                                  if (DoMethod(grp_nonogram, MUIM_Group_InitChange)) {
                                    DoMethod(grp_nonogram, OM_ADDMEMBER, obj_nonogram);
                                    DoMethod(grp_nonogram, MUIM_Group_ExitChange);
                                  }
                                  DoMethod(dsp_lives, MUIM_Set, MUIA_LivesDisplay_Lives, gv_lives);
                                  DoMethod(dsp_puzzle_name, MUIM_Set, MUIA_Text_Contents, "");
                                  DoMethod(btn_puzzle_next, MUIM_Set, MUIA_Disabled, TRUE);
                                  DoMethod(win_puzzle, MUIM_Set, MUIA_Window_Open, TRUE);
                                  DoMethod(Win, MUIM_Set, MUIA_Window_Open, FALSE);
                                }
                              }
                              else {
                                STRPTR str = NULL;
                                ULONG state;
                                DoMethod(Win, OM_GET, MUIA_Window_Open, &state);
                                if (!state) {
                                  DoMethod(Win, MUIM_Set, MUIA_Window_Open, TRUE);
                                }

                                switch (result) {
                                  case BP_MEMORY_ERROR:
                                    str = GetCatalogStr(catalog, MSG_ERR_OUT_OF_MEMORY, "Out of memory!");
                                  break;
                                  case BP_NO_PACKS:
                                    str = GetCatalogStr(catalog, MSG_UI_NO_PACKS, "There are no packs to solve!\nPlease put some puzzle packs into packs directory.");
                                  break;
                                  case BP_ALL_PACKS_SOLVED:
                                    str = GetCatalogStr(catalog, MSG_UI_ALL_PACKS_SOLVED, "You have solved all available puzzles! Well done!");
                                  break;
                                  case BP_NO_SUCH_SIZE_IN_PACKS:
                                    str = GetCatalogStr(catalog, MSG_UI_NO_SUCH_SIZE_IN_PACKS, "This puzzle size could not be found in any of the packs.");
                                  break;
                                  case BP_NO_SUCH_SIZE_IN_PACK:
                                    str = GetCatalogStr(catalog, MSG_UI_NO_SUCH_SIZE_IN_PACK, "This pack does not contain this puzzle size.");
                                  break;
                                  case BP_SIZE_SOLVED_IN_ALL:
                                    str = GetCatalogStr(catalog, MSG_UI_SIZE_SOLVED_IN_ALL, "You have solved all puzzles of this size from all packs.");
                                  break;
                                  case BP_SIZE_SOLVED_IN_THIS:
                                    str = GetCatalogStr(catalog, MSG_UI_SIZE_SOLVED_IN_THIS, "You have solved all puzzles of this size from this pack.");
                                  break;
                                }
                                MUI_Request(App, Win, 0, GetCatalogStr(catalog, MSG_GUI_WINDOW_TITLE_WARNING, "Warning!"), GetCatalogStr(catalog, MSG_UI_BUTTON_OK, "*_OK"), str);
                              }
                            }
                            break;
                            case BTN_PUZZLE_EXIT:
                            {
                              ULONG complete;
                              ULONG disabled;
                              DoMethod(obj_nonogram, OM_GET, MUIA_Nonogram_Complete, &complete);
                              DoMethod(obj_nonogram, OM_GET, MUIA_Disabled, &disabled);
                              if (!complete && !disabled) {
                                if (!MUI_Request(App, win_puzzle, 0, GetCatalogStr(catalog, MSG_GUI_WINDOW_TITLE_WARNING, "Warning!"), GetCatalogStr(catalog, MSG_UI_BUTTON_YES_NO, "_Yes|*_No"),
                                  GetCatalogStr(catalog, MSG_UI_CONFIRM_PUZZLE_QUIT, "\33cQuitting this puzzle will cost you \33b%ld pts\33n!\nDo you still want to quit?"), gbd->size)) {
                                  break;
                                }
                                profile->score -= gbd->size;
                                DoMethod(dsp_score, MUIM_Set, MUIA_IntegerDisplay_Value, profile->score);
                              }
                              DoMethod(Win, MUIM_Set, MUIA_Window_Open, TRUE);
                              DoMethod(win_puzzle, MUIM_Set, MUIA_Window_Open, FALSE);
                              if (DoMethod(grp_nonogram, MUIM_Group_InitChange)) {
                                DoMethod(grp_nonogram, OM_REMMEMBER, obj_nonogram);
                                DoMethod(grp_nonogram, MUIM_Group_ExitChange);

                                MUI_DisposeObject(obj_nonogram); obj_nonogram = NULL;
                                freeGameBoardData(gbd); gbd = NULL;
                              }
                            }
                            break;
                            case EVENT_GAMELOSE:
                              DoMethod(obj_nonogram, MUIM_Set, MUIA_Disabled, TRUE);
                              MUI_Request(App, win_puzzle, 0, GetCatalogStr(catalog, MSG_GUI_WINDOW_TITLE_SORRY, "Sorry!"), GetCatalogStr(catalog, MSG_UI_BUTTON_LOSE, "*_Rats!!"),
                                GetCatalogStr(catalog, MSG_UI_INFORM_PUZZLE_LOST, "\33cYou've failed to solve this puzzle\n\nScore lost: %ld pts"), -gbd->size);
                              profile->score -= gbd->size;
                              DoMethod(dsp_score, MUIM_Set, MUIA_IntegerDisplay_Value, profile->score);
                            break;
                            case EVENT_GAMEWIN:
                            {
                              LONG scr_earned;
                              ULONG lives;

                              DoMethod(dsp_lives, OM_GET, MUIA_LivesDisplay_Lives, &lives);
                              scr_earned = (gbd->size * lives * 3) / gv_lives;

                              DoMethod(dsp_puzzle_name, MUIM_Set, MUIA_Text_Contents, gbd->name);
                              DoMethod(btn_puzzle_next, MUIM_Set, MUIA_Disabled, FALSE);
                              MUI_Request(App, win_puzzle, 0, GetCatalogStr(catalog, MSG_GUI_WINDOW_TITLE_CONGRATULATIONS, "Congratulations!"), GetCatalogStr(catalog, MSG_UI_BUTTON_WIN, "*_Hooray!"),
                                GetCatalogStr(catalog, MSG_UI_INFORM_PUZZLE_WIN, "\33cYou have solved this puzzle!\nSolution: \33b%s\33n\nScore earned: %ld pts"), (ULONG)gbd->name, scr_earned);
                              profile->score += scr_earned;
                              DoMethod(dsp_score, MUIM_Set, MUIA_IntegerDisplay_Value, profile->score);
                              //Mark this puzzle as solved in profile:
                              setBit(profile->packStates[gbd->pack_index], gbd->puzzle_index);
                            }
                            break;
                          }
                          if(running && signals) signals = Wait(signals | SIGBREAKF_CTRL_C);
                          if (signals & SIGBREAKF_CTRL_C) break;
                        }

                        set(Win, MUIA_Window_Open, FALSE);

                        MUI_DisposeObject(App);
                      }
                      else rc = 20;

                      MUI_DeleteCustomClass(cl_IntegerDisplay);
                    }

                    MUI_DeleteCustomClass(cl_SizeSelector);
                  }

                  MUI_DeleteCustomClass(cl_Nonogram);
                }

                MUI_DeleteCustomClass(cl_Square);
              }

              MUI_DeleteCustomClass(cl_HintsDisplay);
            }

            MUI_DeleteCustomClass(cl_LivesDisplay);
          }
        #if defined(__amigaos4__)
          DropInterface((struct Interface *)IMUIMaster);
        }
        else rc = 20;
        #endif
        CloseLibrary(MUIMasterBase);
      }
      else rc = 20;

      freeProfileFiles(profiles);
    }

    freePacks(packs);
  }

  return(rc);
}
///
///cleanup
/***********************************************
 * Clean up before exit                        *
 * - Free allocated resources here.            *
 ***********************************************/
void CleanUp(struct Config *config)
{
  if (config)
  {
    CloseCatalog(catalog);

    #if defined(__amigaos4__)
    FreeSysObject(ASOT_MEMPOOL, memoryPool);
    #else
    DeletePool(memoryPool);
    #endif

    // free command line arguments
    #if RDARGS_OPTIONS
      if (config->RDArgs)
        FreeArgs(config->RDArgs);
    #endif

    FreeMem(config, sizeof(struct Config));
  }
}
///
