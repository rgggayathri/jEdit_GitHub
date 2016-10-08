/*
 * ScriptServer.h - part of jEditLauncher package
 * Copyright (C) 2001 John Gellene
 * jgellene@nyc.rr.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * Notwithstanding the terms of the General Public License, the author grants
 * permission to compile and link object code generated by the compilation of
 * this program with object code and libraries that are not subject to the
 * GNU General Public License, provided that the executable output of such
 * compilation shall be distributed with source code on substantially the
 * same basis as the jEditLauncher package of which this program is a part.
 * By way of example, a distribution would satisfy this condition if it
 * included a working makefile for any freely available make utility that
 * runs on the Windows family of operating systems. This condition does not
 * require a licensee of this software to distribute any proprietary software
 * (including header files and libraries) that is licensed under terms
 * prohibiting redistribution to third parties.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id: ScriptServer.h,v 1.2 2001/09/05 11:24:52 jgellene Exp $
 */

#ifndef __SCRIPTSERVER_H_
#define __SCRIPTSERVER_H_

#include "resource.h"       // main symbols
#include "RegistryParser.h"
#include "ServConn.h"

class RegistryServer;


// interface IScriptServer

interface IScriptServer
{
	virtual HRESULT STDMETHODCALLTYPE ProcessScript
		(unsigned char *szScript) = 0;
	virtual HRESULT STDMETHODCALLTYPE FindServer
		(VARIANT_BOOL* pVFound) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetServerPort
		(ULONG* pPort) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetServerKey
		(ULONG* pKey) = 0;
};

class CScriptServer : public IScriptServer
{
public:
	CScriptServer(RegistryParser *pParser)
		: firstScript(0), lastScript(0), m_hMutex(0),
		  m_pConn(0)
	{
		m_hMutex = CreateMutex(0, FALSE, _T("jEditLauncher"));
		m_pConn = new ServerConnection(pParser->GetServerFilePath());
	}
	~CScriptServer()
	{
		ReleaseMutex(m_hMutex);
		delete m_pConn;
		while(Script_* pScr = popScript()) delete pScr;
	}

public:
	virtual HRESULT STDMETHODCALLTYPE ProcessScript(unsigned char *szScript);
	virtual HRESULT STDMETHODCALLTYPE FindServer(VARIANT_BOOL* pVFound);
	virtual HRESULT STDMETHODCALLTYPE GetServerPort(ULONG* pPort);
	virtual HRESULT STDMETHODCALLTYPE GetServerKey(ULONG* pKey);
protected:
	struct Script_ {
		unsigned char *szScript;
		Script_ *next;
		Script_(unsigned char* szBuf)
			: szScript(0), next(0)
		{
			int len = strlen((const char*)szBuf) + 1;
			szScript = (unsigned char*)CoTaskMemAlloc(len);
			//ZeroMemory(szScript, len);
			CopyMemory(szScript, szBuf, len);
			//MessageBox(0, (char*)szScript, "new buf inside Script_", 0);
		}
		~Script_() { CoTaskMemFree(szScript); }
	};
	Script_ *firstScript, *lastScript;
	void pushScript(unsigned char *szBuf)
	{
//		MessageBox(0, (char*)szBuf, "new buf for script", 0);
		Script_ *pScr = new Script_(szBuf);
		if(firstScript == 0)
			firstScript = pScr;
		else
			lastScript->next = pScr;
		lastScript = pScr;
	}
	Script_* popScript()
	{
		Script_* pScr = firstScript;
		if(pScr != 0)
			firstScript = pScr->next;
		if(lastScript == pScr)
			lastScript = 0;
		return pScr;
	}
	HRESULT STDMETHODCALLTYPE SendScript();

private:
	HANDLE m_hMutex;
	ServerConnection *m_pConn;
	RegistryParser *m_pRegParser;
};


#endif //__SCRIPTSERVER_H_