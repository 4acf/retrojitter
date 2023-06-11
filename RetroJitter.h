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

/*
	RetroJitter.h
*/

#pragma once

#ifndef RETROJITTER_H
#define RETROJITTER_H

typedef unsigned char		u_char;
typedef unsigned short		u_short;
typedef unsigned short		u_int16;
typedef unsigned long		u_long;
typedef short int			int16;
#define PF_TABLE_BITS	12
#define PF_TABLE_SZ_16	4096

#define PF_DEEP_COLOR_AWARE 1	// make sure we get 16bpc pixels; 
								// AE_Effect.h checks for this.

#include "AEConfig.h"

#ifdef AE_OS_WIN
	typedef unsigned short PixelType;
	#include <Windows.h>
#endif

#include "entry.h"
#include "AE_Effect.h"
#include "AE_EffectCB.h"
#include "AE_Macros.h"
#include "Param_Utils.h"
#include "AE_EffectCBSuites.h"
#include "String_Utils.h"
#include "AE_GeneralPlug.h"
#include "AEFX_ChannelDepthTpl.h"
#include "AEGP_SuiteHandler.h"

#include "RetroJitter_Strings.h"

#include <stdlib.h>
#include <assert.h>

/* Versioning information */

#define	MAJOR_VERSION	1
#define	MINOR_VERSION	0
#define	BUG_VERSION		0
#define	STAGE_VERSION	PF_Stage_DEVELOP
#define	BUILD_VERSION	1


/* Parameter defaults */

#define	RETROJITTER_RESOLUTION_MIN		1
#define	RETROJITTER_RESOLUTION_MAX		240
#define	RETROJITTER_RESOLUTION_DFLT		240

#define	RETROJITTER_DISTANCE_MIN		0
#define	RETROJITTER_DISTANCE_MAX		2
#define	RETROJITTER_DISTANCE_DFLT		0

#define	RETROJITTER_SPEED_MIN			1
#define	RETROJITTER_SPEED_MAX			30
#define	RETROJITTER_SPEED_DFLT			30

#define RETROJITTER_PREVIEW_DFLT		FALSE

enum {
	RETROJITTER_INPUT = 0,
	RETROJITTER_RESOLUTION,
	RETROJITTER_DISTANCE,
	RETROJITTER_SPEED,
	RETROJITTER_PREVIEW,
	RETROJITTER_NUM_PARAMS
};

enum {
	RESOLUTION_DISK_ID = 1,
	DISTANCE_DISK_ID,
	SPEED_DISK_ID, 
	PREVIEW_DISK_ID
};

typedef struct ParamInfo{
	PF_FpLong	resF;
	PF_FpLong	distF;
	PF_FpLong	spdF;
	PF_FpLong	preF;
} ParamInfo, *ParamInfoP, **ParamInfoH;


extern "C" {

	DllExport
	PF_Err
	EffectMain(
		PF_Cmd			cmd,
		PF_InData		*in_data,
		PF_OutData		*out_data,
		PF_ParamDef		*params[],
		PF_LayerDef		*output,
		void			*extra);

}

#endif // RETROJITTER_H