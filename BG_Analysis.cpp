#include <weh.h>
#include "BG_Analysis.h"
#include "bkeSci\bkescintilla.h"
#include "loli\loli_island.h"
#include "BKS_info.h"

BG_Analysis::BG_Analysis(const QString &p)
{
	stop = false;
	cancel = false;
	newmacrofile = false;
	pdir = p;
	backup_topclo = new BKE_VarClosure();
	backup_topclo->cloneFrom(global_bke_info.glo);
	topclo = new BKE_VarClosure();;
	topclo->cloneFrom(global_bke_info.glo);
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
	msgmutex.lock();
	auto p = data[file];
	msgmutex.unlock();
	if (p->fileNodes.empty())
	{
		p->infos2_mutex.lock();
		p->infos2.emplace_back(2, 14, 0, 1);
		p->infos2_mutex.unlock();
		return;
	}
	auto node = p->fileNodes.begin();
	if (!(*node)->isLabel() || (*node)->name != "register")
	{
		p->infos2_mutex.lock();
		p->infos2.emplace_back(2, 14, (*node)->startPos, (*node)->endPos + 1 - (*node)->startPos);
		p->infos2_mutex.unlock();
		return;
	}
	node++;
	while (!cancel && node != p->fileNodes.end())
	{
		if ((*node)->isCommand() && (*node)->name == "return")
			return;
		if ((*node)->isCommand() && (*node)->name == "import")
		{
			auto n = (*node)->findIndex("file", 0);
			if (!n || n->name.isEmpty())
			{
				p->infos2_mutex.lock();
				p->infos2.emplace_back(2, 15, (*node)->startPos, (*node)->endPos + 1 - (*node)->startPos);
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
						p->infos2.emplace_back(2, 16, n->startPos, n->endPos + 1 - n->startPos);
						p->infos2_mutex.unlock();
					}
					else if (std::find(backup_macrofiles.begin(), backup_macrofiles.end(), f) == backup_macrofiles.end())
					{
						backup_macrofiles.push_back(f);
						parseMacro(f);
					}
				}
			}
		}
		else if ((*node)->isCommand() && (*node)->name == "macro")
		{
			auto n = (*node)->findIndex("name", 0);
			if (!n || n->name.isEmpty())
			{
				p->infos2_mutex.lock();
				p->infos2.emplace_back(2, 17, (*node)->startPos, (*node)->endPos + 1 - (*node)->startPos);
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
						p->infos2.emplace_back(2, 18, (*node)->startPos, (*node)->endPos + 1 - (*node)->startPos);
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
								m.paramqueue.emplace_back((*node)->cmdValue[i]->name, QString(""));
							else
								m.paramqueue.emplace_back((*node)->cmdParam[i]->name, (*node)->cmdValue[i]->name);
						}
					}
				}
				else
				{
					p->infos2_mutex.lock();
					p->infos2.emplace_back(2, 19, (*node)->startPos, (*node)->endPos + 1 - (*node)->startPos);
					p->infos2_mutex.unlock();
				}
			}
		}
		else if ((*node)->isExp())
		{
			PAModule pa((*node)->name);
			pa.analysisToClosure(backup_topclo);
		}
		node++;
	}
}

void BG_Analysis::run()
{
	while (!this->stop)
	{
		cancel = false;
		QString cur;
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

		if (cur.isEmpty())
		{
			//no file to analysis, test macro
			while (newmacrofile)
			{
				cur_state = STATE_PARSEMACRO;
				//analysis macros
				backup_macrodata.clear();
				backup_topclo->clear();
				backup_topclo->cloneFrom(global_bke_info.glo);

				msgmutex.lock();
				backup_macrofiles.clear();
				backup_macrofiles.push_back("macro.bkscr");
				auto m_it = backup_macrofiles.begin();
				msgmutex.unlock();

				parseMacro("macro.bkscr");

				//while (!cancel && m_it != backup_macrofiles.end())
				//{
				//	//analysis one by one
				//	msgmutex.lock();
				//	QString thisfile = *m_it;
				//	auto p = data[thisfile];
				//	//backup_macrofiles.pop_front();
				//	msgmutex.unlock();

				//	if (p->fileNodes.empty())
				//	{
				//		p->infos2_mutex.lock();
				//		p->infos2.emplace_back(2, 14, 0, 1);
				//		p->infos2_mutex.unlock();
				//		continue;
				//	}
				//	auto node = p->fileNodes.begin();
				//	if (!(*node)->isLabel() || (*node)->name != "register")
				//	{
				//		p->infos2_mutex.lock();
				//		p->infos2.emplace_back(2, 14, (*node)->startPos, (*node)->endPos + 1 - (*node)->startPos);
				//		p->infos2_mutex.unlock();
				//		continue;
				//	}
				//	node++;
				//	while (!cancel && node != p->fileNodes.end())
				//	{
				//		if ((*node)->isCommand() && (*node)->name == "return")
				//			break;
				//		if ((*node)->isCommand() && (*node)->name == "import")
				//		{
				//			auto n = (*node)->findIndex("file", 0);
				//			if (!n || n->name.isEmpty())
				//			{
				//				p->infos2_mutex.lock();
				//				p->infos2.emplace_back(2, 15, (*node)->startPos, (*node)->endPos + 1 - (*node)->startPos);
				//				p->infos2_mutex.unlock();
				//			}
				//			else
				//			{
				//				PAModule pa(n->name);
				//				bool suc;
				//				QString mname = pa.getStringValue(&suc);
				//				if (suc)
				//				{
				//					auto f = searchFile(mname);
				//					if (f.isEmpty())
				//					{
				//						p->infos2_mutex.lock();
				//						p->infos2.emplace_back(2, 16, n->startPos, n->endPos + 1 - n->startPos);
				//						p->infos2_mutex.unlock();
				//					}
				//					else if (std::find(backup_macrofiles.begin(), backup_macrofiles.end(), f) == backup_macrofiles.end())
				//					{
				//						backup_macrofiles.push_back(f);
				//					}
				//				}
				//			}
				//		}
				//		else if ((*node)->isCommand() && (*node)->name == "macro")
				//		{
				//			auto n = (*node)->findIndex("name", 0);
				//			if (!n || n->name.isEmpty())
				//			{
				//				p->infos2_mutex.lock();
				//				p->infos2.emplace_back(2, 17, (*node)->startPos, (*node)->endPos + 1 - (*node)->startPos);
				//				p->infos2_mutex.unlock();
				//			}
				//			else
				//			{
				//				PAModule pa(n->name);
				//				bool suc;
				//				QString mname = pa.getStringValue(&suc);
				//				if (suc)
				//				{
				//					if (CmdList.find(mname) != CmdList.end() || SpecialCmdList.find(mname) != SpecialCmdList.end())
				//					{
				//						p->infos2_mutex.lock();
				//						p->infos2.emplace_back(2, 18, (*node)->startPos, (*node)->endPos + 1 - (*node)->startPos);
				//						p->infos2_mutex.unlock();
				//					}
				//					else
				//					{
				//						auto &m = backup_macrodata[mname];
				//						m.name = mname;
				//						m.definefile = thisfile;
				//						m.comment = (*node)->comment;
				//						m.pos = (*node)->startPos;
				//						for (int i = 0; i < (*node)->cmdParam.size(); i++)
				//						{
				//							if ((*node)->cmdParam[i] && (*node)->cmdParam[i]->name == "name")
				//								continue;
				//							if (!(*node)->cmdParam[i] || (*node)->cmdParam[i]->name.isEmpty())
				//								m.paramqueue.emplace_back((*node)->cmdValue[i]->name, QString(""));
				//							else
				//								m.paramqueue.emplace_back((*node)->cmdParam[i]->name, (*node)->cmdValue[i]->name);
				//						}
				//					}
				//				}
				//				else
				//				{
				//					p->infos2_mutex.lock();
				//					p->infos2.emplace_back(2, 19, (*node)->startPos, (*node)->endPos + 1 - (*node)->startPos);
				//					p->infos2_mutex.unlock();
				//				}
				//			}
				//		}
				//		else if ((*node)->isExp())
				//		{
				//			PAModule pa((*node)->name);
				//			pa.analysisToClosure(backup_topclo, false);
				//		}
				//		node++;
				//	}
				//	m_it++;
				//}

				if (!cancel)
				{
					msgmutex.lock();
					macrodata = backup_macrodata;
					macrofiles = backup_macrofiles;
					topclo->clear();
					topclo->cloneFrom(backup_topclo);
					msgmutex.unlock();
				}

				newmacrofile = false;
				break;
			}
			cur_state = STATE_IDLE;
			msleep(50);
			continue;
		}
		
		msgmutex.lock();
		if (!filebuf.contains(cur))
		{
			QString en;
			if (!LOLI::AutoRead(en, pdir + cur))
			{
				msgmutex.unlock();
				continue;
			}
			else
			{
				filebuf[cur] = en.toUtf8();
			}
		}
		auto &sci = filebuf[cur];
		ParseData *p = new ParseData(sci, topclo);
		msgmutex.unlock();

		while (!stop && !cancel)
		{
			cur_state = STATE_PARSEFILE;
			bool res = p->Parse();
			if (!res)
			{
				//if (std::find(macrofiles.begin(), macrofiles.end(), cur) == macrofiles.end())
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
				break;
			}
		}

		msgmutex.lock();
		if (data[cur] != p)
			delete p;
		msgmutex.unlock();
	}
}