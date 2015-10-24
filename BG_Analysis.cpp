#include <weh.h>
#include "BG_Analysis.h"
#include "bkeSci\bkescintilla.h"
#include "loli\loli_island.h"

BG_Analysis::BG_Analysis(const QString &p)
{
	stop = false;
	cancel = false;
	newmacrofile = false;
	pdir = p;
	this->start();
}


BG_Analysis::~BG_Analysis()
{
	stop = true;
	cancel = true;
	wait();
}

void BG_Analysis::run()
{
	while (!this->stop)
	{
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
				//analysis macros
				backup_macrodata.clear();

				msgmutex.lock();
				macrofiles.clear();
				macrofiles.push_back("macro.bkscr");
				auto m_it = macrofiles.begin();
				msgmutex.unlock();

				while (!cancel && m_it != macrofiles.end())
				{
					//analysis one by one
					msgmutex.lock();
					QString thisfile = *m_it;
					auto p = data[thisfile];
					//macrofiles.pop_front();
					msgmutex.unlock();

					if (p->fileNodes.empty())
					{
						p->infos2_mutex.lock();
						p->infos2.emplace_back(2, 14, 0, 1);
						p->infos2_mutex.unlock();
						continue;
					}
					auto node = p->fileNodes.begin();
					if (!(*node)->isLabel() || (*node)->name != "register")
					{
						p->infos2_mutex.lock();
						p->infos2.emplace_back(2, 14, (*node)->startPos, (*node)->endPos + 1 - (*node)->startPos);
						p->infos2_mutex.unlock();
						continue;
					}
					node++;
					while (!cancel && node != p->fileNodes.end())
					{
						if ((*node)->isCommand() && (*node)->name == "return")
							break;
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
								auto f = searchFile(n->name);
								if (f.isEmpty())
								{
									p->infos2_mutex.lock();
									p->infos2.emplace_back(2, 16, n->startPos, n->endPos + 1 - n->startPos);
									p->infos2_mutex.unlock();
								}
								else if (std::find(macrofiles.begin(), macrofiles.end(), f) == macrofiles.end())
								{
									macrofiles.push_back(f);
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
								auto &m = backup_macrodata[n->name];
								m.name = n->name;
								m.definefile = thisfile;
								m.pos = (*node)->startPos;
								for (int i = 0; i < (*node)->cmdParam.size(); i++)
								{
									if (!(*node)->cmdParam[i] || (*node)->cmdParam[i]->name.isEmpty())
										m.paramqueue.emplace_back((*node)->cmdValue[i]->name, QString(""));
									else
										m.paramqueue.emplace_back((*node)->cmdParam[i]->name, (*node)->cmdValue[i]->name);
								}
							}
						}
						node++;
					}
					m_it++;
				}

				msgmutex.lock();
				macrodata = backup_macrodata;
				msgmutex.unlock();

				newmacrofile = false;
				break;
			}
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
		msgmutex.unlock();

		ParseData *p = new ParseData(sci);

		while (!stop && !cancel)
		{
			bool res = p->Parse();
			if (!res)
			{
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

		cancel = false;
	}
}