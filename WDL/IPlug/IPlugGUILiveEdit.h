#ifndef _IPLUGGUILIVEEDIT_
#define _IPLUGGUILIVEEDIT_

/*
Youlean - IPlugGUILiveEdit - live GUI editing class

Copyright (C) 2016 and later, Youlean

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.
2. This notice may not be removed or altered from any source distribution.

*/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <typeinfo>
#include "IControl.h"
#include "IGraphics.h"
#include "IPlugGUIResize.h"

using namespace std;

class IPlugGUILiveEdit
{
public:
	IPlugGUILiveEdit() {}
	~IPlugGUILiveEdit() {}

	void EditGUI(IPlugBase* pPlug, IGraphics* pGraphics, WDL_PtrList<IControl>* pControls, LICE_IBitmap* pDrawBitmap, 
		IMouseMod* liveEditingMod, int* liveGridSize, int* liveSnap, int* liveKeyDown, bool* liveToogleEditing, int* liveMouseCapture,
		bool* liveMouseDragging, int* mMouseX, int* mMouseY, int width, int height, double guiScaleRatio)
	{
		// Moving controls --------------------------------------------------------------------
	
		if (pPlug->GetGUIResize()) liveScaledGridSize = int((double)*liveGridSize * guiScaleRatio);
		else liveScaledGridSize = *liveGridSize;
		
		if (pPlug->GetGUIResize()) viewMode = pPlug->GetGUIResize()->GetViewMode();

		// Toogle live editing
		if (*liveToogleEditing)
		{
			if (*liveKeyDown == 19)
			{
				*liveToogleEditing = false;
				*liveKeyDown = -1;
			}
		}
		else
		{
			if (*liveKeyDown == 19)
			{
				*liveToogleEditing = true;
				*liveKeyDown = -1;
			}
		}


		// If mouse was clicked
		if (*liveToogleEditing)
		{
			pGraphics->SetAllControlsDirty();

			// Draw control rects
			int controlSize = pControls->GetSize();

			if (pPlug->GetGUIResize())
			{
				controlSize -= 3;
				if (*liveMouseCapture > controlSize) *liveMouseCapture = -1;
			}

			for (int j = 1; j < controlSize; j++)
			{
				IControl* pControl = pControls->Get(j);

				IRECT drawRECT = *pControl->GetRECT();

				// T
				LICE_DashedLine(pDrawBitmap, drawRECT.L, drawRECT.T, drawRECT.R, drawRECT.T, 2, 2,
					LICE_RGBA(EDIT_COLOR.R, EDIT_COLOR.G, EDIT_COLOR.B, EDIT_COLOR.A));
				//B
				LICE_DashedLine(pDrawBitmap, drawRECT.L, drawRECT.B, drawRECT.R, drawRECT.B, 2, 2,
					LICE_RGBA(EDIT_COLOR.R, EDIT_COLOR.G, EDIT_COLOR.B, EDIT_COLOR.A));
				//L
				LICE_DashedLine(pDrawBitmap, drawRECT.L, drawRECT.T, drawRECT.L, drawRECT.B, 2, 2,
					LICE_RGBA(EDIT_COLOR.R, EDIT_COLOR.G, EDIT_COLOR.B, EDIT_COLOR.A));
				//R
				LICE_DashedLine(pDrawBitmap, drawRECT.R, drawRECT.T, drawRECT.R, drawRECT.B, 2, 2,
					LICE_RGBA(EDIT_COLOR.R, EDIT_COLOR.G, EDIT_COLOR.B, EDIT_COLOR.A));

			}

			WDL_String str;
			str.SetFormatted(32, "x: %i, y: %i", *mMouseX, *mMouseY);
			IText txt(20, &EDIT_COLOR, defaultFont);
			IRECT rect(width - 150, height - 20, width, height);
			pGraphics->DrawIText(&txt, str.Get(), &rect);

			// Draw resizing handles
			int liveHandleSize = int(8.0 * guiScaleRatio);

			for (int j = 1; j < controlSize; j++)
			{
				IControl* pControl = pControls->Get(j);

				IRECT drawRECT = *pControl->GetRECT();
				IRECT handle = IRECT(drawRECT.R - liveHandleSize, drawRECT.B - liveHandleSize, drawRECT.R, drawRECT.B);
				pGraphics->FillTriangle(&EDIT_COLOR, handle.L, handle.B, handle.R, handle.B, handle.R, handle.T, 0);
			}

			bool overControlHandle = false;
			// Find if over control handle
			for (int j = 1; j < controlSize; j++)
			{
				IControl* pControl = pControls->Get(j);

				IRECT drawRECT = *pControl->GetRECT();
				IRECT handle = IRECT(drawRECT.R - liveHandleSize, drawRECT.B - liveHandleSize, drawRECT.R, drawRECT.B);

				if (drawRECT.Contains(*mMouseX, *mMouseY))
				{
					overControlHandle = handle.Contains(*mMouseX, *mMouseY);
				}
			}

			if (!*liveMouseDragging)
			{
				// Change cursor when over handle
				if (overControlHandle) SetCursor(LoadCursor(NULL, IDC_SIZENWSE));
				else SetCursor(LoadCursor(NULL, IDC_ARROW));
			}

			if (liveEditingMod->R)
			{
				IControl* pControl = pControls->Get(*liveMouseCapture);

				liveSelectedRECT = *pControl->GetRECT();
				liveSelectedTargetRECT = *pControl->GetTargetRECT();
				liveControlNumber = *liveMouseCapture;

				// Find where mouse was clicked
				if (*liveMouseCapture != lastliveMouseCapture)
				{
					liveClickedX = *mMouseX;
					liveClickedY = *mMouseY;
					liveClickedRECT = liveSelectedRECT;
					liveClickedTargetRECT = liveSelectedTargetRECT;

					IRECT handle = IRECT(liveClickedRECT.R - liveHandleSize, liveClickedRECT.B - liveHandleSize, liveClickedRECT.R, liveClickedRECT.B);
					liveClickedOnHandle = handle.Contains(liveClickedX, liveClickedY);
				}
			}

			if (liveEditingMod->L)
			{
				IControl* pControl = pControls->Get(*liveMouseCapture);

				liveSelectedRECT = *pControl->GetRECT();
				liveSelectedTargetRECT = *pControl->GetTargetRECT();
				liveControlNumber = *liveMouseCapture;

				// Find where mouse was clicked
				if (*liveMouseCapture != lastliveMouseCapture)
				{
					liveClickedX = *mMouseX;
					liveClickedY = *mMouseY;
					liveClickedRECT = liveSelectedRECT;
					liveClickedTargetRECT = liveSelectedTargetRECT;

					IRECT handle = IRECT(liveClickedRECT.R - liveHandleSize, liveClickedRECT.B - liveHandleSize, liveClickedRECT.R, liveClickedRECT.B);
					liveClickedOnHandle = handle.Contains(liveClickedX, liveClickedY);
				}


				// Change cursor when clicked on handle
				if (liveClickedOnHandle || overControlHandle) SetCursor(LoadCursor(NULL, IDC_SIZENWSE));
				else SetCursor(LoadCursor(NULL, IDC_ARROW));

				// Prevent editing
				if (*liveMouseDragging || liveEditingMod->C)
				{
					IRECT drawArea;
					if (!liveClickedOnHandle)
					{
						drawArea.L = liveClickedRECT.L + (*mMouseX - liveClickedX);
						drawArea.T = liveClickedRECT.T + (*mMouseY - liveClickedY);
						drawArea.R = liveClickedRECT.R + (*mMouseX - liveClickedX);
						drawArea.B = liveClickedRECT.B + (*mMouseY - liveClickedY);
					}
					else
					{
						drawArea.L = liveClickedRECT.L;
						drawArea.T = liveClickedRECT.T;
						drawArea.R = liveClickedRECT.R + (*mMouseX - liveClickedX);
						drawArea.B = liveClickedRECT.B + (*mMouseY - liveClickedY);
					}

					IRECT targetArea;
					if (!liveClickedOnHandle)
					{
						targetArea.L = liveClickedRECT.L + (*mMouseX - liveClickedX);
						targetArea.T = liveClickedRECT.T + (*mMouseY - liveClickedY);
						targetArea.R = liveClickedRECT.R + (*mMouseX - liveClickedX);
						targetArea.B = liveClickedRECT.B + (*mMouseY - liveClickedY);
					}
					else
					{
						targetArea.L = liveClickedRECT.L;
						targetArea.T = liveClickedRECT.T;
						targetArea.R = liveClickedRECT.R + (*mMouseX - liveClickedX);
						targetArea.B = liveClickedRECT.B + (*mMouseY - liveClickedY);
					}

					// Snap to grid
					if (!liveEditingMod->A)
					{
						if (!liveClickedOnHandle)
						{
							int gridL = (drawArea.L / liveScaledGridSize) * liveScaledGridSize;
							int gridT = (drawArea.T / liveScaledGridSize) * liveScaledGridSize;

							int diffL = gridL - drawArea.L;
							int diffT = gridT - drawArea.T;

							drawArea.L += diffL;
							drawArea.T += diffT;
							drawArea.R += diffL;
							drawArea.B += diffT;

							targetArea.L += diffL;
							targetArea.T += diffT;
							targetArea.R += diffL;
							targetArea.B += diffT;
						}
						else
						{
							int gridR = (drawArea.R / liveScaledGridSize) * liveScaledGridSize;
							int gridB = (drawArea.B / liveScaledGridSize) * liveScaledGridSize;

							int diffR = gridR - drawArea.R;
							int diffB = gridB - drawArea.B;

							drawArea.R += diffR;
							drawArea.B += diffB;

							targetArea.R += diffR;
							targetArea.B += diffB;
						}
					}

					// Snap to other control
					if (liveEditingMod->C)
					{
						int snapSize = *liveSnap + 1;

						int snapL = 0;
						int snapMinL = 999999999;
						int snapMaxL = -999999999;
						int prevsnapMinL = 999999999;
						int prevsnapMaxL = -999999999;

						int snapT = 0;
						int snapMinT = 999999999;
						int snapMaxT = -999999999;
						int prevsnapMinT = 999999999;
						int prevsnapMaxT = -999999999;

						bool didSnappedT = false;
						bool didSnappedL = false;

						IRECT lineMinL, lineMinT;
						IRECT lineMaxL, lineMaxT;
						IRECT lineL, lineT;

						for (int index = 0; index < controlSize; index++)
						{
							if (index == *liveMouseCapture) continue;

							IControl* pSnapControl = pControls->Get(index);
							IRECT tmpDrawArea = *pSnapControl->GetRECT();
							int tmpSnapL;
							IRECT tmpRECTL;

							int tmpDrawArea_L = tmpDrawArea.L;
							int tmpDrawArea_LM = tmpDrawArea.L + tmpDrawArea.W() / 2;
							int tmpDrawArea_R = tmpDrawArea.R;
							int tmpDrawArea_T = tmpDrawArea.T;
							int tmpDrawArea_TM = tmpDrawArea.T + tmpDrawArea.H() / 2;
							int tmpDrawArea_B = tmpDrawArea.B;

							int drawArea_L = drawArea.L;
							int drawArea_LM = drawArea.L + drawArea.W() / 2;
							int drawArea_R = drawArea.R;
							int drawArea_T = drawArea.T;
							int drawArea_TM = drawArea.T + drawArea.H() / 2;
							int drawArea_B = drawArea.B;

							// Find snap to L
							for (int j = 0; j < 9; j++)
							{
								if (liveClickedOnHandle)
								{
									if ((j / 3) * 3 == j) continue;
								}

								if (j == 0) // L to L
								{
									tmpSnapL = tmpDrawArea_L - drawArea_L;
									tmpRECTL = IRECT(tmpDrawArea_L, tmpDrawArea_TM, drawArea_L, drawArea_TM);
								}
								if (j == 1) // L to LM
								{
									tmpSnapL = tmpDrawArea_L - drawArea_LM;
									tmpRECTL = IRECT(tmpDrawArea_L, tmpDrawArea_TM, drawArea_LM, drawArea_TM);
								}
								if (j == 2) // L to R
								{
									tmpSnapL = tmpDrawArea_L - drawArea_R;
									tmpRECTL = IRECT(tmpDrawArea_L, tmpDrawArea_TM, drawArea_R, drawArea_TM);
								}
								if (j == 3) // LM to L
								{
									tmpSnapL = tmpDrawArea_LM - drawArea_L;
									tmpRECTL = IRECT(tmpDrawArea_LM, tmpDrawArea_TM, drawArea_L, drawArea_TM);
								}
								if (j == 4) // LM to LM
								{
									tmpSnapL = tmpDrawArea_LM - drawArea_LM;
									tmpRECTL = IRECT(tmpDrawArea_LM, tmpDrawArea_TM, drawArea_LM, drawArea_TM);
								}
								if (j == 5) // LM to R
								{
									tmpSnapL = tmpDrawArea_LM - drawArea_R;
									tmpRECTL = IRECT(tmpDrawArea_LM, tmpDrawArea_TM, drawArea_R, drawArea_TM);
								}
								if (j == 6) // R to L
								{
									tmpSnapL = tmpDrawArea_R - drawArea_L;
									tmpRECTL = IRECT(tmpDrawArea_R, tmpDrawArea_TM, drawArea_L, drawArea_TM);
								}
								if (j == 7) // R to LM
								{
									tmpSnapL = tmpDrawArea_R - drawArea_LM;
									tmpRECTL = IRECT(tmpDrawArea_R, tmpDrawArea_TM, drawArea_LM, drawArea_TM);
								}
								if (j == 8) // R to R
								{
									tmpSnapL = tmpDrawArea_R - drawArea_R;
									tmpRECTL = IRECT(tmpDrawArea_R, tmpDrawArea_TM, drawArea_R, drawArea_TM);
								}

								// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
								if (tmpSnapL < snapSize && tmpSnapL >= 0)
								{
									snapMinL = IPMIN(snapMinL, tmpSnapL);

									if (snapMinL != prevsnapMinL)
									{
										lineMinL = tmpRECTL;
									}
									prevsnapMinL = snapMinL;

									didSnappedL = true;
								}
								if (tmpSnapL > -snapSize && tmpSnapL <= 0)
								{
									snapMaxL = IPMAX(snapMaxL, tmpSnapL);

									if (snapMaxL != prevsnapMaxL)
									{
										lineMaxL = tmpRECTL;
									}
									prevsnapMaxL = snapMaxL;

									didSnappedL = true;
								}
							}
						}

						if (didSnappedL)
						{
							if (snapMinL <= abs(snapMaxL))
							{
								lineL = lineMinL;
								snapL = snapMinL;
							}
							else
							{
								lineL = lineMaxL;
								snapL = snapMaxL;
							}

							// Snap control
							if (snapL != 0)
							{
								if (!liveClickedOnHandle)
								{
									drawArea.L = drawArea.L + snapL;
									drawArea.R = drawArea.R + snapL;
								}
								else drawArea.R = drawArea.R + snapL;

								if (!liveClickedOnHandle)
								{
									targetArea.L = targetArea.L + snapL;
									targetArea.R = targetArea.R + snapL;
								}
								else targetArea.R = targetArea.R + snapL;
							}

							// Draw snap line
							LICE_DashedLine(pDrawBitmap, lineL.L, lineL.T, lineL.R + snapL, lineL.B, 2, 2,
								LICE_RGBA(EDIT_COLOR.R, EDIT_COLOR.G, EDIT_COLOR.B, EDIT_COLOR.A));
						}

						for (int index = 0; index < controlSize; index++)
						{
							if (index == *liveMouseCapture) continue;

							IControl* pSnapControl = pControls->Get(index);
							IRECT tmpDrawArea = *pSnapControl->GetRECT();
							int tmpSnapT;
							IRECT tmpRECTT;

							int tmpDrawArea_L = tmpDrawArea.L;
							int tmpDrawArea_LM = tmpDrawArea.L + tmpDrawArea.W() / 2;
							int tmpDrawArea_R = tmpDrawArea.R;
							int tmpDrawArea_T = tmpDrawArea.T;
							int tmpDrawArea_TM = tmpDrawArea.T + tmpDrawArea.H() / 2;
							int tmpDrawArea_B = tmpDrawArea.B;

							int drawArea_L = drawArea.L;
							int drawArea_LM = drawArea.L + drawArea.W() / 2;
							int drawArea_R = drawArea.R;
							int drawArea_T = drawArea.T;
							int drawArea_TM = drawArea.T + drawArea.H() / 2;
							int drawArea_B = drawArea.B;

							// Find snap to T
							for (int j = 0; j < 9; j++)
							{
								if (liveClickedOnHandle)
								{
									if ((j / 3) * 3 == j) continue;
								}

								if (j == 0) // T to T
								{
									tmpSnapT = tmpDrawArea_T - drawArea_T;
									tmpRECTT = IRECT(tmpDrawArea_LM, tmpDrawArea_T, drawArea_LM, drawArea_T);
								}
								if (j == 1) // T to TM
								{
									tmpSnapT = tmpDrawArea_T - drawArea_TM;
									tmpRECTT = IRECT(tmpDrawArea_LM, tmpDrawArea_T, drawArea_LM, drawArea_TM);
								}
								if (j == 2) // T to B
								{
									tmpSnapT = tmpDrawArea_T - drawArea_B;
									tmpRECTT = IRECT(tmpDrawArea_LM, tmpDrawArea_T, drawArea_LM, drawArea_B);
								}
								if (j == 3) // TM to T
								{
									tmpSnapT = tmpDrawArea_TM - drawArea_T;
									tmpRECTT = IRECT(tmpDrawArea_LM, tmpDrawArea_TM, drawArea_LM, drawArea_T);
								}
								if (j == 4) // TM to TM
								{
									tmpSnapT = tmpDrawArea_TM - drawArea_TM;
									tmpRECTT = IRECT(tmpDrawArea_LM, tmpDrawArea_TM, drawArea_LM, drawArea_TM);
								}
								if (j == 5) // TM to B
								{
									tmpSnapT = tmpDrawArea_TM - drawArea_B;
									tmpRECTT = IRECT(tmpDrawArea_LM, tmpDrawArea_TM, drawArea_LM, drawArea_B);
								}
								if (j == 6) // B to T
								{
									tmpSnapT = tmpDrawArea_B - drawArea_T;
									tmpRECTT = IRECT(tmpDrawArea_LM, tmpDrawArea_B, drawArea_LM, drawArea_T);
								}
								if (j == 7) // B to TM
								{
									tmpSnapT = tmpDrawArea_B - drawArea_TM;
									tmpRECTT = IRECT(tmpDrawArea_LM, tmpDrawArea_B, drawArea_LM, drawArea_TM);
								}
								if (j == 8) // B to B
								{
									tmpSnapT = tmpDrawArea_B - drawArea_B;
									tmpRECTT = IRECT(tmpDrawArea_LM, tmpDrawArea_B, drawArea_LM, drawArea_B);
								}

								// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
								if (tmpSnapT < snapSize && tmpSnapT >= 0)
								{
									snapMinT = IPMIN(snapMinT, tmpSnapT);

									if (snapMinT != prevsnapMinT)
									{
										lineMinT = tmpRECTT;
									}
									prevsnapMinT = snapMinT;

									didSnappedT = true;
								}
								if (tmpSnapT > -snapSize && tmpSnapT <= 0)
								{
									snapMaxT = IPMAX(snapMaxT, tmpSnapT);

									if (snapMaxT != prevsnapMaxT)
									{
										lineMaxT = tmpRECTT;
									}
									prevsnapMaxT = snapMaxL;

									didSnappedT = true;
								}
							}
						}

						if (didSnappedT)
						{
							if (snapMinT <= abs(snapMaxT))
							{
								lineT = lineMinT;
								snapT = snapMinT;
							}
							else
							{
								lineT = lineMaxT;
								snapT = snapMaxT;
							}

							// Snap control
							if (snapT != 0)
							{
								if (!liveClickedOnHandle)
								{
									drawArea.T = drawArea.T + snapT;
									drawArea.B = drawArea.B + snapT;
								}
								else drawArea.B = drawArea.B + snapT;


								if (!liveClickedOnHandle)
								{
									targetArea.T = targetArea.T + snapT;
									targetArea.B = targetArea.B + snapT;
								}
								else targetArea.B = targetArea.B + snapT;
							}

							// Draw snap line
							LICE_DashedLine(pDrawBitmap, lineT.L, lineT.T, lineT.R, lineT.B + snapT, 2, 2,
								LICE_RGBA(EDIT_COLOR.R, EDIT_COLOR.G, EDIT_COLOR.B, EDIT_COLOR.A));
						}
					}

					// Prevent moving background
					if (liveControlNumber > 0)
					{
						pControl->SetDrawRECT(drawArea);
						pControl->SetTargetRECT(targetArea);
					}

					liveSelectedRECT = drawArea;
				}
			}

			*liveMouseDragging = false;
			lastliveMouseCapture = *liveMouseCapture;
		}

		if (*liveToogleEditing)
		{
			// Outline selected rect
			if (liveControlNumber > 0) pGraphics->DrawRect(&EDIT_COLOR, pControls->Get(liveControlNumber)->GetRECT());

			if (drawGridToogle) DrawGrid(pDrawBitmap, width, height);

			// Check if gui resize is active, if so scale out rect
			IRECT printRECT;
			if (pPlug->GetGUIResize())
			{
				printRECT.L = int((double)liveSelectedRECT.L * guiScaleRatio);
				printRECT.T = int((double)liveSelectedRECT.T * guiScaleRatio);
				printRECT.R = int((double)liveSelectedRECT.R * guiScaleRatio);
				printRECT.B = int((double)liveSelectedRECT.B * guiScaleRatio);
			}
			else
			{
				printRECT = liveSelectedRECT;
			}

			// Print selected control
			int textSize = 15;
			WDL_String controlNumber;

			// Different outputs if control is hidden
			if (liveControlNumber > 0 && !pGraphics->GetControl(liveControlNumber)->IsHidden())
				controlNumber.SetFormatted(100, "N:%i IRECT(%i,%i,%i,%i)", liveControlNumber, printRECT.L, printRECT.T, printRECT.R, printRECT.B);
			if (liveControlNumber > 0 && pGraphics->GetControl(liveControlNumber)->IsHidden())
				controlNumber.SetFormatted(100, "N:%i IRECT(%i,%i,%i,%i) (HIDDEN)", liveControlNumber, printRECT.L, printRECT.T, printRECT.R, printRECT.B);

			IText txtControlNumber(textSize, &EDIT_COLOR, defaultFont, IText::kStyleNormal, IText::kAlignNear, 0, IText::kQualityClearType);
			IRECT textRect;
			if (liveControlNumber > 0) pGraphics->MeasureIText(&txtControlNumber, controlNumber.Get(), &textRect);
			IRECT rectControlNumber(liveSelectedRECT.L, liveSelectedRECT.T - textSize, liveSelectedRECT.L + textRect.R, liveSelectedRECT.T);

			if (rectControlNumber.T < 0)
			{
				rectControlNumber.T = liveSelectedRECT.B;
				rectControlNumber.B = liveSelectedRECT.B + textSize;
			}

			if (liveControlNumber > 0) pGraphics->FillIRect(&controlTextBackgroundColor, &rectControlNumber);
			if (liveControlNumber > 0) pGraphics->DrawIText(&txtControlNumber, controlNumber.Get(), &rectControlNumber);
			
			if (liveEditingMod->R) DoPopupMenu(pPlug, pGraphics, *mMouseX, *mMouseY, guiScaleRatio);

			// Write to file
			CreateLayoutCode(pPlug, pGraphics, guiScaleRatio, viewMode);
		}
	}

	void BackupIRECTs(IPlugBase* pPlug, IGraphics* pGraphics, const char* category, double guiScaleRatio)
	{
		int controlSize = pGraphics->GetNControls();
		if (pPlug->GetGUIResize()) controlSize -= 3;
		
		for (int i = 1; i < controlSize; i++)
		{
			IControl* pControl = pGraphics->GetControl(i);
			IRECT drawRECT = *pControl->GetRECT();
			IRECT targetRECT = *pControl->GetTargetRECT();

			if (pPlug->GetGUIResize())
			{
				drawRECT.L = int((double)drawRECT.L * guiScaleRatio);
				drawRECT.T = int((double)drawRECT.T * guiScaleRatio);
				drawRECT.R = int((double)drawRECT.R * guiScaleRatio);
				drawRECT.B = int((double)drawRECT.B * guiScaleRatio);

				targetRECT.L = int((double)targetRECT.L * guiScaleRatio);
				targetRECT.T = int((double)targetRECT.T * guiScaleRatio);
				targetRECT.R = int((double)targetRECT.R * guiScaleRatio);
				targetRECT.B = int((double)targetRECT.B * guiScaleRatio);
			}

			WDL_String IRECT_Position;
			WDL_String IRECT_Name;
			IRECT_Name.SetFormatted(128, "IRECT%i", i);
			
			// Set L
			WDL_String IRECT_NameDL;
			IRECT_NameDL.Set(IRECT_Name.Get(), 128);
			IRECT_NameDL.Append("_drawL");
			IRECT_Position.SetFormatted(128, "%i", drawRECT.L);
			SetIntToFile(category, IRECT_NameDL.Get(), IRECT_Position.Get());

			WDL_String IRECT_NameTL;
			IRECT_NameTL.Set(IRECT_Name.Get(), 128);
			IRECT_NameTL.Append("_targetL");
			IRECT_Position.SetFormatted(128, "%i", targetRECT.L);
			SetIntToFile(category, IRECT_NameTL.Get(), IRECT_Position.Get());

			// Set T
			WDL_String IRECT_NameDT;
			IRECT_NameDT.Set(IRECT_Name.Get(), 128);
			IRECT_NameDT.Append("_drawT");
			IRECT_Position.SetFormatted(128, "%i", drawRECT.T);
			SetIntToFile(category, IRECT_NameDT.Get(), IRECT_Position.Get());

			WDL_String IRECT_NameTT;
			IRECT_NameTT.Set(IRECT_Name.Get(), 128);
			IRECT_NameTT.Append("_targetT");
			IRECT_Position.SetFormatted(128, "%i", targetRECT.T);
			SetIntToFile(category, IRECT_NameTT.Get(), IRECT_Position.Get());

			// Set R
			WDL_String IRECT_NameDR;
			IRECT_NameDR.Set(IRECT_Name.Get(), 128);
			IRECT_NameDR.Append("_drawR");
			IRECT_Position.SetFormatted(128, "%i", drawRECT.R);
			SetIntToFile(category, IRECT_NameDR.Get(), IRECT_Position.Get());

			WDL_String IRECT_NameTR;
			IRECT_NameTR.Set(IRECT_Name.Get(), 128);
			IRECT_NameTR.Append("_targetR");
			IRECT_Position.SetFormatted(128, "%i", targetRECT.R);
			SetIntToFile(category, IRECT_NameTR.Get(), IRECT_Position.Get());

			// Set B
			WDL_String IRECT_NameDB;
			IRECT_NameDB.Set(IRECT_Name.Get(), 128);
			IRECT_NameDB.Append("_drawB");
			IRECT_Position.SetFormatted(128, "%i", drawRECT.B);
			SetIntToFile(category, IRECT_NameDB.Get(), IRECT_Position.Get());

			WDL_String IRECT_NameTB;
			IRECT_NameTB.Set(IRECT_Name.Get(), 128);
			IRECT_NameTB.Append("_targetB");
			IRECT_Position.SetFormatted(128, "%i", targetRECT.B);
			SetIntToFile(category, IRECT_NameTB.Get(), IRECT_Position.Get());
		}
	}

	void LoadIRECTsFromFile(IPlugBase* pPlug, IGraphics* pGraphics, const char* category)

	{
		int controlSize = pGraphics->GetNControls();
		if (pPlug->GetGUIResize()) controlSize -= 3;

		for (int i = 1; i < controlSize; i++)
		{
			IControl* pControl = pGraphics->GetControl(i);
			IRECT drawRECT;
			IRECT targetRECT;

			WDL_String IRECT_Name;
			IRECT_Name.SetFormatted(128, "IRECT%i", i);

			// Set L
			WDL_String IRECT_NameDL;
			IRECT_NameDL.Set(IRECT_Name.Get(), 128);
			IRECT_NameDL.Append("_drawL");
			drawRECT.L = GetIntFromFile(category, IRECT_NameDL.Get());

			WDL_String IRECT_NameTL;
			IRECT_NameTL.Set(IRECT_Name.Get(), 128);
			IRECT_NameTL.Append("_targetL");
			targetRECT.L = GetIntFromFile(category, IRECT_NameTL.Get());

			// Set T
			WDL_String IRECT_NameDT;
			IRECT_NameDT.Set(IRECT_Name.Get(), 128);
			IRECT_NameDT.Append("_drawT");
			drawRECT.T = GetIntFromFile(category, IRECT_NameDT.Get());

			WDL_String IRECT_NameTT;
			IRECT_NameTT.Set(IRECT_Name.Get(), 128);
			IRECT_NameTT.Append("_targetT");
			targetRECT.T = GetIntFromFile(category, IRECT_NameTT.Get());

			// Set R
			WDL_String IRECT_NameDR;
			IRECT_NameDR.Set(IRECT_Name.Get(), 128);
			IRECT_NameDR.Append("_drawR");
			drawRECT.R = GetIntFromFile(category, IRECT_NameDR.Get());

			WDL_String IRECT_NameTR;
			IRECT_NameTR.Set(IRECT_Name.Get(), 128);
			IRECT_NameTR.Append("_targetR");
			targetRECT.R = GetIntFromFile(category, IRECT_NameTR.Get());

			// Set B
			WDL_String IRECT_NameDB;
			IRECT_NameDB.Set(IRECT_Name.Get(), 128);
			IRECT_NameDB.Append("_drawB");
			drawRECT.B = GetIntFromFile(category, IRECT_NameDB.Get());

			WDL_String IRECT_NameTB;
			IRECT_NameTB.Set(IRECT_Name.Get(), 128);
			IRECT_NameTB.Append("_targetB");
			targetRECT.B = GetIntFromFile(category, IRECT_NameTB.Get());

			if (drawRECT.L >= 0 && drawRECT.T >= 0 && drawRECT.R >= 0 && drawRECT.B >= 0)
			{
				if (targetRECT.L >= 0 && targetRECT.T >= 0 && targetRECT.R >= 0 && targetRECT.B >= 0)
				{
					pControl->SetDrawRECT(drawRECT);
					pControl->SetTargetRECT(targetRECT);
				}
			}
		}
	}

	void SetIntToFile(const char * category, const char * variable_name, const char * variable_value)
	{
		WritePrivateProfileString(category, variable_name, variable_value, "C:/LiveOut.txt");
	}

	int GetIntFromFile(const char * category, const char * variable_name)
	{
		return GetPrivateProfileInt(category, variable_name, -1, "C:/LiveOut.txt");
	}

	void WriteToTextFile(const char* data)
	{
		ofstream myfile;
		myfile.open("LiveEditLayout.h");
		myfile << data;
		myfile.close();
	}

	void CreateLayoutCode(IPlugBase* pPlug, IGraphics* pGraphics, double guiScaleRatio, int viewMode)
	{
		string code;

		code = 
			"// Do not edit. All of this is generated automatically \n"
			"// Copyright Youlean 2016 \n \n"
            "#include <vector>\n"
			"#include \"IGraphics.h\" \n"
			"#include \"IPlugGUIResize.h\" \n\n"
			"class LiveEditLayout \n"
			"{ \n"
			"public: \n"
			"	LiveEditLayout() {} \n \n"
			"	~LiveEditLayout() {} \n \n"
			"	void SetControlPositions(IGraphics* pGraphics) \n"
			"	{ \n"
			"	    // Backup original control pointers\n"
			"		for (int i = 0; i < pGraphics->GetNControls(); i++) \n"
			"			originalPointers.push_back(pGraphics->GetControl(i));\n"
			"\n"
			"	    // --------------------------------------------------------------------\n\n"
			;

		
		// Write all controls positions
		int controlSize = default_layers.size();
		if (pPlug->GetGUIResize()) controlSize -= 3;

		for (int i = 1; i < controlSize; i++)
		{
			IControl* pControl = default_layers[i];
			IRECT drawRECT = *pControl->GetRECT();
			IRECT targetRECT = *pControl->GetTargetRECT();

			if (pPlug->GetGUIResize())
			{
				drawRECT.L = int((double)drawRECT.L * guiScaleRatio);
				drawRECT.T = int((double)drawRECT.T * guiScaleRatio);
				drawRECT.R = int((double)drawRECT.R * guiScaleRatio);
				drawRECT.B = int((double)drawRECT.B * guiScaleRatio);

				targetRECT.L = int((double)targetRECT.L * guiScaleRatio);
				targetRECT.T = int((double)targetRECT.T * guiScaleRatio);
				targetRECT.R = int((double)targetRECT.R * guiScaleRatio);
				targetRECT.B = int((double)targetRECT.B * guiScaleRatio);
			}

			WDL_String drawValue, targetValue, hiddenValue;

			// Get derived class name
			code.append("        // ");
			code.append(pControl->GetDerivedClassName());
			code.append("\n");
			
			hiddenValue.SetFormatted(128, "		pGraphics->GetControl(%i)->Hide(", i);
			if (pControl->IsHidden()) hiddenValue.Append("true");
			else hiddenValue.Append("false");
			hiddenValue.Append("); \n");
			code.append(hiddenValue.Get());

			drawValue.SetFormatted(128, "		pGraphics->GetControl(%i)->SetDrawRECT(IRECT(%i, %i, %i, %i)); \n", i, drawRECT.L, drawRECT.T, drawRECT.R, drawRECT.B);
			code.append(drawValue.Get());

			targetValue.SetFormatted(128, "		pGraphics->GetControl(%i)->SetTargetRECT(IRECT(%i, %i, %i, %i)); \n", i, targetRECT.L, targetRECT.T, targetRECT.R, targetRECT.B);
			code.append(targetValue.Get());

			code.append("\n");
		}

		// Reordering control layers
		code.append
		(
			"	    // --------------------------------------------------------------------\n\n"
			"	    // Reordering control layers\n"
		);

		WDL_String layoutMove;

		// Backup current control layers
		for (int i = 0; i < current_layers.size(); i++)
			current_layers[i] = pGraphics->GetControl(i);

		for (int i = 0; i < controlSize; i++)
		{
			IControl* pControl = default_layers[i];
			layoutMove.SetFormatted(128, "		pGraphics->ReplaceControl(%i, originalPointers[%i]); \n", FindPointerPosition(pControl, current_layers), i);
			code.append(layoutMove.Get());
		}
		

		code.append("	}\n\n");

		// If GUI resize is enabled
		code.append
		(
			"	void SetGUIResizeLayout(IGraphics* pGraphics, IPlugGUIResize* pGUIResize)\n"
			"	{\n"
		);


		// Set GUI Resize code -----------------------------------------------------------------------------------------------------------
		if (pPlug->GetGUIResize())
		{
			code_view_mode.resize(IPMAX(viewMode + 1, code_view_mode.size()));

			code.append
			(
				"		IControl* pControl;\n"
			);

			code_view_mode[viewMode].clear();
			WDL_String getC, viewCode;
			for (int i = 0; i < controlSize; i++)
			{
				IControl* pControl = default_layers[i];
				IRECT drawRECT = *pControl->GetRECT();
				IRECT targetRECT = *pControl->GetTargetRECT();

				if (pPlug->GetGUIResize())
				{
					drawRECT.L = int((double)drawRECT.L * guiScaleRatio);
					drawRECT.T = int((double)drawRECT.T * guiScaleRatio);
					drawRECT.R = int((double)drawRECT.R * guiScaleRatio);
					drawRECT.B = int((double)drawRECT.B * guiScaleRatio);

					targetRECT.L = int((double)targetRECT.L * guiScaleRatio);
					targetRECT.T = int((double)targetRECT.T * guiScaleRatio);
					targetRECT.R = int((double)targetRECT.R * guiScaleRatio);
					targetRECT.B = int((double)targetRECT.B * guiScaleRatio);
				}

				getC.SetFormatted(128, "		pControl = pGraphics->GetControl(%i); \n", FindPointerPosition(pControl, current_layers));

				viewCode.SetFormatted(128, "		pGUIResize->LiveEditSetLayout(%i, pControl, IRECT(%i, %i, %i, %i), IRECT(%i, %i, %i, %i)",
					viewMode, drawRECT.L, drawRECT.T, drawRECT.R, drawRECT.B, targetRECT.L, targetRECT.T, targetRECT.R, targetRECT.B);
				if (pControl->IsHidden()) viewCode.Append(", true);\n");
				else  viewCode.Append(", false);\n");

				code_view_mode[viewMode].append(getC.Get());
				code_view_mode[viewMode].append(viewCode.Get());

				// Update current view mode
				IControl* tmpControl = pGraphics->GetControl(FindPointerPosition(pControl, current_layers));
				pPlug->GetGUIResize()->LiveEditSetLayout(
					viewMode, 
					tmpControl, 
					IRECT(drawRECT.L, drawRECT.T, drawRECT.R, drawRECT.B), 
					IRECT(targetRECT.L, targetRECT.T, targetRECT.R, targetRECT.B), 
					pControl->IsHidden());
			}

			// Append all view code code
			for (int i = 0; i < code_view_mode.size(); i++)
			{
				code.append("\n");
				WDL_String mode;
				mode.SetFormatted(128, "		// View Mode: (%i) ------------------------------------------------------------------------------------------------\n", i);
				code.append(mode.Get());
				code.append("\n");
				code.append(code_view_mode[i]);
				code.append("\n");
			}
		}


		code.append("    }\n\n");
		// End
		code.append
		(
			"private:\n"
			"	std::vector <IControl*> originalPointers;\n"

		);

		code.append("}; ");
		WriteToTextFile(code.c_str());
	}

	void DrawGrid(LICE_IBitmap* pDrawBitmap, int width, int height)
	{
		if (liveScaledGridSize > 1)
		{
			// Vertical Lines grid
			for (int i = 0; i < width; i += liveScaledGridSize)
			{
				LICE_Line(pDrawBitmap, i, 0, i, height,
					LICE_RGBA(EDIT_COLOR.R, EDIT_COLOR.G, EDIT_COLOR.B, EDIT_COLOR.A), 0.17f);
			}

			// Horisontal Lines grid
			for (int i = 0; i < height; i += liveScaledGridSize)
			{
				LICE_Line(pDrawBitmap, 0, i, width, i,
					LICE_RGBA(EDIT_COLOR.R, EDIT_COLOR.G, EDIT_COLOR.B, EDIT_COLOR.A), 0.17f);
			}
		}
		else
		{
			LICE_FillRect(pDrawBitmap, 0, 0, width, height,
				LICE_RGBA(EDIT_COLOR.R, EDIT_COLOR.G, EDIT_COLOR.B, EDIT_COLOR.A), 0.11f);
		}
	}

	void DoPopupMenu(IPlugBase* pPlug, IGraphics* pGraphics, int x, int y, double guiScaleRatio)
	{
		IPopupMenu menu;
		
		// Item 0
		menu.AddItem("Reset Control Position");
		if (liveControlNumber <= 0) menu.SetItemState(0, IPopupMenuItem::kDisabled, true);

		// Item 1
		menu.AddItem("Reset Control Size");
		if (liveControlNumber <= 0) menu.SetItemState(1, IPopupMenuItem::kDisabled, true);

		// Item 2
		if (pGraphics->GetControl(liveControlNumber)->IsHidden()) menu.AddItem("Show Control");
		else menu.AddItem("Hide Control");
		if (liveControlNumber <= 0) menu.SetItemState(2, IPopupMenuItem::kDisabled, true);

		// Item 3
		menu.AddSeparator();

		// Item 4
		menu.AddItem("Bring to Front");
		if (liveControlNumber <= 0) menu.SetItemState(4, IPopupMenuItem::kDisabled, true);

		// Item 5
		menu.AddItem("Send to Back");
		if (liveControlNumber <= 0) menu.SetItemState(5, IPopupMenuItem::kDisabled, true);

		// Item 6
		menu.AddSeparator();
		
		// Item 7
		menu.AddItem("Bring Forward");
		if (liveControlNumber <= 0) menu.SetItemState(7, IPopupMenuItem::kDisabled, true);

		// Item 8
		menu.AddItem("Send Backward");
		if (liveControlNumber <= 0) menu.SetItemState(8, IPopupMenuItem::kDisabled, true);

		// Item 9
		menu.AddSeparator();

		// Item 10
		if (drawGridToogle) menu.AddItem("Hide Grid");
		else menu.AddItem("Show Grid");

		// Item 11
		menu.AddSeparator();

		// Item 12
		menu.AddItem("Clear All Edits");

		if (pGraphics->CreateIPopupMenu(&menu, x, y))
		{
			int itemChosen = menu.GetChosenItemIdx();
			
			// Reset Control Position
			if (itemChosen == 0)
			{
				if (liveControlNumber > 0)
				{
					IRECT drawRECT;
					IRECT targetRECT;

					drawRECT.L = int((double)default_draw_rect[liveControlNumber].L * guiScaleRatio);
					drawRECT.T = int((double)default_draw_rect[liveControlNumber].T * guiScaleRatio);
					drawRECT.R = drawRECT.L + liveSelectedRECT.W();
					drawRECT.B = drawRECT.T + liveSelectedRECT.H();

					targetRECT.L = int((double)default_terget_rect[liveControlNumber].L * guiScaleRatio);
					targetRECT.T = int((double)default_terget_rect[liveControlNumber].T * guiScaleRatio);
					targetRECT.R = targetRECT.L + liveSelectedTargetRECT.W();
					targetRECT.B = targetRECT.T + liveSelectedTargetRECT.H();

					liveSelectedRECT = drawRECT;
					liveSelectedTargetRECT = targetRECT;

					pGraphics->GetControl(liveControlNumber)->SetDrawRECT(drawRECT);
					pGraphics->GetControl(liveControlNumber)->SetTargetRECT(targetRECT);
				}
			}

			// Reset Control Size
			if (itemChosen == 1)
			{
				IRECT drawRECT;
					IRECT targetRECT;

					drawRECT.L = liveSelectedRECT.L;
					drawRECT.T = liveSelectedRECT.T;
					drawRECT.R = drawRECT.L + int((double)default_draw_rect[liveControlNumber].W() * guiScaleRatio);
					drawRECT.B = drawRECT.T + int((double)default_draw_rect[liveControlNumber].H() * guiScaleRatio);

					targetRECT.L = liveSelectedTargetRECT.L;
					targetRECT.T = liveSelectedTargetRECT.T;
					targetRECT.R = targetRECT.L + int((double)default_terget_rect[liveControlNumber].W() * guiScaleRatio);
					targetRECT.B = targetRECT.T +int((double)default_terget_rect[liveControlNumber].H() * guiScaleRatio);

					liveSelectedRECT = drawRECT;
					liveSelectedTargetRECT = targetRECT;

					pGraphics->GetControl(liveControlNumber)->SetDrawRECT(drawRECT);
					pGraphics->GetControl(liveControlNumber)->SetTargetRECT(targetRECT);
			}

			// Show/Hide Control
			if (itemChosen == 2)
			{
					if (pGraphics->GetControl(liveControlNumber)->IsHidden()) pGraphics->GetControl(liveControlNumber)->Hide(false);
					else pGraphics->GetControl(liveControlNumber)->Hide(true);
			}

			// Bring to Front
			if (itemChosen == 4)
			{
					int controlSize = pGraphics->GetNControls();
					if (pPlug->GetGUIResize()) controlSize -= 3;

					control_move_from.push_back(liveControlNumber);
					control_move_to.push_back(controlSize - 1);
					liveControlNumber = control_move_to.back();

					pGraphics->MoveControlLayers(control_move_from.back(), control_move_to.back());
			}

			// Send to Back
			if (itemChosen == 5)
			{
					int controlSize = pGraphics->GetNControls();
					if (pPlug->GetGUIResize()) controlSize -= 3;

					control_move_from.push_back(liveControlNumber);
					control_move_to.push_back(1);
					liveControlNumber = control_move_to.back();

					pGraphics->MoveControlLayers(control_move_from.back(), control_move_to.back());
			}

			// Bring Forward
			if (itemChosen == 7)
			{
					int controlSize = pGraphics->GetNControls();
					if (pPlug->GetGUIResize()) controlSize -= 3;

					if (liveControlNumber + 1 < controlSize)
					{
						control_move_from.push_back(liveControlNumber);
						control_move_to.push_back(liveControlNumber + 1);
						liveControlNumber = control_move_to.back();

						pGraphics->SwapControlLayers(control_move_from.back(), control_move_to.back());
					}
			}

			// Send Backward
			if (itemChosen == 8)
			{
					int controlSize = pGraphics->GetNControls();
					if (pPlug->GetGUIResize()) controlSize -= 3;

					if (liveControlNumber - 1 > 0)
					{
						control_move_from.push_back(liveControlNumber);
						control_move_to.push_back(liveControlNumber - 1);
						liveControlNumber = control_move_to.back();

						pGraphics->SwapControlLayers(control_move_from.back(), control_move_to.back());
					}
			}

			// Show/Hide Grid
			if (itemChosen == 10)
			{
				if (drawGridToogle) drawGridToogle = false;
				else drawGridToogle = true;
			}

			// Clear Edits
			if (itemChosen == 12)
			{
				for (int i = 0; i < pGraphics->GetNControls(); i++)
				{
					pGraphics->ReplaceControl(i, default_layers[i]);
					IControl* pControl = pGraphics->GetControl(i);
					pControl->SetDrawRECT(default_draw_rect[i]);
					pControl->SetTargetRECT(default_terget_rect[i]);
					pControl->Hide(!default_is_hidden[i]);
				}

				liveControlNumber = -1;
				liveClickedRECT = IRECT(0, 0, 0, 0);
				liveClickedTargetRECT = IRECT(0, 0, 0, 0);
				
				// Write to file
				CreateLayoutCode(pPlug, pGraphics, guiScaleRatio, viewMode);
			}
		}
	}

	void StoreDefaults(IGraphics* pGraphics)
	{
		for (int i = 0; i < pGraphics->GetNControls(); i++)
		{
			IControl* pControl = pGraphics->GetControl(i);
			IRECT drawRECT = *pControl->GetRECT();
			IRECT targetRECT = *pControl->GetTargetRECT();

			default_draw_rect.push_back(drawRECT);
			default_terget_rect.push_back(targetRECT);
			default_is_hidden.push_back(!pControl->IsHidden());
			default_layers.push_back(pControl);
			current_layers.push_back(pControl);
		}
	}

	void UndoMovingControlLayers(IGraphics* pGraphics)
	{
		for (int i = control_move_to.size() - 1; i > 0; i--)
		{
			pGraphics->MoveControlLayers(control_move_to[i], control_move_from[i]);
		}
	}

	void DoMovingControlLayers(IGraphics* pGraphics)
	{
		for (int i = 0; i < control_move_to.size(); i++)
		{
			pGraphics->MoveControlLayers(control_move_to[i], control_move_from[i]);
		}
	}

	void GetMouseOverControl(IPlugBase* pPlug, IGraphics* pGraphics, int mouseX, int mouseY)
	{
		int controlSize = pGraphics->GetNControls();
		if (pPlug->GetGUIResize()) controlSize -= 3;

		for (int i = controlSize; i >= 0; i--)
		{
			IControl* pControl = pGraphics->GetControl(i);
			pControl->IsHit(mouseX, mouseY);

			mouseOverControl = i;
			return;
		}
		mouseOverControl = -1;
	}

	int FindPointerPosition(IControl* pControl, vector <IControl*> vControl)
	{
		for (int i = 0; i < vControl.size(); i++)
		{
			if (pControl == vControl[i]) return i;
		}
		return -1;
	}

private:
	// Live editing stuff
	char buf[512]; // temp buffer for writing integers to profile strings
	char* defaultFont = "Tahoma";
	IColor EDIT_COLOR = IColor(255, 255, 255, 255);
	IColor controlTextBackgroundColor = IColor(122, 0, 0, 0);
	IRECT liveSelectedRECT = IRECT(0, 0, 0, 0);
	IRECT liveSelectedTargetRECT = IRECT(0, 0, 0, 0);
	IRECT liveClickedRECT = IRECT(0, 0, 0, 0);
	IRECT liveClickedTargetRECT = IRECT(0, 0, 0, 0);
	int liveControlNumber = -1;
	int lastliveMouseCapture = -1;
	int liveClickedX = 0, liveClickedY = 0;
	int liveScaledGridSize = 1;
	bool liveClickedOnHandle = false;
	bool liveLastMouseDownL = false;
	int liveMouseXLock = 0;
	int liveMouseYLock = 0;
	bool drawGridToogle = false;
	int mouseOverControl = -1;
	vector <IRECT> default_draw_rect;
	vector <IRECT> default_terget_rect;
	vector <bool> default_is_hidden;
	vector <IControl*> default_layers;
	vector <bool> control_visibility;
	vector <int> control_move_from;
	vector <int> control_move_to;
	vector <IControl*> current_layers;
	vector <string> code_view_mode;
	int viewMode = 0;
};
#endif