/// \file
/// \author Matthew Allen

#include "Lgi.h"
#include "GXmlTreeUi.h"
#include "GCheckBox.h"
#include "GButton.h"
#include "GRadioGroup.h"
#include "GSlider.h"
#include "GCombo.h"
#include "GEdit.h"
#include "GTree.h"
#include "GControlTree.h"
#include "GUtf8.h"

#include <ctype.h>

struct Mapping
{
	int Id;
	int Hint;
	GXmlTreeUi::CreateListItem ListItemFactory;
	GXmlTreeUi::CreateTreeItem TreeItemFactory;
	GVariant ChildElements;
	void *User;

	Mapping()
	{
		Id = 0;
		Hint = GV_NULL;
		ListItemFactory = 0;
		TreeItemFactory = 0;
		User = 0;
	}

	void LoadTree(GTreeNode *n, GXmlTag *t)
	{
		for (GXmlTag *c=t->Children.First(); c; c=t->Children.Next())
		{
			GTreeItem *i = TreeItemFactory(User);
			if (i)
			{
				i->XmlIo(c, false);
				n->Insert(i);

				LoadTree(i, c);
			}
		}
	}

	void SaveTree(GTreeNode *n, GXmlTag *t)
	{
		for (GTreeItem *i = n->GetChild(); i; i = i->GetNext())
		{
			GXmlTag *n = new GXmlTag(ChildElements.Str());
			if (n)
			{
				i->XmlIo(n, true);
				t->InsertTag(n);

				SaveTree(i, n);
			}
		}
	}
};

class GXmlTreeUiPriv
{
public:
	GHashTbl<const char*,Mapping*> Maps;
	
	~GXmlTreeUiPriv()
	{
	    Maps.DeleteObjects();
	}
};

GXmlTreeUi::GXmlTreeUi()
{
	d = new GXmlTreeUiPriv;
}

GXmlTreeUi::~GXmlTreeUi()
{
	DeleteObj(d);
}

void GXmlTreeUi::EmptyAll(GViewI *Ui)
{
	if (Ui)
	{
		for (Mapping *m=d->Maps.First(); m; m=d->Maps.Next())
		{
			if (m->Hint == GV_STRING)
				Ui->SetCtrlName(m->Id, 0);
		}
	}
}

void GXmlTreeUi::EnableAll(GViewI *Ui, bool Enable)
{
	if (Ui)
	{
		for (Mapping *m=d->Maps.First(); m; m=d->Maps.Next())
		{
			Ui->SetCtrlEnabled(m->Id, Enable);
		}
	}
}

void GXmlTreeUi::Map(const char *Attr, int UiIdent, int Type)
{
	if (UiIdent > 0 &&
		(Attr != 0 || Type == GV_DOM))
	{		
		Mapping *m = new Mapping;
		if (m)
		{
			m->Id = UiIdent;
			m->Hint = Type;
			
			LgiAssert(!d->Maps.Find(Attr));
			d->Maps.Add(Attr, m);
		}
	}
	else
	{
		LgiAssert(0);
	}
}

void GXmlTreeUi::Map(const char *Attr, int UiIdent, CreateListItem Factory, const char *ChildElements, void *User)
{
	if (Attr && UiIdent > 0 && Factory && ChildElements)
	{
		Mapping *m = new Mapping;
		if (m)
		{
			m->Id = UiIdent;
			m->ListItemFactory = Factory;
			m->Hint = GV_LIST;
			m->ChildElements = ChildElements;
			m->User = User;
			
			LgiAssert(!d->Maps.Find(Attr));
			d->Maps.Add(Attr, m);
		}
	}
	else
	{
		LgiAssert(0);
	}
}

void GXmlTreeUi::Map(const char *Attr, int UiIdent, CreateTreeItem Factory, const char *ChildElements, void *User)
{
	if (Attr && UiIdent > 0 && Factory && ChildElements)
	{
		Mapping *m = new Mapping;
		if (m)
		{
			m->Id = UiIdent;
			m->TreeItemFactory = Factory;
			m->Hint = GV_CUSTOM;
			m->ChildElements = ChildElements;
			m->User = User;

			LgiAssert(!d->Maps.Find(Attr));
			d->Maps.Add(Attr, m);
		}
	}
	else
	{
		LgiAssert(0);
	}
}

void GXmlTreeUi::EmptyMaps()
{
	d->Maps.DeleteObjects();
}

int GetCtrlType(GViewI *v)
{
	if (v)
	{
		if (dynamic_cast<GControlTree*>(v))
		{
			return GV_DOM;
		}
		else if (dynamic_cast<GCheckBox*>(v) ||
				dynamic_cast<GButton*>(v) ||
				dynamic_cast<GRadioButton*>(v))
		{
			return GV_BOOL;
		}
		else if (dynamic_cast<GSlider*>(v) ||
				dynamic_cast<GCombo*>(v) ||
				dynamic_cast<GRadioGroup*>(v))
		{
			return GV_INT32;
		}
	}

	return GV_STRING;
}

int GetDataType(char *str)
{
	if (str)
	{
		bool Float = false;

		char16 w;
		int Len = strlen(str);
		while ((w = LgiUtf8To32((uint8*&)str, Len)))
		{
			if (strchr("e \t\r\n", w))
			{
				// Doesn't really tell us anything, so ignore it.
				// The 'e' part is sometimes part of a number or
				// ignore that too.
			}
			else if (!IsDigit(w) || w > 255)
			{
				return GV_STRING;
			}
			else if (w == '.')
			{
				Float = true;
			}
		}

		if (Float)
		{
			return GV_DOUBLE;
		}

		return GV_INT32;
	}

	return GV_NULL;
}

bool GXmlTreeUi::Convert(GDom *Tag, GViewI *Ui, bool ToUI)
{
	bool Status = false;

	if (Ui && Tag)
	{
		GVariant v;
		GXmlTag *Xml = dynamic_cast<GXmlTag*>(Tag);
		if (ToUI)
		{
			// Xml -> UI
			const char *Attr;
			for (Mapping *Map = d->Maps.First(&Attr); Map; Map = d->Maps.Next(&Attr))
			{
				if (Map->Hint == GV_LIST)
				{
					if (Xml)
					{
						GXmlTag *t = Xml->GetChildTag(Attr);
						if (!t) continue;
						GList *Lst;
						if (!Ui->GetViewById(Map->Id, Lst)) continue;
						Lst->Empty();
						for (GXmlTag *c=t->Children.First(); c; c=t->Children.Next())
						{
							GListItem *i = Map->ListItemFactory(Map->User);
							if (i)
							{
								i->XmlIo(c, false);
								Lst->Insert(i);
							}
						}
					}
				}
				else if (Map->Hint == GV_CUSTOM)
				{
					if (Xml)
					{
						GXmlTag *t = Xml->GetChildTag(Attr);
						if (!t) continue;

						GTree *Tree;
						if (!Ui->GetViewById(Map->Id, Tree)) continue;
						Tree->Empty();

						Map->LoadTree(Tree, t);
					}
				}
				else if (Map->Hint == GV_DOM)
				{
					GControlTree *ct;
					if (Ui->GetViewById(Map->Id, ct))
					{
						ct->Serialize(Xml, false);
					}
				}
				else if (Tag->GetValue(Attr, v))
				{
					int Type = Map->Hint ? Map->Hint : GetDataType(v.Str());

					if (Type == GV_BOOL ||
							Type == GV_INT32 ||
							Type == GV_INT64)
					{
						Ui->SetCtrlValue(Map->Id, v.CastInt32());
					}
					else
					{
						Ui->SetCtrlName(Map->Id, v.Str());
					}
					Status = true;
				}
				else
				{
					GEdit *c;
					if (Ui->GetViewById(Map->Id, c))
						c->Name("");
				}
			}
		}
		else
		{
			// UI -> Xml
			const char *Attr;
			for (Mapping *Map = d->Maps.First(&Attr); Map; Map = d->Maps.Next(&Attr))
			{
				GViewI *c = Ui->FindControl(Map->Id);
				if (c)
				{
					int Type = Map->Hint ? Map->Hint : GetCtrlType(c);

					switch (Type)
					{
						case GV_LIST:
						{
							if (!Xml) break;
							GXmlTag *Child = Xml->GetChildTag(Attr, true);
							if (!Child) break;
							GList *Lst = dynamic_cast<GList*>(c);
							if (!Lst) break;
							Child->Empty(true);
							Child->SetTag(Attr);

							List<GListItem> All;
							Lst->GetAll(All);
							for (GListItem *i = All.First(); i; i = All.Next())
							{
								GXmlTag *n = new GXmlTag(Map->ChildElements.Str());
								if (n)
								{
									i->XmlIo(n, true);
									Child->InsertTag(n);
								}
							}
							break;
						}
						case GV_CUSTOM: // GTree
						{
							if (!Xml) break;
							GXmlTag *Child = Xml->GetChildTag(Attr, true);
							
							if (!Child) break;
							GTree *Tree = dynamic_cast<GTree*>(c);
							
							if (!Tree) break;
							Child->Empty(true);
							Child->SetTag(Attr);
							
							Map->SaveTree(Tree, Child);
							break;
						}
						case GV_INT32:
						case GV_BOOL:
						{
							Tag->SetValue(Attr, v = c->Value());
							Status = true;
							break;
						}
						case GV_DOM:
						{
							GControlTree *ct;
							if (Ui->GetViewById(Map->Id, ct))
							{
								ct->Serialize(Xml, true);
							}
							break;
						}
						default:
						{
							char *Str = c->Name();
							
							if (ValidStr(Str))
								v = Str;
							else
								v.Empty();

							Tag->SetValue(Attr, v);
							Status = true;
							break;
						}
					}
				}
				else
				{
					v.Empty();
					Tag->SetValue(Attr, v);
				}
			}
		}
	}

	return Status;
}

