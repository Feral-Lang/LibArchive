#include "ArchiveType.hpp"

//////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////// Archive Class //////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

VarArchive::VarArchive(const ModuleLoc *loc, archive *const val, int mode, bool owner)
	: Var(loc, false, false), val(val), mode((OpenMode)mode), owner(owner)
{}
VarArchive::~VarArchive()
{
	if(owner && val) {
		if(mode == OM_READ) archive_read_free(val);
		else if(mode == OM_WRITE) archive_write_free(val);
	}
}

Var *VarArchive::copy(const ModuleLoc *loc) { return new VarArchive(loc, val, mode, false); }

void VarArchive::set(Var *from)
{
	if(owner && val) archive_read_free(val);
	owner = false;
	mode  = as<VarArchive>(from)->mode;
	val   = as<VarArchive>(from)->val;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////// Archive Entry Class ////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

VarArchiveEntry::VarArchiveEntry(const ModuleLoc *loc, archive_entry *const val, bool owner)
	: Var(loc, false, false), val(val), owner(owner)
{}
VarArchiveEntry::~VarArchiveEntry() { archive_entry_free(val); }

Var *VarArchiveEntry::copy(const ModuleLoc *loc) { return new VarArchiveEntry(loc, val, false); }

void VarArchiveEntry::set(Var *from)
{
	if(owner && val) archive_entry_free(val);
	owner = false;
	val   = as<VarArchiveEntry>(from)->val;
}
