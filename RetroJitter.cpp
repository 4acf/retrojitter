/*******************************************************************/
/*                                                                 */
/*                      ADOBE CONFIDENTIAL                         */
/*                   _ _ _ _ _ _ _ _ _ _ _ _ _                     */
/*                                                                 */
/* Copyright 2007 Adobe Systems Incorporated                       */
/* All Rights Reserved.                                            */
/*                                                                 */
/* NOTICE:  All information contained herein is, and remains the   */
/* property of Adobe Systems Incorporated and its suppliers, if    */
/* any.  The intellectual and technical concepts contained         */
/* herein are proprietary to Adobe Systems Incorporated and its    */
/* suppliers and may be covered by U.S. and Foreign Patents,       */
/* patents in process, and are protected by trade secret or        */
/* copyright law.  Dissemination of this information or            */
/* reproduction of this material is strictly forbidden unless      */
/* prior written permission is obtained from Adobe Systems         */
/* Incorporated.                                                   */
/*                                                                 */
/*******************************************************************/

/*	RetroJitter.cpp	

	This is a compiling husk of a project. Fill it in with interesting
	pixel processing code.
	
	Revision History

	Version		Change													Engineer	Date
	=======		======													========	======
	1.0			(seemed like a good idea at the time)					bbb			6/1/2002

	1.0			Okay, I'm leaving the version at 1.0,					bbb			2/15/2006
				for obvious reasons; you're going to 
				copy these files directly! This is the
				first XCode version, though.

	1.0			Let's simplify this barebones sample					zal			11/11/2010

	1.0			Added new entry point									zal			9/18/2017

*/

#include "RetroJitter.h"

bool invertflag = true;

static PF_Err 
About (	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	AEGP_SuiteHandler suites(in_data->pica_basicP);
	
	suites.ANSICallbacksSuite1()->sprintf(	out_data->return_msg,
											"%s v%d.%d\r%s",
											STR(StrID_Name), 
											MAJOR_VERSION, 
											MINOR_VERSION, 
											STR(StrID_Description));
	return PF_Err_NONE;
}

static PF_Err 
GlobalSetup (	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	out_data->my_version = PF_VERSION(	MAJOR_VERSION, 
										MINOR_VERSION,
										BUG_VERSION, 
										STAGE_VERSION, 
										BUILD_VERSION);

	out_data->out_flags =  PF_OutFlag_DEEP_COLOR_AWARE;	// just 16bpc, not 32bpc

	return PF_Err_NONE;
}

static PF_Err 
ParamsSetup (	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	PF_Err		err		= PF_Err_NONE;
	PF_ParamDef	def;	

	AEFX_CLR_STRUCT(def);

	PF_ADD_FLOAT_SLIDERX(	STR(StrID_Resolution_Param_Name), 
							RETROJITTER_RESOLUTION_MIN, 
							RETROJITTER_RESOLUTION_MAX, 
							RETROJITTER_RESOLUTION_MIN, 
							RETROJITTER_RESOLUTION_MAX, 
							RETROJITTER_RESOLUTION_DFLT,
							PF_Precision_INTEGER,
							0,
							0,
							RESOLUTION_DISK_ID);

	
	AEFX_CLR_STRUCT(def);

	PF_ADD_FLOAT_SLIDERX(	STR(StrID_Distance_Param_Name), 
							RETROJITTER_DISTANCE_MIN,
							RETROJITTER_DISTANCE_MAX,
							RETROJITTER_DISTANCE_MIN,
							RETROJITTER_DISTANCE_MAX,
							RETROJITTER_DISTANCE_DFLT,
							PF_Precision_TENTHS,
							0,
							0,
							DISTANCE_DISK_ID);

	AEFX_CLR_STRUCT(def);

	PF_ADD_FLOAT_SLIDERX(	STR(StrID_Speed_Param_Name),
							RETROJITTER_SPEED_MIN,
							RETROJITTER_SPEED_MAX,
							RETROJITTER_SPEED_MIN,
							RETROJITTER_SPEED_MAX,
							RETROJITTER_SPEED_DFLT,
							PF_Precision_INTEGER,
							0,
							0,
							SPEED_DISK_ID);	

	AEFX_CLR_STRUCT(def);

	PF_ADD_CHECKBOX(		"Enable Map Preview",
							"", 
							RETROJITTER_PREVIEW_DFLT,
							NULL,
							PREVIEW_DISK_ID);
	
	out_data->num_params = RETROJITTER_NUM_PARAMS;

	return err;
}

static PF_Err
RetroJitter8InvertMap (
	void		*refcon, 
	A_long		xL, 
	A_long		yL, 
	PF_Pixel8	*inP, 
	PF_Pixel8	*outP)
{
	PF_Err		err = PF_Err_NONE;

	ParamInfo	*piP	= reinterpret_cast<ParamInfo*>(refcon);

	outP->red	= abs(outP->red	  - 255);
	outP->green = abs(outP->green - 255);
	outP->blue	= abs(outP->blue  - 255);

	return err;
}

//8 bpc pixel sampling
PF_Pixel* sampleIntegral32(PF_EffectWorld& def, int x, int y) {

	return (PF_Pixel*)((char*)def.data +
		(y * def.rowbytes) +
		(x * sizeof(PF_Pixel)));

}

void UpdateColor8(int x, int y, float distance, PF_EffectWorld *output, PF_EffectWorld *map, PF_LayerDef *ld) {

	PF_Pixel* outputPixel = sampleIntegral32(*output, x, y);
	PF_Pixel* mapPixel	  =	sampleIntegral32(*map, x, y);
	
	//handle edge pixels
	if (x + distance > ld->width || x - distance < 0) {
		PF_Pixel* ldPixel = sampleIntegral32(*ld, x, y);
		outputPixel->red = (distance - floor(distance)) * ldPixel->red;
		outputPixel->green = (distance - floor(distance)) * ldPixel->green;
		outputPixel->blue = (distance - floor(distance)) * ldPixel->blue;
	}

	else {
	
		if (mapPixel->red == 0)
			distance *= -1;		//displacement on black lines goes opposite direction
			//distance = 0;		//black lines are ignored

		//integer distances (no color blending needed)
		if (distance == floor(distance)) {

			PF_Pixel* ldPixel = sampleIntegral32(*ld, x + distance, y);

			outputPixel->red = ldPixel->red;
			outputPixel->green = ldPixel->green;
			outputPixel->blue = ldPixel->blue;

		}

		//distance is not an integer value, colors between the pixels are blended together
		else {

			float rightFloat = distance - floor(distance);
			float leftFloat = 1 - rightFloat;

			PF_Pixel* rightldPixel = sampleIntegral32(*ld, x + floor(distance) + 1, y);
			PF_Pixel* leftldPixel = sampleIntegral32(*ld, x + floor(distance), y);

			outputPixel->red = (rightFloat * rightldPixel->red) + (leftFloat * leftldPixel->red);
			outputPixel->green = (rightFloat * rightldPixel->green) + (leftFloat * leftldPixel->green);
			outputPixel->blue = (rightFloat * rightldPixel->blue) + (leftFloat * leftldPixel->blue);

		}

	}

}

//Bresenham's Algorithm, works with diagonal lines which means it is a bit overkill for horizontal but who care
void DrawLine(int x0, int y0, int x1, int y1, PF_EffectWorld *output, bool white) {

	int dx, dy, p, x, y;
	dx = x1 - x0;
	dy = y1 - y0;

	x = x0;
	y = y0;

	p = 2 * dy - dx;

	while (x < x1) {

		if (p >= 0) {

			PF_Pixel* p = sampleIntegral32(*output, x, y);
			p->red = 255 * (int)white;
			p->green = 255 * (int)white;
			p->blue = 255 * (int)white;
			p->alpha = 255;

			y = y + 1;
			p = p + 2 * dy - 2 * dx;

		}

		else {

			PF_Pixel* p = sampleIntegral32(*output, x, y);
			p->red = 255 * (int)white;
			p->green = 255 * (int)white;
			p->blue = 255 * (int)white;
			p->alpha = 255;

			p = p + 2 * dy;

		}

		x = x + 1;

	}
}

static PF_Err 
Render (
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	PF_Err				err		= PF_Err_NONE;
	AEGP_SuiteHandler	suites(in_data->pica_basicP);
	ParamInfo			piP;
	AEFX_CLR_STRUCT(piP);
	A_long				linesL	= 0;

	PF_EffectWorld		map;
	AEFX_CLR_STRUCT(map);

	ERR(PF_NEW_WORLD(output->width, output->height, PF_PixelFormat_ARGB32, &map));

	piP.resF 	= params[RETROJITTER_RESOLUTION]->u.fs_d.value;
	piP.distF	= params[RETROJITTER_DISTANCE]->u.fs_d.value;
	piP.spdF	= params[RETROJITTER_SPEED]->u.fs_d.value;
	piP.preF = params[RETROJITTER_PREVIEW]->u.bd.value;

	linesL = output->extent_hint.bottom - output->extent_hint.top;
	A_long thicknessL = (A_long)(output->height * 0.5) / (piP.resF);
	A_long thicknesscountL = thicknessL;
	bool whiteflagB = true;

	int framenumber = in_data->current_time / in_data->time_step;
	const float roundedframerate = round(float(in_data->time_scale) / float(in_data->time_step));
	A_long framelength = roundedframerate / piP.spdF;

	//this giant if else block has a lot of repeated code but i couldn't get it to work in the "better" way 
	if (piP.preF == FALSE) {

		//drawing displacement map
		for (int i = 0; i <= output->height; i++) {

			if (thicknesscountL == 0)
				whiteflagB = false;

			if (thicknesscountL == thicknessL)
				whiteflagB = true;

			if (whiteflagB) {
				thicknesscountL--;
			}

			else {
				thicknesscountL++;
			}

			
			DrawLine(0, i, output->width, i, &map, whiteflagB);

		}

		//inverting displacement map
		if (framenumber % framelength == 0)
			invertflag = !invertflag;

		if (invertflag){

			ERR(suites.Iterate8Suite1()->iterate(in_data,
				0,									// progress base
				linesL,								// progress final
				&params[RETROJITTER_INPUT]->u.ld,	// src 
				NULL,								// area - null for all pixels
				(void*)&piP,						// refcon - your custom data pointer
				RetroJitter8InvertMap,				// pixel function pointer
				&map));

		}

		ERR(PF_COPY(&params[RETROJITTER_INPUT]->u.ld, output, NULL, NULL));

		//applying map to input layer, bootleg function that iterates over entire image
		for (int i = 0; i < output->height; i++) {
			for (int j = 0; j < output->width; j++) {
				UpdateColor8(j, i, piP.distF, output, &map, &params[RETROJITTER_INPUT]->u.ld);
			}
		}
		

	}

	else {

		//drawing displacement map
		for (int i = 0; i <= output->height; i++) {

			if (thicknesscountL == 0)
				whiteflagB = false;

			if (thicknesscountL == thicknessL)
				whiteflagB = true;

			if (whiteflagB) {
				thicknesscountL--;
			}

			else {
				thicknesscountL++;
			}

			DrawLine(0, i, output->width, i, output, whiteflagB);

		}

		//inverting displacement map
		if (framenumber % framelength == 0)
			invertflag = !invertflag;

		if (invertflag) {

			ERR(suites.Iterate8Suite1()->iterate(in_data,
				0,									// progress base
				linesL,								// progress final
				&params[RETROJITTER_INPUT]->u.ld,	// src 
				NULL,								// area - null for all pixels
				(void*)&piP,						// refcon - your custom data pointer
				RetroJitter8InvertMap,				// pixel function pointer
				output));

		}

	}

	return err;
}

extern "C" DllExport
PF_Err PluginDataEntryFunction(
	PF_PluginDataPtr inPtr,
	PF_PluginDataCB inPluginDataCallBackPtr,
	SPBasicSuite* inSPBasicSuitePtr,
	const char* inHostName,
	const char* inHostVersion)
{
	PF_Err result = PF_Err_INVALID_CALLBACK;

	result = PF_REGISTER_EFFECT(
		inPtr,
		inPluginDataCallBackPtr,
		"RetroJitter", // Name
		"ADBE RetroJitter", // Match Name
		"Sample Plug-ins", // Category
		AE_RESERVED_INFO); // Reserved Info

	return result;
}


PF_Err
EffectMain(
	PF_Cmd			cmd,
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output,
	void			*extra)
{
	PF_Err		err = PF_Err_NONE;
	
	try {
		switch (cmd) {
			case PF_Cmd_ABOUT:

				err = About(in_data,
							out_data,
							params,
							output);
				break;
				
			case PF_Cmd_GLOBAL_SETUP:

				err = GlobalSetup(	in_data,
									out_data,
									params,
									output);
				break;
				
			case PF_Cmd_PARAMS_SETUP:

				err = ParamsSetup(	in_data,
									out_data,
									params,
									output);
				break;
				
			case PF_Cmd_RENDER:

				err = Render(	in_data,
								out_data,
								params,
								output);
				break;
		}
	}
	catch(PF_Err &thrown_err){
		err = thrown_err;
	}
	return err;
}

