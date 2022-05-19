/******************************************************************************
 * HintDisplay                                                                *
 ******************************************************************************/
/******************************************************************************
 This file is part of AmiNonogram.
 Copyright (C) 2022 Ibrahim Alper Sönmez
******************************************************************************/

//Public Attributes
#define MUIA_HintDisplay_HintArray 0x80430001 //(I..)

//Public Methods
#define MUIM_HintDisplay_Reveal    0x80430001
#define MUIM_HintDisplay_RevealAll 0x80430002

//Public Functions
struct MUI_CustomClass* MUI_Create_HintDisplay(void);
