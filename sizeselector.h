/******************************************************************************
 * SizeSelector                                                               *
 ******************************************************************************/
/******************************************************************************
 This file is part of AmiNonogram.
 Copyright (C) 2022 Ibrahim Alper Sönmez
******************************************************************************/

#define MAX_PUZZLE_SIZE 50

//Public Attributes
#define MUIA_SizeSelector_Value    0x80470001 //(.SG)

//Public Methods
#define MUIM_SizeSelector_Increase 0x80470001
#define MUIM_SizeSelector_Decrease 0x80470002

//Public Functions
struct MUI_CustomClass* MUI_Create_SizeSelector(void);
