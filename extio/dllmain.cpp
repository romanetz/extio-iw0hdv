/*
 * Extio for Airspy
 *
 * Copyright 2015 by Andrea Montefusco IW0HDV
 *
 * Licensed under GNU General Public License 3.0 or later. 
 * Some rights reserved. See COPYING, AUTHORS.
 *
 * @license GPL-3.0+ <http://spdx.org/licenses/GPL-3.0+>
 *
 */


// dllmain.cpp : Defines the entry point for the DLL application.
#if !defined __MINGW32__
#pragma once
#endif

#include <typeinfo>
#include <iostream>

#define _DONT_DECLARE_TEMPLATE_
#include "log.h"
#include "dllmain.h"
#include "util.h"     // for shared macro
#include "git-sha1.h"

#pragma data_seg (".SS_DLLMAIN")
int Dll::instance_ SHARED = 0 ;
#pragma data_seg()

HMODULE Dll::hMod = 0;
int Dll::local_instance = 0;

Dll::Dll (HMODULE hm)
{
	hMod = hm;
}

/**
 * Instance number tracker
 */
void Dll::inc () 
{	
	instance_ = instance_ + 1;
	local_instance = instance_;
}
void Dll::dec()
{
	if (instance_) instance_ = instance_ - 1;
	local_instance = 0;
}

int Dll::GetInstanceNumber () { return local_instance; }
int Dll::GetInstanceQuantity () { return instance_; }

/*
 * pointer to the child objects that has to register here before 
 * the DllMain is called with DLL_PROCESS_ATTACH
 * typically that is a global singleton that is initialized by C++ run time
 *
 */
static Dll *pObj = 0;

Dll * Dll::getObj () 
{ 
    return pObj;
}

void Dll :: Register(Dll *singleton)
{
	fprintf (stderr, "Dll:: %p %p\n", pObj, singleton);
	// only one class instance can be registered here !
	if (singleton && (pObj == 0)) pObj = singleton;
}


BOOL APIENTRY DllMain ( HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved )
{
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
		#if CONSOLE
		AllocConsole();
		AttachConsole( GetCurrentProcessId() ) ;
		freopen( "CON", "w", stdout ) ;
		freopen( "CON", "w", stderr ) ;
		#endif
		if (pObj == 0) {
		#if CONSOLE
		    fprintf (stderr, "FATAL: no registered OBJECT\n");
		#endif
			return FALSE;
		} else 
			pObj->SetHModule(hModule);

		// LOG_OPEN moved to derived class ProcessAttach method
		LOG_OPEN(pObj->name(), 1);

		#if CONSOLE
		std::cerr << "OBJECT: [" << pObj << "] (" << typeid(*pObj).name() << ")" << "\n";
		std::cerr << "----------------------------" << "\n";
		#endif
		
		if (pObj) {
			pObj->inc ();
			std::cerr << "1111111111111111111111111111111111111111" << "\n";
			pObj->ProcessAttach ();
			std::cerr << "2222222222222222222222222222222222222222" << "\n";
		} else {
			fprintf (stderr, "FATAL: unable to create derived class object\n");
		}
		#if defined __MINGW32__
		LOGT("%s, module handle: %p\n", "Compiled with MinGW", pObj->GetMyHandle());
		LOGT("git sha1: %s built on: %s\r\n", GetGitSHA1 (), " - " __DATE__ ", " __TIME__ " - ");
		#endif
		std::cerr << "333333333333333333333333333333333333333333333" << "\n";
		break;

	case DLL_THREAD_ATTACH:
		if (pObj) pObj->ThreadAttach ();
		break;

	case DLL_THREAD_DETACH:
		if (pObj) pObj->ThreadDetach ();
		break;

	case DLL_PROCESS_DETACH:
		if (pObj) {
			pObj->ProcessDetach ();
			LOGT("DLL_PROCESS_DETACH: %s\n", "BEFORE");
			pObj->dec();
			LOGT("DLL_PROCESS_DETACH: %s\n", "AFTER");
			delete pObj, pObj = 0;
			LOG_CLOSE;
#if !defined NDEBUG && (defined _MSC_VER)
			_CrtDumpMemoryLeaks();
#endif
		}
		break;
	}
	return TRUE;
}
