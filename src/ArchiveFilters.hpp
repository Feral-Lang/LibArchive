#pragma once

#include "ArchiveType.hpp"

//////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////// Functions ////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

Var *feralArchiveApplyFilter(Interpreter &vm, const ModuleLoc *loc, Span<Var *> args,
			     const StringMap<AssnArgData> &assn_args)
{
	VarArchive *ar = as<VarArchive>(args[0]);
	archive *a     = ar->get();
	if(!args[1]->is<VarInt>()) {
		vm.fail(args[1]->getLoc(),
			"expected filter id to be of type 'int', found: ", vm.getTypeName(args[1]));
		return nullptr;
	}

	// list available here:
	// https://github.com/libarchive/libarchive/blob/c400064a1c63d122340d09d8ce3f671d4cf24b6e/libarchive/archive.h#L266
	int filter = as<VarInt>(args[1])->get();
	switch(filter) {
	case ARCHIVE_FILTER_NONE: {
		if(ar->getMode() == OM_READ) archive_read_support_filter_none(a);
		else archive_write_add_filter_none(a);
		break;
	}
	case ARCHIVE_FILTER_GZIP: {
		if(ar->getMode() == OM_READ) archive_read_support_filter_gzip(a);
		else archive_write_add_filter_gzip(a);
		break;
	}
	case ARCHIVE_FILTER_BZIP2: {
		if(ar->getMode() == OM_READ) archive_read_support_filter_bzip2(a);
		else archive_write_add_filter_bzip2(a);
		break;
	}
	case ARCHIVE_FILTER_COMPRESS: {
		if(ar->getMode() == OM_READ) archive_read_support_filter_compress(a);
		else archive_write_add_filter_compress(a);
		break;
	}
	case ARCHIVE_FILTER_PROGRAM: {
		if(args.size() < 3) {
			vm.fail(loc, "expected filter program to be passed as third argument");
			return nullptr;
		}
		if(!args[2]->is<VarStr>()) {
			vm.fail(args[2]->getLoc(),
				"expected filter program to be of type 'str', found: ",
				vm.getTypeName(args[2]));
			return nullptr;
		}
		if(ar->getMode() == OM_READ)
			archive_read_support_filter_program(a, as<VarStr>(args[2])->get().c_str());
		else archive_write_add_filter_program(a, as<VarStr>(args[2])->get().c_str());
		break;
	}
	case ARCHIVE_FILTER_LZMA: {
		if(ar->getMode() == OM_READ) archive_read_support_filter_lzma(a);
		else archive_write_add_filter_lzma(a);
		break;
	}
	case ARCHIVE_FILTER_XZ: {
		if(ar->getMode() == OM_READ) archive_read_support_filter_xz(a);
		else archive_write_add_filter_xz(a);
		break;
	}
	case ARCHIVE_FILTER_UU: {
		if(ar->getMode() == OM_READ) archive_read_support_filter_uu(a);
		else archive_write_add_filter_uuencode(a);
		break;
	}
	case ARCHIVE_FILTER_RPM: {
		if(ar->getMode() == OM_READ) archive_read_support_filter_rpm(a);
		else {
			vm.fail(args[2]->getLoc(), "filter type RPM not implemented for writer");
			return nullptr;
		}
		break;
	}
	case ARCHIVE_FILTER_LZIP: {
		if(ar->getMode() == OM_READ) archive_read_support_filter_lzip(a);
		else archive_write_add_filter_lzip(a);
		break;
	}
	case ARCHIVE_FILTER_LRZIP: {
		if(ar->getMode() == OM_READ) archive_read_support_filter_lrzip(a);
		else archive_write_add_filter_lrzip(a);
		break;
	}
	case ARCHIVE_FILTER_LZOP: {
		if(ar->getMode() == OM_READ) archive_read_support_filter_lzop(a);
		else archive_write_add_filter_lzop(a);
		break;
	}
	case ARCHIVE_FILTER_GRZIP: {
		if(ar->getMode() == OM_READ) archive_read_support_filter_grzip(a);
		else archive_write_add_filter_grzip(a);
		break;
	}
	case ARCHIVE_FILTER_LZ4: {
		if(ar->getMode() == OM_READ) archive_read_support_filter_lz4(a);
		else archive_write_add_filter_lz4(a);
		break;
	}
	case ARCHIVE_FILTER_ZSTD: {
		if(ar->getMode() == OM_READ) archive_read_support_filter_zstd(a);
		else archive_write_add_filter_zstd(a);
		break;
	}
	default: {
		vm.fail(loc, "invalid filter found: ", filter);
		return nullptr;
	}
	}
	return args[0];
}