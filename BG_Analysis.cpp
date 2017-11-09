#include <weh.h>
#include "BG_Analysis.h"
#include "bkeSci/bkescintilla.h"
#include "loli/loli_island.h"
#include "BKS_info.h"
#include "CmdListLoader.h"

#if WIN32
#include <Windows.h>
#include <DbgHelp.h>
//for windows debug


unsigned int runAddress;

EXCEPTION_POINTERS firstException;

void dumpDebug(CONTEXT *ctx = NULL)
{
	PSYMBOL_INFO symbol;            // Debugging symbol's information.  
	IMAGEHLP_LINE64 source_info;        // Source information (file name & line number)  
	DWORD displacement;         // Source line displacement.  

	// Initialize PSYMBOL_INFO structure.  
	// Allocate a properly-sized block.  
	symbol = (PSYMBOL_INFO)malloc(sizeof(SYMBOL_INFO) + (FILENAME_MAX - 1) * sizeof(TCHAR));
	memset(symbol, 0, sizeof(SYMBOL_INFO) + (FILENAME_MAX - 1) * sizeof(TCHAR));
	symbol->SizeOfStruct = sizeof(SYMBOL_INFO);  // SizeOfStruct *MUST BE* set to sizeof(SYMBOL_INFO).  
	symbol->MaxNameLen = FILENAME_MAX;

	// Initialize IMAGEHLP_LINE64 structure.  
	memset(&source_info, 0, sizeof(IMAGEHLP_LINE64));
	source_info.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

	CONTEXT context;
	if (ctx)
		context = *ctx;
	else
		RtlCaptureContext(&context);
	STACKFRAME64 frame;
	memset(&frame, 0, sizeof(frame));
	frame.AddrPC.Offset = context.Eip;
	frame.AddrPC.Mode = AddrModeFlat;
	frame.AddrFrame.Offset = context.Ebp;
	frame.AddrFrame.Mode = AddrModeFlat;
	frame.AddrStack.Offset = context.Esp;
	frame.AddrStack.Mode = AddrModeFlat;

	auto process = GetCurrentProcess();  // Get current process & thread.  
	auto thread = GetCurrentThread();

	if (!SymInitialize(process, NULL, TRUE))
		return;

	// Enumerate call stack frame.  
	while (StackWalk64(IMAGE_FILE_MACHINE_I386, process, thread, &frame,
		&context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL))
	{
		if (frame.AddrFrame.Offset == 0)    // End reaches.  
			break;

		if (SymFromAddr(process, frame.AddrPC.Offset, NULL, symbol))// Get symbol.  
			qDebug() << " < " << symbol->Name << "\n";

		if (SymGetLineFromAddr64(process, frame.AddrPC.Offset, &displacement, &source_info))
		{
			// Get source information.  
			qDebug() << "\t[ " << source_info.FileName << ":" << source_info.LineNumber << "] at addr 0x" << QString::asprintf("%08X", frame.AddrPC.Offset);
		}
		else
		{
			if (GetLastError() == 0x1E7)       // If err_code == 0x1e7, no symbol was found.  
			{
				qDebug() << "\tNo debug symbol loaded for this function.\n";
			}
		}
	}

	SymCleanup(process);    // Clean up and exit.  
	free(symbol);
}

void simpleDump(PEXCEPTION_POINTERS ExceptionInfo);

LONG CALLBACK VectoredHandler(_In_ PEXCEPTION_POINTERS ExceptionInfo)
{
	firstException = *ExceptionInfo;
	if (0)
	{
		simpleDump(ExceptionInfo);
	}
	return EXCEPTION_CONTINUE_SEARCH;
}

void simpleDump(PEXCEPTION_POINTERS ExceptionInfo)
{
	//dumpDebug(ExceptionInfo->ContextRecord);
	//int s;
	//RtlUnwind(NULL, NULL, ExceptionInfo->ExceptionRecord, &s);
	HANDLE hDumpFile = CreateFile(L"MiniDump.dmp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hDumpFile != INVALID_HANDLE_VALUE){
		//Dump信息  
		MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
		dumpInfo.ExceptionPointers = ExceptionInfo;
		dumpInfo.ThreadId = GetCurrentThreadId();
		dumpInfo.ClientPointers = TRUE;
		//写入Dump文件内容  
		MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpScanMemory, &dumpInfo, NULL, NULL);
		CloseHandle(hDumpFile);
	}
}

#endif

void dumpExcept(Var_Except &e, int code)
{
	QString logfile = BKE_CURRENT_DIR + "/log.txt";
	QFile log(logfile);
	log.open(QIODevice::OpenMode::enum_type::Append | QIODevice::OpenMode::enum_type::WriteOnly);
	QTextStream out(&log);
	QString tm = QTime::currentTime().toString("hh-mm-ss");
	out << tm;
	out << " Var_Except:" << QString::fromStdWString(e.getMsg()) << "\nCode:" << code;
	out << "\n";
	//out << "from:" << QString::fromStdWString(e._file) << " " << QString::fromStdWString(e._line) << "" << QString::fromStdWString(e._func);
}

typedef void(*dumExcept2)(std::exception &e, int code);

void dumpExcept(std::exception &e, int code)
{
	QString logfile = BKE_CURRENT_DIR + "/log.txt";
	QFile log(logfile);
	log.open(QIODevice::OpenMode::enum_type::Append | QIODevice::OpenMode::enum_type::WriteOnly);
	QTextStream out(&log);
	QString tm = QTime::currentTime().toString("hh-mm-ss");
	out << tm;
	out << " std_exception:" << QString::fromLocal8Bit(e.what()) << "\nCode:" << code;
	out << "\n";
#ifdef WIN32
	CONTEXT context;
	context = *firstException.ContextRecord;

	//跳过最后CxxException和RaiseException
	STACKFRAME64 frame;
	memset(&frame, 0, sizeof(frame));
	frame.AddrPC.Offset = context.Eip;
	frame.AddrPC.Mode = AddrModeFlat;
	frame.AddrFrame.Offset = context.Ebp;
	frame.AddrFrame.Mode = AddrModeFlat;
	frame.AddrStack.Offset = context.Esp;
	frame.AddrStack.Mode = AddrModeFlat;
	auto process = GetCurrentProcess();  // Get current process & thread.  
	auto thread = GetCurrentThread();
	StackWalk64(IMAGE_FILE_MACHINE_I386, process, thread, &frame, &context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL);
	StackWalk64(IMAGE_FILE_MACHINE_I386, process, thread, &frame, &context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL);
	StackWalk64(IMAGE_FILE_MACHINE_I386, process, thread, &frame, &context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL);
	firstException.ExceptionRecord->ExceptionAddress = (PVOID)context.Eip;
	memcpy(firstException.ContextRecord, &context, sizeof(context));

	//RtlCaptureContext(&context);
	out << QString::asprintf("Crash Address : %08X\n", context.Eip);
	out << QString::asprintf("Reference Address : %08X  (testAddr)\n", runAddress);
	out << "Call Stack:\n";
	int raweip = context.Eip;
	while (1)
	{
		StackWalk64(IMAGE_FILE_MACHINE_I386, process, thread, &frame, &context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL);
		if (!frame.AddrPC.Offset || frame.AddrPC.Offset == raweip)
			break;
		if (*(unsigned char *)(frame.AddrPC.Offset - 5) == 0xE8)
		{
			int offset = *(int*)(frame.AddrPC.Offset - 4);
			int dst = frame.AddrPC.Offset + offset;
			if (*(unsigned char*)dst == 0xE9)
				dst += 5 + *(int*)(dst + 1);
			out << QString::asprintf("%08X => %08X\n", (int)(frame.AddrPC.Offset - 5), dst);
		}
		else
		{
			unsigned char s[] = { 0x55, 0x8B, 0xEC };
			int curpos = raweip - 5;
			int maxsearchpos = curpos - 10000;
			if (maxsearchpos < 0)
				maxsearchpos = 0;
			while (curpos >= maxsearchpos)
			{
				if (!memcmp((void*)curpos, s, 3))
					break;
				else
					curpos--;
			}
			int callpos = frame.AddrPC.Offset;
			if (*(unsigned char *)(frame.AddrPC.Offset - 3) == 0xFF)
				callpos = frame.AddrPC.Offset - 3;
			if (*(unsigned char *)(frame.AddrPC.Offset - 2) == 0xFF)
				callpos = frame.AddrPC.Offset - 2;
			if (curpos > 0)
				out << QString::asprintf("Dynamic Call from %08X => %08X (Most Possible)\n", callpos, curpos);
			else
				out << QString::asprintf("Dynamic Call from %08X\n", callpos);
		}
		raweip = frame.AddrPC.Offset;
	}
	//dump registers
	out << QString::asprintf("\nRegisters:\nEAX:%08X\nEBX:%08X\nECX:%08X\nEDX:%08X\nEDI:%08X\nESI:%08X\nEBP:%08X\nESP:%08X\nEIP:%08X\n",
		firstException.ContextRecord->Eax, firstException.ContextRecord->Ebx, firstException.ContextRecord->Ecx, firstException.ContextRecord->Edx, 
		firstException.ContextRecord->Edi, firstException.ContextRecord->Esi, firstException.ContextRecord->Ebp, firstException.ContextRecord->Esp,
		firstException.ContextRecord->Eip);
	out << QString::asprintf("\nStacks:\n");
	int ebp = firstException.ContextRecord->Ebp;
	int esp = firstException.ContextRecord->Esp;
	if (*(unsigned char*)ebp == 0xE9)
		ebp += 5 + *(int*)(ebp + 1);
	while (esp < ebp)
	{
		out << QString::asprintf("%08X\n", *(int*)esp);
		esp += 4;
	}
	out << "\n";
#endif
}

void BG_Analysis::notifyExit()
{
#ifdef WIN32
	MessageBox(NULL, L"分析线程崩溃，请尽快保存工程后重启Creator", L"BKE_Creator", 0);
#endif
	this->exit(0);
}

QList<QPair<QString, int>> BG_Analysis::getCmdList()
{
	QList<QPair<QString, int>> result;
	{
		QStringList tmp;
		for (auto &it : CmdList)
		{
			tmp.push_back(it.name);
		}
		std::sort(tmp.begin(), tmp.end(), [](const QString &l, const QString &r) {
			QBkeCmdInfo &linfo = CmdList[l];
			QBkeCmdInfo &rinfo = CmdList[r];
			if (linfo.priority > rinfo.priority)
				return true;
			else if (linfo.priority == rinfo.priority)
				return l > r;
			return false;
		});
		for (auto &it : tmp)
		{
			result << qMakePair(it, 1);
		}
	}
	
	for (auto &it : SpecialCmdList.keys())
	{
		result << qMakePair(it, 1);
	}
	msgmutex.lock();
	for (auto it = macrodata.begin(); it != macrodata.end(); it++)
	{
		result << qMakePair(it.key(), 3);
	}
	msgmutex.unlock();
	return result;
}

BG_Analysis::BG_Analysis(const QString &p)
	: msgmutex(QMutex::Recursive)
{
	stop = false;
	cancel = false;
	newmacrofile = false;
	pdir = p;
	backup_topclo = new BKE_VarClosure();
	//backup_topclo->cloneFrom(global_bke_info.glo);
	BKE_hashmap<void*, void*> pMap;
	pMap[BKE_VarClosure::global()] = BKE_VarClosure::global();
	backup_topclo->assignStructure(global_bke_info.glo, pMap, true);
	topclo = new BKE_VarClosure();
	//topclo->cloneFrom(global_bke_info.glo);
	pMap.clear();
	pMap[BKE_VarClosure::global()] = BKE_VarClosure::global();
	topclo->assignStructure(global_bke_info.glo, pMap, true);
	this->start();
}

BG_Analysis::~BG_Analysis()
{
	stop = true;
	cancel = true;
	wait();
	if (topclo)
		topclo->release();
	if (backup_topclo)
		backup_topclo->release();

	for (auto &it : data)
		delete it;
}

void BG_Analysis::parseMacro(const QString &file)
{
	ParseData *p;
	try
	{
		msgmutex.lock();
		p = data[file];
		msgmutex.unlock();
	}
	catch (std::exception &e)
	{
		dumpExcept(e, 2);
		notifyExit();
	}
	try
	{
		if (p->fileNodes.empty())
		{
			p->infos2_mutex.lock();
			p->infos2.push_back({ 2, 14, 0, 1 });
			p->infos2_mutex.unlock();
			return;
		}
	}
	catch (std::exception &e)
	{
		dumpExcept(e, 3);
		notifyExit();
	}
	QMap<int, BaseNode*>::iterator node;
	try
	{
		node = p->fileNodes.begin();
		if (!(*node)->isLabel() || (*node)->name != "register")
		{
			p->infos2_mutex.lock();
			p->infos2.push_back({ 2, 14, (*node)->startPos, (*node)->endPos + 1 - (*node)->startPos });
			p->infos2_mutex.unlock();
			return;
		}
		node++;
	}
	catch (std::exception &e)
	{
		dumpExcept(e, 3);
		notifyExit();
	}
	while (!cancel && node != p->fileNodes.end())
	{
		if ((*node)->isCommand() && (*node)->name == "return")
			return;
		if ((*node)->isCommand() && (*node)->name == "import")
		{
			auto n = (*node)->findIndex("file", 0);
			try
			{
				if (!n || n->name.isEmpty())
				{
					p->infos2_mutex.lock();
					p->infos2.push_back({ 2, 15, (*node)->startPos, (*node)->endPos + 1 - (*node)->startPos });
					p->infos2_mutex.unlock();
				}
				else
				{
					PAModule pa(n->name);
					bool suc;
					QString mname = pa.getStringValue(&suc);
					if (suc)
					{
						auto f = searchFile(mname);
						if (f.isEmpty())
						{
							p->infos2_mutex.lock();
							p->infos2.push_back({ 2, 16, n->startPos, n->endPos + 1 - n->startPos });
							p->infos2_mutex.unlock();
						}
						else if (std::find(backup_macrofiles.begin(), backup_macrofiles.end(), f) == backup_macrofiles.end())
						{
							backup_macrofiles.push_back(f);
							qDebug() << f;
							parseMacro(f);
						}
					}
				}
			}
			catch (std::exception &e)
			{
				dumpExcept(e, 6);
				notifyExit();
			}
		}
		else if ((*node)->isCommand() && (*node)->name == "macro")
		{
			auto n = (*node)->findIndex("name", 0);
			try
			{
				if (!n || n->name.isEmpty())
				{
					p->infos2_mutex.lock();
					p->infos2.push_back({ 2, 17, (*node)->startPos, (*node)->endPos + 1 - (*node)->startPos });
					p->infos2_mutex.unlock();
				}
				else
				{
					PAModule pa(n->name);
					bool suc;
					QString mname = pa.getStringValue(&suc);
					if (suc)
					{
						if (CmdList.find(mname) != CmdList.end() || SpecialCmdList.find(mname) != SpecialCmdList.end())
						{
							p->infos2_mutex.lock();
							p->infos2.push_back({ 2, 18, (*node)->startPos, (*node)->endPos + 1 - (*node)->startPos });
							p->infos2_mutex.unlock();
						}
						else
						{
							auto &m = backup_macrodata[mname];
							m.name = mname;
							m.definefile = file;
							m.comment = (*node)->comment;
							m.pos = (*node)->startPos;
							for (int i = 0; i < (*node)->cmdParam.size(); i++)
							{
								if ((*node)->cmdParam[i] && (*node)->cmdParam[i]->name == "name")
									continue;
								if (!(*node)->cmdParam[i] || (*node)->cmdParam[i]->name.isEmpty())
									m.paramqueue.push_back({ (*node)->cmdValue[i]->name, QString("") });
								else
									m.paramqueue.push_back({ (*node)->cmdParam[i]->name, (*node)->cmdValue[i]->name });
							}
						}
					}
					else
					{
						p->infos2_mutex.lock();
						p->infos2.push_back({ 2, 19, (*node)->startPos, (*node)->endPos + 1 - (*node)->startPos });
						p->infos2_mutex.unlock();
					}
				}
			}
			catch (std::exception &e)
			{
				dumpExcept(e, 7);
				notifyExit();
			}
		}
		else if ((*node)->isExp())
		{
			try
			{
				PAModule pa((*node)->name);
				pa.analysisToClosure(backup_topclo);
			}
			catch (Var_Except &e)
			{
				dumpExcept(e, 4);
				newmacrofile = false;
			}
			catch (std::exception &e)
			{
				dumpExcept(e, 5);
				notifyExit();
			}
		}
		node++;
	}
}

#if WIN32

void testAddr()
{
	void* stack[2];
	CaptureStackBackTrace(0, 2, stack, NULL);
	runAddress = (unsigned int)stack[1] + (*(unsigned int*)((unsigned int)stack[1] - 4));
	if (*(unsigned char *)runAddress == 0xE9) //jump		//似乎debug下才有
	{
		runAddress += 5 + (*(unsigned int*)(runAddress + 1));
	}
}

extern LONG WINAPI ApplicationCrashHandler(EXCEPTION_POINTERS *pException);

#endif

void BG_Analysis::run()
{
#if WIN32
//	_set_se_translator(TranslateSEHtoCE);
	AddVectoredExceptionHandler(0, VectoredHandler);
	SetUnhandledExceptionFilter(ApplicationCrashHandler);
	//testAddr();
	__asm call testAddr;
	//dumpDebug();
#endif
	while (!this->stop)
	{
		try
		{
			QString cur;
			try
			{
				cancel = false;
				msgmutex.lock();
				cur = curfile;
				if (cur.isEmpty())
				{
					if (!listfile.isEmpty())
					{
						curfile = listfile.front();
						listfile.pop_front();
						cur = curfile;
					}
				}
				msgmutex.unlock();
			}
			catch (std::exception &e)
			{
				dumpExcept(e, 8);
				notifyExit();
			}
			if (cur.isEmpty())
			{
				//no file to analysis, test macro
				while (newmacrofile)
				{
					try
					{
						cur_state = STATE_PARSEMACRO;
						//analysis macros
						backup_macrodata.clear();
						backup_topclo->clear();
						//backup_topclo->cloneFrom(global_bke_info.glo);
						BKE_hashmap<void*, void*> pMap;
						pMap[BKE_VarClosure::global()] = BKE_VarClosure::global();
						backup_topclo->assignStructure(global_bke_info.glo, pMap, true);

						msgmutex.lock();
						backup_macrofiles.clear();
						backup_macrofiles.push_back("macro.bkscr");
						auto m_it = backup_macrofiles.begin();
						msgmutex.unlock();
					}
					catch (std::exception &e)
					{
						dumpExcept(e, 9);
						notifyExit();
					}

					parseMacro("macro.bkscr");

					try
					{
						if (!cancel)
						{
							msgmutex.lock();
							macrodata = backup_macrodata;
							macrofiles = backup_macrofiles;
							topclo->clear();
							//topclo->cloneFrom(backup_topclo);
							BKE_hashmap<void*, void*> pMap;
							pMap[BKE_VarClosure::global()] = BKE_VarClosure::global();
							topclo->assignStructure(backup_topclo, pMap, true);
							msgmutex.unlock();
						}
					}
					catch (std::exception &e)
					{
						dumpExcept(e, 10);
						notifyExit();
					}

					newmacrofile = false;
					break;
				}
				cur_state = STATE_IDLE;
				msleep(50);
				continue;
			}

			try
			{
				msgmutex.lock();
				if (!filebuf.contains(cur))
				{
					QString en;
					if (!LOLI::AutoRead(en, pdir + cur))
					{
						curfile.clear();
						msgmutex.unlock();
						continue;
					}
					else
					{
						filebuf[cur] = en.toUtf8();
					}
				}
			}
			catch (std::exception &e)
			{
				dumpExcept(e, 11);
				notifyExit();
			}
			auto &sci = filebuf[cur];
			ParseData *p = new ParseData(sci, topclo);
			msgmutex.unlock();

			while (!stop && !cancel)
			{
				cur_state = STATE_PARSEFILE;
				bool res;
				try
				{
					res = p->Parse();
				}
				catch (std::exception &e)
				{
					dumpExcept(e, 12);
					notifyExit();
				}
				if (!res)
				{
					//if (std::find(macrofiles.begin(), macrofiles.end(), cur) == macrofiles.end())
					try
					{
						{
							auto it = p->fileNodes.begin();
							while (!stop && !cancel && it != p->fileNodes.end())
							{
								while (it != p->fileNodes.end() && !(*it)->isExp())
									++it;
								if (it != p->fileNodes.end())
								{
									PAModule pa((*it)->name);
									pa.analysisToClosure(p->fileclo);
									++it;
								}
							}
						}
						msgmutex.lock();
						ParseData *c = data[cur];
						if (c)
							delete c;
						data[cur] = p;
						curfile.clear();
						msgmutex.unlock();
					}
					catch (std::exception &e)
					{
						dumpExcept(e, 13);
						notifyExit();
					}
					break;
				}
			}

			try
			{
				msgmutex.lock();
				if (data[cur] != p)
					delete p;
				msgmutex.unlock();
			}
			catch (std::exception &e)
			{
				dumpExcept(e, 14);
				notifyExit();
			}
		}
		catch (Var_Except &e)
		{
			dumpExcept(e, 0);
#if WIN32
			simpleDump(&firstException);
#endif
			newmacrofile = false;
		}
		catch (std::exception &e)
		{
			dumpExcept(e, 1);
#if WIN32
			simpleDump(&firstException);
 #endif
			newmacrofile = false;
			notifyExit();
		}
	}
}
