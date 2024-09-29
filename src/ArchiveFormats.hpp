#pragma once

#include "ArchiveType.hpp"

//////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////// Functions ////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

Var *feralArchiveApplyFormat(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
			     const StringMap<AssnArgData> &assn_args)
{
	VarArchive *ar = as<VarArchive>(args[0]);
	archive *a     = ar->get();
	if(!args[1]->is<VarInt>()) {
		vm.fail(args[1]->getLoc(),
			"expected format id to be of type 'int', found: ", vm.getTypeName(args[1]));
		return nullptr;
	}
	// list available here:
	// https://github.com/libarchive/libarchive/blob/c400064a1c63d122340d09d8ce3f671d4cf24b6e/libarchive/archive.h#L312
	int format = as<VarInt>(args[1])->get();

	switch(format) {
	case ARCHIVE_FORMAT_CPIO: {
		if(ar->getMode() == OM_READ) archive_read_support_format_cpio(a);
		else archive_write_set_format_cpio(a);
		break;
	}
	case ARCHIVE_FORMAT_TAR_USTAR:
	case ARCHIVE_FORMAT_TAR_PAX_INTERCHANGE:
	case ARCHIVE_FORMAT_TAR_PAX_RESTRICTED:
	case ARCHIVE_FORMAT_TAR_GNUTAR:
	case ARCHIVE_FORMAT_TAR: {
		if(ar->getMode() == OM_READ) archive_read_support_format_tar(a);
		else {
			if(format == ARCHIVE_FORMAT_TAR_USTAR) {
				archive_write_set_format_ustar(a);
			} else if(format == ARCHIVE_FORMAT_TAR_PAX_INTERCHANGE) {
				archive_write_set_format_pax(a);
			} else if(format == ARCHIVE_FORMAT_TAR_PAX_RESTRICTED) {
				archive_write_set_format_pax_restricted(a);
			} else if(format == ARCHIVE_FORMAT_TAR_GNUTAR) {
				archive_write_set_format_gnutar(a);
			} else {
				vm.fail(
				args[1]->getLoc(),
				"required specific writing format for tar: one of "
				"FORMAT_TAR_{USTAR, PAX_INTERCHANGE, PAX_RESTRICTED, GNUTAR}");
				return nullptr;
			}
		}
		break;
	}
	case ARCHIVE_FORMAT_ZIP: {
		if(ar->getMode() == OM_READ) archive_read_support_format_zip(a);
		else archive_write_set_format_zip(a);
		break;
	}
	case ARCHIVE_FORMAT_AR:
	case ARCHIVE_FORMAT_AR_BSD: {
		if(ar->getMode() == OM_READ) archive_read_support_format_ar(a);
		else archive_write_set_format_ar_bsd(a);
		break;
	}
	case ARCHIVE_FORMAT_MTREE: {
		if(ar->getMode() == OM_READ) archive_read_support_format_mtree(a);
		else archive_write_set_format_mtree(a);
		break;
	}
	case ARCHIVE_FORMAT_RAW: {
		if(ar->getMode() == OM_READ) archive_read_support_format_raw(a);
		else archive_write_set_format_raw(a);
		break;
	}
	case ARCHIVE_FORMAT_XAR: {
		if(ar->getMode() == OM_READ) archive_read_support_format_xar(a);
		else archive_write_set_format_xar(a);
		break;
	}
	case ARCHIVE_FORMAT_7ZIP: {
		if(ar->getMode() == OM_READ) archive_read_support_format_7zip(a);
		else archive_write_set_format_7zip(a);
		break;
	}
	case ARCHIVE_FORMAT_WARC: {
		if(ar->getMode() == OM_READ) archive_read_support_format_warc(a);
		else archive_write_set_format_warc(a);
		break;
	}
	default: {
		vm.fail(args[1]->getLoc(), "invalid format found: ", format);
		return nullptr;
	}
	}
	return args[0];
}