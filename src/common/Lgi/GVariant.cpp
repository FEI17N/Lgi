#include <stdlib.h>
#include <stdio.h>

#include "Lgi.h"
#include "GVariant.h"
#include "GToken.h"

const char *GVariant::TypeToString(GVariantType t)
{
	switch (t)
	{
		case GV_NULL:		return "NULL";
		case GV_INT32:		return "int32";
		case GV_INT64:		return "int64";
		case GV_BOOL:		return "bool";
		case GV_DOUBLE:		return "double";
		case GV_STRING:		return "String";
		case GV_BINARY:		return "Binary";
		case GV_LIST:		return "List";
		case GV_DOM:		return "Dom";
		case GV_DOMREF:		return "DomReference";
		case GV_VOID_PTR:	return "VoidPtr";
		case GV_DATETIME:	return "DateTime";
		case GV_HASHTABLE:	return "HashTable";
		case GV_OPERATOR:	return "Operator";
		case GV_CUSTOM:		return "Custom";
		case GV_WSTRING:	return "WString";
		case GV_GSURFACE:	return "Surface";
		case GV_GVIEW:		return "View";
		case GV_GMOUSE:		return "MouseEvent";
		case GV_GKEY:		return "KeyboardEvent";
		// case GV_GFILE:		return "File";
		case GV_STREAM:		return "Stream";
	}
	
	return NULL;
}

GVariant::GVariant()
{
	Type = GV_NULL;
	User = 0;
	ZeroObj(Value);
}

GVariant::GVariant(int i)
{
	Type = GV_INT32;
	User = 0;
	Value.Int = i;
}

GVariant::GVariant(GVariant const &v)
{
	Type = GV_NULL;
	User = 0;
	ZeroObj(Value);

	*this = v;
}

GVariant::GVariant(int64 i)
{
	Type = GV_INT64;
	User = 0;
	Value.Int64 = i;
}

GVariant::GVariant(uint64 i)
{
	Type = GV_INT64;
	User = 0;
	Value.Int64 = i;
}

GVariant::GVariant(double i)
{
	Type = GV_DOUBLE;
	User = 0;
	Value.Dbl = i;
}

GVariant::GVariant(const char *s)
{
	User = 0;
	Value.String = NewStr(s);
	Type = Value.String ? GV_STRING : GV_NULL;
}

GVariant::GVariant(const char16 *s)
{
	Value.WString = NewStrW(s);
	User = 0;
	Type = Value.WString ? GV_WSTRING : GV_NULL;
}

GVariant::GVariant(void *p)
{
	Type = GV_NULL;
	User = 0;
	*this = p;
}

GVariant::GVariant(GDom *p)
{
	Type = GV_NULL;
	User = 0;
	*this = p;
}

GVariant::GVariant(GDom *p, char *name)
{
	Type = GV_NULL;
	User = 0;
	SetDomRef(p, name);
}

GVariant::GVariant(GDateTime *d)
{
	Type = GV_NULL;
	User = 0;
	*this = d;
}

GVariant::GVariant(GOperator Op)
{
	Type = GV_OPERATOR;
	User = 0;
	Value.Op = Op;
}

GVariant::~GVariant()
{
	Empty();
}

bool GVariant::operator ==(GVariant &v)
{
	switch (Type)
	{
		case GV_NULL:
			return v.Type == Type;
		case GV_INT32:
			return Value.Int == v.CastInt32();
		case GV_INT64:
			return Value.Int64 == v.CastInt64();
		case GV_BOOL:
			return Value.Bool == v.CastBool();
		case GV_DOUBLE:
			return Value.Dbl == v.CastDouble();
		case GV_STRING:
		{
			char *s = v.Str();
			if (Value.String && s)
				return !strcmp(Value.String, s);
			break;
		}
		case GV_WSTRING:
		{
			char16 *w = v.WStr();
			if (Value.WString && w)
				return !StrcmpW(Value.WString, w);
			break;
		}
		case GV_BINARY:
		{
			if (v.Type == Type &&
				Value.Binary.Data == v.Value.Binary.Data &&
				Value.Binary.Length == v.Value.Binary.Length)
			{
				return true;
			}
			break;
		}
		case GV_LIST:
		{
			if (!Value.Lst || !v.Value.Lst)
				return false;

			if (Value.Lst->Length() != v.Value.Lst->Length())
				return false;

			GVariant *a, *b;
			for (a = Value.Lst->First(), b = v.Value.Lst->First();
				a && b;
				a = Value.Lst->Next(), b = v.Value.Lst->Next())
			{
				if (!(*a == *b))
					break;
			}
			return !a && !b;
		}
		case GV_DOMREF:
		{
			return Value.DomRef.Dom == v.Value.DomRef.Dom &&
				Value.DomRef.Name != 0 &&
				v.Value.DomRef.Name != 0 &&
				!stricmp(Value.DomRef.Name, v.Value.DomRef.Name);
		}
		case GV_DATETIME:
		{
			if (Value.Date && v.Value.Date)
			{
				return Value.Date->Compare(v.Value.Date) == 0;
			}
			break;
		}
		case GV_DOM:
			return Value.Dom == v.Value.Dom;
		case GV_OPERATOR:
			return Value.Op == v.Value.Op;
		case GV_CUSTOM:
			return Value.Custom == v.Value.Custom;
		case GV_GSURFACE:
			return Value.Surface.Ptr == v.Value.Surface.Ptr;
		case GV_GVIEW:
			return Value.View == v.Value.View;
		/*
		case GV_GFILE:
			return Value.File.Ptr == v.Value.File.Ptr;
		*/
		case GV_STREAM:
			return Value.Stream.Ptr == v.Value.Stream.Ptr;
		case GV_GMOUSE:
			return Value.Mouse == v.Value.Mouse;
		case GV_GKEY:
			return Value.Key == v.Value.Key;
		case GV_VOID_PTR:
			return Value.Ptr == v.Value.Ptr;
		case GV_HASHTABLE:
		{
			LgiAssert(0);
			break;
		}
	}

	return false;
}

GVariant &GVariant::operator =(GDateTime *d)
{
	Empty();

	if (d)
	{
		Type = GV_DATETIME;
		Value.Date = new GDateTime;
		if (Value.Date)
		{
			*Value.Date = *d;
			// if (Dirty) *Dirty = true;
		}
	}

	return *this;
}

GVariant &GVariant::operator =(bool i)
{
	Empty();
	Type = GV_BOOL;
	Value.Bool = i;
	// if (Dirty) *Dirty = true;

	return *this;
}

GVariant &GVariant::operator =(int i)
{
	Empty();
	Type = GV_INT32;
	Value.Int = i;
	// if (Dirty) *Dirty = true;

	return *this;
}

#ifdef BEOS
GVariant &GVariant::operator =(int32 i)
{
	Empty();
	Type = GV_INT32;
	Value.Int = i;
	// if (Dirty) *Dirty = true;

	return *this;
}
#endif

GVariant &GVariant::operator =(int64 i)
{
	Empty();
	Type = GV_INT64;
	Value.Int64 = i;
	// if (Dirty) *Dirty = true;

	return *this;
}

GVariant &GVariant::operator =(double i)
{
	Empty();
	Type = GV_DOUBLE;
	Value.Dbl = i;
	// if (Dirty) *Dirty = true;

	return *this;
}

GVariant &GVariant::operator =(const char *s)
{
	Empty();
	if (s)
	{
		Type = GV_STRING;
		Value.String = NewStr(s);
	}

	return *this;
}

GVariant &GVariant::operator =(const char16 *s)
{
	Empty();
	if (s)
	{
		Type = GV_WSTRING;
		Value.WString = NewStrW(s);
	}
	// if (Dirty) *Dirty = true;

	return *this;
}

GVariant &GVariant::operator =(void *p)
{
	Empty();
	if (p)
	{
		Type = GV_VOID_PTR;
		Value.Ptr = p;
		// if (Dirty) *Dirty = true;
	}

	return *this;
}

GVariant &GVariant::operator =(GDom *p)
{
	Empty();
	if (p)
	{
		Type = GV_DOM;
		Value.Dom = p;
		// if (Dirty) *Dirty = true;
	}

	return *this;
}

GVariant &GVariant::operator =(GView *p)
{
	Empty();
	if (p)
	{
		Type = GV_GVIEW;
		Value.View = p;
		// if (Dirty) *Dirty = true;
	}

	return *this;
}

GVariant &GVariant::operator =(GMouse *p)
{
	Empty();
	if (p)
	{
		Type = GV_GMOUSE;
		Value.Mouse = p;
		// if (Dirty) *Dirty = true;
	}

	return *this;
}

GVariant &GVariant::operator =(GKey *p)
{
	Empty();
	if (p)
	{
		Type = GV_GKEY;
		Value.Key = p;
		// if (Dirty) *Dirty = true;
	}

	return *this;
}

GVariant &GVariant::operator =(GVariant const &i)
{
	Empty();
	Type = i.Type;
	User = i.User;

	switch (Type)
	{
		case GV_NULL:
		{
			break;
		}
		case GV_INT32:
		{
			Value.Int = i.Value.Int;
			break;
		}
		case GV_BOOL:
		{
			Value.Bool = i.Value.Bool;
			break;
		}
		case GV_INT64:
		{
			Value.Int64 = i.Value.Int64;
			break;
		}
		case GV_DOUBLE:
		{
			Value.Dbl = i.Value.Dbl;
			break;
		}
		case GV_STRING:
		{
			Value.String = NewStr(((GVariant&)i).Str());
			break;
		}
		case GV_WSTRING:
		{
			Value.WString = NewStrW(i.Value.WString);
			break;
		}
		case GV_BINARY:
		{
			SetBinary(i.Value.Binary.Length, i.Value.Binary.Data);
			break;
		}
		case GV_LIST:
		{
			SetList(i.Value.Lst);
			break;
		}
		case GV_DOM:
		{
			Value.Dom = i.Value.Dom;
			break;
		}
		case GV_VOID_PTR:
		case GV_GVIEW:
		case GV_GMOUSE:
		case GV_GKEY:
		{
			Value.Ptr = i.Value.Ptr;
			break;
		}
		case GV_DATETIME:
		{
			if (i.Value.Date)
			{
				Value.Date = new GDateTime;
				if (Value.Date)
				{
					*Value.Date = *i.Value.Date;
				}
			}
			break;
		}
		case GV_HASHTABLE:
		{
			if ((Value.Hash = new GHashTable))
			{
				char *k;
				if (i.Value.Hash)
				{
					for (void *p = i.Value.Hash->First(&k); p; p = i.Value.Hash->Next(&k))
					{
						GVariant *var = (GVariant*)p;
						Value.Hash->Add(k, new GVariant(*var));
					}
				}
			}
			break;
		}
		case GV_CUSTOM:
		{
			Value.Custom.Data = i.Value.Custom.Data;
			Value.Custom.Dom = i.Value.Custom.Dom;
			break;
		}
		case GV_GSURFACE:
		{
			Value.Surface = i.Value.Surface;
			if (Value.Surface.Own &&
				Value.Surface.Ptr)
				Value.Surface.Ptr->AddRef();
			break;
		}
		/*
		case GV_GFILE:
		{
			Value.File = i.Value.File;
			if (Value.File.Own &&
				Value.File.Ptr)
				Value.File.Ptr->AddRef();
			break;
		}
		*/
		case GV_STREAM:
		{
			Value.Stream.Ptr = i.Value.Stream.Ptr;
			Value.Stream.Own = false;			
			break;
		}
		default:
		{
			printf("%s:%i - Unknown variant type '%i'\n", _FL, Type);
			LgiAssert(0);
			break;
		}
	}

	// if (Dirty) *Dirty = true;

	return *this;
}

bool GVariant::SetDomRef(GDom *obj, char *name)
{
	Empty();

	Type = GV_DOMREF;
	Value.DomRef.Dom = obj;
	Value.DomRef.Name = NewStr(name);

	return Value.DomRef.Name != 0;
}

bool GVariant::SetBinary(int Len, void *Data, bool Own)
{
	bool Status = false;

	Empty();

	Type = GV_BINARY;
	Value.Binary.Length = Len;
	if (Own)
	{
		Value.Binary.Data = Data;
		Status = true;
	}
	else
	{
		if ((Value.Binary.Data = new uchar[Value.Binary.Length]))
		{
			if (Data)
				memcpy(Value.Binary.Data, Data, Value.Binary.Length);
			else
				memset(Value.Binary.Data, 0, Value.Binary.Length);

			Status = true;
		}
	}

	return Status;
}

bool GVariant::SetList(List<GVariant> *Lst)
{
	Empty();
	Type = GV_LIST;

	if ((Value.Lst = new List<GVariant>) && Lst)
	{
		for (GVariant *s=Lst->First(); s; s=Lst->Next())
		{
			GVariant *New = new GVariant;
			if (New)
			{
				*New = *s;
				Value.Lst->Insert(New);
			}
		}
	}

	return Value.Lst != 0;
}

bool GVariant::SetHashTable(GHashTable *Table, bool Copy)
{
	Empty();
	Type = GV_HASHTABLE;

	if (Copy && Table)
	{
		if ((Value.Hash = new GHashTable))
		{
			char *k;
			for (void *p = Table->First(&k); p; p = Table->Next(&k))
			{
				Value.Hash->Add(k, p);
			}
		}
	}
	else
	{
		Value.Hash = Table ? Table : new GHashTable;
	}

	return Value.Hash != 0;
}

bool GVariant::SetSurface(class GSurface *Ptr, bool Own)
{
    Empty();
    Type = GV_GSURFACE;
    Value.Surface.Ptr = Ptr;
    if ((Value.Surface.Own = Own))
    {
        Value.Surface.Ptr->AddRef();
    }
    
    return true;
}

void GVariant::OwnStr(char *s)
{
	Empty();
	Type = s ? GV_STRING : GV_NULL;
	Value.String = s;
}

void GVariant::OwnStr(char16 *w)
{
	Empty();
	Type = w ? GV_WSTRING : GV_NULL;
	Value.WString = w;
}

char *GVariant::ReleaseStr()
{
	char *Ret = Str();
	if (Ret)
	{
		Value.String = 0;
		Type = GV_NULL;
	}
	return Ret;
}

char *GVariant::Str()
{
	if (Type == GV_STRING)
		return Value.String;

	if (Type == GV_WSTRING)
	{
		char *u = LgiNewUtf16To8(Value.WString);
		DeleteArray(Value.WString);
		Type = GV_STRING;
		return Value.String = u;
	}

	return 0;
}

char16 *GVariant::ReleaseWStr()
{
	char16 *Ret = WStr();
	if (Ret)
	{
		Value.WString = 0;
		Type = GV_NULL;
	}
	return Ret;
}

char16 *GVariant::WStr()
{
	if (Type == GV_WSTRING)
		return Value.WString;

	if (Type == GV_STRING)
	{
		char16 *w = LgiNewUtf8To16(Value.String);
		DeleteArray(Value.String);
		Type = GV_WSTRING;
		return Value.WString = w;
	}

	return 0;
}

void GVariant::Empty()
{
	switch (Type)
	{
		default:
			break;
		case GV_CUSTOM:
		{
			Value.Custom.Data = 0;
			Value.Custom.Dom = 0;
			break;
		}
		case GV_DOMREF:
		{
			DeleteArray(Value.DomRef.Name);
			Value.DomRef.Dom = 0;
			break;
		}
		case GV_STRING:
		{
			DeleteArray(Value.String);
			break;
		}
		case GV_WSTRING:
		{
			DeleteArray(Value.WString);
			break;
		}
		case GV_BINARY:
		{
			char *d = (char*) Value.Binary.Data;
			DeleteArray(d);
			Value.Binary.Data = 0;
			break;
		}
		case GV_DATETIME:
		{
			DeleteObj(Value.Date);
			break;
		}
		case GV_LIST:
		{
			if (Value.Lst)
			{
				Value.Lst->DeleteObjects();
				DeleteObj(Value.Lst);
			}
			break;
		}
		case GV_HASHTABLE:
		{
			if (Value.Hash)
			{
				for (GVariant *v = (GVariant*) Value.Hash->First(); v; v = (GVariant*) Value.Hash->Next())
				{
					DeleteObj(v);
				}
				DeleteObj(Value.Hash);
			}
			break;
		}
		case GV_GSURFACE:
		{
		    if (Value.Surface.Own &&
		        Value.Surface.Ptr)
		    {
				Value.Surface.Ptr->DecRef();
				Value.Surface.Ptr = NULL;
		    }
		    break;
		}
		/*
		case GV_GFILE:
		{
			if (Value.File.Ptr &&
				Value.File.Own)
			{
				Value.File.Ptr->DecRef();
				Value.File.Ptr = NULL;
			}
			break;
		}
		*/
		case GV_STREAM:
		{
			if (Value.Stream.Ptr)
			{
				if (Value.Stream.Own)
					delete Value.Stream.Ptr;
				Value.Stream.Ptr = NULL;
			}
			break;
		}
	}

	Type = GV_NULL;
	ZeroObj(Value);
}

int64 GVariant::Length()
{
	switch (Type)
	{
		case GV_NULL:
			return 0;
		case GV_INT32:
			return sizeof(Value.Int);
		case GV_INT64:
			return sizeof(Value.Int64);
		case GV_BOOL:
			return sizeof(Value.Bool);
		case GV_DOUBLE:
			return sizeof(Value.Dbl);
		case GV_STRING:
			return Value.String ? strlen(Value.String) : 0;
		case GV_BINARY:
			return Value.Binary.Length;
		case GV_LIST:
		{
			int64 Sz = 0;
			if (Value.Lst)
			{
				List<GVariant>::I it = Value.Lst->Start();
				for (GVariant *v=*it; v; v=*++it)
					Sz += v->Length();
			}
			return Sz;
		}
		case GV_DOM:
		{
			GVariant v;
			if (Value.Dom)
				Value.Dom->GetValue("length", v);
			return v.CastInt32();
		}
		case GV_DOMREF:
			break;
		case GV_VOID_PTR:
			return sizeof(Value.Ptr);
		case GV_DATETIME:
			return sizeof(*Value.Date);
		case GV_HASHTABLE:
		{
			int64 Sz = 0;
			if (Value.Hash)
			{
				for (GVariant *v=(GVariant*)Value.Hash->First();
					v;
					v=(GVariant*)Value.Hash->Next())
					Sz += v->Length();
			}
			return Sz;
		}
		case GV_OPERATOR:
			return sizeof(Value.Op);
		case GV_CUSTOM:
			break;
		case GV_WSTRING:
			return Value.WString ? StrlenW(Value.WString) * sizeof(char16) : 0;
		case GV_GSURFACE:
		{
			int64 Sz = 0;
			if (Value.Surface.Ptr)
			{
				GRect r = Value.Surface.Ptr->Bounds();
				int Bytes = Value.Surface.Ptr->GetBits() >> 3;
				Sz = r.X() * r.Y() * Bytes;
			}
			return Sz;
		}
		case GV_GVIEW:
			return sizeof(GView);
		case GV_GMOUSE:
			return sizeof(GMouse);
		case GV_GKEY:
			return sizeof(GKey);
		case GV_STREAM:
			return Value.Stream.Ptr->GetSize();
	}
	
	return 0;
}

bool GVariant::IsInt()
{
	return Type == GV_INT32 || Type == GV_INT64;
}

bool GVariant::IsBool()
{
	return Type == GV_BOOL;
}

bool GVariant::IsDouble()
{
	return Type == GV_DOUBLE;
}

bool GVariant::IsString()
{
	return Type == GV_STRING;
}

bool GVariant::IsBinary()
{
	return Type == GV_BINARY;
}

bool GVariant::IsNull()
{
	return Type == GV_NULL;
}

#define IsList() (Type == GV_LIST && Value.Lst)

GVariant &GVariant::Cast(GVariantType NewType)
{
	if (NewType != Type)
	{
		switch (NewType)
		{
			default:
			{
				// No conversion possible
				break;
			}
			case GV_INT32:
			{
				*this = (int)CastInt32();
				break;
			}
			case GV_INT64:
			{
				*this = CastInt64();
				break;
			}
			case GV_BOOL:
			{
				if (Type == GV_DOUBLE)
				{
					*this = Value.Dbl != 0.0;
				}
				else
				{
					*this = CastInt32() != 0;
				}
				break;
			}
			case GV_DOUBLE:
			{
				*this = CastDouble();
				break;
			}
			case GV_STRING:
			{
				*this = CastString();
				break;
			}
			case GV_DATETIME:
			{
				switch (Type)
				{
					case GV_STRING:
					{
						// String -> GDateTime
						GDateTime *Dt = new GDateTime;
						if (Dt)
						{
							Dt->Set(Value.String);
							Empty();
							Value.Date = Dt;
							Type = NewType;
						}
						break;
					}
					case GV_INT64:
					{
						// Int64 (system date) -> GDateTime
						GDateTime *Dt = new GDateTime;
						if (Dt)
						{
							Dt->Set((uint64)Value.Int64);
							Empty();
							Value.Date = Dt;
							Type = NewType;
						}
						break;
					}
					default:
					{
						// No conversion available
						break;
					}
				}
				break;
			}
		}
	}

	return *this;
}

void *GVariant::CastVoidPtr()
{
	switch (Type)
	{
		default:
			break;
		case GV_STRING:
			return Value.String;
		case GV_BINARY:
			return Value.Binary.Data;
		case GV_LIST:
			return Value.Lst;
		case GV_DOM:
			return Value.Dom;
		case GV_DOMREF:
			return Value.DomRef.Dom;
		case GV_VOID_PTR:
			return Value.Ptr;
		case GV_DATETIME:
			return Value.Date;
		case GV_HASHTABLE:
			return Value.Hash;
		case GV_CUSTOM:
			return Value.Custom.Data;
		case GV_WSTRING:
			return Value.WString;
		case GV_GSURFACE:
			return Value.Surface.Ptr;
		case GV_GVIEW:
			return Value.View;
		case GV_GMOUSE:
			return Value.Mouse;
		case GV_GKEY:
			return Value.Key;
	}

	return 0;
}

GDom *GVariant::CastDom()
{
	switch (Type)
	{
		default:
			break;
		case GV_DOM:
			return Value.Dom;
		case GV_DOMREF:
			return Value.DomRef.Dom;
		case GV_STREAM:
			return Value.Stream.Ptr;
		case GV_GSURFACE:
			return Value.Surface.Ptr;
		case GV_CUSTOM:
			return Value.Custom.Dom;
	}

	return NULL;
}

bool GVariant::CastBool()
{
	switch (Type)
	{
		default:
			LgiAssert(0);
			break;
		case GV_NULL:
			return false;
		case GV_INT32:
			return Value.Int != 0;
		case GV_INT64:
			return Value.Int64 != 0;
		case GV_BOOL:
			return Value.Bool;
		case GV_DOUBLE:
			return Value.Dbl != 0.0;
		case GV_BINARY:
			return Value.Binary.Data != NULL;
		case GV_LIST:
			return Value.Lst != NULL;
		case GV_DOM:
			return Value.Dom != NULL;
		case GV_DOMREF:
			return Value.DomRef.Dom != NULL;
		case GV_VOID_PTR:
			return Value.Ptr != NULL;
		case GV_GVIEW:
			return Value.View != NULL;
		case GV_GMOUSE:
			return Value.Mouse != NULL;
		case GV_GKEY:
			return Value.Key != NULL;
		case GV_DATETIME:
			return Value.Date != NULL;
		case GV_HASHTABLE:
			return Value.Hash != NULL;
		case GV_OPERATOR:
			return Value.Op != OpNull;
		case GV_CUSTOM:
			return Value.Custom.Dom != 0 && Value.Custom.Data != 0;
		/*
		case GV_GFILE:
			return Value.File.Ptr != NULL;
		*/
		case GV_STREAM:
			return Value.Stream.Ptr != NULL;

		// As far as I understand this is the behavour in Python, which I'm using for
		// a reference to what the "correct" thing to do here is. Basically it's treating
		// the string like a pointer instead of a value. If the pointer is valid the
		// conversion to bool return true, and false if it's not a valid pointer. This
		// means things like if (!StringVariant) evaluate correctly in the scripting engine
		// but it means that if you want to evaluate the value of the varient you should
		// use CastInt32 instead.
		case GV_STRING:
			return ValidStr(Value.String);
		case GV_WSTRING:
			return ValidStrW(Value.WString);
	}

	return false;
}

double GVariant::CastDouble()
{
	switch (Type)
	{
		default:
			break;
		case GV_BOOL:
			return Value.Bool ? 1.0 : 0.0;
		case GV_DOUBLE:
			return Value.Dbl;
		case GV_INT32:
			return (double)Value.Int;
		case GV_INT64:
			return (double)Value.Int64;
		case GV_STRING:
			return Value.String ? atof(Value.String) : 0;
		case GV_DOMREF:
		{
			static GVariant v;
			if (Value.DomRef.Dom)
			{
				if (Value.DomRef.Dom->GetValue(Value.DomRef.Name, v))
				{
					return v.CastDouble();
				}
			}
			break;
		}
	}
	
	return 0;
}

int32 GVariant::CastInt32()
{
	switch (Type)
	{
		default:
			break;
		case GV_BOOL:
			return (int32)Value.Bool;
		case GV_DOUBLE:
			return (int32)Value.Dbl;
		case GV_INT32:
			return Value.Int;
		case GV_INT64:
			return (int32)Value.Int64;
		case GV_STRING:
			return Value.String ? atoi(Value.String) : 0;
		case GV_DOM:
			return Value.Dom != 0;
		case GV_DOMREF:
		{
			static GVariant v;
			if (Value.DomRef.Dom)
			{
				if (Value.DomRef.Dom->GetValue(Value.DomRef.Name, v))
				{
					return v.CastInt32();
				}
			}
			break;
		}
		case GV_LIST:
			return Value.Lst != 0;
		case GV_HASHTABLE:
			return Value.Hash != 0;
	}
	
	return 0;
}

int64 GVariant::CastInt64()
{
	switch (Type)
	{
		default:
			break;
		case GV_BOOL:
			return (int64)Value.Bool;
		case GV_DOUBLE:
			return (int64)Value.Dbl;
		case GV_INT32:
			return Value.Int;
		case GV_INT64:
			return Value.Int64;
		case GV_STRING:
		{
			if (Value.String)
			{
				#ifdef _MSC_VER
				return _atoi64(Value.String);
				#else
				return atoll(Value.String);
				#endif
			}
			break;
		}
		case GV_DOMREF:
		{
			static GVariant v;
			if (Value.DomRef.Dom)
			{
				if (Value.DomRef.Dom->GetValue(Value.DomRef.Name, v))
				{
					return v.CastInt64();
				}
			}
			break;
		}
	}
	
	return 0;
}

char *GVariant::CastString()
{
	char i[40];

	switch (Type)
	{
		default:
			break;
		case GV_LIST:
		{
			GStringPipe p(256);
			
			List<GVariant>::I it = Value.Lst->Start();
			bool First = true;
			
			p.Print("{");
			for (GVariant *v = *it; v; v = *++it)
			{
				p.Print("%s%s", First ? "" : ", ", v->CastString());
				First = false;
			}
			p.Print("}");
			OwnStr(p.NewStr());
			return Str();
			break;
		}
		case GV_HASHTABLE:
		{
			GStringPipe p(256);

			p.Print("{");
			
			char *k;
			bool First = true;
			for (GVariant *v = (GVariant*)Value.Hash->First(&k);
				v;
				v = (GVariant*)Value.Hash->Next(&k))
			{
				p.Print("%s%s = %s", First ? "" : ", ", k, v->CastString());
				First = false;
			}
			p.Print("}");
			OwnStr(p.NewStr());
			return Str();
			break;
		}
		case GV_DOMREF:
		{
			static GVariant v;
			if (Value.DomRef.Dom)
			{
				if (Value.DomRef.Dom->GetValue(Value.DomRef.Name, v))
				{
					return v.CastString();
				}
			}
			break;
		}
		case GV_INT32:
		{
			sprintf_s(i, sizeof(i), "%i", Value.Int);
			*this = i;
			return Str();
		}
		case GV_DOUBLE:
		{
			sprintf_s(i, sizeof(i), "%f", Value.Dbl);
			*this = i;
			return Str();
		}
		case GV_BOOL:
		{
			sprintf_s(i, sizeof(i), "%i", Value.Bool);
			*this = i;
			return Str();
		}
		case GV_INT64:
		{
			sprintf_s(i, sizeof(i), LGI_PrintfInt64, Value.Int64);
			*this = i;
			return Str();
		}
		case GV_STRING:
		case GV_WSTRING:
		{
			return Str();
		}
		case GV_DATETIME:
		{
			if (Value.Date)
			{
				char s[64];
				Value.Date->Get(s, sizeof(s));
				*this = s;
				return Str();
			}
			break;
		}
	}
	
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////
GDom *GDom::ResolveObject(const char *Var, char *Name, char *Array)
{
	GDom *Object = this;

	// Tokenise the string
	GArray<char*> t;
	for (const char *s = Var; s && *s; )
	{
		const char *e = s;
		while (*e && *e != '.')
		{
			if (*e == '[')
			{
				e++;
				while (*e && *e != ']')
				{
					if (*e == '\"' || *e == '\'')
					{
						char d = *e++;
						while (*e && *e != d)
						{
							e++;
						}
						if (*e == d) e++;
					}
					else e++;
				}
				if (*e == ']') e++;
			}
			else e++;
		}
		
		size_t Len = e - s;
		if (Len < 1 || Len > 255)
		{
			Object = 0;
			break;
		}
		
		char Part[256];
		memcpy(Part, s, Len);
		Part[Len] = 0;
		while (strchr(" \t", Part[Len-1]))
		{
			Len--;
		}

		t[t.Length()] = NewStr(Part, Len);
		s = *e ? e + 1 : e;
	}

	// Process elements
	for (int i=0; i<t.Length(); i++)
	{
		char Base[256];
		char Index[256] = "";

		// Parse out object name parts
		char *Obj = t[i];
		if (*Obj == '[')
		{
			Object = 0;
			break;
		}
		
		char *ArrStart = strchr(Obj, '[');
		if (ArrStart)
		{
			*ArrStart++ = 0;
			char *ArrEnd = 0;
			if (strchr("\'\"", *ArrStart))
			{
				ArrEnd = strchr(ArrStart + 1, ArrStart[0]);
				ArrStart++;
			}
			else
			{
				ArrEnd = strchr(ArrStart, ']');
			}
			if (ArrEnd)
			{
				*ArrEnd = 0;
				strcpy_s(Index, sizeof(Index), ArrStart);
			}
		}
		strcpy_s(Base, sizeof(Base), Obj);

		// Process parts
		if (i == t.Length()-1)
		{
			strcpy_s(Name, 256, Base);
			strcpy_s(Array, 256, Index);
			
			char *e = Name + strlen(Name) - 1;
			while (e > Name && strchr(" \t\r\n", *e))
			{
				*e-- = 0;
			}
		}
		else
		{
			GVariant v;
			if (Index[0])
			{
				if (Object->GetVariant(Base, v, Index))
				{
					if (v.Type == GV_LIST)
					{
						int N = atoi(Index);
						GVariant *Element = v.Value.Lst->ItemAt(N);
						if (Element && Element->Type == GV_DOM)
						{
							Object = Element->Value.Dom;
						}
						else
						{
							Object = 0;
							goto ResolveDone;
						}
					}
					else if (v.Type == GV_DOM)
					{
						Object = v.Value.Dom;
					}
					else
					{
						Object = 0;
						goto ResolveDone;
					}
				}
				else
				{
					Object = 0;
					goto ResolveDone;
				}
			}
			else
			{
				if (Object->GetVariant(Obj, v) &&
					v.Type == GV_DOM)
				{
					Object = v.Value.Dom;
				}
				else
				{
					Object = 0;
					goto ResolveDone;
				}
			}
		}
	}

ResolveDone:
	t.DeleteArrays();
	return Object;
}

struct GDomPropMap : public GHashTbl<const char *, GDomProperty>
{
	GDomPropMap() : GHashTbl<const char *, GDomProperty>(0, false, NULL, ObjNone)
	{
		Define("Length", ObjLength);
		Define("Type", ObjType);
		Define("Name", ObjName);
		Define("Style", ObjStyle);
		Define("Class", ObjClass);
		Define("Field", ObjField);
		Define("Debug", ObjDebug);
		Define("textContent", ObjTextContent);
		Define("innerHTML", ObjInnerHtml);

		Define("List", TypeList);
		Define("HashTable", TypeHashTable);
		Define("File", TypeFile);
		Define("Surface", TypeSurface);
		Define("Int", TypeInt);
		Define("Double", TypeDouble);
		Define("String", TypeString);
		Define("DateTime", TypeDateTime);
		
		Define("Year", DateYear);
		Define("Month", DateMonth);
		Define("Day", DateDay);
		Define("Hour", DateHour);
		Define("Minute", DateMin);
		Define("Second", DateSec);
		Define("Date", DateDate);
		Define("Time", DateTime);
		Define("DateAndTime", DateDateAndTime);
		Define("DateInt64", DateInt64);
		Define("SetNow", DateSetNow);

		Define("Join", StrJoin);
		Define("Split", StrSplit);
		Define("Find", StrFind);
		Define("Rfind", StrRfind);
		Define("Lower", StrLower);
		Define("Upper", StrUpper);
		Define("Strip", StrStrip);
		Define("Sub", StrSub);
		
		Define("X", SurfaceX);
		Define("Y", SurfaceY);
		Define("Bits", SurfaceBits);
		Define("ColourSpace", SurfaceColourSpace);
		Define("IncludeCursor", SurfaceIncludeCursor);

		Define("Add", ContainerAdd);
		Define("Delete", ContainerDelete);
		Define("HasKey", ContainerHasKey);
		Define("Sort", ContainerSort);
		Define("Children", ContainerChildren);
		Define("Span", ContainerSpan);
		Define("Align", ContainerAlign);
		Define("VAlign", ContainerVAlign);

		Define("Open", FileOpen);
		Define("Read", FileRead);
		Define("Write", FileWrite);
		Define("Pos", FilePos);
		Define("Close", FileClose);
		Define("Modified", FileModified);
		Define("Folder", FileFolder);
		Define("Encoding", FileEncoding);
	}
	
	void Define(const char *s, GDomProperty p)
	{
		#if defined(_DEBUG)
		GDomProperty e = Find(s);
		LgiAssert(e == ObjNone);
		#endif
		Add(s, p);
	} 
	
} DomPropMap;

GDomProperty GStringToProp(const char *Str)
{
	return DomPropMap.Find(Str);
}

bool GDom::GetValue(const char *Var, GVariant &Value)
{
	bool Status = false;

	if (Var)
	{
		if (_OnAccess(true))
		{
			char Name[256] = "", Arr[256] = "";
			GDom *Object = ResolveObject(Var, Name, Arr);
			if (Object)
			{
				if (Name[0] == 0)
				{
					LgiTrace("%s:%i - Warning name parse failed for '%s'\n", _FL, Var);
				}
				else
				{
					bool ValidArray = ValidStr(Arr);
					Status = Object->GetVariant(Name, Value, ValidArray ? Arr : 0);
				}
			}

			_OnAccess(false);
		}
		else
		{
			LgiTrace("%s:%i - Locking error\n", _FL);
			LgiAssert(0);
		}
	}

	return Status;
}

bool GDom::SetValue(const char *Var, GVariant &Value)
{
	bool Status = false;

	if (Var)
	{
		// GMutex *Sem = dynamic_cast<GMutex*>(this);
		if (_OnAccess(true))
		{
			char Name[256], Arr[256] = "";
			GDom *Object = ResolveObject(Var, Name, Arr);
			if (Object)
			{
				if (Name[0] == 0)
					LgiTrace("%s:%i - Warning name parse failed for '%s'\n", _FL, Var);
				else
				{
					bool Valid = ValidStr(Arr);
					Status = Object->SetVariant(Name, Value, Valid ? Arr : 0);
				}
			}

			_OnAccess(false);
		}
		else
		{
			LgiTrace("%s:%i - Locking error\n", _FL);
			LgiAssert(0);
		}
	}

	return Status;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
GCustomType::GCustomType(const char *name, int pack) : FldMap(0, true, NULL, -1)
{
	Name = name;
	Pack = 1;
	Size = 0;
}

GCustomType::GCustomType(const char16 *name, int pack) : FldMap(0, true, NULL, -1)
{
	Name = name;
	Pack = 1;
	Size = 0;
}

GCustomType::~GCustomType()
{
}

size_t GCustomType::Sizeof()
{
	return (size_t)PadSize();
}

int GCustomType::PadSize()
{
	if (Pack > 1)
	{
		// Bump size to the pack boundary...
		int Remain = Size % Pack;
		if (Remain)
			return Size + Pack - Remain;
	}
	return Size;
}

int GCustomType::IndexOf(const char *Field)
{
	return FldMap.Find(Field);
}

int GCustomType::AddressOf(const char *Field)
{
	if (!Field)
		return -1;
	for (unsigned i=0; i<Flds.Length(); i++)
	{
		if (!strcmp(Flds[i].Name, Field))
			return (int)i;
	}
	return -1;
}

bool GCustomType::DefineField(const char *Name, GCustomType *Type, int ArrayLen)
{
	if (ArrayLen < 1)
	{
		LgiAssert(!"Can't have zero size field.");
		return false;
	}

	if (Name == NULL || Type == NULL)
	{
		LgiAssert(!"Invalid parameter.");
		return false;
	}

	if (FldMap.Find(Name) >= 0)
	{
		LgiAssert(!"Field already exists.");
		return false;
	}
	FldMap.Add(Name, Flds.Length());

	CustomField &Def = Flds.New();
	
	Size = PadSize();
	Def.Offset = Size;
	
	Def.Name = Name;
	Def.Type = GV_CUSTOM;
	Def.Bytes = Type->Sizeof();
	Def.ArrayLen = ArrayLen;
	Size += Def.Bytes * ArrayLen;

	return true;
}

bool GCustomType::DefineField(const char *Name, GVariantType Type, int Bytes, int ArrayLen)
{
	if (ArrayLen < 1)
	{
		LgiAssert(!"Can't have zero size field.");
		return false;
	}

	if (Name == NULL)
	{
		LgiAssert(!"No field name.");
		return false;
	}

	if (FldMap.Find(Name) >= 0)
	{
		LgiAssert(!"Field already exists.");
		return false;
	}
	FldMap.Add(Name, Flds.Length());

	CustomField &Def = Flds.New();
	
	Size = PadSize();
	Def.Offset = Size;
	
	Def.Name = Name;
	Def.Type = Type;
	Def.Bytes = Bytes;
	Def.ArrayLen = ArrayLen;
	Size += Bytes * ArrayLen;

	return true;
}

GCustomType::Method *GCustomType::GetMethod(const char *Name)
{
	return MethodMap.Find(Name);
}

GCustomType::Method *GCustomType::DefineMethod(const char *Name, GArray<GString> &Params, int Address)
{
	Method *m = MethodMap.Find(Name);
	if (m)
	{
		LgiAssert(!"Method already defined.");
		return NULL;
	}
	
	m = &Methods.New();
	m->Name = Name;
	m->Params = Params;
	m->Address = Address;
	
	MethodMap.Add(Name, m);	
	
	return m;
}

bool GCustomType::CustomField::GetVariant(const char *Field, GVariant &Value, char *Array)
{
	GDomProperty p = GStringToProp(Field);
	switch (p)
	{
		case ObjName: // Type: String
			Value = Name;
			break;
		case ObjLength: // Type: Int32
			Value = Bytes;
			break;
		default:
			return false;
	}
	
	return true;
}

int GCustomType::CustomField::Sizeof()
{
	switch (Type)
	{
		case GV_INT32:
			return sizeof(int32);
		case GV_INT64:
			return sizeof(int64);
		case GV_BOOL:
			return sizeof(bool);
		case GV_DOUBLE:
			return sizeof(double);
		case GV_STRING:
			return sizeof(char);
		case GV_DATETIME:
			return sizeof(GDateTime);
		case GV_HASHTABLE:
			return sizeof(GHashTable);
		case GV_OPERATOR:
			return sizeof(GOperator);
		case GV_GMOUSE:
			return sizeof(GMouse);
		case GV_GKEY:
			return sizeof(GKey);
		case GV_CUSTOM:
			return Nested->Sizeof();
		default:
			LgiAssert(!"Unknown type.");
			break;
	}
	
	return 0;
}

bool GCustomType::Get(int Index, GVariant &Out, uint8 *This, int ArrayIndex)
{
	if (Index < 0 ||
		Index >= Flds.Length() ||
		!This)
	{
		LgiAssert(!"Invalid parameter error.");
		return false;
	}

	CustomField &Def = Flds[Index];
	if (ArrayIndex < 0 || ArrayIndex >= Def.ArrayLen)
	{
		LgiAssert(!"Array out of bounds.");
		return false;
	}

	uint8 *Ptr = This + Def.Offset;
	Out.Empty();
	
	switch (Def.Type)
	{
		case GV_STRING:
		{
			int Len;
			for (Len = 0; Ptr[Len] && Len < Def.ArrayLen-1; Len++)
				;
			Out.OwnStr(NewStr((char*)Ptr, Len));
			break;
		}
		case GV_WSTRING:
		{
			char16 *p = (char16*)Ptr;
			int Len;
			for (Len = 0; p[Len] && Len < Def.ArrayLen-1; Len++)
				;
			Out.OwnStr(NewStrW(p, Len));
			break;
		}
		case GV_INT32:
		case GV_INT64:
		{
			switch (Def.Bytes)
			{
				case 1:
				{
					Out.Value.Int = Ptr[ArrayIndex];
					Out.Type = GV_INT32;
					break;
				}
				case 2:
				{
					Out.Value.Int = ((uint16*)Ptr)[ArrayIndex];
					Out.Type = GV_INT32;
					break;
				}
				case 4:
				{
					Out.Value.Int = ((uint32*)Ptr)[ArrayIndex];
					Out.Type = GV_INT32;
					break;
				}
				case 8:
				{
					Out.Value.Int64 = ((uint64*)Ptr)[ArrayIndex];
					Out.Type = GV_INT64;
					break;
				}
				default:
				{
					LgiAssert(!"Unknown integer size.");
					return false;
				}
			}
			break;			
		}
		case GV_MAX:
		{
			Out = *((GVariant*)Ptr);
			break;
		}
		default:
		{
			LgiAssert(!"Impl this type.");
			return false;
		}
	}

	return true;
}

bool GCustomType::Set(int Index, GVariant &In, uint8 *This, int ArrayIndex)
{
	if (Index < 0 ||
		Index >= Flds.Length() ||
		!This)
	{
		LgiAssert(!"Invalid parameter error.");
		return false;
	}

	CustomField &Def = Flds[Index];
	uint8 *Ptr = This + Def.Offset;
	if (ArrayIndex < 0 || ArrayIndex >= Def.ArrayLen)
	{
		LgiAssert(!"Array out of bounds.");
		return false;
	}
	
	switch (Def.Type)
	{
		case GV_STRING:
		{
			char *s = In.Str();
			if (!s)
			{
				*Ptr = 0;
				break;
			}
			
			if (Def.Bytes == 1)
			{
				// Straight up string copy...
				if (s)
					strcpy_s((char*)Ptr, Def.ArrayLen, s);
				else
					*Ptr = 0;
			}
			else if (Def.Bytes == sizeof(char16))
			{
				// utf8 -> wide conversion...
				const void *In = Ptr;
				int Len = strlen(s);
				int Ch = LgiBufConvertCp(Ptr, LGI_WideCharset, Def.ArrayLen-1, In, "utf-8", Len);
				if (Ch >= 0)
				{
					// Null terminate
					Ptr[Ch] = 0;
				}
				else
				{
					LgiAssert(!"LgiBufConvertCp failed.");
					return false;
				}
			}
			break;
		}
		case GV_WSTRING:
		{
			char16 *p = (char16*)Ptr;
			char16 *w = In.WStr();
			if (!w)
			{
				*p = 0;
				break;
			}
			if (Def.Bytes == sizeof(char16))
			{
				// Straight string copy...
				wcscpy_s(p, Def.ArrayLen, w);
			}
			else
			{
				// Conversion to utf-8
				const void *In = Ptr;
				int Len = StrlenW(w) * sizeof(char16);
				int Ch = LgiBufConvertCp(Ptr, "utf-8", Def.ArrayLen-sizeof(char16),
										In, LGI_WideCharset, Len);
				if (Ch >= 0)
				{
					// Null terminate
					p[Ch/sizeof(char16)] = 0;
				}
				else
				{
					LgiAssert(!"LgiBufConvertCp failed.");
					return false;
				}
			}
			break;
		}
		case GV_INT32:
		case GV_INT64:
		{
			switch (Def.Bytes)
			{
				case 1:
				{
					Ptr[ArrayIndex] = In.CastInt32();
					break;
				}
				case 2:
				{
					((uint16*)Ptr)[ArrayIndex] = In.CastInt32();
					break;
				}
				case 4:
				{
					((uint32*)Ptr)[ArrayIndex] = In.CastInt32();
					break;
				}
				case 8:
				{
					((uint64*)Ptr)[ArrayIndex] = In.CastInt64();
					break;
				}
				default:
				{
					LgiAssert(!"Unknown integer size.");
					return false;
				}
			}
			break;			
		}
		case GV_MAX:
		{
			*((GVariant*)Ptr) = In;
			break;
		}
		default:
			LgiAssert(!"Impl this type.");
			break;
	}

	return true;
}

bool GCustomType::GetVariant(const char *Field, GVariant &Value, char *Array)
{
	GDomProperty p = GStringToProp(Field);
	switch (p)
	{
		case ObjName: // Type: String
		{
			Value = Name;
			return true;
		}
		case ObjType: // Type: String
		{
			Value = "GCustomType";
			return true;
		}
		case ObjLength: // Type: Int32
		{
			Value = (int)Sizeof();
			return true;
		}
		case ObjField: // Type: CustomField[]
		{
			if (Array)
			{
				int Index = atoi(Array);
				if (Index >= 0 && Index < Flds.Length())
				{
					Value = (GDom*)&Flds[Index];
					return true;
				}			
			}
			else
			{
				Value = (int)Flds.Length();
				break;
			}			
			break;
		}
	}

	LgiAssert(0);
	return false;
}

bool GCustomType::SetVariant(const char *Name, GVariant &Value, char *Array)
{
	LgiAssert(0);
	return false;
}

bool GCustomType::CallMethod(const char *MethodName, GVariant *ReturnValue, GArray<GVariant*> &Args)
{
	if (!MethodName || !ReturnValue)
		return false;
	
	if (!_stricmp(MethodName, "New"))
	{
		ReturnValue->Empty();
		ReturnValue->Type = GV_CUSTOM;
		ReturnValue->Value.Custom.Dom = this;
		ReturnValue->Value.Custom.Data = new uint8[Sizeof()];
		return true;
	}
	
	if (!_stricmp(MethodName, "Delete")) // Type: (Object)
	{
		for (unsigned i=0; i<Args.Length(); i++)
		{
			GVariant *v = Args[i];
			if (v && v->Type == GV_CUSTOM)
			{
				DeleteArray(v->Value.Custom.Data);
				v->Empty();
			}
		}
		return true;
	}

	LgiAssert(0);
	return false;
}
