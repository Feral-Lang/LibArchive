#pragma once

#include <archive.h>
#include <archive_entry.h>
#include <VM/Interpreter.hpp>

enum OpenMode
{
	OM_READ,
	OM_WRITE,
};

using namespace fer;

class VarArchive : public Var
{
	archive *val;
	OpenMode mode;
	bool owner;

public:
	VarArchive(ModuleLoc loc, archive *const val, int mode, bool owner = true);
	~VarArchive();

	Var *copy(ModuleLoc loc);
	void set(Var *from);

	inline archive *const get() { return val; }
	inline const OpenMode &getMode() const { return mode; }
};

class VarArchiveEntry : public Var
{
	archive_entry *val;
	bool owner;

public:
	VarArchiveEntry(ModuleLoc loc, archive_entry *const val, bool owner = true);
	~VarArchiveEntry();

	Var *copy(ModuleLoc loc);
	void set(Var *from);

	inline archive_entry *const get() { return val; }
};
