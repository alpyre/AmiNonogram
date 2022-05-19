/******************************************************************************
 * LivesDisplay                                                               *
 ******************************************************************************/
/******************************************************************************
 This file is part of AmiNonogram.
 Copyright (C) 2022 Ibrahim Alper Sönmez
******************************************************************************/

//Public Attributes
#define MUIA_LivesDisplay_MaxLives 0x80460001 //(I.G)
#define MUIA_LivesDisplay_Lives    0x80460002 //(ISG)

//Public Methods
#define MUIM_LivesDisplay_LoseLife 0x80460001

//Public Functions
struct MUI_CustomClass* MUI_Create_LivesDisplay(void);
