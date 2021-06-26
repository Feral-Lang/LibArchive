/*
	MIT License

	Copyright (c) 2020 Feral Language repositories

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so.
*/

#include "../include/archive_type.hpp"

//////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////// Functions ////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

var_base_t *feral_archive_apply_filter(vm_state_t &vm, const fn_data_t &fd)
{
	var_archive_t *ar = ARCHIVE(fd.args[0]);
	archive *a	  = ar->get();
	if(!fd.args[1]->istype<var_int_t>()) {
		vm.fail(fd.args[1]->src_id(), fd.args[1]->idx(),
			"expected filter id to be of type 'int', found: %s",
			vm.type_name(fd.args[1]).c_str());
		return nullptr;
	}

	// list available here:
	// https://github.com/libarchive/libarchive/blob/c400064a1c63d122340d09d8ce3f671d4cf24b6e/libarchive/archive.h#L266
	int filter = mpz_get_si(INT(fd.args[1])->get());
	switch(filter) {
	case ARCHIVE_FILTER_NONE: {
		if(ar->mode() == OM_READ) archive_read_support_filter_none(a);
		else
			archive_write_add_filter_none(a);
		break;
	}
	case ARCHIVE_FILTER_GZIP: {
		if(ar->mode() == OM_READ) archive_read_support_filter_gzip(a);
		else
			archive_write_add_filter_gzip(a);
		break;
	}
	case ARCHIVE_FILTER_BZIP2: {
		if(ar->mode() == OM_READ) archive_read_support_filter_bzip2(a);
		else
			archive_write_add_filter_bzip2(a);
		break;
	}
	case ARCHIVE_FILTER_COMPRESS: {
		if(ar->mode() == OM_READ) archive_read_support_filter_compress(a);
		else
			archive_write_add_filter_compress(a);
		break;
	}
	case ARCHIVE_FILTER_PROGRAM: {
		if(fd.args.size() < 3) {
			vm.fail(fd.src_id, fd.idx,
				"expected filter program to be passed as third argument");
			return nullptr;
		}
		if(!fd.args[2]->istype<var_str_t>()) {
			vm.fail(fd.args[2]->src_id(), fd.args[2]->idx(),
				"expected filter program to be of type 'str', found: %s",
				vm.type_name(fd.args[2]).c_str());
			return nullptr;
		}
		if(ar->mode() == OM_READ)
			archive_read_support_filter_program(a, STR(fd.args[2])->get().c_str());
		else
			archive_write_add_filter_program(a, STR(fd.args[2])->get().c_str());
		break;
	}
	case ARCHIVE_FILTER_LZMA: {
		if(ar->mode() == OM_READ) archive_read_support_filter_lzma(a);
		else
			archive_write_add_filter_lzma(a);
		break;
	}
	case ARCHIVE_FILTER_XZ: {
		if(ar->mode() == OM_READ) archive_read_support_filter_xz(a);
		else
			archive_write_add_filter_xz(a);
		break;
	}
	case ARCHIVE_FILTER_UU: {
		if(ar->mode() == OM_READ) archive_read_support_filter_uu(a);
		else
			archive_write_add_filter_uuencode(a);
		break;
	}
	case ARCHIVE_FILTER_RPM: {
		if(ar->mode() == OM_READ) archive_read_support_filter_rpm(a);
		else {
			vm.fail(fd.args[2]->src_id(), fd.args[2]->idx(),
				"filter type RPM not implemented for writer");
			return nullptr;
		}
		break;
	}
	case ARCHIVE_FILTER_LZIP: {
		if(ar->mode() == OM_READ) archive_read_support_filter_lzip(a);
		else
			archive_write_add_filter_lzip(a);
		break;
	}
	case ARCHIVE_FILTER_LRZIP: {
		if(ar->mode() == OM_READ) archive_read_support_filter_lrzip(a);
		else
			archive_write_add_filter_lrzip(a);
		break;
	}
	case ARCHIVE_FILTER_LZOP: {
		if(ar->mode() == OM_READ) archive_read_support_filter_lzop(a);
		else
			archive_write_add_filter_lzop(a);
		break;
	}
	case ARCHIVE_FILTER_GRZIP: {
		if(ar->mode() == OM_READ) archive_read_support_filter_grzip(a);
		else
			archive_write_add_filter_grzip(a);
		break;
	}
	case ARCHIVE_FILTER_LZ4: {
		if(ar->mode() == OM_READ) archive_read_support_filter_lz4(a);
		else
			archive_write_add_filter_lz4(a);
		break;
	}
	case ARCHIVE_FILTER_ZSTD: {
		if(ar->mode() == OM_READ) archive_read_support_filter_zstd(a);
		else
			archive_write_add_filter_zstd(a);
		break;
	}
	default: {
		vm.fail(fd.args[1]->src_id(), fd.args[1]->idx(), "invalid filter found: %d",
			filter);
		return nullptr;
	}
	}
	return fd.args[0];
}