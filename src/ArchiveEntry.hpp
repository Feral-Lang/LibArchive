#pragma once

#include <fcntl.h>

#include "ArchiveType.hpp"

//////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////// Functions ////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

Var *feralArchiveEntryNew(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
			  const StringMap<AssnArgData> &assn_args)
{
	archive_entry *ae = archive_entry_new();
	if(ae == nullptr) {
		vm.fail(loc, "failed to init archive_entry object");
		return nullptr;
	}
	return vm.makeVar<VarArchiveEntry>(loc, ae);
}

Var *feralArchiveEntryClear(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
			    const StringMap<AssnArgData> &assn_args)
{
	archive_entry_clear(as<VarArchiveEntry>(args[0])->get());
	return args[0];
}

Var *feralArchiveEntrySetPathname(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
				  const StringMap<AssnArgData> &assn_args)
{
	if(!args[1]->is<VarStr>()) {
		vm.fail(loc, "expected a string for archive_entry pathname, found: ",
			vm.getTypeName(args[1]));
		return nullptr;
	}
	archive_entry_set_pathname(as<VarArchiveEntry>(args[0])->get(),
				   as<VarStr>(args[1])->get().c_str());
	return args[0];
}

Var *feralArchiveEntrySetSize(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
			      const StringMap<AssnArgData> &assn_args)
{
	if(!args[1]->is<VarInt>()) {
		vm.fail(loc, "expected an integer for archive_entry size, found: ",
			vm.getTypeName(args[1]));
		return nullptr;
	}
	archive_entry_set_size(as<VarArchiveEntry>(args[0])->get(), as<VarInt>(args[1])->get());
	return args[0];
}

Var *feralArchiveEntrySetFiletype(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
				  const StringMap<AssnArgData> &assn_args)
{
	if(!args[1]->is<VarInt>()) {
		vm.fail(loc, "expected an integer for archive_entry filetype, found: ",
			vm.getTypeName(args[1]));
		return nullptr;
	}
	archive_entry_set_filetype(as<VarArchiveEntry>(args[0])->get(), as<VarInt>(args[1])->get());
	return args[0];
}

Var *feralArchiveEntrySetPerm(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
			      const StringMap<AssnArgData> &assn_args)
{
	if(!args[1]->is<VarInt>()) {
		vm.fail(loc, "expected an integer for archive_entry permissions, found: ",
			vm.getTypeName(args[1]));
		return nullptr;
	}
	archive_entry_set_perm(as<VarArchiveEntry>(args[0])->get(), as<VarInt>(args[1])->get());
	return args[0];
}